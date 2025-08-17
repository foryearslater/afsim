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

#ifndef WEAPONOBSERVER_HPP
#define WEAPONOBSERVER_HPP

#include "WeaponToolsExport.hpp"

class Tool;
#include "UtCallbackHolder.hpp"
class WsfMover;
class WsfGuidanceComputer;
class WsfTrack;
#include "WsfWeaponEffects.hpp"
class WsfWeaponEngagement;

//! WeaponObserver is an observation utility to capture data on a weapon engagement.
//! The instance will return information upon a weapon in flight, and after the weapon
//! terminates, will allow queries on time of flight, hit or miss indication, etc.

class WT_EXPORT WeaponObserver
{
public:
   WeaponObserver();
   virtual ~WeaponObserver();

   void Initialize(WsfSimulation& aSimulation);

   //! Enumeration indicating the state of the engagement, usually so that concerned
   //! parties can take action only after the engagement is terminated.
   enum EngagementState
   {
      cIDLE,      //!< A weapon is not currently in flight.
      cIN_FLIGHT, //!< A weapon is in flight.
      cTERMINATED //!< A previous weapon has terminated, and data may be extracted on the
                  //!< result of the engagement.
   };

   //! Returns a flag to indicate that the intended target was killed by the weapon.
   bool EngagementLethal() const { return mState == cTERMINATED && mResult == WsfWeaponEffects::cKILLED; }

   //! Returns a flag to indicate that the weapon timed out with no lethal terminal effect.
   bool WeaponTimedOut() const { return mWeaponTimedOut; }

   //! Returns the time of flight of the weapon, in seconds.
   double TimeOfFlight() const { return mTimeOfFlight; }

   //! @name Simulation observer callbacks
   //@{

   virtual void MoverBurnedOut(double aSimTime, WsfMover* aMoverPtr);

   virtual void GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr);

   virtual void PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr);

   virtual void PlatformDeleted(double aSimTime, WsfPlatform* aPlatformPtr);

   virtual void WeaponFired(double aSimTime, const WsfWeaponEngagement* aEngagementPtr, const WsfTrack* aTargetTrackPtr);

   virtual void WeaponTerminated(double aSimTime, const WsfWeaponEngagement* aEngagementPtr);

   //@}

   void LaunchAborted(double aSimTime);

   //! Returns the state of the most recent engagement, which still may be in progress.
   EngagementState GetState() const { return mState; }

   //! Externally reset the Observer state to Idle.
   void ResetState();

   //! Externally mark the state as IN_FLIGHT
   void SetInFlight(double aSimTime, WsfPlatform* aPlatformPtr);

   //! Returns a flag to indicate an externally-provided "All Done" status.
   bool AllDone() const { return mAllDone; }

   //! Externally sets the "All Done" status.
   void SetAllDone() { mAllDone = true; }

   //! Disable Ground Fusing (ignore MSL and AGL collisions).
   void DisableGroundFusing() { mDisableGroundFusing = true; }

   //! Returns horizontal distance from launch point to impact point.
   double HorizontalRange() const;

   //! Returns (Forward, Right, Down) offset of impact relative to launch position and attitude.
   void GetImpactRelToLaunch(double aOffset[3]) const;

   //! Returns the latitude, longitude and altitude of the launch platform at time of launch.
   void GetLaunchLocLLA(double& aLaunchLat, double& aLaunchLon, double& aLaunchAlt) const
   {
      aLaunchLat = mLaunchLat;
      aLaunchLon = mLaunchLon;
      aLaunchAlt = mLaunchAlt;
   }

   //! Returns the latitude, longitude and altitude of the weapon at termination.
   void GetImpactLocLLA(double& aImpactLat, double& aImpactLon, double& aImpactAlt) const
   {
      aImpactLat = mImpactLat;
      aImpactLon = mImpactLon;
      aImpactAlt = mImpactAlt;
   }

   //! Returns the final of the weapon at termination
   double GetFinalSpeed() const { return mFinalSpeed; }

   //! Returns the final flight path angle of the weapon at termination
   double GetFinalFlightPathAngle() const { return mFinalFlightPathAngle; }

   //! Return the time of launch.
   double GetLaunchTime() const { return mLaunchTime; }

   WsfPlatform* GetWeaponPlatform() const;

   void SetToolPtr(Tool* aToolPtr);

   void GetBurnOutInfo(double& aTimeOfFlight, double& aRange, double& aAltitude, double& aSpeed, double& aFlightPathAngle);

private:
   WsfSimulation*           mSimulationPtr;
   UtCallbackHolder         mCallbacks;
   bool                     mAllDone;
   bool                     mDisableGroundFusing;
   EngagementState          mState;
   bool                     mWeaponTimedOut;
   size_t                   mWeaponPlatformIndex;
   double                   mLaunchTime;
   double                   mImpactTime;
   double                   mTimeOfFlight;
   WsfWeaponEffects::Result mResult;

   double mLaunchLat;
   double mLaunchLon;
   double mLaunchAlt;
   double mLaunchHdg;
   double mImpactLat;
   double mImpactLon;
   double mImpactAlt;

   double mBO_TimeOfFlight;
   double mBO_GreatCircleRange;
   double mBO_Altitude;
   double mBO_Speed;
   double mBO_FlightPathAngle;

   double mFinalSpeed;
   double mFinalFlightPathAngle;
   Tool*  mToolPtr;
};

#endif
