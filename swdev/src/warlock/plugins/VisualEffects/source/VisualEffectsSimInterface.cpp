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

#include "VisualEffectsSimInterface.hpp"

#include "UtEllipsoidalEarth.hpp"
#include "UtMemory.hpp"
#include "VaScenario.hpp"
#include "VisualEffectsSimEvents.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationObserver.hpp"
#include "WsfTypes.hpp"
#include "WsfUtil.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"
#include "visual_effect/WkfVisualEffects.hpp"

WkVisualEffects::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<VisualEffectsSimEvent>(aPluginName)
{
}

void WkVisualEffects::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);
   mAppearanceMap.clear();
   mCallbacks.Clear();

   // Whenever a platform's appearance changes, determine what (if any) visual effects to display/remove.
   mCallbacks.Add(
      WsfObserver::PlatformAppearanceChanged(&aSimulation)
         .Connect(
            [&](double aSimTime, WsfPlatform* aPlatformPtr, WsfObserver::AppearanceType aType)
            {
               if (aPlatformPtr != nullptr)
               {
                  std::string         platformName  = aPlatformPtr->GetName();
                  DisEntityAppearance newAppearance = static_cast<DisEntityAppearance>(aPlatformPtr->GetAppearance());

                  double xyz[3] = {0, 0, 0};
                  aPlatformPtr->GetLocationWCS(xyz);
                  double lla[3] = {0.0, 0.0, 0.0};
                  UtEllipsoidalEarth::ConvertECEFToLLA(xyz, lla[0], lla[1], lla[2]);
                  vespa::VaPosition pos(lla[0], lla[1], lla[2], xyz[0], xyz[1], xyz[2]);

                  EffectInfo effectInfo;
                  effectInfo.mPosition = pos;
                  std::string desc;

                  // Trailing effects
                  unsigned int trailingEffects = newAppearance.GetTrailingEffects();
                  if (mAppearanceMap[platformName].GetTrailingEffects() != trailingEffects)
                  {
                     switch (aPlatformPtr->GetSpatialDomain())
                     {
                     case WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR:
                     {
                        switch (mAppearanceMap[platformName].GetTrailingEffects())
                        {
                        case 1:
                        {
                           AddSimEvent(ut::make_unique<RemoveTrailingEffectEvent<wkf::ContrailEffect>>(platformName, true));
                           break;
                        }
                        case 2:
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::RocketSmokeEffect>>(platformName, true));
                           break;
                        }
                        case 3:
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::DamageSmokeEffect>>(platformName, true));
                           break;
                        }
                        }

                        switch (trailingEffects)
                        {
                        case 1:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::ContrailEffect>>(effectInfo, desc));
                           break;
                        }
                        case 2:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::RocketSmokeEffect>>(effectInfo, desc));
                           break;
                        }
                        case 3:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::DamageSmokeEffect>>(effectInfo, desc));
                           break;
                        }
                        case 0:
                        default:
                        {
                           break;
                        }
                        }
                        break;
                     }
                     default:
                     {
                        break;
                     }
                     }
                  }

                  // Smoke effects
                  unsigned int smokeEffects = newAppearance.GetSmoke();
                  if (mAppearanceMap[platformName].GetSmoke() != smokeEffects)
                  {
                     switch (aPlatformPtr->GetSpatialDomain())
                     {
                     case WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR:
                     {
                        switch (mAppearanceMap[platformName].GetSmoke())
                        {
                        case 1:
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::SmokePlumeEffect>>(platformName, true));
                           break;
                        }
                        case 2:
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::EngineSmokeEffect>>(platformName, true));
                           break;
                        }
                        case 3:
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::SmokePlumeEffect>>(platformName, true));
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::EngineSmokeEffect>>(platformName, true));
                           break;
                        }
                        }

                        switch (smokeEffects)
                        {
                        case 1:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::SmokePlumeEffect>>(effectInfo, desc));
                           break;
                        }
                        case 2:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::EngineSmokeEffect>>(effectInfo, desc));
                           break;
                        }
                        case 3:
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::SmokePlumeEffect>>(effectInfo, desc));
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::EngineSmokeEffect>>(effectInfo, desc));
                           break;
                        }
                        case 0:
                        default:
                        {
                           break;
                        }
                        }
                     }
                     default:
                     {
                        break;
                     }
                     }
                  }

                  // Fire effects
                  unsigned int fireEffect = newAppearance.GetFlaming();
                  if (mAppearanceMap[platformName].GetFlaming() != fireEffect)
                  {
                     switch (aPlatformPtr->GetSpatialDomain())
                     {
                     case WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR:
                     {
                        if (fireEffect == 1)
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::FireEffect>>(effectInfo, desc));
                        }
                        else
                        {
                           AddSimEvent(ut::make_unique<RemoveTrailingEffectEvent<wkf::FireEffect>>(platformName, true));
                        }
                     }
                     default:
                     {
                        break;
                     }
                     }
                  }

                  unsigned int afterburner = newAppearance.GetAfterburner();
                  if (mAppearanceMap[platformName].GetAfterburner() != afterburner)
                  {
                     switch (aPlatformPtr->GetSpatialDomain())
                     {
                     case WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR:
                     {
                        if (afterburner == 1)
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::AfterburnerEffect>>(effectInfo, desc));
                        }
                        else
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::AfterburnerEffect>>(platformName, true));
                        }
                     }
                     default:
                     {
                        break;
                     }
                     }
                  }

                  // TODO: Launch flash cannot be properly implemented until WsfEntityKind exists.

                  unsigned int engineOperatingEffect = newAppearance.GetPowerplant();
                  if (mAppearanceMap[platformName].GetPowerplant() != engineOperatingEffect)
                  {
                     switch (aPlatformPtr->GetSpatialDomain())
                     {
                     case WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR:
                     {
                        if (engineOperatingEffect == 1)
                        {
                           effectInfo.mSourcePlatformName = platformName;
                           AddSimEvent(ut::make_unique<EffectEvent<wkf::EngineOperatingEffect>>(effectInfo, desc));
                        }
                        else
                        {
                           AddSimEvent(
                              ut::make_unique<RemoveTrailingEffectEvent<wkf::EngineOperatingEffect>>(platformName, true));
                        }
                     }
                     default:
                     {
                        break;
                     }
                     }
                  }

                  mAppearanceMap[platformName] = newAppearance;
               }
            }));

   // Whenever a weapon detonates, it creates an explosion regardless of whether it connected with its target.
   mCallbacks.Add(
      WsfObserver::WeaponTerminated(&aSimulation)
         .Connect(
            [&](double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
            {
               if (aEngagementPtr != nullptr)
               {
                  std::string desc   = "";
                  double      xyz[3] = {0, 0, 0};

                  // If engagement resulted in an explosion, attempt to find explosion position and generate description.
                  if (aEngagementPtr->GetGeometryResult() != WsfWeaponEngagement::cIN_PROGRESS &&
                      aEngagementPtr->GetGeometryResult() != WsfWeaponEngagement::cDUD &&
                      aEngagementPtr->GetGeometryResult() != WsfWeaponEngagement::cPART_DISABLED_OR_DESTROYED)
                  {
                     // Any result can draw explosion at weapon.
                     if (aEngagementPtr->GetWeaponPlatform())
                     {
                        aEngagementPtr->GetWeaponLocationWCS(aEngagementPtr->GetWeaponPlatform(), xyz);
                        desc = std::to_string(aSimTime) + ": " + aEngagementPtr->GetWeaponPlatform()->GetName() +
                               " detonated";
                        if (aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cFAR_AWAY_IN_AIR ||
                            aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cFAR_AWAY_ABOVE_GROUND ||
                            aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cFAR_AWAY_GROUND_IMPACT)
                        {
                           desc += " and missed";
                        }
                        else if (aEngagementPtr->GetTargetPlatform())
                        {
                           desc += " and hit " + aEngagementPtr->GetTargetPlatform()->GetName();
                        }
                     }
                     // Close explosions can be drawn at target if no weapon platform.
                     else if (aEngagementPtr->GetTargetPlatform() &&
                              ((aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cTARGET_IMPACT) ||
                               (aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cTARGET_PROXIMITY_ABOVE_GROUND) ||
                               (aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cTARGET_PROXIMITY_AIR_BURST) ||
                               (aEngagementPtr->GetGeometryResult() == WsfWeaponEngagement::cTARGET_PROXIMITY_GROUND_IMPACT)))
                     {
                        aEngagementPtr->GetTargetLocationWCS(aEngagementPtr->GetTargetPlatform(), xyz);
                        desc = std::to_string(aSimTime) + ": " + aEngagementPtr->GetTargetPlatform()->GetName() +
                               " was hit by a weapon";
                     }
                  }

                  if (desc != "")
                  {
                     double lla[3] = {0.0, 0.0, 0.0};
                     UtEllipsoidalEarth::ConvertECEFToLLA(xyz, lla[0], lla[1], lla[2]);
                     vespa::VaPosition pos(lla[0], lla[1], lla[2], xyz[0], xyz[1], xyz[2]);

                     EffectInfo effectInfo;
                     effectInfo.mPosition = pos;

                     AddSimEvent(ut::make_unique<EffectEvent<wkf::ExplosionEffect>>(effectInfo, desc));
                     AddSimEvent(ut::make_unique<EffectEvent<wkf::ExplosionDebrisEffect>>(effectInfo, desc));
                     AddSimEvent(ut::make_unique<EffectEvent<wkf::IconifiedExplosionEffect>>(effectInfo, desc));
                  }
               }
            }));

   mCallbacks.Add(
      WsfObserver::PlatformInitialized(&aSimulation)
         .Connect(
            [&](double aTime, WsfPlatform* aPlatformPtr)
            { mAppearanceMap.emplace(aPlatformPtr->GetName(), static_cast<DisUint32>(aPlatformPtr->GetAppearance())); }));

   // The simulation was paused, so the effects should stop playing.
   mCallbacks.Add(WsfObserver::SimulationPausing(&aSimulation)
                     .Connect([&]() { AddSimEvent(ut::make_unique<EffectPauseOrResumeEvent>(true)); }));

   // The simulation resumed, so the effects should also resume.
   mCallbacks.Add(WsfObserver::SimulationResuming(&aSimulation)
                     .Connect([&]() { AddSimEvent(ut::make_unique<EffectPauseOrResumeEvent>(false)); }));
}
