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

#ifndef P6DOF_DATASIMEVENTS_HPP
#define P6DOF_DATASIMEVENTS_HPP

#include "P6DOF_DataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkP6DOF_Data
{

class P6DOF_DataEvent : public warlock::SimEvent
{
public:
   P6DOF_DataEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aData) = 0;
};

class UpdateEvent : public P6DOF_DataEvent
{
public:
   explicit UpdateEvent(const DataContainer::PlatformData& aData)
      : P6DOF_DataEvent(true)
      , mData(aData)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.SetPlatformData(mData); }

private:
   DataContainer::PlatformData mData;
};

class PlatformAddedEvent : public P6DOF_DataEvent
{
public:
   explicit PlatformAddedEvent(const std::string& aPlatformName)
      : P6DOF_DataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.AddP6dofPlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformRemovedEvent : public P6DOF_DataEvent
{
public:
   explicit PlatformRemovedEvent(const std::string& aPlatformName)
      : P6DOF_DataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.RemoveP6dofPlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformClearEvent : public P6DOF_DataEvent
{
public:
   PlatformClearEvent() = default;
   void Process(DataContainer& aDataContainer) override { aDataContainer.ClearPlatforms(); }
};

} // namespace WkP6DOF_Data

#endif
