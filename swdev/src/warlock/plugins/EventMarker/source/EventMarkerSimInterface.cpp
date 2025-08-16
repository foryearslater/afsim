// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "EventMarkerSimInterface.hpp"

#include "UtEllipsoidalEarth.hpp"
#include "UtTime.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"
#include "event_marker/WkfEventMarkerDisplayInterface.hpp"

WkEventMarker::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<EventMarkSimEvent>(aPluginName)
{
}

void WkEventMarker::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimCompleteEvent>());
   }
}

void WkEventMarker::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
   mCallbacks.Add(WsfObserver::PlatformBroken(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfPlatform* aPlatformPtr)
                        {
                           UtTime time; // Temporary time variable to format with user preferred format.

                           // Formatting of the tool tip string
                           std::string desc =
                              aPlatformPtr->GetName() + " damaged @ " +
                              time.ToString(aSimTime, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting());

                           AddSimEvent(ut::make_unique<PlatformMarkerEvent>(aSimTime, "DAMAGED", desc, aPlatformPtr));
                        }));

   mCallbacks.Add(WsfObserver::PlatformDeleted(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfPlatform* aPlatformPtr)
                        {
                           UtTime time; // Temporary time variable to format with user preferred format.

                           // Formatting of the tool tip string
                           std::string desc =
                              aPlatformPtr->GetName() + " removed @ " +
                              time.ToString(aSimTime, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting());

                           AddSimEvent(ut::make_unique<PlatformMarkerEvent>(aSimTime, "REMOVED", desc, aPlatformPtr));
                        }));

   mCallbacks.Add(
      WsfObserver::WeaponTerminated(&aSimulation)
         .Connect(
            [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
            {
               if (aEngagementPtr)
               {
                  UtTime time; // Temporary time variable to format with user preferred format.

                  switch (aEngagementPtr->GetGeometryResult())
                  {
                  case WsfWeaponEngagement::cDUD:
                  case WsfWeaponEngagement::cFAR_AWAY_ABOVE_GROUND:
                  case WsfWeaponEngagement::cFAR_AWAY_GROUND_IMPACT:
                  case WsfWeaponEngagement::cFAR_AWAY_IN_AIR:
                  case WsfWeaponEngagement::cPART_DISABLED_OR_DESTROYED:
                  {
                     if (aEngagementPtr->GetWeaponPlatform())
                     {
                        // Formatting of the tool tip string
                        std::string desc = aEngagementPtr->GetWeaponPlatform()->GetName() + " misses ";
                        if (aEngagementPtr->GetTargetPlatform())
                        {
                           desc += aEngagementPtr->GetTargetPlatform()->GetName() + " @ " +
                                   time.ToString(aSimTime,
                                                 wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting());
                        }

                        AddSimEvent(ut::make_unique<WeaponMarkerEvent>(aSimTime, "WEAPON_MISSED", desc, aEngagementPtr));
                     }
                     break;
                  }
                  case WsfWeaponEngagement::cTARGET_IMPACT:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_ABOVE_GROUND:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_AIR_BURST:
                  case WsfWeaponEngagement::cTARGET_PROXIMITY_GROUND_IMPACT:
                  {
                     if (aEngagementPtr->GetWeaponPlatform())
                     {
                        // Formatting of the tool tip string
                        std::string desc = aEngagementPtr->GetWeaponPlatform()->GetName() + " hits ";
                        if (aEngagementPtr->GetTargetPlatform())
                        {
                           desc += aEngagementPtr->GetTargetPlatform()->GetName() + " @ " +
                                   time.ToString(aSimTime,
                                                 wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting());
                        }

                        AddSimEvent(ut::make_unique<WeaponMarkerEvent>(aSimTime, "WEAPON_HIT", desc, aEngagementPtr));
                     }
                     break;
                  }
                  case WsfWeaponEngagement::cIN_PROGRESS:
                     break;
                  }
               }
            }));
}
