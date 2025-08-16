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

#include "TaskStatusDataContainer.hpp"

// ============================================================================
// WsfTaskManager::TaskList WkTaskStatus::ProcessorData::GetAssignedData(const WsfTrackId& aTrackId)
// {
//    WsfTaskManager::TaskList taskList;
//    auto mapIter = mAssignedTaskList.find(aTrackId);
//    if (mapIter != mAssignedTaskList.end())
//    {
//       taskList = (*mapIter).second;
//    }
//    return taskList;
// }

// ============================================================================
void WkTaskStatus::DataContainer::SetPlatformOfInterest(const std::string& aPlatformName)
{
   mTaskProcessors.clear();
   mValid              = false;
   mPlatformOfInterest = aPlatformName;
}

bool WkTaskStatus::ProcessorData::operator==(const ProcessorData& aRhs) const
{
   return (mAssignedTaskList == aRhs.mAssignedTaskList) && (mReceivedTaskList == aRhs.mReceivedTaskList);
}

// ============================================================================
void WkTaskStatus::DataContainer::SetData(const std::string& aPlatformName, const TaskProcessors& aTaskProcessorData)
{
   mValid = false;
   if (mPlatformOfInterest == aPlatformName)
   {
      auto currIt = mTaskProcessors.find(aPlatformName);
      auto newIt  = aTaskProcessorData.find(aPlatformName);
      if (currIt == mTaskProcessors.end() || newIt == aTaskProcessorData.end() || currIt->second != newIt->second)
      {
         mTaskProcessors = aTaskProcessorData;
         mValid          = true;
         emit TaskChanged();
      }
   }
}

// ============================================================================
const std::vector<WkTaskStatus::ProcessorData>* WkTaskStatus::DataContainer::GetData(const std::string& aPlatformName) const
{
   auto data = mTaskProcessors.find(aPlatformName);
   if (data != mTaskProcessors.end())
   {
      return &(data->second);
   }
   return nullptr;
}
