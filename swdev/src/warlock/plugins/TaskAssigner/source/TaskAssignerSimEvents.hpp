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

#ifndef TASKASSIGNERSIMEVENTS_HPP
#define TASKASSIGNERSIMEVENTS_HPP

#include "TaskAssignerDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkTaskAssigner
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
   TaskUpdateEvent(const std::string&                    aPlatformName,
                   CommandChainData&                     aCommandChainData,
                   ObjectList&                           aCommData,
                   std::map<std::string, JammerList>&    aJammers,
                   std::map<std::string, ProcessorList>& aProcessors,
                   std::map<std::string, ProcessorList>& aTaskProcessors,
                   std::map<std::string, SensorList>&    aSensors,
                   std::map<std::string, WeaponList>&    aWeapons)
      : TaskEvent(true) // set to recurring
      , mPlatformOfInterest(aPlatformName)
      , mCommandChainData(aCommandChainData)
      , mCommunicationDevices(aCommData)
      , mJammers(aJammers)
      , mProcessors(aProcessors)
      , mTaskProcessors(aTaskProcessors)
      , mSensors(aSensors)
      , mWeapons(aWeapons)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::string mPlatformOfInterest;

   CommandChainData mCommandChainData;
   ObjectList       mCommunicationDevices;

   // Platform components
   std::map<std::string, JammerList>    mJammers;
   std::map<std::string, ProcessorList> mProcessors;
   std::map<std::string, ProcessorList> mTaskProcessors;
   std::map<std::string, SensorList>    mSensors;
   std::map<std::string, WeaponList>    mWeapons;
};

class DeletePlatformEvent : public TaskEvent
{
public:
   explicit DeletePlatformEvent(const std::string& aPlatformName)
      : mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::string mPlatformName;
};

} // namespace WkTaskAssigner

#endif