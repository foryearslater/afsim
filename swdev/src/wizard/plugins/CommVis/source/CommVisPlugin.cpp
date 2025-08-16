// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CommVisPlugin.hpp"

#include <QMenu>

#include "ProxyWatcher.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"
#include "comm_vis/WkfCommVisDialog.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WizCommVis::Plugin, "CommVis", "Provides a communication visualization plugin for Wizard.", "wizard")

WizCommVis::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   // Provide access from tools menu
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenuPtr  = mainWindowPtr->GetMenuByName("Tools");

   if (toolsMenuPtr)
   {
      auto dlgActionPtr = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("star"), "CommVis...", mainWindowPtr);
      connect(dlgActionPtr, &QAction::triggered, [this]() { Start(); });
      toolsMenuPtr->addAction(dlgActionPtr);
   }

   mProxyCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Plugin::OnProxyAvailable, this);
}

QList<wkf::Tip> WizCommVis::Plugin::GetTips() const
{
   QList<wkf::Tip> tips;
   tips.append(wkf::Tip("Comms Visualization",
                        "You can view communication network activity.",
                        "wkf_plugin/wiz_comm_vis.html",
                        "Comms Visualization"));
   return tips;
}

void WizCommVis::Plugin::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (mProxyRoot.GetProxy() == aProxyPtr)
   {
      return;
   }
   mProxyRoot = aProxyPtr;
   mProxyCallbacks.Add(
      wizard::WatchProxy(mProxyRoot.platforms().GetPath()).AnyUpdate.Connect(&Plugin::HandleProxyChanged, this));
   mProxyCallbacks.Add(
      wizard::WatchProxy(mProxyRoot.networkTypes().GetPath()).AnyUpdate.Connect(&Plugin::HandleProxyChanged, this));
   if (!mProxyRoot.IsNull())
   {
      ReInitialize();
   }
}

void WizCommVis::Plugin::HandleProxyChanged(const wizard::ProxyChange& aProxyChange)
{
   ReInitialize();
}

void WizCommVis::Plugin::ReInitialize()
{
   if (mDialog)
   {
      InitializeNetworks();
      InitializeComms();

      // The pendingStartEvent must be sent (and be sent last) to trigger off the initial build of the scene
      wkf::CommEvent pendingStartEvent(0.0, wkf::CommEventType::PendingStartEvent);
      mCommData.AddCommData(pendingStartEvent);
      mDialog->ReInitialize();
      mDialog->Update();
   }
}

void WizCommVis::Plugin::Start()
{
   if (mDialog != nullptr)
   {
      delete mDialog;
   }

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDialog                        = ut::qt::make_qt_ptr<WizCommVis::CommVisDialog>(mCommData, mainWindowPtr);

   if (mProxyRoot.IsValid())
   {
      InitializeNetworks();
      InitializeComms();

      // The pendingStartEvent must be sent (and be sent last) to trigger off the initial build of the scene
      wkf::CommEvent pendingStartEvent(0.0, wkf::CommEventType::PendingStartEvent);
      mCommData.AddCommData(pendingStartEvent);

      mDialog->Update();
   }
   mDialog->show();
}

void WizCommVis::Plugin::InitializeNetworks()
{
   // Add the default network
   wkf::CommEvent              addNetworkEvent(0.0, wkf::CommEventType::AddNetwork);
   wkf::CommEvent::NetworkInfo networkInfo;
   networkInfo.mNetworkName    = "default";
   networkInfo.mNetworkAddress = "";
   networkInfo.mNetworkType    = "WSF_COMM_NETWORK_P2P";
   addNetworkEvent.SetNetworkInfo(networkInfo);
   mCommData.AddCommData(addNetworkEvent);

   // Loop through and add all the other networks
   WsfPM_ObjectMap networkTypes = mProxyRoot.networkTypes();
   for (WsfPM_ObjectMap::iterator i = networkTypes.begin(); i != networkTypes.end(); ++i)
   {
      WsfPProxyNode curNet          = networkTypes.At(i);
      std::string   addressRangeVal = curNet.GetValue()["networkAddress"].ValueToString();
      if (addressRangeVal.empty())
      {
         addressRangeVal = curNet.GetValue()["address"].ValueToString();
      }

      wkf::CommEvent              addNetworkEvent(0.0, wkf::CommEventType::AddNetwork);
      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = curNet.GetName();
      networkInfo.mNetworkAddress = addressRangeVal;
      networkInfo.mNetworkType    = curNet.GetTypeName();
      addNetworkEvent.SetNetworkInfo(networkInfo);

      mCommData.AddCommData(addNetworkEvent);
   }
}

void WizCommVis::Plugin::InitializeComms()
{
   WsfPM_PlatformMap platforms = mProxyRoot.platforms();
   for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
   {
      WsfPM_Platform curPlatform  = platforms.AtT(i);
      std::string    platformName = curPlatform.GetName();
      WsfPM_CommMap  comms        = curPlatform.Comms();

      // Iterate through the comms and gather information about them
      for (WsfPM_CommMap::iterator j = comms.begin(); j != comms.end(); ++j)
      {
         wkf::CommEvent addCommEvent(0.0, wkf::CommEventType::AddComm);

         WsfPM_Comm curComm  = comms.AtT(j);
         auto       commName = curComm.Name();
         auto       type     = curComm.Type();

         auto networkAddress = curComm.NetworkAddress();
         if (networkAddress.empty())
         {
            // NetworkAddress wasn't set, try Address
            networkAddress = curComm.Address();
         }

         auto networkName = curComm.GetNetworkNameString();
         if (networkName.empty())
         {
            networkName = ResolveCommNetworkName(networkAddress, platformName, commName);
         }

         auto address         = curComm.Address();
         auto linkPlatform    = curComm.LinkPlatform();
         auto linkComm        = curComm.LinkComm();
         auto localLink       = curComm.LocalLink();
         auto linkAddress     = curComm.LinkAddress();
         auto routerName      = curComm.RouterName();
         auto gatewayPlatform = curComm.GatewayPlatform();
         auto gatewayComm     = curComm.GatewayComm();

         // Need to get automated interface linking
         if (routerName.empty())
         {
            routerName = "default";
         }
         WsfPM_RouterMap routers = curPlatform.Routers();
         for (WsfPM_RouterMap::iterator j = routers.begin(); j != routers.end(); ++j)
         {
            WsfPM_Router curRouter = routers.AtT(j);
            routerName             = curRouter.Name();
            if (!routerName.empty())
            {
               wkf::CommEvent::RouterInfo routerInfo;
               routerInfo.mRouterName              = routerName;
               routerInfo.mRouterType              = curRouter.GetTypeName();
               routerInfo.mGatewayAddress          = curRouter.GatewayAddress();
               routerInfo.mGatewayComm             = curRouter.Gateway();
               routerInfo.mHopLimit                = curRouter.HopLimit();
               routerInfo.mUsingDefaultInterface   = curRouter.UseDefaultProtocol();
               routerInfo.mUsingMulticastInterface = curRouter.UseMulticastProtocol();
               routerInfo.mPlatformName            = platformName;
               routerInfo.mCommName                = commName;
               addCommEvent.SetRouterInfo(routerInfo);

               if (!gatewayComm.empty() && (routerInfo.mGatewayAddress.empty() || routerInfo.mGatewayComm.empty()))
               {
                  wkf::CommEvent::GatewayInfo gatewayInfo;
                  gatewayInfo.mPlatformName          = platformName;
                  gatewayInfo.mGatewayName           = routerInfo.mGatewayComm;
                  gatewayInfo.mGatewayAddress        = routerInfo.mGatewayAddress;
                  gatewayInfo.mGatewayRemotePlatform = gatewayPlatform;
                  gatewayInfo.mGatewayRemoteComm     = gatewayComm;

                  addCommEvent.SetGatewayInfo(gatewayInfo);
               }
            }
         }

         std::vector<wkf::CommEvent::LinkInfo> links;
         size_t                                linkCount = curComm.LinksCount();
         for (size_t i = 0; i < linkCount; ++i)
         {
            auto                     currentProxyLink = curComm.GetLink(i);
            auto                     linkDestPlat     = currentProxyLink.LinkDestinationPlatformName();
            auto                     linkDestComm     = currentProxyLink.LinkDestinationCommName();
            wkf::CommEvent::LinkInfo commLink;
            commLink.mDestinationPlatformName = currentProxyLink.LinkDestinationPlatformName();
            commLink.mDestinationCommName     = currentProxyLink.LinkDestinationCommName();
            if (commLink.mDestinationPlatformName == "link_address")
            {
               commLink.mDestinationAddress = commLink.mDestinationCommName;
            }
            else
            {
               commLink.mDestinationAddress = linkAddress;
            }
            commLink.mSourcePlatformName = platformName;
            commLink.mSourceAddress      = address;
            commLink.mSourceCommName     = commName;
            links.emplace_back(commLink);
         }
         addCommEvent.SetLinks(links);

         wkf::CommEvent::NetworkInfo networkInfo;
         networkInfo.mNetworkName    = networkName;
         networkInfo.mNetworkAddress = networkAddress;
         networkInfo.mNetworkType    = GetCommNetworkType(networkName);
         addCommEvent.SetNetworkInfo(networkInfo);

         wkf::CommEvent::CommInfo commInfo;
         commInfo.mCommName     = commName;
         commInfo.mCommAddress  = address;
         commInfo.mPlatformName = platformName;
         commInfo.mCommType     = type;
         addCommEvent.SetCommInfo(commInfo);

         mCommData.AddCommData(addCommEvent);
      }
   }
}

const std::string WizCommVis::Plugin::ResolveCommNetworkName(const std::string& aNetworkAddress,
                                                             const std::string& aPlatformName,
                                                             const std::string& aCommName) const
{
   std::string networkName = "default";

   if (!aNetworkAddress.empty())
   {
      // When no network name was given, try to resolve by the address if given one
      WsfPM_ObjectMap networkTypes = mProxyRoot.networkTypes();
      networkName                  = aPlatformName + "." + aCommName;
      unsigned int count           = 1;
      for (WsfPM_ObjectMap::iterator i = networkTypes.begin(); i != networkTypes.end(); ++i)
      {
         WsfPProxyNode curNet          = networkTypes.At(i);
         std::string   addressRangeVal = curNet.GetValue()["networkAddress"].ValueToString();
         if (addressRangeVal.empty())
         {
            addressRangeVal = curNet.GetValue()["address"].ValueToString();
         }
         if (addressRangeVal == aNetworkAddress)
         {
            return curNet.GetName();
         }
         else if (curNet.GetValue()["network_name"].ValueToString() == networkName + "." + std::to_string(count))
         {
            ++count;
         }
      }
      // no network was found but an address was given, create a new network based on this comm and the owning platform
      networkName = networkName + "." + std::to_string(count);
   }

   return networkName;
}

const std::string WizCommVis::Plugin::GetCommNetworkType(const std::string& aNetworkName) const
{
   auto netNode = mProxyRoot.networkTypes() + aNetworkName;
   auto pathStr = netNode.GetPathString();
   auto netType = netNode.GetTypeName();

   return netType;
}
