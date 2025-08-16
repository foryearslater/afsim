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

#include "TaskStatusSimInterface.hpp"

#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTaskObserver.hpp"
#include "WsfTaskProcessor.hpp"

// ============================================================================
WkTaskStatus::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<TaskEvent>(aPluginName)
{
}

// ============================================================================
void WkTaskStatus::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

// ============================================================================
void WkTaskStatus::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest.clear();
}

// ============================================================================
void WkTaskStatus::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   const WsfPlatform* platformPtr = nullptr;

   mMutex.lock();
   if (!mPlatformOfInterest.empty())
   {
      platformPtr = aSimulation.GetPlatformByName(mPlatformOfInterest);
   }
   mMutex.unlock();

   if (platformPtr)
   {
      std::map<std::string, std::vector<ProcessorData>> taskProcessorsData;
      GetComponentData(platformPtr, taskProcessorsData);
      AddSimEvent(ut::make_unique<TaskUpdateEvent>(platformPtr->GetName(), taskProcessorsData));
   }
}

// ============================================================================
void WkTaskStatus::SimInterface::GetComponentData(const WsfPlatform*                                 aPlatformPtr,
                                                  std::map<std::string, std::vector<ProcessorData>>& aTaskProcessorsData)

{
   std::vector<ProcessorData> taskProcessorData;
   GetComponentData(aPlatformPtr, taskProcessorData);
   aTaskProcessorsData.emplace(std::make_pair(aPlatformPtr->GetName(), taskProcessorData));
}

// ============================================================================
void WkTaskStatus::SimInterface::GetComponentData(const WsfPlatform*          aPlatformPtr,
                                                  std::vector<ProcessorData>& aTaskProcessorData)
{
   // Processors
   for (WsfComponentList::RoleIterator<WsfProcessor> iter(*aPlatformPtr); !iter.AtEnd(); ++iter)
   {
      // Task processor
      if ((*iter)->IsA_TypeOf("WSF_TASK_PROCESSOR"))
      {
         WsfTaskProcessor* taskProcessorPtr = dynamic_cast<WsfTaskProcessor*>(*iter);

         std::map<WsfTrackId, WsfTaskManager::TaskList> assignedTaskList;
         for (const auto& task : taskProcessorPtr->AssignedTaskList())
         {
            auto mapIter = assignedTaskList.find(task.GetLocalTrackId());
            if (mapIter == assignedTaskList.end())
            {
               // Entry is NOT found
               WsfTaskManager::TaskList taskList;
               taskList.push_back(task);
               assignedTaskList.emplace(std::make_pair(task.GetLocalTrackId(), taskList));
            }
            else
            {
               // Entry is found
               (*mapIter).second.push_back(task);
            }
         }

         std::map<WsfTrackId, WsfTaskManager::TaskList> receivedTaskList;
         for (const auto& task : taskProcessorPtr->ReceivedTaskList())
         {
            auto mapIter = receivedTaskList.find(task.GetLocalTrackId());
            if (mapIter == receivedTaskList.end())
            {
               // Entry is NOT found
               WsfTaskManager::TaskList taskList;
               taskList.push_back(task);
               receivedTaskList.emplace(std::make_pair(task.GetLocalTrackId(), taskList));
            }
            else
            {
               // Entry is found
               (*mapIter).second.push_back(task);
            }
         }

         aTaskProcessorData.emplace_back((*iter)->GetName(), (*iter)->GetType(), assignedTaskList, receivedTaskList);
      }
   }
}
