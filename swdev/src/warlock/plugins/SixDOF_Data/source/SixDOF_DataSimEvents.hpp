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

#ifndef SIXDOFDATASIMEVENTS_HPP
#define SIXDOFDATASIMEVENTS_HPP

#include "SixDOF_DataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkSixDOF_Data
{

class SixDOF_DataEvent : public warlock::SimEvent
{
public:
   SixDOF_DataEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aData) = 0;
};

class UpdateEvent : public SixDOF_DataEvent
{
public:
   explicit UpdateEvent(const DataContainer::PlatformData& aData)
      : SixDOF_DataEvent(true)
      , mData(aData)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.SetPlatformData(mData); }

private:
   DataContainer::PlatformData mData;
};

class PlatformAddedEvent : public SixDOF_DataEvent
{
public:
   explicit PlatformAddedEvent(const std::string& aPlatformName)
      : SixDOF_DataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.AddPlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformRemovedEvent : public SixDOF_DataEvent
{
public:
   explicit PlatformRemovedEvent(const std::string& aPlatformName)
      : SixDOF_DataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.RemovePlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformClearEvent : public SixDOF_DataEvent
{
public:
   PlatformClearEvent() = default;
   void Process(DataContainer& aDataContainer) override { aDataContainer.ClearPlatforms(); }
};

} // namespace WkSixDOF_Data

#endif
