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

#ifndef SIMCONTROLLERSIMEVENTS_HPP
#define SIMCONTROLLERSIMEVENTS_HPP

#include <string>

#include "SimControllerDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkSimController
{
class SimControllerEvent : public warlock::SimEvent
{
public:
   SimControllerEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }
   virtual void Process(SimControllerDataContainer& aState) = 0;
};

class SimPausingEvent : public SimControllerEvent
{
public:
   SimPausingEvent()
      : SimControllerEvent(false)
   {
   }

   void Process(SimControllerDataContainer& aState) override;
};

class SimResumingEvent : public SimControllerEvent
{
public:
   SimResumingEvent()
      : SimControllerEvent(false)
   {
   }

   void Process(SimControllerDataContainer& aState) override;
};

class SimCompleteEvent : public SimControllerEvent
{
public:
   SimCompleteEvent(double aSimTime)
      : SimControllerEvent(false)
      , mSimTime(aSimTime)
   {
   }

   void Process(SimControllerDataContainer& aState) override;

private:
   double mSimTime;
};

class SimClockRateEvent : public SimControllerEvent
{
public:
   SimClockRateEvent(double aClockRate)
      : SimControllerEvent(false)
      , mClockRate(aClockRate)
   {
   }

   void Process(SimControllerDataContainer& aState) override;

private:
   double mClockRate;
};

class SimStartingEvent : public SimControllerEvent
{
public:
   SimStartingEvent(bool aPaused)
      : SimControllerEvent(false)
      , mPaused(aPaused)
   {
   }

   void Process(SimControllerDataContainer& aState) override;

private:
   bool              mPaused;
   const std::string mStateString;
};

class TimeBehindEvent : public SimControllerEvent
{
public:
   TimeBehindEvent(bool aIsBehind, const std::string& aApplicationId)
      : SimControllerEvent(false)
      , mIsBehind(aIsBehind)
      , mApplicationId(aApplicationId)
   {
   }

   void Process(SimControllerDataContainer& aState) override;

private:
   bool        mIsBehind;
   std::string mApplicationId;
};


class SimStateEvent : public SimControllerEvent
{
public:
   SimStateEvent(const SimControllerDataContainer::SimState& aState)
      : SimControllerEvent(true)
      , mState(aState)
   {
   }

   void Process(SimControllerDataContainer& aState) override;

private:
   SimControllerDataContainer::SimState mState;
};
} // namespace WkSimController

#endif
