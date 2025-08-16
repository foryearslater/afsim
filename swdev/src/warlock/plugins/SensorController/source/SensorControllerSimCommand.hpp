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
#ifndef SENSORCONTROLLERSIMCOMMANDS_HPP
#define SENSORCONTROLLERSIMCOMMANDS_HPP

#include <string>

#include "WkSimInterface.hpp"
#include "WkfTrack.hpp"

namespace WkSensorController
{
class SensorCommand : public warlock::SimCommand
{
public:
   SensorCommand(const std::string& aPlatform, int aId)
      : SimCommand()
      , mPlatform(aPlatform)
      , mSensorId(aId)
   {
   }

protected:
   std::string mPlatform;
   int         mSensorId;
};

class TurnOnCommand : public SensorCommand
{
public:
   TurnOnCommand(const std::string& aPlatform, int aId)
      : SensorCommand(aPlatform, aId)
   {
   }

   void Process(WsfSimulation& aSimulation) override;
};

class TurnOffCommand : public SensorCommand
{
public:
   TurnOffCommand(const std::string& aPlatform, int aId)
      : SensorCommand(aPlatform, aId)
   {
   }

   void Process(WsfSimulation& aSimulation) override;
};

class SlewToAzElCommand : public SensorCommand
{
public:
   SlewToAzElCommand(const std::string& aPlatform, int aId, double aAzimuth, double aElevation)
      : SensorCommand(aPlatform, aId)
      , mAzimuth(aAzimuth)
      , mElevation(aElevation){};

   void Process(WsfSimulation& aSimulation) override;

private:
   double mAzimuth;
   double mElevation;
};

class SlewToTrackCommand : public SensorCommand
{
public:
   SlewToTrackCommand(const std::string& aPlatform, int aId, const wkf::TrackId& aTrackId)
      : SensorCommand(aPlatform, aId)
      , mTrackId(aTrackId)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   wkf::TrackId mTrackId;
};
} // namespace WkSensorController
#endif
