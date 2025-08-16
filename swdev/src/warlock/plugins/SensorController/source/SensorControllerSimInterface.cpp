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

#include "SensorControllerSimInterface.hpp"

#include <UtMemory.hpp>

#include "WsfSimulation.hpp"

WkSensorController::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<SensorControllerEvent>(aPluginName)
{
}

//////////////////////////////////////////////////////////////
//! On SimulationInitialize, add all of the sensor callbacks to listen for new/updated sensor data.
//!
//! @param aSimulation The simulation
void WkSensorController::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();

   mCallbacks.Add(WsfObserver::SensorTurnedOn(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { UpdateSensor(aSensor); }));
   mCallbacks.Add(WsfObserver::SensorTurnedOff(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { UpdateSensor(aSensor); }));
   mCallbacks.Add(WsfObserver::SensorOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { UpdateSensor(aSensor); }));
   mCallbacks.Add(WsfObserver::SensorNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { UpdateSensor(aSensor); }));
   mCallbacks.Add(WsfObserver::SensorBroken(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { UpdateSensor(aSensor); }));
   mCallbacks.Add(
      WsfObserver::SensorModeActivated(&aSimulation)
         .Connect([this](double aSimTime, WsfSensor* aSensor, WsfSensorMode* aMode) { UpdateSensor(aSensor); }));
   mCallbacks.Add(
      WsfObserver::SensorModeDeactivated(&aSimulation)
         .Connect([this](double aSimTime, WsfSensor* aSensor, WsfSensorMode* aMode) { UpdateSensor(aSensor); }));
   mCallbacks.Add(WsfObserver::PlatformAdded(&aSimulation)
                     .Connect([this](double aSimTime, WsfPlatform* aPlatform) { AddSlewingCompleteCallback(aPlatform); }));
}

// Notify the plugin that the sensor has been altered.
void WkSensorController::SimInterface::UpdateSensor(WsfSensor* aSensorPtr)
{
   double minAz   = std::max<double>(aSensorPtr->GetMinAzSlew(), aSensorPtr->GetMinAzCue());
   double maxAz   = std::min<double>(aSensorPtr->GetMaxAzSlew(), aSensorPtr->GetMaxAzCue());
   double minEl   = std::max<double>(aSensorPtr->GetMinElSlew(), aSensorPtr->GetMinElCue());
   double maxEl   = std::min<double>(aSensorPtr->GetMaxElSlew(), aSensorPtr->GetMaxElCue());
   bool   canSlew = aSensorPtr->GetSlewMode() != WsfArticulatedPart::SlewMode::cSLEW_FIXED &&
                  aSensorPtr->GetCueMode() != WsfArticulatedPart::SlewMode::cSLEW_FIXED;
   AddSimEvent(ut::make_unique<SensorEvent>(aSensorPtr->GetPlatform()->GetName(),
                                            aSensorPtr->GetName(),
                                            aSensorPtr->GetUniqueId(),
                                            aSensorPtr->CanBeTurnedOn(),
                                            aSensorPtr->CanBeTurnedOff(),
                                            canSlew,
                                            minAz,
                                            maxAz,
                                            minEl,
                                            maxEl));
}

// Add a callback for when a sensor has completed slewing to all sensors.
void WkSensorController::SimInterface::AddSlewingCompleteCallback(WsfPlatform* aPlatformPtr)
{
   for (auto component : aPlatformPtr->GetComponents())
   {
      WsfSensor* sensor = dynamic_cast<WsfSensor*>(component);
      if (sensor != nullptr)
      {
         mCallbacks.Add(sensor->SlewComplete.Connect(
            [this, sensor](double aSimTime)
            { AddSimEvent(ut::make_unique<SlewCompleteEvent>(sensor->GetPlatform()->GetName(), sensor->GetName())); }));
      }
   }
}
