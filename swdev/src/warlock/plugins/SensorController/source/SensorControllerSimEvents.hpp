// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SENSORCONTROLLERSIMEVENTS_HPP
#define SENSORCONTROLLERSIMEVENTS_HPP

#include <string>

#include "SensorControllerDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkSensorController
{
class SensorControllerEvent : public warlock::SimEvent
{
public:
   explicit SensorControllerEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aData) = 0;
};

class SensorEvent : public SensorControllerEvent
{
public:
   SensorEvent(const std::string& aPlatform,
               const std::string& aSensor,
               int                aSensorId,
               bool               aCanBeTurnedOn,
               bool               aCanBeTurnedOff,
               bool               aIsSlewable,
               double             aAzMin,
               double             aAzMax,
               double             aElMin,
               double             aElMax)
      : SensorControllerEvent()
      , mPlatform(aPlatform)
      , mSensor(aSensor)
      , mSensorId(aSensorId)
      , mCanBeTurnedOn(aCanBeTurnedOn)
      , mCanBeTurnedOff(aCanBeTurnedOff)
      , mIsSlewable(aIsSlewable)
      , mAzMin(aAzMin)
      , mAzMax(aAzMax)
      , mElMin(aElMin)
      , mElMax(aElMax)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::string mPlatform;
   std::string mSensor;
   int         mSensorId;
   bool        mCanBeTurnedOn;
   bool        mCanBeTurnedOff;
   bool        mIsSlewable;
   double      mAzMin;
   double      mAzMax;
   double      mElMin;
   double      mElMax;
};

class SlewCompleteEvent : public SensorControllerEvent
{
public:
   SlewCompleteEvent(const std::string& aPlatform, const std::string& aSensor)
      : mPlatform(aPlatform)
      , mSensor(aSensor)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::string mPlatform;
   std::string mSensor;
};
} // namespace WkSensorController

#endif
