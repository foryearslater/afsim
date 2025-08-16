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

#include "SensorControllerSimCommand.hpp"

#include <memory>

#include "WkObserver.hpp"
#include "WsfArticulatedPart.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackManager.hpp"

//////////////////////////////////////////////////////////////
//! Process the command to turn off a sensor.
//!
//! @param aSimulation The simulation
void WkSensorController::TurnOffCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr = aSimulation.GetPlatformByName(mPlatform);
   if (platformPtr != nullptr)
   {
      auto sensorPtr = platformPtr->GetArticulatedPart(mSensorId);
      if (sensorPtr)
      {
         sensorPtr->TurnOff(aSimulation.GetSimTime());
         WkObserver::SimulationUserAction(aSimulation)("Sensor " + sensorPtr->GetName() + " Turned Off", platformPtr);
      }
   }
}

//////////////////////////////////////////////////////////////
//! Process the command to turn on a sensor.
//!
//! @param aSimulation The simulation
void WkSensorController::TurnOnCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr = aSimulation.GetPlatformByName(mPlatform);
   if (platformPtr != nullptr)
   {
      auto sensorPtr = platformPtr->GetArticulatedPart(mSensorId);
      if (sensorPtr)
      {
         sensorPtr->TurnOn(aSimulation.GetSimTime());
         WkObserver::SimulationUserAction(aSimulation)("Sensor " + sensorPtr->GetName() + " Turned On", platformPtr);
      }
   }
}

//////////////////////////////////////////////////////////////
//! Process the command to cue/slew a sensor to the absolute AzEl.
//!
//! @param aSimulation The simulation
void WkSensorController::SlewToAzElCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr = aSimulation.GetPlatformByName(mPlatform);
   if (platformPtr != nullptr)
   {
      auto sensorPtr = platformPtr->GetArticulatedPart(mSensorId);
      if (sensorPtr != nullptr)
      {
         double heading, pitch, roll;
         double az = mAzimuth;
         double el = mElevation;
         platformPtr->GetOrientationNED(heading, pitch, roll);
         double ned[3] = {heading, 0, 0};

         sensorPtr->GetAbsoluteCuedOrientation(az, el, ned);
         if (sensorPtr->WithinSlewLimits(az, el))
         {
            sensorPtr->SetCuedOrientation(az, el);
            std::string action = "Slew Sensor " + sensorPtr->GetName() + " To Az: " + std::to_string(mAzimuth) +
                                 "(rad) El: " + std::to_string(el) + "(rad)";
            WkObserver::SimulationUserAction(aSimulation)(action, platformPtr);
         }
      }
   }
}

//////////////////////////////////////////////////////////////
//! Process the command to cue/slew a sensor to the a track location.
//!
//! @param aSimulation The simulation
void WkSensorController::SlewToTrackCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr = aSimulation.GetPlatformByName(mPlatform);
   if (platformPtr != nullptr)
   {
      auto trackPtr =
         platformPtr->GetTrackManager().FindTrack(WsfTrackId(mTrackId.GetPlatformName(), mTrackId.GetLocalTrackId()));
      if (trackPtr != nullptr)
      {
         auto sensorPtr = dynamic_cast<WsfSensor*>(platformPtr->GetArticulatedPart(mSensorId));
         if (sensorPtr != nullptr)
         {
            double locationWCS[3];
            double relLocWCS[3];
            double azimuth   = 0.0;
            double elevation = 0.0;

            trackPtr->GetExtrapolatedLocationWCS(aSimulation.GetSimTime(), locationWCS);

            sensorPtr->GetRelativeLocationWCS(locationWCS, relLocWCS);
            sensorPtr->ComputeAspect(relLocWCS, azimuth, elevation);
            if (sensorPtr->WithinSlewLimits(azimuth, elevation))
            {
               sensorPtr->SetCuedLocationWCS(locationWCS);
               sensorPtr->StartTracking(aSimulation.GetSimTime(), *trackPtr, sensorPtr->GetCurrentModeName());
               std::string action = "Slew Sensor " + sensorPtr->GetName() + " To Track " + mTrackId;
               WkObserver::SimulationUserAction(aSimulation)(action, platformPtr);
            }
         }
      }
   }
}
