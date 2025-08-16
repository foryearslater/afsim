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

#ifndef HUD_SIMEVENTS_HPP
#define HUD_SIMEVENTS_HPP

#include <string>

#include "WkSimInterface.hpp"
#include "hud/WkfHUD_DataContainer.hpp"

// This is a collection of SimEvents that are used in the HeadsUpDisplay

namespace HeadsUpDisplay
{
class HeadsUpDisplayEvent : public warlock::SimEvent
{
public:
   virtual void Process(wkf::HUD_DataContainer& aData) = 0;
};

// This defines a base class for SimEvents for the Common HUD
class WkHeadsUpDisplayEvent : public HeadsUpDisplayEvent
{
public:
   WkHeadsUpDisplayEvent(bool aRecurring = false) {}
   virtual void Process(wkf::HUD_DataContainer& aData) = 0;
};

// This is the SimEvent for sim initialization
class SimulationInitializingEvent : public WkHeadsUpDisplayEvent
{
public:
   SimulationInitializingEvent()
      : WkHeadsUpDisplayEvent(false)
   {
   }

   void Process(wkf::HUD_DataContainer& aData) override;
};

// This is the SimEvent for a wall clock read event
class UpdateEvent : public WkHeadsUpDisplayEvent
{
public:
   UpdateEvent(const wkf::HUD_DataContainer& aData)
      : WkHeadsUpDisplayEvent(true)
      , mData(aData)
   {
   }

   void Process(wkf::HUD_DataContainer& aData) override;

private:
   wkf::HUD_DataContainer mData;
};
} // namespace HeadsUpDisplay

#endif // HUD_SIMEVENTS_HPP
