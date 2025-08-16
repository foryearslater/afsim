// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PROJECTORSIMEVENTS_HPP
#define PROJECTORSIMEVENTS_HPP

#include <map>
#include <string>

#include "WkSimInterface.hpp"

namespace Projector
{
class Plugin;

struct SensorState
{
   bool   mUsable{false};
   double mAzWidth{0.0};
   double mElWidth{0.0};
   double mTilt{0.0};
   double mYaw{0.0};
   double mPitch{0.0};
   double mRoll{0.0};
};

class ProjectorEvent : public warlock::SimEvent
{
public:
   ProjectorEvent(const std::string& aPlatformName, const std::string& aSensorName, bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
      , mPlatformName(aPlatformName)
      , mSensorName(aSensorName)
   {
   }

   virtual void Process(Plugin* aPluginPtr) = 0;

protected:
   std::string mPlatformName;
   std::string mSensorName;
};

class UpdateEvent : public ProjectorEvent
{
public:
   UpdateEvent(const std::string& aPlatformName, const std::string& aSensorName, const SensorState& aSensorState)
      : ProjectorEvent(aPlatformName, aSensorName, true)
      , mSensorState(aSensorState)
   {
   }

   void Process(Plugin* aPluginPtr) override;

private:
   SensorState mSensorState;
};

class SensorProjectableEvent : public ProjectorEvent
{
public:
   SensorProjectableEvent(const std::string& aPlatformName, const std::string& aSensorName, bool aGood)
      : ProjectorEvent(aPlatformName, aSensorName, false)
      , mGood(aGood)
   {
   }
   void Process(Plugin* aPluginPtr) override;

private:
   bool mGood;
};
} // namespace Projector


#endif
