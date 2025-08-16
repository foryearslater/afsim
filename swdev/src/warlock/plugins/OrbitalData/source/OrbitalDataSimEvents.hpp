//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef ORBITALDATASIMEVENTS_HPP
#define ORBITALDATASIMEVENTS_HPP

#include "OrbitalDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkOrbitalData
{

class OrbitalDataEvent : public warlock::SimEvent
{
public:
   OrbitalDataEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aData) = 0;
};

class UpdateEvent : public OrbitalDataEvent
{
public:
   explicit UpdateEvent(const DataContainer::OrbitalData& aData)
      : OrbitalDataEvent(true)
      , mData(aData)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.SetPlatformData(mData); }

private:
   DataContainer::OrbitalData mData;
};

class PlatformAddedEvent : public OrbitalDataEvent
{
public:
   explicit PlatformAddedEvent(const std::string& aPlatformName)
      : OrbitalDataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.AddOrbitalPlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformRemovedEvent : public OrbitalDataEvent
{
public:
   explicit PlatformRemovedEvent(const std::string& aPlatformName)
      : OrbitalDataEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.RemoveOrbitalPlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class PlatformClearEvent : public OrbitalDataEvent
{
public:
   PlatformClearEvent() = default;
   void Process(DataContainer& aDataContainer) override { aDataContainer.ClearPlatforms(); }
};

} // namespace WkOrbitalData

#endif
