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

#include "PlatformHistorySimInterface.hpp"

#include "WsfSensor.hpp"
#include "WsfSensorResult.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"

WkPlatformHistory::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<PlatformHistoryEvent>(aPluginName)
{
}

void WkPlatformHistory::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   mCallbacks.Add(WsfObserver::SensorDetectionChanged(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, size_t aTargetIndex, WsfSensorResult& aResult)
                        {
                           AddSimEvent(ut::make_unique<DetectedEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                      aTargetIndex,
                                                                      aResult.Detected()));
                        }));

   mCallbacks.Add(WsfObserver::SensorTrackInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, const WsfTrack* aTrackPtr)
                        {
                           AddSimEvent(ut::make_unique<TrackedEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                     aTrackPtr->GetTargetIndex(),
                                                                     true));
                        }));

   mCallbacks.Add(WsfObserver::SensorTrackDropped(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, const WsfTrack* aTrackPtr)
                        {
                           AddSimEvent(ut::make_unique<TrackedEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                     aTrackPtr->GetTargetIndex(),
                                                                     false));
                        }));

   mCallbacks.Add(
      WsfObserver::WeaponFired(&aSimulation)
         .Connect(
            [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr, const WsfTrack* aTargetTrackPtr)
            {
               AddSimEvent(ut::make_unique<AttackedEvent>(aEngagementPtr->GetFiringPlatformIndex(),
                                                          aTargetTrackPtr ? aTargetTrackPtr->GetTargetIndex() :
                                                                            aEngagementPtr->GetTargetPlatformIndex(),
                                                          true));
            }));

   mCallbacks.Add(WsfObserver::WeaponTerminated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
                        {
                           AddSimEvent(ut::make_unique<AttackedEvent>(aEngagementPtr->GetFiringPlatformIndex(),
                                                                      aEngagementPtr->GetTargetPlatformIndex(),
                                                                      false));
                        }));
}

void WkPlatformHistory::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform.GetIndex()));
}
