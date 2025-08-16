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

#ifndef TASKSTATUSSIMEVENTS_HPP
#define TASKSTATUSSIMEVENTS_HPP

#include "TaskStatusDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkTaskStatus
{
class TaskEvent : public warlock::SimEvent
{
public:
   explicit TaskEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aData) = 0;
};

class TaskUpdateEvent : public TaskEvent
{
public:
   // TaskUpdateEvent is a recurring event, which means only the most recent
   // TaskUpdateEvent message is relevant and the old ones may be discarded.
   // Events may be generated on the simulation faster than the GUI will
   // process them due to the ability to change the simulation clock rate,
   // by setting aRecurring = true, significantly less processing is required
   // on the GUI thread to handle stale messages.
   TaskUpdateEvent(const std::string& aPlatformName, std::map<std::string, std::vector<ProcessorData>>& aTaskProcessors)
      : TaskEvent(true) // set to recurring
      , mPlatformOfInterest(aPlatformName)
      , mTaskProcessors(aTaskProcessors)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::string                                       mPlatformOfInterest;
   std::map<std::string, std::vector<ProcessorData>> mTaskProcessors;
};
} // namespace WkTaskStatus

#endif