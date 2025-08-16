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

#include "EventMarkerSimEvents.hpp"

#include <sstream>

#include "UtEllipsoidalEarth.hpp"
#include "WkfScenario.hpp"
#include "WsfPlatform.hpp"

void SimCompleteEvent::Process(wkf::EventMarkerDisplayInterface* aDisplayInterface,
                               const wkf::EventMarkerPrefObject* aPrefObject)
{
   aDisplayInterface->DeleteEventMarkers();
}

MarkerEvent::MarkerEvent(double aTime, const std::string& aType, std::string aDescription, const WsfPlatform* aReferencePlatform)
   : EventMarkSimEvent()
   , mTime(aTime)
   , mType(aType)
   , mSummary(std::move(aDescription))
{
   if (aReferencePlatform != nullptr)
   {
      double xyz[3];
      aReferencePlatform->GetLocationWCS(xyz);
      double lla[3] = {0.0, 0.0, 0.0};
      UtEllipsoidalEarth::ConvertECEFToLLA(xyz, lla[0], lla[1], lla[2]);
      mPosition = vespa::VaPosition(lla[0], lla[1], lla[2], xyz[0], xyz[1], xyz[2]);
   }
}

PlatformMarkerEvent::PlatformMarkerEvent(double             aTime,
                                         const std::string& aType,
                                         std::string        aDescription,
                                         const WsfPlatform* aReferencePlatform)
   : MarkerEvent(aTime, aType, std::move(aDescription), aReferencePlatform)
{
   if (aReferencePlatform != nullptr)
   {
      mInfo.mPlatform = aReferencePlatform->GetName();
   }
}


void PlatformMarkerEvent::Process(wkf::EventMarkerDisplayInterface* aDisplayInterface,
                                  const wkf::EventMarkerPrefObject* aPrefObject)
{
   if (aPrefObject->GetShowEvent(mType))
   {
      aDisplayInterface->AddMarker(wkf::make_entity<wkf::PlatformEventMarker>(mTime, mType, mInfo), mPosition);
   }
}

WeaponMarkerEvent::WeaponMarkerEvent(double                     aTime,
                                     const std::string&         aType,
                                     std::string                aDescription,
                                     const WsfWeaponEngagement* aEngagement)
   : MarkerEvent(aTime, aType, std::move(aDescription), (aEngagement ? aEngagement->GetWeaponPlatform() : nullptr))
{
   if (aEngagement != nullptr)
   {
      WsfPlatform* firing = aEngagement->GetFiringPlatform();
      if (firing != nullptr)
      {
         mInfo.mFiringName = firing->GetName();
      }

      WsfPlatform* weapon = aEngagement->GetWeaponPlatform();
      if (weapon != nullptr)
      {
         mInfo.mWeaponName  = weapon->GetName();
         mInfo.mWeaponSpeed = weapon->GetSpeed();
      }

      WsfPlatform* target = aEngagement->GetTargetPlatform();
      if (target != nullptr)
      {
         mInfo.mTargetName  = target->GetName();
         mInfo.mTargetSpeed = target->GetSpeed();
      }

      WsfWeaponEffects* effects = aEngagement->GetWeaponEffects();
      if (effects != nullptr)
      {
         mInfo.mInterceptPk = effects->GetInterceptPk();
         mInfo.mPkDrawn     = effects->GetPkDrawn();
      }

      mInfo.mMissDistance = aEngagement->GetMissDistance(nullptr);

      std::string missDistance;
      if (WsfWeaponEngagement::EnumToString(missDistance, aEngagement->GetGeometryResult()))
      {
         mInfo.mResult = missDistance;
      }
      else
      {
         mInfo.mResult = "Unknown";
      }
   }
}

void WeaponMarkerEvent::Process(wkf::EventMarkerDisplayInterface* aDisplayInterface,
                                const wkf::EventMarkerPrefObject* aPrefObject)
{
   if (aPrefObject->GetShowEvent(mType))
   {
      aDisplayInterface->AddMarker(wkf::make_entity<wkf::WeaponEventMarker>(mTime, mType, mInfo), mPosition);
   }
}
