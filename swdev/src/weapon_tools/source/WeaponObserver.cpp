// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WeaponObserver.hpp"

#include <cassert>
#include <cmath>

#include "Tool.hpp"
#include "UtCallback.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfBallisticMissileLaunchComputer.hpp"
#include "WsfGuidedMoverBase.hpp"
#include "WsfMover.hpp"
#include "WsfMoverObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationObserver.hpp"
#include "WsfStringId.hpp"
#include "WsfWeaponEffects.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponFuse.hpp"
#include "WsfWeaponObserver.hpp"

WeaponObserver::WeaponObserver()
   : mSimulationPtr(nullptr)
   , mCallbacks()
   , mAllDone(false)
   , mDisableGroundFusing(false)
   , mState(cIDLE)
   , mWeaponTimedOut(false)
   , mWeaponPlatformIndex(0)
   , mLaunchTime(0.0)
   , mImpactTime(0.0)
   , mTimeOfFlight(0.0)
   , mResult(WsfWeaponEffects::cINVALID)
   , mLaunchLat(0.0)
   , mLaunchLon(0.0)
   , mLaunchAlt(0.0)
   , mLaunchHdg(0.0)
   , mImpactLat(0.0)
   , mImpactLon(0.0)
   , mImpactAlt(0.0)
   , mBO_TimeOfFlight(0.0)
   , mBO_GreatCircleRange(0.0)
   , mBO_Altitude(0.0)
   , mBO_Speed(0.0)
   , mBO_FlightPathAngle(0.0)
   , mFinalSpeed(0.0)
   , mFinalFlightPathAngle(0.0)
   , mToolPtr(nullptr)
{
}

WeaponObserver::~WeaponObserver() {}

void WeaponObserver::Initialize(WsfSimulation& aSimulation)
{
   mSimulationPtr = &aSimulation;
   mCallbacks.Add(WsfObserver::MoverBurnedOut(&aSimulation).Connect(&WeaponObserver::MoverBurnedOut, this));
   mCallbacks.Add(
      WsfObserver::GuidanceComputerPhaseChanged(&aSimulation).Connect(&WeaponObserver::GuidanceComputerPhaseChanged, this));
   mCallbacks.Add(WsfObserver::PlatformAdded(&aSimulation).Connect(&WeaponObserver::PlatformAdded, this));
   mCallbacks.Add(WsfObserver::PlatformDeleted(&aSimulation).Connect(&WeaponObserver::PlatformDeleted, this));
   mCallbacks.Add(WsfObserver::WeaponFired(&aSimulation).Connect(&WeaponObserver::WeaponFired, this));
   mCallbacks.Add(WsfObserver::WeaponTerminated(&aSimulation).Connect(&WeaponObserver::WeaponTerminated, this));
}

//! Specify what weapon Tool is to interact with this object.  The purpose for this method is to
//! allow the Observer objects to register for such methods as PlatformAdded(), and callbacks to
//! be made back into virtual methods for the Tool class object methods of the same name.
void WeaponObserver::SetToolPtr(Tool* aToolPtr)
{
   mToolPtr = aToolPtr;
   assert(mToolPtr != nullptr);
}

//! Simulation observer.
void WeaponObserver::MoverBurnedOut(double aSimTime, WsfMover* aMoverPtr)
{
   // We received a callback indicating SOMEWHERE, a mover burned out.  Was
   // it one we care about?   Do a check here, and ignore if not concerned.

   if ((mState == cIN_FLIGHT) && (aMoverPtr->GetPlatform()->GetIndex() == mWeaponPlatformIndex))
   {
      // This was our own weapon platform.
      // (Note here we are using spherical earth.)

      double velWCS[3];
      aMoverPtr->GetPlatform()->GetVelocityWCS(velWCS);
      mBO_Speed = UtVec3d::Magnitude(velWCS);
      double ellipsoidalLaunchLocWCS[3];
      UtEntity::ConvertLLAToWCS(mLaunchLat, mLaunchLon, mLaunchAlt, ellipsoidalLaunchLocWCS);
      double unitSphericalLaunchLocWCS[3];
      UtEllipsoidalEarth::ConvertEllipsoidalToSpherical(ellipsoidalLaunchLocWCS, unitSphericalLaunchLocWCS);
      UtVec3d::Normalize(unitSphericalLaunchLocWCS);
      double ellipsoidalLocWCS[3];
      aMoverPtr->GetPlatform()->GetLocationWCS(ellipsoidalLocWCS);
      double sphericalLocWCS[3];
      UtEllipsoidalEarth::ConvertEllipsoidalToSpherical(ellipsoidalLocWCS, sphericalLocWCS);
      double radius = UtVec3d::Magnitude(sphericalLocWCS);
      mBO_Altitude  = radius - UtSphericalEarth::cEARTH_RADIUS;
      double unitSphericalLocWCS[3];
      UtVec3d::Set(unitSphericalLocWCS, sphericalLocWCS);
      UtVec3d::Normalize(unitSphericalLocWCS);
      mBO_TimeOfFlight     = aSimTime - mLaunchTime;
      double dot           = UtVec3d::DotProduct(unitSphericalLocWCS, unitSphericalLaunchLocWCS);
      double sectorAngle   = acos(dot);
      mBO_GreatCircleRange = sectorAngle * UtSphericalEarth::cEARTH_RADIUS;

      // The flight path angle (spherical sense) is the arccos of linear velocity dot displacement.
      double cosine = UtVec3d::DotProduct(velWCS, sphericalLocWCS) / (radius * mBO_Speed);
      if (cosine > 1.0)
      {
         cosine = 1.0;
      }
      if (cosine < -1.0)
      {
         cosine = -1.0;
      }
      double angle        = acos(cosine);
      mBO_FlightPathAngle = UtMath::cPI_OVER_2 - angle;

      if (mToolPtr != nullptr)
      {
         mToolPtr->MoverBurnout(aSimTime, aMoverPtr);
      }
   }
}

//! Simulation observer.
void WeaponObserver::GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr)
{
   if (mToolPtr != nullptr)
   {
      mToolPtr->GuidanceComputerPhaseChanged(aSimTime, aComputerPtr);
   }
}

//! Simulation observer.
void WeaponObserver::PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr)
{
   if (mToolPtr != nullptr)
   {
      mToolPtr->PlatformAdded(aSimTime, aPlatformPtr);
   }
}

//! Simulation observer.
void WeaponObserver::PlatformDeleted(double aSimTime, WsfPlatform* aPlatformPtr)
{
   // Catch the case where the thing that was fired whas not actually a weapon (i.e.: did not have an associated
   // weapon_effects definition), but it did have a fuse to catch crashing into the ground. (typically when generating a
   // space launch computer).
   if ((mState == cIN_FLIGHT) && (aPlatformPtr->GetIndex() == mWeaponPlatformIndex))
   {
      mState                = cTERMINATED;
      mImpactTime           = aSimTime;
      mTimeOfFlight         = aSimTime - mLaunchTime;
      mImpactLat            = 0.0;
      mImpactLon            = 0.0;
      mImpactAlt            = 0.0;
      mFinalSpeed           = 0.0;
      mFinalFlightPathAngle = 0.0;
      aPlatformPtr->GetLocationLLA(mImpactLat, mImpactLon, mImpactAlt);
      double velNED[3];
      aPlatformPtr->GetVelocityNED(velNED);
      mFinalSpeed           = UtVec3d::Magnitude(velNED);
      mFinalFlightPathAngle = atan2(-velNED[2], sqrt((velNED[0] * velNED[0]) + (velNED[1] * velNED[1])));
   }
}

//! Simulation observer
void WeaponObserver::WeaponFired(double                     aSimTime,
                                 const WsfWeaponEngagement* aEngagementPtr,
                                 const WsfTrack* /* aTargetTrackPtr */)
{
   if ((mToolPtr == nullptr) || (aEngagementPtr == nullptr))
   {
      return;
   }

   // Ignore platforms that did not directly originate from the launching platform (such as spent stages)
   if (aEngagementPtr->GetFiringPlatform() != mToolPtr->GetLauncherPlatform())
   {
      return;
   }

   assert((mState == cIDLE) || (mState == cTERMINATED));

   mState          = cIN_FLIGHT;
   mLaunchTime     = aSimTime;
   mImpactTime     = 0.0;
   mTimeOfFlight   = 0.0;
   mResult         = WsfWeaponEffects::cINVALID;
   mWeaponTimedOut = false;

   mLaunchLat = 0.0;
   mLaunchLon = 0.0;
   mLaunchAlt = 0.0;
   mLaunchHdg = 0.0;

   mWeaponPlatformIndex = aEngagementPtr->GetWeaponPlatformIndex();
   WsfPlatform* pPtr    = mSimulationPtr->GetPlatformByIndex(mWeaponPlatformIndex);
   if (pPtr != nullptr)
   {
      pPtr->GetLocationLLA(mLaunchLat, mLaunchLon, mLaunchAlt);
      double notUsed1, notUsed2;
      pPtr->GetOrientationNED(mLaunchHdg, notUsed1, notUsed2);

      if (mDisableGroundFusing)
      {
         // Find the first processor of type WsfWeaponFuse
         for (WsfComponentList::RoleIterator<WsfProcessor> iter(*pPtr); !iter.AtEnd(); ++iter)
         {
            auto* fusePtr = dynamic_cast<WsfWeaponFuse*>(*iter);
            if (fusePtr != nullptr)
            {
               // For Air-to-Air, shut off the ground hit near sea-level.
               fusePtr->SetAGL_Criteria(WsfWeaponFuse::cNO_LIMIT, 0.0);
               fusePtr->SetMSL_Criteria(WsfWeaponFuse::cNO_LIMIT, 0.0);
               break;
            }
         }
      }
   }
}

//! Simulation observer
void WeaponObserver::WeaponTerminated(double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
{
   if ((mToolPtr == nullptr) || (aEngagementPtr == nullptr))
   {
      return;
   }

   // Ignore platforms that did not directly originate from the launching platform (such as spent stages)
   if (aEngagementPtr->GetFiringPlatform() != mToolPtr->GetLauncherPlatform())
   {
      return;
   }

   assert(mState == cIN_FLIGHT);

   mState        = cTERMINATED;
   mImpactTime   = aSimTime;
   mTimeOfFlight = aEngagementPtr->GetCompletionTime() - mLaunchTime;
   mResult       = aEngagementPtr->GetTargetResult();

   mImpactLat            = 0.0;
   mImpactLon            = 0.0;
   mImpactAlt            = 0.0;
   mFinalSpeed           = 0.0;
   mFinalFlightPathAngle = 0.0;

   WsfPlatform* pPtr = mSimulationPtr->GetPlatformByIndex(aEngagementPtr->GetWeaponPlatformIndex());
   if (pPtr != nullptr)
   {
      pPtr->GetLocationLLA(mImpactLat, mImpactLon, mImpactAlt);
      double velNED[3];
      pPtr->GetVelocityNED(velNED);
      mFinalSpeed           = UtVec3d::Magnitude(velNED);
      mFinalFlightPathAngle = atan2(-velNED[2], sqrt((velNED[0] * velNED[0]) + (velNED[1] * velNED[1])));

      // Find the first processor of type WsfWeaponFuse
      for (WsfComponentList::RoleIterator<WsfProcessor> iter(*pPtr); !iter.AtEnd(); ++iter)
      {
         auto* fusePtr = dynamic_cast<WsfWeaponFuse*>(*iter);
         if (fusePtr != nullptr)
         {
            mWeaponTimedOut = fusePtr->GetReason() == WsfWeaponFuse::cTOF_LIMIT;
            break;
         }
      }
   }
}

//! The current launch was aborted (the weapon was never fired).
//! @param aSimTime The current simulation time.
void WeaponObserver::LaunchAborted(double aSimTime)
{
   mState        = cTERMINATED;
   mImpactTime   = aSimTime;
   mTimeOfFlight = 0.0;
   mResult       = WsfWeaponEffects::cINVALID;
}

//! Externally reset the Observer state to Idle.
void WeaponObserver::ResetState()
{
   mState  = cIDLE;
   mResult = WsfWeaponEffects::cINVALID;
}

//! Externally reset the Observer state to Inflight.
//! Normally this is done when the WeaponFired event is detected, but no such event occurs when the thing fired is
//! not a weapon (does not have a weapon_effect). (See SpaceLaunchComputerGenerator::PlatformAdded).
void WeaponObserver::SetInFlight(double aSimTime, WsfPlatform* aPlatformPtr)
{
   assert((mState == cIDLE) || (mState == cTERMINATED));

   mState               = cIN_FLIGHT;
   mImpactTime          = 0.0;
   mTimeOfFlight        = 0.0;
   mResult              = WsfWeaponEffects::cINVALID;
   mWeaponTimedOut      = false;
   mWeaponPlatformIndex = aPlatformPtr->GetIndex();
   mLaunchTime          = aSimTime;
   aPlatformPtr->GetLocationLLA(mLaunchLat, mLaunchLon, mLaunchAlt);
   double notUsed1, notUsed2;
   aPlatformPtr->GetOrientationNED(mLaunchHdg, notUsed1, notUsed2);
}

//! Return the pointer to the weapon platform.
//! @note This may be zero.
WsfPlatform* WeaponObserver::GetWeaponPlatform() const
{
   return mSimulationPtr->GetPlatformByIndex(mWeaponPlatformIndex);
}

//! Calculates the horizontal range traversed by an explicit weapon during its flyout, from launch to impact.
//! (This does not guarantee that the impact was a target kill.)
double WeaponObserver::HorizontalRange() const
{
   // Set default return value
   double result = 0.0;

   // Determine if the launch and impact points are equal
   bool latitudesEqual  = (fabs(mLaunchLat - mImpactLat) < std::numeric_limits<double>::epsilon());
   bool longitudesEqual = (fabs(mLaunchLon - mImpactLon) < std::numeric_limits<double>::epsilon());

   // The points differ so perform the horizontal range calculation
   if (!latitudesEqual || !longitudesEqual)
   {
      // Set up the launcher location and orientation
      UtEntity launchLoc;
      launchLoc.SetLocationLLA(mLaunchLat, mLaunchLon, mLaunchAlt);
      launchLoc.SetOrientationNED(mLaunchHdg, 0.0, 0.0);

      // Convert the impact point to the WCS frame
      double impactWCS[3];
      UtEntity::ConvertLLAToWCS(mImpactLat, mImpactLon, mImpactAlt, impactWCS);

      // Find the impact point in the launchers NED frame
      double nedDownRange[3];
      launchLoc.ConvertWCSToNED(impactWCS, nedDownRange);

      // Calculate the down range from the launcher
      result = sqrt(nedDownRange[0] * nedDownRange[0] + nedDownRange[1] * nedDownRange[1]);
   }
   return result;
}

//! Calculates the (downrange, crossrange, vertical offset) of the impact location of a weapon,
//! relative to the wings-level launch heading of the firing entity.
//! (This does not guarantee that the impact was a target kill.)
//!   @param aOffset The relative offset of the impact, (forward, right, down) from the launch position.
void WeaponObserver::GetImpactRelToLaunch(double aOffset[3]) const
{
   // Set default return value
   aOffset[0] = 0.0;
   aOffset[1] = 0.0;
   aOffset[2] = 0.0;

   // Determine if the launch and impact points are equal
   bool latitudesEqual  = (fabs(mLaunchLat - mImpactLat) < std::numeric_limits<double>::epsilon());
   bool longitudesEqual = (fabs(mLaunchLon - mImpactLon) < std::numeric_limits<double>::epsilon());

   // The points differ so perform the horizontal range calculation
   if (!latitudesEqual || !longitudesEqual)
   {
      // Set up the launcher location and orientation
      UtEntity launchLoc;
      launchLoc.SetLocationLLA(mLaunchLat, mLaunchLon, mLaunchAlt);
      launchLoc.SetOrientationNED(mLaunchHdg, 0.0, 0.0);

      // Convert the impact point to the WCS frame
      double impactWCS[3];
      UtEntity::ConvertLLAToWCS(mImpactLat, mImpactLon, mImpactAlt, impactWCS);

      // Find the impact point in the launchers NED frame
      double nedDownRange[3];
      launchLoc.ConvertWCSToNED(impactWCS, nedDownRange);

      // Calculate the offset from the launcher
      aOffset[0] = cos(mLaunchHdg) * nedDownRange[0] + sin(mLaunchHdg) * nedDownRange[1];
      aOffset[1] = -sin(mLaunchHdg) * nedDownRange[0] + cos(mLaunchHdg) * nedDownRange[1];
      aOffset[2] = nedDownRange[2];
   }
}

//! Accessor to get burnout state information.
//! @param aTimeOfFlight Time of Flight at burnout
//! @param aRange Great Circle range at burnout
//! @param aAltitude Altitude at burnout
//! @param aSpeed Speed at burnout
//! @param aFlightPathAngle Flight path Angle (radians) at burnout
void WeaponObserver::GetBurnOutInfo(double& aTimeOfFlight, double& aRange, double& aAltitude, double& aSpeed, double& aFlightPathAngle)
{
   aTimeOfFlight    = mBO_TimeOfFlight;
   aRange           = mBO_GreatCircleRange;
   aAltitude        = mBO_Altitude;
   aSpeed           = mBO_Speed;
   aFlightPathAngle = mBO_FlightPathAngle;
}
