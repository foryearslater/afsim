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

#include "CommVisSimInterface.hpp"

#include <UtMemory.hpp>

#include "WsfComm.hpp"
#include "WsfCommNetwork.hpp"
#include "WsfCommNetworkManager.hpp"
#include "WsfCommObserver.hpp"
#include "WsfCommProtocolInterface.hpp"
#include "WsfCommProtocolLegacy.hpp"
#include "WsfCommProtocolMulticast.hpp"
#include "WsfCommRouter.hpp"
#include "WsfPlatform.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"

WkCommVis::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<CommVisEvent>(aPluginName)
{
}

void WkCommVis::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
   }
}

void WkCommVis::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   AddSimEvent(ut::make_unique<SimInitializedEvent>());

   QMutexLocker locker(&mMutex);

   mCallbacks.Clear();

   auto networkManager = aSimulation.GetCommNetworkManager();

   mCallbacks.Add(networkManager->CommFrameworkPendingStart.Connect(
      [this, networkManager]()
      {
         AddNetworks(networkManager);
         AddComms(networkManager);

         wkf::CommEvent pendingStartEvent(0.0, wkf::CommEventType::PendingStartEvent);
         auto           commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(pendingStartEvent);
         AddSimEvent(std::move(commUpdateEvent));
      }));

   mCallbacks.Add(WsfObserver::MessageTransmitted(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr, const WsfMessage& aMessage)
                              { MessageTransmitted(aSimTime, aCommPtr, aMessage); }));

   mCallbacks.Add(WsfObserver::MessageHop(&aSimulation)
                     .Connect([this](double            aSimTime,
                                     wsf::comm::Comm*  aReceiverCommPtr,
                                     wsf::comm::Comm*  aDestinationCommPtr,
                                     const WsfMessage& aMessage)
                              { MessageHop(aSimTime, aReceiverCommPtr, aDestinationCommPtr, aMessage); }));

   mCallbacks.Add(WsfObserver::LinkAddedToLocal(&aSimulation)
                     .Connect(
                        [this](double                                aSimTime,
                               wsf::comm::router::Router*            aRouterPtr,
                               wsf::comm::router::ProtocolInterface* aProtocolPtr,
                               wsf::comm::Address*                   aSourceAddressPtr,
                               wsf::comm::Address*                   aDestinationAddressPtr)
                        {
                           wkf::CommEvent::LinkInfo link;
                           link.mSourceAddress      = aSourceAddressPtr->GetAddress();
                           link.mDestinationAddress = aDestinationAddressPtr->GetAddress();
                           std::vector<wkf::CommEvent::LinkInfo> linkInfo;
                           linkInfo.emplace_back(link);
                           wkf::CommEvent linkAddedEvent(aSimTime, wkf::CommEventType::LinkAddedToLocal);
                           linkAddedEvent.SetLinks(linkInfo);
                           auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(linkAddedEvent);
                           AddSimEvent(std::move(commUpdateEvent));
                        }));

   mCallbacks.Add(WsfObserver::LinkRemovedFromLocal(&aSimulation)
                     .Connect(
                        [this](double                                aSimTime,
                               wsf::comm::router::Router*            aRouterPtr,
                               wsf::comm::router::ProtocolInterface* aProtocolPtr,
                               wsf::comm::Address*                   aSourceAddressPtr,
                               wsf::comm::Address*                   aDestinationAddressPtr)
                        {
                           wkf::CommEvent::LinkInfo link;
                           link.mSourceAddress      = aSourceAddressPtr->GetAddress();
                           link.mDestinationAddress = aDestinationAddressPtr->GetAddress();
                           std::vector<wkf::CommEvent::LinkInfo> linkInfo;
                           linkInfo.emplace_back(link);
                           wkf::CommEvent linkAddedEvent(aSimTime, wkf::CommEventType::LinkRemovedFromLocal);
                           linkAddedEvent.SetLinks(linkInfo);
                           auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(linkAddedEvent);
                           AddSimEvent(std::move(commUpdateEvent));
                        }));

   mCallbacks.Add(WsfObserver::CommTurnedOff(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { TurnOffComm(aSimTime, aCommPtr); }));

   mCallbacks.Add(WsfObserver::CommTurnedOn(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { TurnOnComm(aSimTime, aCommPtr); }));

   mCallbacks.Add(WsfObserver::CommNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { TurnOffComm(aSimTime, aCommPtr); }));

   mCallbacks.Add(WsfObserver::CommOperational(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { TurnOnComm(aSimTime, aCommPtr); }));

   mCallbacks.Add(WsfObserver::CommBroken(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { TurnOffComm(aSimTime, aCommPtr); }));

   mCallbacks.Add(WsfObserver::CommRemovedFromManager(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aCommPtr) { RemoveComm(aSimTime, aCommPtr); }));

   locker.unlock();
}

void WkCommVis::SimInterface::AddNetworks(const wsf::comm::NetworkManager* aNetworkManagerPtr)
{
   for (auto& curNetworkStr : aNetworkManagerPtr->GetManagedNetworks())
   {
      auto curNetwork = aNetworkManagerPtr->GetNetwork(curNetworkStr);

      wkf::CommEvent              addNetworkEvent(0.0, wkf::CommEventType::AddNetwork);
      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = curNetwork->GetName();
      networkInfo.mNetworkAddress = curNetwork->GetAddress().GetAddress();
      networkInfo.mNetworkType    = curNetwork->GetScriptClassName();

      if (networkInfo.mNetworkName.empty())
      {
         networkInfo.mNetworkName = "default";
      }
      addNetworkEvent.SetNetworkInfo(networkInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(addNetworkEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}

void WkCommVis::SimInterface::AddComms(const wsf::comm::NetworkManager* aNetworkManagerPtr)
{
   auto comms = aNetworkManagerPtr->GetComms();
   for (auto& curCommAddr : comms)
   {
      auto curComm = aNetworkManagerPtr->GetComm(curCommAddr);

      if (curComm)
      {
         const std::string& networkName    = curComm->GetNetwork();
         auto               commNetwork    = aNetworkManagerPtr->GetNetwork(networkName);
         std::string        networkType    = commNetwork->GetScriptClassName();
         std::string        networkAddress = commNetwork->GetAddress().GetAddress();
         const std::string& commName       = curComm->GetName();
         const std::string& commType       = curComm->GetType();
         std::string        commAddress    = curComm->GetAddress().GetAddress();
         std::string        platformName   = curComm->GetPlatform()->GetName();

         // Get use multicast protocol
         bool routerUseMulticastProtocol = false;
         auto multicastPtr = curComm->GetRouter()->GetComponents().GetComponent<wsf::comm::router::ProtocolMulticast>();
         if (multicastPtr)
         {
            routerUseMulticastProtocol = true;
         }

         // Get use default protocol
         bool routerUseDefaultProtocol = false;
         auto legacyPtr = curComm->GetRouter()->GetComponents().GetComponent<wsf::comm::router::ProtocolLegacy>();
         if (legacyPtr)
         {
            routerUseDefaultProtocol = true;
         }

         // Need to get automated interface linking
         wkf::CommEvent::RouterInfo routerInfo;
         routerInfo.mRouterName     = curComm->GetRouter()->GetName();
         routerInfo.mRouterType     = curComm->GetRouter()->GetType();
         routerInfo.mGatewayAddress = curComm->GetRouter()->GetGateway().GetAddress();
         if (!curComm->GetRouter()->GetGateway().IsNull())
         {
            routerInfo.mGatewayComm = aNetworkManagerPtr->GetComm(curComm->GetRouter()->GetGateway())->GetName();
         }
         routerInfo.mHopLimit                = curComm->GetRouter()->GetHopLimit();
         routerInfo.mUsingDefaultInterface   = routerUseDefaultProtocol;
         routerInfo.mUsingMulticastInterface = routerUseMulticastProtocol;
         routerInfo.mPlatformName            = platformName;
         routerInfo.mCommName                = commName;

         wkf::CommEvent addCommEvent(0.0, wkf::CommEventType::AddComm);

         wkf::CommEvent::NetworkInfo networkInfo;
         networkInfo.mNetworkName    = networkName;
         networkInfo.mNetworkAddress = networkAddress;
         networkInfo.mNetworkType    = networkType;
         addCommEvent.SetNetworkInfo(networkInfo);

         wkf::CommEvent::CommInfo commInfo;
         commInfo.mCommName     = commName;
         commInfo.mCommAddress  = commAddress;
         commInfo.mPlatformName = platformName;
         commInfo.mCommType     = commType;
         addCommEvent.SetCommInfo(commInfo);

         addCommEvent.SetRouterInfo(routerInfo);
         auto gateway = aNetworkManagerPtr->GetComm(curComm->GetGateway());
         if (gateway)
         {
            wkf::CommEvent::GatewayInfo gatewayInfo;
            gatewayInfo.mPlatformName          = platformName;
            gatewayInfo.mGatewayName           = gateway->GetName();
            gatewayInfo.mGatewayAddress        = curComm->GetRouter()->GetGateway().GetAddress();
            gatewayInfo.mGatewayRemotePlatform = curComm->GetGatewayRemotePlatform();
            gatewayInfo.mGatewayRemoteComm     = curComm->GetGatewayRemoteComm();
            gatewayInfo.mGatewayRemoteAddress  = curComm->GetGateway().GetAddress();

            addCommEvent.SetGatewayInfo(gatewayInfo);
         }

         const std::vector<std::pair<std::string, std::string>>& links = curComm->GetLinkPairs();
         // Loop through the pairs and create a list of links to this comm
         std::vector<wkf::CommEvent::LinkInfo> linkInfo;
         for (const auto& curLink : links)
         {
            wkf::CommEvent::LinkInfo curLinkInfo;
            curLinkInfo.mDestinationPlatformName = curLink.first;
            curLinkInfo.mDestinationCommName     = curLink.second;
            linkInfo.emplace_back(curLinkInfo);
         }
         for (const auto& curAddr : curComm->GetLinkAddresses())
         {
            // resolve the link addresses down to their platform and comm
            auto                     curComm = aNetworkManagerPtr->GetComm(curAddr);
            wkf::CommEvent::LinkInfo curLinkInfo;
            curLinkInfo.mDestinationPlatformName = curComm->GetPlatform()->GetName();
            curLinkInfo.mDestinationCommName     = curComm->GetName();
            linkInfo.emplace_back(curLinkInfo);
         }

         addCommEvent.SetLinks(linkInfo);

         auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(addCommEvent);
         AddSimEvent(std::move(commUpdateEvent));
      }
   }
}

void WkCommVis::SimInterface::RemoveComm(double aSimTime, wsf::comm::Comm* aCommPtr)
{
   if (aCommPtr)
   {
      wkf::CommEvent::CommInfo commInfo;
      commInfo.mCommName    = aCommPtr->GetName();
      commInfo.mCommAddress = aCommPtr->GetAddress().GetAddress();

      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = aCommPtr->GetNetwork();
      networkInfo.mNetworkAddress = "";

      wkf::CommEvent removeCommEvent(aSimTime, wkf::CommEventType::RemoveComm);
      removeCommEvent.SetNetworkInfo(networkInfo);
      removeCommEvent.SetCommInfo(commInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(removeCommEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}

void WkCommVis::SimInterface::TurnOffComm(double aSimTime, wsf::comm::Comm* aCommPtr)
{
   if (aCommPtr)
   {
      wkf::CommEvent::CommInfo commInfo;
      commInfo.mCommName    = aCommPtr->GetName();
      commInfo.mCommAddress = aCommPtr->GetAddress().GetAddress();

      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = aCommPtr->GetNetwork();
      networkInfo.mNetworkAddress = "";

      wkf::CommEvent turnOffCommEvent(aSimTime, wkf::CommEventType::CommTurnedOff);
      turnOffCommEvent.SetNetworkInfo(networkInfo);
      turnOffCommEvent.SetCommInfo(commInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(turnOffCommEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}

void WkCommVis::SimInterface::TurnOnComm(double aSimTime, wsf::comm::Comm* aCommPtr)
{
   if (aCommPtr)
   {
      wkf::CommEvent::CommInfo commInfo;
      commInfo.mCommName    = aCommPtr->GetName();
      commInfo.mCommAddress = aCommPtr->GetAddress().GetAddress();

      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = aCommPtr->GetNetwork();
      networkInfo.mNetworkAddress = "";

      wkf::CommEvent turnOnCommEvent(aSimTime, wkf::CommEventType::CommTurnedOn);
      turnOnCommEvent.SetNetworkInfo(networkInfo);
      turnOnCommEvent.SetCommInfo(commInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(turnOnCommEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}

void WkCommVis::SimInterface::MessageTransmitted(double aSimTime, wsf::comm::Comm* aCommPtr, const WsfMessage& aMessage)
{
   if (aCommPtr)
   {
      wkf::CommEvent::CommInfo commInfo;
      commInfo.mCommName    = aCommPtr->GetName();
      commInfo.mCommAddress = aCommPtr->GetAddress().GetAddress();

      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = aCommPtr->GetNetwork();
      networkInfo.mNetworkAddress = "";

      wkf::CommEvent transmitCommEvent(aSimTime, wkf::CommEventType::MessageTransmitted);
      transmitCommEvent.SetNetworkInfo(networkInfo);
      transmitCommEvent.SetCommInfo(commInfo);

      wkf::CommEvent::MessageInfo messageInfo;
      messageInfo.mMessageStr         = aMessage.GetType().GetString();
      messageInfo.mSourceAddress      = aMessage.GetSrcAddr().GetAddress();
      messageInfo.mDestinationAddress = aMessage.GetDstAddr().GetAddress();
      messageInfo.mHopAddress         = aMessage.GetNextHopAddr().GetAddress();
      messageInfo.mSerialNumber       = aMessage.GetSerialNumber();

      transmitCommEvent.SetMessage(messageInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(transmitCommEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}

void WkCommVis::SimInterface::MessageHop(double            aSimTime,
                                         wsf::comm::Comm*  aReceiverCommPtr,
                                         wsf::comm::Comm*  aDestinationCommPtr,
                                         const WsfMessage& aMessage)
{
   if (aReceiverCommPtr && aDestinationCommPtr)
   {
      wkf::CommEvent::CommInfo commInfo;
      commInfo.mCommName    = aReceiverCommPtr->GetName();
      commInfo.mCommAddress = aMessage.GetSrcAddr().GetAddress();

      wkf::CommEvent::NetworkInfo networkInfo;
      networkInfo.mNetworkName    = aReceiverCommPtr->GetNetwork();
      networkInfo.mNetworkAddress = "";

      wkf::CommEvent transmitCommEvent(aSimTime, wkf::CommEventType::MessageHop);
      transmitCommEvent.SetNetworkInfo(networkInfo);
      transmitCommEvent.SetCommInfo(commInfo);

      wkf::CommEvent::MessageInfo messageInfo;
      messageInfo.mMessageStr         = aMessage.GetType().GetString();
      messageInfo.mSourceAddress      = aMessage.GetSrcAddr().GetAddress();
      messageInfo.mDestinationAddress = aMessage.GetDstAddr().GetAddress();
      messageInfo.mHopAddress         = aMessage.GetNextHopAddr().GetAddress();
      messageInfo.mSerialNumber       = aMessage.GetSerialNumber();

      transmitCommEvent.SetMessage(messageInfo);
      auto commUpdateEvent = ut::make_unique<CommVisUpdateEvent>(transmitCommEvent);
      AddSimEvent(std::move(commUpdateEvent));
   }
}
