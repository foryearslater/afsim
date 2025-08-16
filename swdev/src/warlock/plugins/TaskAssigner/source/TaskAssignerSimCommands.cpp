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

#include "TaskAssignerSimCommands.hpp"

#include "WkObserver.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTaskManager.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"

void WkTaskAssigner::TaskCommand::AssignTask(WsfSimulation& aSimulation, const WsfTaskResource& aResource)
{
   // Get the assigner platform
   WsfPlatform* assignerPtr = aSimulation.GetPlatformByName(mAssignerName);

   // Get the assignee; could be a self assignment
   WsfPlatform* assigneePtr = assignerPtr;
   if (mAssignerName != mAssigneeName)
   {
      assigneePtr = aSimulation.GetPlatformByName(mAssigneeName);
   }

   if (assignerPtr && assigneePtr)
   {
      // Get a task manager
      WsfTaskManager* taskManagerPtr = assignerPtr->GetComponent<WsfTaskManager>(mTaskProcessorName);
      if (taskManagerPtr)
      {
         // Find the local track given the index
         WsfTrack* trackPtr = assignerPtr->GetTrackManager().FindTrack(WsfTrackId(mAssignerName, mTrackIndex));
         if (trackPtr)
         {
            taskManagerPtr->AssignTask(aSimulation.GetSimTime(), *trackPtr, mTaskType, aResource, assigneePtr, mCommName);
            WkObserver::SimulationUserAction(
               aSimulation)("Task Assigned to Processor " + mTaskProcessorName + " of Type " + mTaskType, assigneePtr);
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Track not found",
                                                              "The track (" + QString::fromStdString(mAssignerName) +
                                                                 ":" + QString::number(mTrackIndex) +
                                                                 ") was not found.\nUnable to assign task.");
         }
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                           "Task Manager not found",
                                                           "The Task Manager (" +
                                                              QString::fromStdString(mTaskProcessorName) +
                                                              ") was not found.\nUnable to assign task.");
      }
   }
   else
   {
      // Get which platform was not found.
      std::string missingPlatformName = mAssignerName;
      // If assignee was invalid, set it as the name, otherwise set the assigner as the name
      if (!assigneePtr)
      {
         missingPlatformName = mAssigneeName;
      }
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                        "Platform not found",
                                                        "The platform (" + QString::fromStdString(missingPlatformName) +
                                                           ") was not found.\nUnable to assign task.");
   }
}

void WkTaskAssigner::AssignTaskCommand::Process(WsfSimulation& aSimulation)
{
   AssignTask(aSimulation, mResource);
}

// ============================================================================
void WkTaskAssigner::MilAssignJammerTaskCommand::Process(WsfSimulation& aSimulation)
{
   // Get the assigner platform
   WsfPlatform* assignerPtr = aSimulation.GetPlatformByName(mAssignerName);

   // Get the assignee; could be a self assignment
   WsfPlatform* assigneePtr = assignerPtr;
   if (mAssignerName != mAssigneeName)
   {
      assigneePtr = aSimulation.GetPlatformByName(mAssigneeName);
   }

   if (assignerPtr && assigneePtr)
   {
      // Get a task manager
      WsfTaskManager* taskManagerPtr = assignerPtr->GetComponent<WsfTaskManager>(mTaskProcessorName);
      if (taskManagerPtr)
      {
         // Find the local track given the index
         WsfTrack* trackPtr = assignerPtr->GetTrackManager().FindTrack(WsfTrackId(mAssignerName, mTrackIndex));
         if (trackPtr)
         {
            if (trackPtr->FrequencyValid() && (fabs(mResource.mFrequency) < DBL_EPSILON))
            {
               mResource.mFrequency = trackPtr->GetFrequency();
            }
            taskManagerPtr->AssignTask(aSimulation.GetSimTime(), *trackPtr, mTaskType, mResource, assigneePtr, mCommName);
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Track not found",
                                                              "The track (" + QString::fromStdString(mAssignerName) +
                                                                 ":" + QString::number(mTrackIndex) +
                                                                 ") was not found.\nUnable to assign task.");
         }
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                           "Task Manager not found",
                                                           "The Task Manager (" +
                                                              QString::fromStdString(mTaskProcessorName) +
                                                              ") was not found.\nUnable to assign task.");
      }
   }
   else
   {
      // Get which platform was not found.
      std::string missingPlatformName = mAssignerName;
      // If assignee was invalid, set it as the name, otherwise set the assigner as the name
      if (!assigneePtr)
      {
         missingPlatformName = mAssigneeName;
      }
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                        "Platform not found",
                                                        "The platform (" + QString::fromStdString(missingPlatformName) +
                                                           ") was not found.\nUnable to assign task.");
   }
}

// ============================================================================
void WkTaskAssigner::MilAssignWeaponTaskCommand::Process(WsfSimulation& aSimulation)
{
   AssignTask(aSimulation, mResource);
}
