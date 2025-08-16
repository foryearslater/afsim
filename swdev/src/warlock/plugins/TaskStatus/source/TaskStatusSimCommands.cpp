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

#include "TaskStatusSimCommands.hpp"

#include "WkObserver.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTaskManager.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"

// ============================================================================
void WkTaskStatus::CancelTaskCommand::Process(WsfSimulation& aSimulation)
{
   // Get the assigner platform
   WsfPlatform* assignerPtr = aSimulation.GetPlatformByName(mAssignerName);
   if (assignerPtr)
   {
      // Get a task manager
      WsfTaskManager* taskManagerPtr = assignerPtr->GetComponent<WsfTaskManager>(mTaskProcessorName);
      if (taskManagerPtr)
      {
         if (taskManagerPtr->CancelTask(aSimulation.GetSimTime(), mTaskId))
         {
            WkObserver::SimulationUserAction(aSimulation)("Task Cancelled on " + mTaskProcessorName, assignerPtr);
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Cancel Task Failed",
                                                              "Cancel task failed, the specified task was not found");
         }
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                           "Task Manager not found",
                                                           "The Task Manager (" +
                                                              QString::fromStdString(mTaskProcessorName) +
                                                              ") was not found.\nUnable to cancel task.");
      }
   }
   else
   {
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                        "Platform not found",
                                                        "The assignee platform (" + QString::fromStdString(mAssignerName) +
                                                           ") was not found.\nUnable to cancel task.");
   }
}
