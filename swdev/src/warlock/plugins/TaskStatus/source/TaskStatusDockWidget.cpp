// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TaskStatusDockWidget.hpp"

#include <QMenu>
#include <QString>
#include <QTreeWidget>

#include "TaskStatusDataContainer.hpp"
#include "TaskStatusSimCommands.hpp"
#include "TaskStatusSimInterface.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfTask.hpp"
#include "WsfWeaponTaskResource.hpp"

static const std::map<int, QString> resourceTypeToStringMap = {{cTASK_RESOURCE_TYPE_SENSOR, "Track"},
                                                               {cTASK_RESOURCE_TYPE_WEAPON, "Fire"},
                                                               {cTASK_RESOURCE_TYPE_JAMMER, "Jam"},
                                                               {cTASK_RESOURCE_TYPE_PROCESSOR, "Custom"},
                                                               {cTASK_RESOURCE_TYPE_NONE, "Custom"}};

// ============================================================================
WkTaskStatus::DockWidget::DockWidget(SimInterface*   aSimInterfacePtr,
                                     DataContainer&  aData,
                                     QWidget*        aParentPtr,
                                     Qt::WindowFlags aFlags)
   : QDockWidget(aParentPtr, aFlags)
   , mData(aData)
   , mSimInterfacePtr(aSimInterfacePtr)
{
   mUi.setupUi(this);
   setContextMenuPolicy(Qt::CustomContextMenu);
   mUi.mTreeWidget->setColumnCount(7);
   Connect();
}

// ============================================================================
void WkTaskStatus::DockWidget::TreeWidgetContextMenu(const QPoint& aPoint)
{
   QPoint           globalPos = mapToGlobal(aPoint);
   QTreeWidgetItem* itemPtr   = mUi.mTreeWidget->itemAt(mUi.mTreeWidget->viewport()->mapFromGlobal(globalPos));
   if (itemPtr != nullptr)
   {
      if (itemPtr->type() == cTASK)
      {
         // Get the parents
         QTreeWidgetItem* trackParentPtr     = itemPtr->parent();
         QTreeWidgetItem* processorParentPtr = trackParentPtr->parent();
         if (processorParentPtr->type() == cASSIGNED)
         {
            QMenu*   menu   = new QMenu();
            QAction* action = new QAction(QIcon::fromTheme("cross"), "Cancel Task", menu);
            connect(action, &QAction::triggered, this, &DockWidget::CancelTaskHandler);
            action->setProperty(cPROCESSOR_STR, itemPtr->data(0, cPROCESSOR_ROLE));
            action->setProperty(cTASKID_STR, itemPtr->data(0, cTASKID_ROLE));
            menu->addAction(action);
            menu->exec(globalPos);
            delete menu;
         }
      }
      else if (itemPtr->type() == cTRACK)
      {
         // Get the parent
         QTreeWidgetItem* processorParentPtr = itemPtr->parent();
         if (processorParentPtr->type() == cASSIGNED)
         {
            QMenu*   menu            = new QMenu();
            QAction* cancelAllAction = new QAction(QIcon::fromTheme("cross"), "Cancel All Tasks", menu);
            connect(cancelAllAction, &QAction::triggered, this, &DockWidget::CancelAllTasksHandler);
            cancelAllAction->setProperty(cTRACK_STR, itemPtr->data(0, cTRACKINDEX_ROLE));
            menu->addAction(cancelAllAction);

            QAction* abortFiringAtAction = new QAction(QIcon::fromTheme("gun"), "Abort Firing At", menu);
            connect(abortFiringAtAction, &QAction::triggered, this, &DockWidget::AbortFiringHandler);
            abortFiringAtAction->setProperty(cTRACK_STR, itemPtr->data(0, cTRACKINDEX_ROLE));
            menu->addAction(abortFiringAtAction);
            menu->exec(globalPos);
            delete menu;
         }
      }
      else if (itemPtr->type() == cASSIGNED)
      {
         QMenu*   menu   = new QMenu();
         QAction* action = new QAction(QIcon::fromTheme("delete"), "Cancel All Tasks", menu);
         connect(action, &QAction::triggered, this, &DockWidget::CancelAllTasksHandler);
         menu->addAction(action);
         menu->exec(globalPos);
         delete menu;
      }
   }
}

// ============================================================================
void WkTaskStatus::DockWidget::Connect()
{
   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
   connect(&mData, &DataContainer::TaskChanged, this, &DockWidget::TaskChanged);
   connect(this, &QTreeWidget::customContextMenuRequested, this, &DockWidget::TreeWidgetContextMenu);
}

// ============================================================================
void WkTaskStatus::DockWidget::AddTasks(const std::string&                                    aTaskManagerName,
                                        const std::map<WsfTrackId, WsfTaskManager::TaskList>& aTaskList)
{
   int topLevelItemCount = mUi.mTreeWidget->topLevelItemCount();
   for (const auto& track : aTaskList)
   {
      // Track
      auto* trackItem =
         new QTreeWidgetItem(QStringList() << "Track # " << QString::number(track.first.GetLocalTrackNumber()), cTRACK);
      trackItem->setData(0, cTRACKINDEX_ROLE, track.first.GetLocalTrackNumber());

      // Task Info
      for (const auto& task : track.second)
      {
         if (task.IsResourceTypeOf(cTASK_RESOURCE_TYPE_JAMMER))
         {
            WsfJammerTaskResource& jammerResource = (WsfJammerTaskResource&)task.GetResource();
            auto*                  taskItem =
               new QTreeWidgetItem(QStringList()
                                      << GetTaskResourceType(task) // 0 Task Resource Type - Fire, Jam, Track, or Custom
                                      << task.GetAssigneePlatformName().GetString().c_str() // 1 Assignee
                                      << task.GetResourceName().GetString().c_str()         // 2 Resource
                                      << task.GetTaskType().GetString().c_str()             // 3 Task type
                                      << QString::number(jammerResource.mFrequency)         // 4 Frequency
                                      << QString::number(jammerResource.mBandwidth)         // 5 Bandwidth
                                      << QString::number(jammerResource.mBeamNumber)        // 6 Beam number
                                   ,
                                   (int)cTASK);

            // Set this data so that the task can be uniquely identified later
            taskItem->setData(0, cTASKID_ROLE, task.GetTaskId());
            taskItem->setData(0, cPROCESSOR_ROLE, aTaskManagerName.c_str());

            trackItem->addChild(taskItem);
         }
         else
         {
            auto* taskItem =
               new QTreeWidgetItem(QStringList()
                                      << GetTaskResourceType(task) // 0 Task Resource Type - Fire, Jam, Track, or Custom
                                      << task.GetAssigneePlatformName().GetString().c_str() // 1 Assignee
                                      << task.GetResourceName().GetString().c_str()         // 2 Resource
                                      << task.GetTaskType().GetString().c_str()             // 3 Task type
                                      << ""
                                      << ""
                                      << "",
                                   (int)cTASK);

            // Set this data so that the task can be uniquely identified later
            taskItem->setData(0, cTASKID_ROLE, task.GetTaskId());
            taskItem->setData(0, cPROCESSOR_ROLE, aTaskManagerName.c_str());
            trackItem->addChild(taskItem);
         }
      }
      mUi.mTreeWidget->topLevelItem(topLevelItemCount - 1)->addChild(trackItem);
   }
}

// ============================================================================
void WkTaskStatus::DockWidget::CancelTaskHandler()
{
   QAction* actionPtr = dynamic_cast<QAction*>(sender());
   if (actionPtr)
   {
      auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(mUi.mAssignerName->text());
      if (platformPtr)
      {
         if (!warlock::HasPermissionToControlPlatform(*platformPtr))
         {
            QMessageBox::warning(nullptr,
                                 "Cancel Task Failed",
                                 "User does not have permissions to modify platform " +
                                    QString::fromStdString(platformPtr->GetName()) + ".");
         }
         else
         {
            mSimInterfacePtr->AddSimCommand(
               ut::make_unique<CancelTaskCommand>(mUi.mAssignerName->text().toStdString(),
                                                  actionPtr->property(cPROCESSOR_STR).toString().toStdString(),
                                                  actionPtr->property(cTASKID_STR).toUInt()));
         }
      }
   }
}

// ============================================================================
void WkTaskStatus::DockWidget::CancelAllTasksHandler()
{
   // Get the QAction from the sender()
   QAction* actionPtr = dynamic_cast<QAction*>(sender());
   if (actionPtr)
   {
      // Find the top level item that is the "Assigned" tree widget item
      for (int assignedCount = 0; assignedCount < mUi.mTreeWidget->topLevelItemCount(); ++assignedCount)
      {
         QTreeWidgetItem* parentItem = mUi.mTreeWidget->topLevelItem(assignedCount);
         if (parentItem->type() == cASSIGNED)
         {
            // Find the child item that has the same trackIndex as the QAction
            for (int trackCount = 0; trackCount < parentItem->childCount(); ++trackCount)
            {
               // Get the track index from the QAction
               QVariant var = actionPtr->property(cTRACK_STR);

               QTreeWidgetItem* trackItem = parentItem->child(trackCount);
               // If var is invalid this we are going to cancel all tasks for all tracks, since a track was not
               // specified. If a track was specified, only remove the tasks related to the specified track index
               if (!var.isValid() || var.toInt() == trackItem->data(0, cTRACKINDEX_ROLE))
               {
                  // We found the track item that we want to issue an abort to all children that are of "Fire" type
                  for (int taskCount = 0; taskCount < trackItem->childCount(); ++taskCount)
                  {
                     QTreeWidgetItem* taskItem = trackItem->child(taskCount);
                     mSimInterfacePtr->AddSimCommand(
                        ut::make_unique<CancelTaskCommand>(mUi.mAssignerName->text().toStdString(),
                                                           taskItem->data(0, cPROCESSOR_ROLE).toString().toStdString(),
                                                           taskItem->data(0, cTASKID_ROLE).toUInt()));
                  }
                  break;
               }
            }
            break;
         }
      }
   }
}

// ============================================================================
void WkTaskStatus::DockWidget::AbortFiringHandler()
{
   // Get the QAction from the sender()
   QAction* actionPtr = dynamic_cast<QAction*>(sender());
   if (actionPtr)
   {
      // Get the track index from the QAction
      QVariant var = actionPtr->property(cTRACK_STR);
      if (var.isValid())
      {
         int trackIndex = var.toInt();
         // Find the top level item that is the "Assigned" tree widget item
         for (int assignedCount = 0; assignedCount < mUi.mTreeWidget->topLevelItemCount(); ++assignedCount)
         {
            QTreeWidgetItem* parentItem = mUi.mTreeWidget->topLevelItem(assignedCount);
            if (parentItem->type() == cASSIGNED)
            {
               // Find the child item that has the same trackIndex as the QAction
               for (int trackCount = 0; trackCount < parentItem->childCount(); ++trackCount)
               {
                  QTreeWidgetItem* trackItem = parentItem->child(trackCount);
                  if (trackIndex == trackItem->data(0, cTRACKINDEX_ROLE))
                  {
                     // We found the track item that we want to issue an abort to all children that are of "Fire" type
                     for (int taskCount = 0; taskCount < trackItem->childCount(); ++taskCount)
                     {
                        QTreeWidgetItem* taskItem = trackItem->child(taskCount);
                        // Find all the Fire tasks
                        if (taskItem->text(0) == resourceTypeToStringMap.at(cTASK_RESOURCE_TYPE_WEAPON))
                        {
                           mSimInterfacePtr->AddSimCommand(
                              ut::make_unique<CancelTaskCommand>(mUi.mAssignerName->text().toStdString(),
                                                                 taskItem->data(0, cPROCESSOR_ROLE).toString().toStdString(),
                                                                 taskItem->data(0, cTASKID_ROLE).toUInt()));
                        }
                     }
                     break;
                  }
               }
               break;
            }
         }
      }
   }
}

// ============================================================================
QString WkTaskStatus::DockWidget::GetTaskResourceType(const WsfTask& aTask)
{
   auto iter = resourceTypeToStringMap.find(aTask.GetResourceType());
   if (iter != resourceTypeToStringMap.end())
   {
      return iter->second;
   }
   return "NA";
}

// ============================================================================
void WkTaskStatus::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      mData.SetPlatformOfInterest(aPlatformPtr->GetName());
      mSimInterfacePtr->SetPlatformOfInterest(aPlatformPtr->GetName());
   }
   else
   {
      mData.SetPlatformOfInterest("");
      mSimInterfacePtr->SetPlatformOfInterest("");
   }
}

// ============================================================================
void WkTaskStatus::DockWidget::TaskChanged()
{
   // checking visiblity means task status will always be empty when set to viewable, so don't check
   mUi.mTreeWidget->clear();

   // If the platform of interest is valid
   if (mData.IsValid())
   {
      mUi.mAssignerName->setText(mData.GetPlatformOfInterest().c_str());

      const auto* taskProcessorData = mData.GetData(mData.GetPlatformOfInterest());
      if (taskProcessorData && !taskProcessorData->empty())
      {
         // There may be more than one task processor on a platform
         for (const auto& taskProc : *taskProcessorData)
         {
            QString assignedStr = "Assigned ( " + QString::number(taskProc.mAssignedTaskList.size()) + " )";
            auto*   item = new QTreeWidgetItem(QStringList() << assignedStr << taskProc.Name().c_str(), cASSIGNED);
            mUi.mTreeWidget->addTopLevelItem(item);
            AddTasks(taskProc.Name(), taskProc.mAssignedTaskList);

            QString receivedStr = "Received ( " + QString::number(taskProc.mReceivedTaskList.size()) + " )";
            item = new QTreeWidgetItem(QStringList() << receivedStr << taskProc.Name().c_str(), cRECEIVED);
            mUi.mTreeWidget->addTopLevelItem(item);
            AddTasks(taskProc.Name(), taskProc.mReceivedTaskList);
         }
         mUi.mTreeWidget->expandAll();
         for (int i = 0; i < mUi.mTreeWidget->columnCount(); ++i)
         {
            mUi.mTreeWidget->resizeColumnToContents(i);
         }
      }
   }
}
