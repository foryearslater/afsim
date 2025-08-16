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

#include "CommVisDialog.hpp"

#include <QMessageBox>
#include <QToolButton>

#include "CommVisAddCommDialog.hpp"
#include "CommVisAddGatewayDialog.hpp"
#include "CommVisAddNetworkDialog.hpp"
#include "CommVisAddRouterDialog.hpp"
#include "CommVisCommGraphicsItem.hpp"
#include "CommVisGatewayGraphicsItem.hpp"
#include "CommVisNetworkGraphicsItem.hpp"
#include "CommVisRouterGraphicsItem.hpp"
#include "ProxyWatcher.hpp"
#include "UtCast.hpp"
#include "UtQtMemory.hpp"
#include "WsfPM_Root.hpp"

WizCommVis::CommVisDialog::CommVisDialog(wkf::DataContainer& aCommData, QWidget* aParent, Qt::WindowFlags aFlags)
   : wkf::CommVisDialog::CommVisDialog(aCommData, aParent, aFlags)
{
   ConfigureSidePanel();
}

void WizCommVis::CommVisDialog::AddDirectLinksToScene()
{
   // Go through the list of comms and add any direct links
   for (auto& curComm : mCommList)
   {
      for (auto& curLink : curComm->GetLinks())
      {
         QString linkToCommName    = QString::fromStdString(curLink.mDestinationCommName);
         QString linkToCommAddress = QString::fromStdString(curLink.mDestinationAddress);
         for (auto& curLinkComm : mCommList)
         {
            if (curLinkComm->GetName() == linkToCommName)
            {
               if (!FindLink(curLinkComm->GetName(), curComm->GetName()))
               {
                  AddLink(curLinkComm.get(), curComm.get(), GetThemeLinkColor(), true);
               }
            }
            else if (!linkToCommAddress.isEmpty() && curLinkComm->GetAddress() == linkToCommAddress)
            {
               if (!FindLink(curLinkComm->GetName(), curComm->GetName()))
               {
                  AddLink(curLinkComm.get(), curComm.get(), GetThemeLinkColor(), true);
               }
            }
         }
      }
   }
}

wkf::CommVisLink* WizCommVis::CommVisDialog::FindLink(const QString& aSourceCommName, const QString& aDestinationCommName) const
{
   for (auto&& linksIter = mLinksList.begin(); linksIter != mLinksList.end(); ++linksIter)
   {
      auto curLink = linksIter;
      if ((curLink->get()->GetFromDeviceName() == aSourceCommName) &&
          (curLink->get()->GetToDeviceName() == aDestinationCommName))
      {
         return curLink->get();
      }
      else if ((curLink->get()->GetFromDeviceName() == aDestinationCommName) &&
               (curLink->get()->GetToDeviceName() == aSourceCommName))
      {
         return curLink->get();
      }
   }
   return nullptr;
}


void WizCommVis::CommVisDialog::AddNetworkToScene(const QString& aNetworkName,
                                                  const QString& aNetworkAddress,
                                                  const QString& aNetworkType)
{
   auto newNetwork =
      ut::qt::make_qt_ptr<WizCommVis::CommVisNetworkGraphicsItem>(aNetworkName, aNetworkAddress, aNetworkType);
   mNetworkList.emplace_back(newNetwork);
   if (Initialized())
   {
      Build();
   }
}

void WizCommVis::CommVisDialog::AddCommToScene(const wkf::CommEvent& aCommEvent)
{
   auto    commInfo     = aCommEvent.GetCommInfo();
   QString commName     = QString::fromStdString(commInfo.mCommName);
   QString commAddress  = QString::fromStdString(commInfo.mCommAddress);
   QString commType     = QString::fromStdString(commInfo.mCommType);
   QString platformName = QString::fromStdString(commInfo.mPlatformName);

   auto    networkInfo    = aCommEvent.GetNetworkInfo();
   QString networkName    = QString::fromStdString(networkInfo.mNetworkName);
   QString networkAddress = QString::fromStdString(networkInfo.mNetworkAddress);
   QString networkType    = QString::fromStdString(networkInfo.mNetworkType);

   auto    routerInfo    = aCommEvent.GetRouterInfo();
   QString routerName    = QString::fromStdString(routerInfo.mRouterName);
   QString routerAddress = QString::fromStdString(routerInfo.mRouterAddress);
   if (routerName.isEmpty())
   {
      routerName                          = "default";
      routerInfo.mRouterName              = routerName.toStdString();
      routerInfo.mPlatformName            = platformName.toStdString();
      routerInfo.mCommName                = commName.toStdString();
      routerInfo.mHopLimit                = 64;
      routerInfo.mUsingDefaultInterface   = true;
      routerInfo.mUsingMulticastInterface = true;
   }

   auto    gatewayInfo    = aCommEvent.GetGatewayInfo();
   QString gatewayName    = QString::fromStdString(gatewayInfo.mGatewayName);
   QString gatewayAddress = QString::fromStdString(gatewayInfo.mGatewayAddress);

   auto newComm = ut::qt::make_qt_ptr<WizCommVis::CommVisCommGraphicsItem>(platformName, commName, commAddress, commType);

   newComm->SetNetworkInfo(networkName, networkAddress);
   newComm->SetGatewayInfo(gatewayInfo);
   newComm->SetRouterInfo(routerName, routerAddress);
   newComm->SetLinks(aCommEvent.GetLinks());
   newComm->SetPlatform(platformName);
   newComm->SetName(platformName + "'s " + commName);
   mCommList.emplace_back(newComm);
   AddCommToNetwork(networkName, networkAddress, networkType, newComm);

   auto newRouter = ut::qt::make_qt_ptr<WizCommVis::CommVisRouterGraphicsItem>(routerInfo);
   mRouterList.emplace_back(newRouter);

   if (!gatewayName.isEmpty())
   {
      auto newGateway = ut::qt::make_qt_ptr<WizCommVis::CommVisGatewayGraphicsItem>(gatewayInfo);
      newGateway->SetRouterGraphicsItem(newRouter);
      mGatewayList.emplace_back(newGateway);
   }

   if (Initialized())
   {
      Build();
   }
}

void WizCommVis::CommVisDialog::ReInitialize()
{
   CloseChildDialogs();
   ClearData();
}


void WizCommVis::CommVisDialog::ConfigureSidePanel()
{
   // Clear out any widgets currently residing in the side panel
   QLayoutItem* item;
   while ((item = mUi.mSidePanelLayout->takeAt(0)))
   {
      delete item->widget();
      delete item;
   }

   mUi.mSidePanelLayout->setContentsMargins(15, -1, -1, -1);

   AddSidePanelButton("Network");
   AddSidePanelButton("Comm");
   AddSidePanelButton("Router");
   AddSidePanelButton("Gateway");
}


void WizCommVis::CommVisDialog::AddSidePanelButton(const QString& aButtonName)
{
   QString buttonIconName = ":/icons/" + aButtonName.toLower() + "_shadow.png";

   QPointer<QToolButton> panelButton = new QToolButton(this);
   panelButton->setObjectName(aButtonName);
   QIcon icon;
   icon.addFile(buttonIconName, QSize(), QIcon::Normal, QIcon::Off);
   panelButton->setIcon(icon);
   panelButton->setIconSize(QSize(48, 48));
   panelButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
   panelButton->setAutoRaise(true);
   panelButton->setArrowType(Qt::NoArrow);
   panelButton->setText("Add " + aButtonName);

   mUi.mSidePanelLayout->addWidget(panelButton);

   connect(panelButton.data(), &QToolButton::clicked, this, &CommVisDialog::AddNewDevice);
}

void WizCommVis::CommVisDialog::AddNewDevice()
{
   auto* button = qobject_cast<QToolButton*>(sender());
   if (button)
   {
      if (button->objectName() == "Network")
      {
         if (mAddNetworkDialogPtr == nullptr)
         {
            mAddNetworkDialogPtr = ut::qt::make_qt_ptr<CommVisAddNetworkDialog>(this);
         }
         if (mAddNetworkDialogPtr != nullptr)
         {
            mAddNetworkDialogPtr->show();
         }
      }
      else if (button->objectName() == "Comm")
      {
         if (mAddCommDialogPtr == nullptr)
         {
            mAddCommDialogPtr = ut::qt::make_qt_ptr<CommVisAddCommDialog>(this);
         }
         if (mAddCommDialogPtr != nullptr)
         {
            mAddCommDialogPtr->show();
         }
      }
      else if (button->objectName() == "Gateway")
      {
         if (mAddGatewayDialogPtr == nullptr)
         {
            mAddGatewayDialogPtr = ut::qt::make_qt_ptr<CommVisAddGatewayDialog>(this);
         }
         if (mAddGatewayDialogPtr != nullptr)
         {
            mAddGatewayDialogPtr->show();
         }
      }
      else if (button->objectName() == "Router")
      {
         if (mAddRouterDialogPtr == nullptr)
         {
            mAddRouterDialogPtr = ut::qt::make_qt_ptr<CommVisAddRouterDialog>(this);
         }
         if (mAddRouterDialogPtr != nullptr)
         {
            mAddRouterDialogPtr->show();
         }
      }
   }
}

void WizCommVis::CommVisDialog::AddCommToNetwork(const QString&                aNetworkName,
                                                 const QString&                aNetworkAddress,
                                                 const QString&                aNetworkType,
                                                 wkf::CommVisCommGraphicsItem* aCommPtr)
{
   QString networkName = aNetworkName;
   if (aNetworkName == "<local:slave>")
   {
      auto           platName = aCommPtr->GetPlatformName().toStdString();
      WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
      WsfPM_Platform curPlatform = proxyRoot.platforms() + platName;
      auto           cmdrName    = curPlatform.CommanderName("default");
      networkName                = "master:" + QString::fromStdString(cmdrName);
   }

   else if (aNetworkName == "<local:master>")
   {
      auto platName = aCommPtr->GetPlatformName();
      networkName   = "master:" + platName;
   }
   // Find the network graphics item
   bool networkFound = false;
   for (auto& network : mNetworkList)
   {
      if (network->GetName() == networkName)
      {
         mNetworkCommMap[network.get()].emplace_back(aCommPtr);
         networkFound = true;
         break;
      }
      else if (!aNetworkAddress.isEmpty() && network->GetAddress() == aNetworkAddress && network->GetName() == "default")
      {
         network->SetName(networkName);
         mNetworkCommMap[network.get()].emplace_back(aCommPtr);
         networkFound = true;
         break;
      }
   }
   // If the network wasn't found, add it
   if (!networkFound)
   {
      AddNetworkToScene(networkName, aNetworkAddress, aNetworkType);
      auto newNetworkPtr = mNetworkList[mNetworkList.size() - 1].get();
      mNetworkCommMap[newNetworkPtr].emplace_back(aCommPtr);
   }
}
