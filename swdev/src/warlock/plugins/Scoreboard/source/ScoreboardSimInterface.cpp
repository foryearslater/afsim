// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ScoreboardSimInterface.hpp"

#include <UtMemory.hpp>

#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"

WkScoreboard::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<ScoreboardEvent>(aPluginName)
{
}

void WkScoreboard::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
   }
}

void WkScoreboard::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mCallbacks.Clear();

   // Weapon Fired
   mCallbacks.Add(
      WsfObserver::WeaponFired(&aSimulation)
         .Connect(
            [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr, const WsfTrack* aTargetTrackPtr)
            {
               if (aEngagementPtr)
               {
                  AddSimEvent(ut::make_unique<ScoreboardUpdateEvent>(WeaponEvent(aEngagementPtr->GetFiringPlatform(),
                                                                                 aEngagementPtr->GetTargetPlatform(),
                                                                                 aEngagementPtr->GetWeaponPlatform(),
                                                                                 1,
                                                                                 aSimTime,
                                                                                 WeaponEventType::Fire)));
               }
            }));

   // Weapon Hit/Kill and Missed
   // WeaponTerminated callback is used because hit is unreliable and does not show up in the event log (same with missed)
   // Demo Example: ballistic missile shootdown, missile getting destroyed by another missile does not register as a miss or hit
   mCallbacks.Add(
      WsfObserver::WeaponTerminated(&aSimulation)
         .Connect(
            [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
            {
               if (aEngagementPtr)
               {
                  switch (aEngagementPtr->GetGeometryResult())
                  {
                  case WsfWeaponEngagement::cDUD:
                  case WsfWeaponEngagement::cFAR_AWAY_ABOVE_GROUND:
                  case WsfWeaponEngagement::cFAR_AWAY_GROUND_IMPACT:
                  case WsfWeaponEngagement::cFAR_AWAY_IN_AIR:
                  case WsfWeaponEngagement::cPART_DISABLED_OR_DESTROYED:
                  {
                     AddSimEvent(ut::make_unique<ScoreboardUpdateEvent>(WeaponEvent(aEngagementPtr->GetFiringPlatform(),
                                                                                    aEngagementPtr->GetTargetPlatform(),
                                                                                    aEngagementPtr->GetWeaponPlatform(),
                                                                                    1,
                                                                                    aSimTime,
                                                                                    WeaponEventType::Miss)));
                     break;
                  }
                  case WsfWeaponEngagement::cTARGET_IMPACT:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_ABOVE_GROUND:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_AIR_BURST:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_GROUND_IMPACT:
                  {
                     AddSimEvent(ut::make_unique<ScoreboardUpdateEvent>(WeaponEvent(aEngagementPtr->GetFiringPlatform(),
                                                                                    aEngagementPtr->GetTargetPlatform(),
                                                                                    aEngagementPtr->GetWeaponPlatform(),
                                                                                    1,
                                                                                    aSimTime,
                                                                                    WeaponEventType::Hit)));

                     if (aEngagementPtr->GetTargetPlatform() != nullptr &&
                         aEngagementPtr->GetTargetPlatform()->GetDamageFactor() == 1.0)
                     {
                        AddSimEvent(
                           ut::make_unique<ScoreboardUpdateEvent>(WeaponEvent(aEngagementPtr->GetFiringPlatform(),
                                                                              aEngagementPtr->GetTargetPlatform(),
                                                                              aEngagementPtr->GetWeaponPlatform(),
                                                                              1,
                                                                              aSimTime,
                                                                              WeaponEventType::Kill)));
                     }
                     break;
                  }
                  case WsfWeaponEngagement::cIN_PROGRESS:
                     break;
                  }
               }
            }));
   locker.unlock();
}
