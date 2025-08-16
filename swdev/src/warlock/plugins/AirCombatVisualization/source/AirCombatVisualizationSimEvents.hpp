// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef AIRCOMBATVISUALIZATIONSIMEVENTS_HPP
#define AIRCOMBATVISUALIZATIONSIMEVENTS_HPP

#include "WkSimInterface.hpp"
#include "air_combat/WkfAirCombatDataContainer.hpp"

namespace WkAirCombat
{
class AirCombatSimEvent : public warlock::SimEvent
{
public:
   AirCombatSimEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(wkf::DataContainer& aState) = 0;
};

class UpdateEvent : public AirCombatSimEvent
{
public:
   UpdateEvent(const std::vector<wkf::AirCombatDataEngagementSummary>& aData)
      : AirCombatSimEvent(true)
      , mData(aData)
   {
   }
   void Process(wkf::DataContainer& aState) override;

private:
   std::vector<wkf::AirCombatDataEngagementSummary> mData;
};

class PlatformAddedEvent : public AirCombatSimEvent
{
public:
   PlatformAddedEvent(const std::string& aPlatform)
      : AirCombatSimEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(wkf::DataContainer& aState) override;

private:
   std::string mPlatform;
};

class PlatformDeletedEvent : public AirCombatSimEvent
{
public:
   PlatformDeletedEvent(const std::string& aPlatform)
      : AirCombatSimEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(wkf::DataContainer& aState) override;

private:
   std::string mPlatform;
};

class SimulationCompleteEvent : public AirCombatSimEvent
{
public:
   SimulationCompleteEvent()
      : AirCombatSimEvent()
   {
   }

   void Process(wkf::DataContainer& aDataContainer) override;
};
} // namespace WkAirCombat

#endif // AIRCOMBATVISUALIZATIONSIMEVENTS_HPP
