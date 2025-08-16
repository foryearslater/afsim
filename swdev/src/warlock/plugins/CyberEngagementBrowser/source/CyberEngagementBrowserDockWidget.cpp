// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CyberEngagementBrowserDockWidget.hpp"

#include <QMenu>
#include <QTime>

#include "CyberEngagementBrowserSimInterface.hpp"
#include "UtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfSelectorWidgets.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"

WkCyberEngagementBrowser::DockWidget::DockWidget(SimInterface*   aInterfacePtr,
                                                 DataContainer&  aCyberEngagementBrowserData,
                                                 QWidget*        parent,
                                                 Qt::WindowFlags f)
   : QDockWidget(parent, f)
   , mUI()
   , mCyberEngagementBrowserData(aCyberEngagementBrowserData)
   , mSimInterfacePtr(aInterfacePtr)
{
   mUI.setupUi(this);
   mUI.mEngagementTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
   mUI.mEngagementTable->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(&mCyberEngagementBrowserData, &DataContainer::EventAdded, this, &DockWidget::AddEventToTable);
   connect(&mCyberEngagementBrowserData, &DataContainer::EventRemoved, this, &DockWidget::RemoveEventFromTable);
   connect(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>(),
           &wkf::TeamVisibilityPrefObject::TeamVisibilityChanged,
           this,
           &DockWidget::UpdateTable);
   connect(mUI.mEngagementTable, &QDialog::customContextMenuRequested, this, &DockWidget::ShowContextMenu);

   SetColumnHeaders(QStringList() << "Attack Type"
                                  << "Attacker"
                                  << "Attacker Team"
                                  << "Victim"
                                  << "Result");
}

void WkCyberEngagementBrowser::DockWidget::SetColumnHeaders(const QStringList& aHeaders)
{
   mUI.mEngagementTable->setColumnCount(aHeaders.size());
   mUI.mEngagementTable->setHeaderLabels(aHeaders);
   for (int i = 0; i < aHeaders.size(); ++i)
   {
      mUI.mEngagementTable->resizeColumnToContents(i);
   }
}

void WkCyberEngagementBrowser::DockWidget::SetBackgroundColor(QTreeWidgetItem* aItem, QColor aColor)
{
   for (int i = 0; i < aItem->columnCount(); ++i)
   {
      aColor.setAlpha(85);
      aItem->setBackground(i, QBrush(aColor));
   }
}

void WkCyberEngagementBrowser::DockWidget::RemoveEventFromTable(const DataContainer::CyberEvent& aEvent)
{
   for (int i = 0; i < mUI.mEngagementTable->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* currentItem = mUI.mEngagementTable->topLevelItem(i);
      if (currentItem->text(0) == QString::fromStdString(aEvent.mAttackType) &&
          currentItem->text(1) == QString::fromStdString(aEvent.mAttacker) &&
          currentItem->text(3) == QString::fromStdString(aEvent.mVictim))
      {
         delete currentItem;
         break;
      }
   }
}

void WkCyberEngagementBrowser::DockWidget::UpdateTable()
{
   ClearTable();
   for (const auto& evt : mCyberEngagementBrowserData.GetEvents())
   {
      AddEventToTable(evt);
   }
}

void WkCyberEngagementBrowser::DockWidget::AddEventToTable(const DataContainer::CyberEvent& aEvent)
{
   auto*       prefObject = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>();
   std::string team       = aEvent.mAttackTeam;
   if (aEvent.mResult == "Attack Detected" || aEvent.mResult == "Scan Detected" ||
       aEvent.mResult == "Attack Attributed" || aEvent.mResult == "Scan Attributed")
   {
      team = aEvent.mVictimTeam;
   }
   if (prefObject->IsTeamVisible(QString::fromStdString(team)))
   {
      QColor teamColor       = prefObject->GetTeamColor(QString::fromStdString(team));
      auto*  engagementEvent = new QTreeWidgetItem();

      engagementEvent->setText(0, QString::fromStdString(aEvent.mAttackType));
      engagementEvent->setData(0, Qt::UserRole, false); // Will not purge after the given time period.
      if (aEvent.mResult == "Attack Attributed" || aEvent.mResult == "Scan Attributed" ||
          prefObject->IsTeamVisible(QString::fromStdString(aEvent.mAttackTeam)))
      {
         engagementEvent->setText(1, QString::fromStdString(aEvent.mAttacker));
         engagementEvent->setText(2, QString::fromStdString(aEvent.mAttackTeam));
      }
      engagementEvent->setText(3, QString::fromStdString(aEvent.mVictim));
      engagementEvent->setText(4, QString::fromStdString(aEvent.mResult));

      for (int i = 0; i < mUI.mEngagementTable->columnCount(); ++i)
      {
         mUI.mEngagementTable->resizeColumnToContents(i);
      }

      mUI.mEngagementTable->addTopLevelItem(engagementEvent);
      SetBackgroundColor(engagementEvent, teamColor);
      if (aEvent.mResult != "Attacking" && aEvent.mResult != "Scanning")
      {
         engagementEvent->setData(0, Qt::UserRole, true); // Can now be purged after the given time period
         engagementEvent->setData(1, Qt::UserRole, QDateTime::currentDateTime());
      }
   }
}

void WkCyberEngagementBrowser::DockWidget::Update(bool aPurge, int aPurgeTime)
{
   if (aPurge)
   {
      for (int i = 0; i < mUI.mEngagementTable->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* currentItem = mUI.mEngagementTable->topLevelItem(i);

         // Purge non "Attack Initialized" and "Scan Initialized" events
         if (currentItem->data(0, Qt::UserRole).toBool())
         {
            auto deleteTime = currentItem->data(1, Qt::UserRole).toDateTime().addSecs(aPurgeTime);
            if (deleteTime < QDateTime::currentDateTime())
            {
               delete currentItem;
            }
         }
      }
   }
}

void WkCyberEngagementBrowser::DockWidget::HideRows()
{
   QList<QTreeWidgetItem*> rows = mUI.mEngagementTable->selectedItems();
   for (auto row : rows)
   {
      row->setHidden(true);
   }
}

void WkCyberEngagementBrowser::DockWidget::ShowContextMenu(const QPoint& pos)
{
   if (mUI.mEngagementTable->hasFocus())
   {
      QMenu    contextMenu;
      QAction* deleteAction = new QAction(QIcon::fromTheme("delete"), QString("Hide Selected Items"));
      connect(deleteAction, &QAction::triggered, this, &DockWidget::HideRows);
      contextMenu.addAction(deleteAction);
      contextMenu.exec(QCursor::pos());
   }
}
