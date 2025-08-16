// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "NetworkBrowserDockWidget.hpp"

#include <QTreeWidgetItem>

WkNetworkBrowser::DockWidget::DockWidget(const DataContainer& aNetworkData, QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mNetworkData(aNetworkData)
{
   mUi.setupUi(this);

   connect(&mNetworkData, &DataContainer::InfoUpdated, this, &DockWidget::RebuildDisplay);
}

void WkNetworkBrowser::DockWidget::RebuildDisplay()
{
   Clear();

   mUi.siteLabel->setText("Site: " + QString::number(mNetworkData.GetSiteId()));
   mUi.appLabel->setText("Application: " + QString::number(mNetworkData.GetApplicationId()));
   mUi.exerciseLabel->setText("Exercise: " + QString::number(mNetworkData.GetExerciseId()));

   const auto& disConnections = mNetworkData.GetDisConnections();
   for (const auto& connection : disConnections)
   {
      QString connectionString;
      switch (connection.type)
      {
      case WsfDisDevice::cDEVICE_RECORD:
      {
         connectionString = "Record";
         break;
      }
      case WsfDisDevice::cDEVICE_PLAYBACK:
      {
         connectionString = "Playback";
         break;
      }
      case WsfDisDevice::cDEVICE_MULTICAST:
      {
         connectionString = "Multicast";
         break;
      }
      case WsfDisDevice::cDEVICE_BROADCAST:
      {
         connectionString = "Broadcast";
         break;
      }
      case WsfDisDevice::cDEVICE_UNICAST:
      {
         connectionString = "Unicast";
         break;
      }
      default:
         break;
      }

      QTreeWidgetItem* topItem = new QTreeWidgetItem();
      topItem->setText(0, "DIS (" + connectionString + ")");

      QTreeWidgetItem* item1 = new QTreeWidgetItem();
      item1->setText(0, "Address");
      item1->setText(1, QString::fromStdString(connection.ipAddress));
      topItem->addChild(item1);

      QTreeWidgetItem* item2 = new QTreeWidgetItem();
      item2->setText(0, "Interface");
      item2->setText(1, QString::fromStdString(connection.interfaceAddress));
      topItem->addChild(item2);

      QTreeWidgetItem* item3 = new QTreeWidgetItem();
      item3->setText(0, "Send Port");
      item3->setText(1, QString::number(connection.sendPort));
      topItem->addChild(item3);

      QTreeWidgetItem* item4 = new QTreeWidgetItem();
      item4->setText(0, "Receive Port");
      item4->setText(1, QString::number(connection.rcvPort));
      topItem->addChild(item4);

      mUi.treeWidget->addTopLevelItem(topItem);
   }

   const auto& xioConnections = mNetworkData.GetXioConnections();
   for (const auto& connectionPair : xioConnections)
   {
      const DataContainer::XioConnection& connection       = connectionPair.second;
      QString                             connectionString = "";
      switch (connection.type)
      {
      case WsfXIO_InputData::cMULTICAST:
      {
         connectionString = "Multicast";
         break;
      }
      case WsfXIO_InputData::cBROADCAST:
      {
         connectionString = "Broadcast";
         break;
      }
      case WsfXIO_InputData::cUNICAST:
      {
         connectionString = "Unicast";
         break;
      }
      }

      QTreeWidgetItem* topItem = new QTreeWidgetItem();
      topItem->setText(0, "XIO (" + connectionString + ")");

      QTreeWidgetItem* item1 = new QTreeWidgetItem();
      item1->setText(0, "Address");
      item1->setText(1, QString::fromStdString(connection.ipAddress));
      topItem->addChild(item1);

      QTreeWidgetItem* item2 = new QTreeWidgetItem();
      item2->setText(0, "Interface");
      item2->setText(1, QString::fromStdString(connection.interfaceAddress));
      topItem->addChild(item2);

      QTreeWidgetItem* item3 = new QTreeWidgetItem();
      item3->setText(0, "Send Port");
      item3->setText(1, QString::number(connection.sendPort));
      topItem->addChild(item3);

      QTreeWidgetItem* item4 = new QTreeWidgetItem();
      item4->setText(0, "Receive Port");
      item4->setText(1, QString::number(connection.rcvPort));
      topItem->addChild(item4);

      mUi.treeWidget->addTopLevelItem(topItem);
   }

   mUi.treeWidget->expandAll();
   mUi.treeWidget->resizeColumnToContents(0);
}

void WkNetworkBrowser::DockWidget::Clear()
{
   mUi.treeWidget->clear();
}
