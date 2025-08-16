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

#include "Route.hpp"

#include <vector>

#include <QTimer>

#include "Environment.hpp"
#include "ParseResults.hpp"
#include "Platform.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "UtEarth.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "VtkInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPProxyBasicTypes.hpp"
#include "WsfPProxyBasicValues.hpp"
#include "WsfPProxyValue.hpp"
#include "WsfPathComputer.hpp"
#include "orbit/WkfAttachmentOrbit.hpp"
#include "orbit/WkfOrbitPrefObject.hpp"
#include "route/WkfAttachmentRoute.hpp"

MapRoute::Route::Route(wizard::Platform* aPlatformPtr)
   : mPlatformPtr(aPlatformPtr)
   , mAttachmentPtr(nullptr)
   , mSimTime(-1)
   , mDirty(true)
   , mOrbit(ut::EarthWGS84())
{
   mTerrainQuery.SetPlatformName(QString::fromStdString(mPlatformPtr->GetName()));
   ResetCallbacks();
   mV_Callbacks.Add(wkf::Observer::EntityDeleted.Connect(&Route::PlatformDeletedCB, this, aPlatformPtr->GetUniqueId()));
   mV_Callbacks.Add(wkf::Observer::AttachmentDeleted.Connect(&Route::AttachmentDeletedCB, this));
}

void MapRoute::Route::Clear()
{
   if (mAttachmentPtr)
   {
      mPlatformPtr->RemoveAttachment(mAttachmentPtr->GetUniqueId());
      mAttachmentPtr = nullptr;
   }
   else
   {
      wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
      if (orbit)
      {
         orbit->Clear();
      }
   }
}

bool MapRoute::Route::SetOrbitSize(WsfPM_Mover aMover)
{
   // Orbit size specification
   WsfProxy::Double*      eccentricity      = aMover.GetValue().GetAttr("eccentricity");
   WsfProxy::Length*      semiMajorAxis     = aMover.GetValue().GetAttr("semi_major_axis");
   WsfProxy::Length*      periapsisRadius   = aMover.GetValue().GetAttr("periapsis_radius");
   WsfProxy::Length*      periapsisAltitude = aMover.GetValue().GetAttr("periapsis_altitude");
   WsfProxy::Length*      apoapsisRadius    = aMover.GetValue().GetAttr("apoapsis_radius");
   WsfProxy::Length*      apoapsisAltitude  = aMover.GetValue().GetAttr("apoapsis_altitude");
   WsfProxy::Double*      rpd               = aMover.GetValue().GetAttr("revolutions_per_day");
   WsfProxy::AngularRate* meanMotion        = aMover.GetValue().GetAttr("mean_motion");

   double centralBodyRadius = mOrbit.GetCentralBody().GetMeanRadius();

   // Convenience booleans to prevent nullptr dereference below
   auto eccentricitySet      = aMover.GetValue().IsAttributeUsable("eccentricity");
   auto semiMajorAxisSet     = aMover.GetValue().IsAttributeUsable("semi_major_axis");
   auto periapsisRadiusSet   = aMover.GetValue().IsAttributeUsable("periapsis_radius");
   auto periapsisAltitudeSet = aMover.GetValue().IsAttributeUsable("periapsis_altitude");
   auto apoapsisRadiusSet    = aMover.GetValue().IsAttributeUsable("apoapsis_radius");
   auto apoapsisAltitudeSet  = aMover.GetValue().IsAttributeUsable("apoapsis_altitude");
   auto rpdSet               = aMover.GetValue().IsAttributeUsable("revolutions_per_day");
   auto meanMotionSet        = aMover.GetValue().IsAttributeUsable("mean_motion");

   // If we can't determine the orbit size, don't bother trying to draw it.
   if (!semiMajorAxisSet && !periapsisRadiusSet && !periapsisAltitudeSet && !apoapsisRadiusSet &&
       !apoapsisAltitudeSet && !rpdSet && !meanMotionSet)
   {
      mOrbit.SetSemiMajorAxis(centralBodyRadius);
      return false;
   }

   static const UtLengthValue cUNSET{0.0, 0};

   double ecc = (eccentricitySet) ? eccentricity->GetValue() : cUNSET.GetAsUnit();
   double sma = (semiMajorAxis) ? semiMajorAxis->GetValue() : cUNSET;
   double rp  = (periapsisRadius) ? periapsisRadius->GetValue() : cUNSET;
   double zp  = (periapsisAltitude) ? periapsisAltitude->GetValue() : cUNSET;
   double ra  = (apoapsisRadius) ? apoapsisRadius->GetValue() : cUNSET;
   double za  = (apoapsisAltitude) ? apoapsisAltitude->GetValue() : cUNSET;

   bool smaIsSet = semiMajorAxisSet || meanMotionSet || rpdSet;

   if (!smaIsSet)
   {
      if (!eccentricitySet) // We don't have eccentricity or semi_major_axis
      {
         // We need to get both eccentricity and sma from periapsis/apoapsis information
         if (periapsisRadiusSet)
         {
            if (apoapsisRadiusSet)
            {
               // Compute semi_major_axis and eccentricity from periapsis_radius and apoapsis_radius
               mOrbit.SetSemiMajorAxis((rp + ra) / 2.0);
               mOrbit.SetEccentricity((ra - rp) / (ra + rp));
            }
            else if (apoapsisAltitudeSet)
            {
               // Compute semi_major_axis and eccentricity from periapsis_radius and apoapsis_altitude
               ra = za + centralBodyRadius;
               mOrbit.SetSemiMajorAxis((rp + ra) / 2.0);
               mOrbit.SetEccentricity((ra - rp) / (ra + rp));
            }
            else // Compute semi_major_axis from periapsis_radius, assuming eccentricity is zero
            {
               mOrbit.SetSemiMajorAxis(rp);
               mOrbit.SetEccentricity(0.0);
            }
         }
         else if (apoapsisRadiusSet)
         {
            if (periapsisAltitudeSet)
            {
               // Compute semi_major_axis and eccentricity from periapsis_altitude and apoapsis_radius
               rp = zp + centralBodyRadius;
               mOrbit.SetSemiMajorAxis((rp + ra) / 2.0);
               mOrbit.SetEccentricity((ra - rp) / (ra + rp));
            }
            else // Compute semi_major_axis from apoapsis_radius, assuming eccentricity is zero.
            {
               mOrbit.SetSemiMajorAxis(ra);
               mOrbit.SetEccentricity(0.0);
            }
         }
         else if (apoapsisAltitudeSet)
         {
            if (periapsisAltitudeSet)
            {
               // Compute semi_major_axis and eccentricity from periapsis_altitude and apoapsis_altitude
               rp = zp + centralBodyRadius;
               ra = za + centralBodyRadius;
               mOrbit.SetSemiMajorAxis((rp + ra) / 2.0);
               mOrbit.SetEccentricity((ra - rp) / (ra + rp));
            }
            else // Compute semi_major_axis from apoapsis_altitude, assuming eccentricity is zero.
            {
               ra = za + centralBodyRadius;
               mOrbit.SetSemiMajorAxis(ra);
               mOrbit.SetEccentricity(0.0);
            }
         }
         else if (periapsisAltitudeSet)
         {
            // Compute semi_major_axis from periapsis_altitude, assuming eccentricity is zero.
            rp = zp + centralBodyRadius;
            mOrbit.SetSemiMajorAxis(rp);
            mOrbit.SetEccentricity(0.0);
         }
      }
      else // We have eccentricity, but don't know the semi_major_axis
      {
         mOrbit.SetEccentricity(ecc);
         if (periapsisRadiusSet)
         {
            mOrbit.SetSemiMajorAxis(rp / (1 - ecc));
         }
         else if (apoapsisRadiusSet)
         {
            mOrbit.SetSemiMajorAxis(ra / (1 + ecc));
         }
         else if (periapsisAltitudeSet)
         {
            rp = zp + centralBodyRadius;
            mOrbit.SetSemiMajorAxis(rp / (1 - ecc));
         }
         else if (apoapsisAltitudeSet)
         {
            ra = za + centralBodyRadius;
            mOrbit.SetSemiMajorAxis(ra / (1 + ecc));
         }
      }
   }
   else
   {
      static const double cONE_THIRD = 1.0 / 3.0;

      if (meanMotionSet)
      {
         double mm = meanMotion->GetValue();
         sma       = pow(mOrbit.GetCentralBody().GetGravitationalParameter() / mm / mm, cONE_THIRD);
         mOrbit.SetSemiMajorAxis(sma);
      }
      else if (rpdSet)
      {
         double mm = rpd->GetValue() * UtMath::cTWO_PI; // rads per day
         mm        = mm / (24 * 60 * 60);               // rads per second
         sma       = pow(mOrbit.GetCentralBody().GetGravitationalParameter() / mm / mm, cONE_THIRD);
         mOrbit.SetSemiMajorAxis(sma);
      }
      else
      {
         mOrbit.SetSemiMajorAxis(sma);
      }

      if (!eccentricitySet) // We have semi_major_axis, but not eccentricity
      {
         if (periapsisRadiusSet)
         {
            mOrbit.SetEccentricity(1.0 - (rp / sma));
         }
         else if (apoapsisRadiusSet)
         {
            mOrbit.SetEccentricity((ra / sma) - 1.0);
         }
         else if (periapsisAltitudeSet)
         {
            rp = zp + centralBodyRadius;
            mOrbit.SetEccentricity(1.0 - (rp / sma));
         }
         else if (apoapsisAltitudeSet)
         {
            ra = za + centralBodyRadius;
            mOrbit.SetEccentricity((ra / sma) - 1.0);
         }
         else
         {
            // assume eccentricy is zero
            mOrbit.SetEccentricity(0.0);
         }
      }
      else
      {
         mOrbit.SetEccentricity(ecc);
      }
   }
   return true;
}

MapRoute::Route::BuildStatus MapRoute::Route::BuildRoute(WsfPM_Mover aMover, bool aVisible)
{
   // Clear the path list on every build of the route, so that the platform position updates properly.
   ClearPathLists();
   BuildStatus state{BuildStatus::NoPosition};
   if (vaEnv.GetStandardViewer())
   {
      mLastWaypointWithDefinedAltitude = 0;
      mProxyMover                      = std::move(aMover);
      if (mProxyMover.IsOfType("WSF_ROUTE_MOVER"))
      {
         ResetCallbacks();
         if (!aMover.UsingRoute().empty())
         {
            mCallbacks.Add(
               wizard::WatchProxy(mProxyMover.InitialRoute().GetPath()).AnyUpdate.Connect(&Route::OnProxyModified, this));
         }
         state = BuildRoute(aMover, mPlatformPtr->GetPlatform().Heading().GetValue());
         if (mAttachmentPtr)
         {
            mAttachmentPtr->SetStateVisibility(aVisible);
         }
      }
      else if (mProxyMover.IsOfType("WSF_P6DOF_MOVER"))
      {
         ResetCallbacks();
         if (!aMover.GetValue().GetAttr("useRoute").ValueToString().empty())
         {
            mCallbacks.Add(
               wizard::WatchProxy(mProxyMover.InitialRoute().GetPath()).AnyUpdate.Connect(&Route::OnProxyModified, this));
         }
         state = BuildRoute(aMover, mPlatformPtr->GetPlatform().Heading().GetValue());
         if (mAttachmentPtr)
         {
            mAttachmentPtr->SetStateVisibility(aVisible);
         }
      }
      else if (mProxyMover.IsOfType("WSF_SPACE_MOVER_BASE"))
      {
         ResetCallbacks();
         if (SetOrbitSize(aMover)) // returns true if we were able to determine the orbit size
         {
            WsfProxy::Angle* raan                = mProxyMover.GetValue().GetAttr("raan");
            WsfProxy::Angle* inclination         = mProxyMover.GetValue().GetAttr("inclination");
            WsfProxy::Angle* anomaly             = mProxyMover.GetValue().GetAttr("anomaly");
            WsfProxy::Angle* argumentOfPeriapsis = mProxyMover.GetValue().GetAttr("argument_of_periapsis");

            mOrbit.SetRAAN(raan->GetValue());
            mOrbit.SetInclination(inclination->GetValue());
            mOrbit.SetTrueAnomaly(anomaly->GetValue());
            mOrbit.SetArgumentOfPeriapsis(argumentOfPeriapsis->GetValue());

            UpdateEpoch(wizard::Project::Instance()->GetStartDateTime());
            BuildOrbit(aVisible, GetOrbitColor(mProxyMover));
         }
      }
   }
   return state;
}

MapRoute::Route::BuildStatus MapRoute::Route::BuildRoute(WsfPM_Mover aMover, double aHeading)
{
   // The following code generates a path which follows the route.  The code was
   // adapted from WsfWaypointMover, so the path should match WSF exactly
   // _IF_ all of the settings are correct.  Getting the settings correct is difficult,
   // as the mover's preferred acceleration/flight path angle/speed could change at any time.

   // This method should be called after any way-point in the route is modified, as it could affect the entire route.
   ClearPathLists();

   WsfPathComputer                        pc;
   int                                    moverSwitchType = WsfPath::cSWITCH_ON_PASSING;
   WsfPathConstraints                     moverConstraints;
   wkf::AttachmentRoute::WaypointDataList waypoints;

   // Straight line or vertical Attributes:
   if (!aMover.GetValue().GetAttr("maximumSpeed").IsUnset())
      pc.mConstraints.mMaxSpeed = aMover.GetValue().GetAttr("maximumSpeed");
   if (!aMover.GetValue().GetAttr("minimumSpeed").IsUnset())
      pc.mConstraints.mMinSpeed = aMover.GetValue().GetAttr("minimumSpeed");
   if (!aMover.GetValue().GetAttr("maximumLinearAcceleration").IsUnset())
      pc.mConstraints.mMaxLinearAccel = aMover.GetValue().GetAttr("maximumLinearAcceleration");
   if (!aMover.GetValue().GetAttr("maximumClimbRate").IsUnset())
      pc.mConstraints.mMaxClimbRate = aMover.GetValue().GetAttr("maximumClimbRate");
   if (!aMover.GetValue().GetAttr("maximumFlightPathAngle").IsUnset())
      pc.mConstraints.mMaxFlightPathAngle = aMover.GetValue().GetAttr("maximumFlightPathAngle");
   if (!aMover.GetValue().GetAttr("maximumAltitude").IsUnset())
      pc.mConstraints.mMaxAltitude = aMover.GetValue().GetAttr("maximumAltitude");
   if (!aMover.GetValue().GetAttr("minimumAltitude").IsUnset())
      pc.mConstraints.mMinAltitude = aMover.GetValue().GetAttr("minimumAltitude");

   // Turn Attributes:
   if (!aMover.GetValue().GetAttr("turnRateLimit").IsUnset())
      pc.mConstraints.mTurnRateLimit = aMover.GetValue().GetAttr("turnRateLimit");
   if (!aMover.GetValue().GetAttr("bankAngleLimit").IsUnset())
   {
      pc.mConstraints.mRollAngleLimit = aMover.GetValue().GetAttr("bankAngleLimit");
      if (pc.mConstraints.mRollAngleLimit > 0.0)
      {
         pc.mConstraints.mMaxRadialAccel = tan(pc.mConstraints.mRollAngleLimit) * UtEarth::cACCEL_OF_GRAVITY;
      }
   }
   if (!aMover.GetValue().GetAttr("bodyG_Limit").IsUnset())
      pc.mConstraints.mBodyVertLimit = aMover.GetValue().GetAttr("bodyG_Limit");
   if (!aMover.GetValue().GetAttr("maximumRadialAcceleration").IsUnset())
      pc.mConstraints.mMaxRadialAccel = aMover.GetValue().GetAttr("maximumRadialAcceleration");
   if (!aMover.GetValue().GetAttr("rollRateLimit").IsUnset())
      pc.mConstraints.mRollRateLimit = aMover.GetValue().GetAttr("rollRateLimit");
   if (!aMover.GetValue().GetAttr("headingPursuitGain").IsUnset())
      pc.mConstraints.mHeadingPursuitGain = aMover.GetValue().GetAttr("headingPursuitGain");
   if (!aMover.GetValue().GetAttr("maximumTurnRate").IsUnset())
      pc.mConstraints.mMaxTurnRate = aMover.GetValue().GetAttr("maximumTurnRate");

   // Copied from the mover... indicates of the controlled object is constrained to the ground
   if (!aMover.GetValue().GetAttr("isOnGround").IsUnset())
      pc.mConstraints.mIsOnGround = aMover.GetValue().GetAttr("isOnGround");

   moverConstraints = pc.mConstraints;
   moverSwitchType =
      aMover.GetValue().GetAttr("switchOnPassing") ? WsfPath::cSWITCH_ON_PASSING : WsfPath::cSWITCH_ON_APPROACH;

   WsfPM_Route route = aMover.InitialRoute();
   BuildStatus status{BuildStatus::NoPosition};
   if ((route.IsValid()) && (route.WaypointCount() > 0))
   {
      WsfPM_Waypoint wp0 = route.GetWaypoint(0);
      WsfPathTarget  target;
      WsfPathState   state;
      // if this is an offset type position, set state lat/lon to that of the platform
      if (wp0.GetWaypointType() == WsfPM_Waypoint::cOFFSET)
      {
         state.mLatitude  = mPlatformPtr->GetPlatform().Position().GetLatitude();
         state.mLongitude = mPlatformPtr->GetPlatform().Position().GetLongitude();
      }
      else
      {
         state.mLatitude  = wp0.Position().GetLatitude();
         state.mLongitude = wp0.Position().GetLongitude();
         status           = BuildStatus::InitialPosition;
      }

      SetStateAltitude(state, wp0);

      double heading            = aHeading;
      state.mOrientationNED[0]  = heading;
      state.mOrientationNED[1]  = 0.0;
      state.mOrientationNED[2]  = 0.0;
      double speed              = wp0.Speed().GetValue();
      state.mVelocityNED[0]     = cos(heading) * speed;
      state.mVelocityNED[1]     = sin(heading) * speed;
      state.mVelocityNED[2]     = 0.0;
      state.mAccelerationNED[0] = state.mAccelerationNED[1] = state.mAccelerationNED[2] = 0.0;
      bool                     usingComputedClimbRate(false);
      bool                     usingComputedLinearAcceleration(false);
      bool                     usingComputedSpeed(false);
      size_t                   wptIndex = 0;
      size_t                   nextidx  = 0;
      std::set<WsfPM_Waypoint> usedWaypoints;
      bool                     end = false;
      WsfPM_Waypoint           wp  = route.GetWaypoint(wptIndex);
      while (!end)
      {
         WsfPM_Waypoint nextwpt;
         nextidx++;
         if (nextidx < route.WaypointCount())
         {
            nextwpt = route.GetWaypoint(nextidx); // this isn't right if we hit a goto
         }

         if (!wp.Speed().IsUnset())
         {
            target.mSpeed = wp.Speed().GetValue();
            target.mTargetType |= WsfPathTarget::cSPEED;
         }
         auto pathList = ut::make_unique<WsfPathList>();
         if (wptIndex >= 1)
         {
            // Keep speed and altitude targets if not changed
            target.mTargetType &= (WsfPathTarget::cALTITUDE | WsfPathTarget::cSPEED);
            target.mDistance = target.mTime = 0.0;
            if (usingComputedClimbRate)
            {
               usingComputedClimbRate              = false;
               pc.mConstraints.mMaxClimbRate       = WsfPathConstraints::cLARGE_DOUBLE;
               pc.mConstraints.mMaxFlightPathAngle = WsfPathConstraints::cDEFAULT_MAX_FLIGHT_ANGLE;
            }
            if (usingComputedLinearAcceleration)
            {
               usingComputedLinearAcceleration = false;
               pc.mConstraints.mMaxLinearAccel = 1.0e9;
            }
            if (usingComputedSpeed)
            {
               usingComputedSpeed = false;
               target.mTargetType &= ~WsfPathTarget::cSPEED;
            }
            if (wp.GetWaypointType() == WsfPM_Waypoint::cPOSITION)
            {
               target.mLatitude  = wp.Position().GetLatitude();
               target.mLongitude = wp.Position().GetLongitude();
               target.mTargetType |= WsfPathTarget::cLOCATION;
               status = BuildStatus::SecondPosition;
            }
            else if (wp.GetWaypointType() == WsfPM_Waypoint::cOFFSET)
            {
               // Prevent routes from mixing offset and position waypoints, besides the initial waypoint
               if (status == BuildStatus::SecondPosition)
               {
                  return BuildStatus::MixedWaypointError;
               }

               double x          = wp.Offset().mX;
               double y          = wp.Offset().mY;
               double range      = sqrt(x * x + y * y);
               double relBearing = atan2(y, x);
               double headingDeg = relBearing * UtMath::cDEG_PER_RAD;
               UtSphericalEarth::ExtrapolateGreatCirclePosition(state.mLatitude,
                                                                state.mLongitude,
                                                                headingDeg,
                                                                range,
                                                                target.mLatitude,
                                                                target.mLongitude);
               target.mTargetType |= WsfPathTarget::cLOCATION;
            }
            else if (wp.GetWaypointType() == WsfPM_Waypoint::cTURN)
            {
               if (!wp.Distance().IsUnset())
               {
                  target.mDistance = wp.Distance().GetValue();
               }
               else if (!wp.Time().IsUnset())
               {
                  target.mTime = wp.Time().GetValue();
               }

               double heading = 0.0;
               if (!wp.HeadingChange().IsUnset())
               {
                  const double headingChange = wp.HeadingChange().GetValue();
                  heading = (wp.GetTurnDirection() == WsfPM_Waypoint::cTD_RIGHT) ? headingChange : -headingChange;
                  target.mTargetType |= WsfPathTarget::cRELATIVE_TURN;
               }
               else if (!wp.Heading().IsUnset())
               {
                  heading = wp.Heading().GetValue();
               }

               target.mTargetType |= WsfPathTarget::cHEADING;
               target.mHeading = heading;
            }
            SetTargetAltitude(target, route, wptIndex);

            switch (wp.GetTurnDirection())
            {
            case WsfPM_Waypoint::cTD_SHORTEST:
               target.mTurnDirection = WsfPathTarget::cTURN_DIR_SHORTEST;
               break;
            case WsfPM_Waypoint::cTD_LEFT:
               target.mTurnDirection = WsfPathTarget::cTURN_DIR_LEFT;
               break;
            case WsfPM_Waypoint::cTD_RIGHT:
               target.mTurnDirection = WsfPathTarget::cTURN_DIR_RIGHT;
               break;
            }
            bool switchOnApproach = (!wp.SwitchOnApproach().IsUnset() && wp.SwitchOnApproach().IsTrue()) ||
                                    (wp.SwitchOnApproach().IsUnset() && moverSwitchType == WsfPath::cSWITCH_ON_APPROACH);
            if ((nextwpt.IsValid()) && switchOnApproach && (nextwpt.GetWaypointType() == WsfPM_Waypoint::cPOSITION))
            {
               target.mTargetType |= WsfPathTarget::cTURN_ON_APPROACH;
               target.mNextPointLat = nextwpt.Position().GetLatitude();
               target.mNextPointLon = nextwpt.Position().GetLongitude();
               if (wp.RadialAcceleration().GetValue() > 0)
               {
                  target.mNextPointRadialAccel = wp.RadialAcceleration().GetValue();
               }
               else if (wp.RadialAcceleration().IsUnset())
               {
                  target.mNextPointRadialAccel = pc.GetConstraints().mMaxRadialAccel;
               }
               if (!aMover.GetValue().GetAttr("maximumRadialAcceleration").IsUnset())
               {
                  target.mNextPointRadialAccel = aMover.GetValue().GetAttr("maximumRadialAcceleration");
               }
            }
            else
            {
               target.mTargetType &= ~WsfPathTarget::cTURN_ON_APPROACH;
            }
            const unsigned int cLOCATION_AND_ALTITUDE = (WsfPathTarget::cLOCATION | WsfPathTarget::cALTITUDE);
            const unsigned int cLOCATION_AND_SPEED    = (WsfPathTarget::cLOCATION | WsfPathTarget::cSPEED);

            WsfPathConstraints& pathConstraints = pc.GetConstraints();

            // If target is position/altitude && climb rate or flight angle are not constrained,
            // Compute a flight angle based on distance
            if (pathConstraints.mMaxClimbRate == WsfPathConstraints::cLARGE_DOUBLE &&
                pathConstraints.mMaxFlightPathAngle == WsfPathConstraints::cDEFAULT_MAX_FLIGHT_ANGLE &&
                (cLOCATION_AND_ALTITUDE == (target.mTargetType & cLOCATION_AND_ALTITUDE)))
            {
               double heading, distance;
               double altitudeChange = fabs(target.mAltitude - state.mAltitude);
               if (altitudeChange > 1)
               {
                  UtSphericalEarth::GreatCircleHeadingAndDistance(state.mLatitude,
                                                                  state.mLongitude,
                                                                  target.mLatitude,
                                                                  target.mLongitude,
                                                                  heading,
                                                                  distance);
                  double flightAngle                  = atan2(altitudeChange, distance);
                  pathConstraints.mMaxFlightPathAngle = flightAngle;
                  usingComputedClimbRate              = true;
               }
            }
            if (cLOCATION_AND_SPEED == (target.mTargetType & cLOCATION_AND_SPEED) &&
                pathConstraints.mMaxLinearAccel == WsfPathConstraints::cDEFAULT_MAX_LINEAR_ACCEL && target.mTime == 0.0)
            {
               double heading, distance;
               double speed       = UtVec3d::Magnitude(state.mVelocityNED);
               double speedChange = fabs(target.mSpeed - speed);
               if (speedChange > .5)
               {
                  UtSphericalEarth::GreatCircleHeadingAndDistance(state.mLatitude,
                                                                  state.mLongitude,
                                                                  target.mLatitude,
                                                                  target.mLongitude,
                                                                  heading,
                                                                  distance);
                  double avgSpeed          = (speed + target.mSpeed) / 2.0;
                  double timeToDestination = distance / avgSpeed;
                  if (timeToDestination > 1.0E-9)
                  {
                     pathConstraints.mMaxLinearAccel = speedChange / timeToDestination;
                  }
                  else
                  {
                     pathConstraints.mMaxLinearAccel = 1.0E9;
                  }
                  usingComputedLinearAcceleration = true;
               }
            }
            //! Routes are sometimes set up to loop on 1 point.  Handle this case explicitly with a 360 turn.
            if (wp == nextwpt && (target.mTargetType & WsfPathTarget::cLOCATION))
            {
               if (UtVec2d((target.mLatitude - state.mLatitude), (target.mLongitude - state.mLongitude)).MagnitudeSquared() <
                   0.0001)
               {
                  target.mTargetType &= ~WsfPathTarget::cLOCATION;
                  target.mTargetType |= WsfPathTarget::cRELATIVE_TURN | WsfPathTarget::cHEADING;
                  double turnSign = (wp.GetTurnDirection() == WsfPM_Waypoint::cTD_RIGHT) ? 1.0 : -1.0;
                  target.mHeading = turnSign * UtMath::cTWO_PI;
               }
            }
            if (target.mTargetType & WsfPathTarget::cALTITUDE)
            {
               target.mAltitude =
                  UtMath::Limit(target.mAltitude, pc.GetConstraints().mMinAltitude, pc.GetConstraints().mMaxAltitude);
            }
            if (target.mTargetType & WsfPathTarget::cSPEED)
            {
               target.mSpeed = UtMath::Limit(target.mSpeed, pc.GetConstraints().mMinSpeed, pc.GetConstraints().mMaxSpeed);
            }
            pc.ComputePath(state, target, *pathList);

            int goToIndex = FindGotoIndex(route, wp);

            AddWaypoint(wp,
                        pathList->GetEndState().mLatitude,
                        pathList->GetEndState().mLongitude,
                        pathList->GetEndState().mAltitude,
                        waypoints,
                        goToIndex);

            state = pathList->GetEndState();
            // Add pause segment if waypoint has a pause time
            if (!wp.PauseTime().IsUnset())
            {
               WsfPathList::PauseSegment* pauseSeg = new WsfPathList::PauseSegment;
               pauseSeg->Initialize(wp.PauseTime().GetValue(), state);
               pathList->Append(pauseSeg, state);
            }
         }
         else
         {
            int goToIndex = FindGotoIndex(route, wp);

            AddWaypoint(wp, state.mLatitude, state.mLongitude, state.mAltitude, waypoints, goToIndex);

            WsfPathList::PauseSegment* pauseSeg  = new WsfPathList::PauseSegment;
            WsfProxy::Time             pauseTime = wp.PauseTime();
            pauseSeg->Initialize(pauseTime.IsUnset() ? 0.0 : (double)pauseTime.GetValue(), state);
            pathList->Append(pauseSeg, state);
         }
         mPathLists.push_back(std::move(pathList));

         if (!wp.ClimbRate().IsUnset())
         {
            pc.GetConstraints().mMaxClimbRate =
               std::min(moverConstraints.mMaxClimbRate, (double)wp.ClimbRate().GetValue());
         }
         else if (wp.UseDefaultClimbRate())
         {
            pc.GetConstraints().mMaxClimbRate = moverConstraints.mMaxClimbRate;
         }

         if (!wp.LinearAcceleration().IsUnset())
         {
            pc.GetConstraints().mMaxLinearAccel =
               std::min(moverConstraints.mMaxLinearAccel, (double)wp.LinearAcceleration().GetValue());
         }
         else if (wp.UseDefaultLinearAcceleration())
         {
            pc.GetConstraints().mMaxLinearAccel = moverConstraints.mMaxLinearAccel;
         }
         if (!wp.RadialAcceleration().IsUnset())
         {
            pc.GetConstraints().mMaxRadialAccel =
               std::min(moverConstraints.mMaxRadialAccel, (double)wp.RadialAcceleration().GetValue());
         }
         else if (wp.UseDefaultRadialAcceleration())
         {
            pc.GetConstraints().mMaxRadialAccel = moverConstraints.mMaxRadialAccel;
         }
         if (!nextwpt.IsValid()) // last waypoint
         {
            end        = true;
            mEndOfPath = wp.EndOfPath();
            if (mEndOfPath == WsfPM_Waypoint::cEP_DEFAULT)
            {
               if (route.WaypointCount() == 1)
               {
                  // Assume extrapolation for single-point initial routes, unless overridden in route
                  mEndOfPath = WsfPM_Waypoint::cEP_EXTRAPOLATE;
               }
               else
               {
                  mEndOfPath = WsfPM_Waypoint::cEP_EXTRAPOLATE;
                  if (!aMover.GetValue().GetAttr("endOfPath").IsUnset())
                  {
                     int eop;
                     aMover.GetValue().GetAttr("endOfPath").GetIntegerValue(eop);
                     if (eop >= WsfPM_Waypoint::cEP_EXTRAPOLATE || eop <= WsfPM_Waypoint::cEP_REMOVE)
                     {
                        mEndOfPath = static_cast<WsfPM_Waypoint::EndOfPathOption>(eop);
                     }
                  }
               }
            }
            if (mEndOfPath == WsfPM_Waypoint::cEP_EXTRAPOLATE)
            {
               target.mTargetType &= ~(WsfPathTarget::cLOCATION | WsfPathTarget::cHEADING | WsfPathTarget::cALTITUDE |
                                       WsfPathTarget::cSPEED | WsfPathTarget::cTIME_TO_POINT);
               target.mTargetType |= WsfPathTarget::cEXTRAPOLATE;
               target.mDistance = target.mTime = 0.0;

               auto endPathList = ut::make_unique<WsfPathList>();
               pc.ComputePath(state, target, *endPathList);
               mPathLists.push_back(std::move(endPathList));
            }
         }
         if (usedWaypoints.find(wp) != usedWaypoints.end())
            break;
         usedWaypoints.insert(wp);
         wptIndex++;
         wp = nextwpt;
      }
   }

   if (!waypoints.empty())
   {
      if (!mAttachmentPtr)
      {
         mAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*mPlatformPtr, vaEnv.GetStandardViewer());
         vespa::VaAttachment::LoadAttachment(*mAttachmentPtr);
         mAttachmentPtr->SetSelectionEnabled(true);
         mAttachmentPtr->ShowIndices(false);
      }
      mAttachmentPtr->AddWaypoints(waypoints);
      mAttachmentPtr->MakeWaypointsMovable(true);
   }

   return status;
}

void MapRoute::Route::SetPositionByTime(double aTime)
{
   if (!mDirty && (aTime == mSimTime))
   {
      return;
   }
   mSimTime = aTime;
   mDirty   = false;

   // Use base-class if no path list is generated.
   if (mPathLists.empty())
   {
      SetPositionByDefaultState();
      return;
   }

   // Start time is stored in VtPlayerWsf, no concept of a "start time" for a route yet
   // if (aTime < mStartTime) return -1;

   double totalTime = 0.0;

   WsfPathState state;
   WsfPathList* path         = nullptr;
   bool         foundSegment = false;
   for (const auto& it : mPathLists)
   {
      path = it.get();
      if (path)
      {
         if (aTime <= totalTime + path->GetDuration())
         {
            path->GetState(aTime - totalTime, state);
            foundSegment = true;
            break;
         }
         totalTime += path->GetDuration();
      }
   }

   if (!foundSegment)
   {
      // Time exceeds end of the route, check end of path option
      if (mEndOfPath == WsfPM_Waypoint::cEP_REMOVE)
      {
         if (path)
         {
            state = path->GetEndState();
         }
      }
      else if (mEndOfPath == WsfPM_Waypoint::cEP_STOP)
      {
         if (path)
         {
            state = path->GetEndState();
         }
      }
      else if (mEndOfPath == WsfPM_Waypoint::cEP_EXTRAPOLATE)
      {
         // Should not get here
         if (path)
         {
            path->GetState(aTime - totalTime, state);
         }
         else
         {
            state.mLatitude = state.mLongitude = 0.0;
            UtVec3d::Set(state.mVelocityNED, 0.0);
            UtVec3d::Set(state.mAccelerationNED, 0.0);
            UtVec3d::Set(state.mOrientationNED, 0.0);
         }
      }
   }

   UtEntity ent;
   ent.SetLocationLLA(state.mLatitude, state.mLongitude, state.mAltitude);
   ent.SetOrientationNED(state.mOrientationNED[0], state.mOrientationNED[1], state.mOrientationNED[2]);
   double wcs[3];
   double wcso[3];
   ent.GetLocationWCS(wcs);
   ent.GetOrientationWCS(wcso[0], wcso[1], wcso[2]);
   mPlatformPtr->SetPositionOrientation(wcs, wcso[0], wcso[1], wcso[2]);
}

void MapRoute::Route::ResetColor()
{
   UtColor color(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide()));
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (orbit)
   {
      if (orbit->ColorIsDefined())
      {
         orbit->SetColor(orbit->GetDefinedColor());
      }
      else
      {
         orbit->SetColor(color);
      }
   }
   if (mAttachmentPtr)
   {
      mAttachmentPtr->SetTeamColor(color);
      mAttachmentPtr->UpdateTeamColor();
   }
}

void MapRoute::Route::ClearPathLists()
{
   mPathLists.clear();
}

bool MapRoute::Route::FindWaypoint(const WsfPM_Route& aRoute, const std::string& aLabel, WsfPM_Waypoint& aWp, size_t& aIndex) const
{
   for (unsigned int i = 0; i < aRoute.WaypointCount(); ++i)
   {
      WsfPM_Waypoint wp = aRoute.GetWaypoint(i);
      if (wp.IsValid())
      {
         if (wp.Label() == aLabel)
         {
            aIndex = i;
            aWp    = wp;
            return true;
         }
      }
   }
   return false;
}

void MapRoute::Route::AddWaypoint(const WsfPM_Waypoint&                   aWp,
                                  double                                  aLat,
                                  double                                  aLon,
                                  double                                  aAlt,
                                  wkf::AttachmentRoute::WaypointDataList& aWaypoints,
                                  int                                     aGoToIndex) const
{
   int type = aWp.GetWaypointType();
   if (type == WsfPM_Waypoint::cTURN)
   {
      type += aWp.GetTurnDirection() << 8;
   }
   // if this is an offset type waypoint, use the passed in lat/lon
   if (type != WsfPM_Waypoint::cPOSITION)
   {
      aWaypoints.emplace_back(aLat, aLon, aAlt, aGoToIndex, aWp.Label(), type);
   }
   // this used to use the passed in position, but I didn't like how it moves the users' waypoints.  We will ignore them
   // until we figure out how to better use them. (the lines ought to move, not the points)
   else
   {
      aWaypoints.emplace_back(aWp.Position().GetLatitude(), aWp.Position().GetLongitude(), aAlt, aGoToIndex, aWp.Label(), type);
   }
}

void MapRoute::Route::UpdateEpoch(const UtCalendar& aEpoch)
{
   mEpoch                      = aEpoch;
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (orbit)
   {
      mOrbit.SetEpoch(mEpoch.GetEpoch());
      SetPositionByDefaultState();
      BuildOrbit(orbit->GetDefinedColor());
   }
}

void MapRoute::Route::SetOrbitLineWidth(int aWidth)
{
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (orbit)
   {
      orbit->SetLineWidth(aWidth);
   }
}

void MapRoute::Route::SetOrbitPeriods(unsigned int aPeriods)
{
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (orbit)
   {
      orbit->SetPeriods(aPeriods);
   }
}

void MapRoute::Route::AttachmentDeletedCB(vespa::VaAttachment* aAttachmentPtr, const vespa::VaAttachmentSubId& aSubpartId)
{
   if ((aAttachmentPtr == mAttachmentPtr) && (aSubpartId == 0)) // if there is a subpart, it is not a full deletion
   {
      mAttachmentPtr = nullptr;
   }
}

void MapRoute::Route::PlatformDeletedCB(vespa::VaEntity* aEntityPtr)
{
   mAttachmentPtr = nullptr; // vtk will get rid of it
}

void MapRoute::Route::SetPositionByDefaultState()
{
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (orbit)
   {
      if (mOrbit.IsValid())
      {
         UtVec3d lECI;
         UtVec3d vECI;
         double  wcs[3];
         double  vned[3];
         double  owcs[3];
         mOrbit.ComputeState(lECI, vECI);
         UtEntity ent;
         ent.SetECI_ReferenceEpoch(mEpoch);
         ent.SetTime(0);
         ent.SetLocationECI(lECI.GetData());
         ent.SetVelocityECI(vECI.GetData());
         ent.GetLocationWCS(wcs);
         ent.GetVelocityNED(vned);

         double h = atan2(vned[1], vned[0]);
         double p = atan2(vned[2], sqrt(vned[0] * vned[0] + vned[1] * vned[1]));
         ent.SetOrientationNED(h, p, 0);
         ent.GetOrientationWCS(owcs[0], owcs[1], owcs[2]);

         mPlatformPtr->SetPositionOrientation(wcs, owcs[0], owcs[1], owcs[2]);
         return;
      }
   }
   UtLatPos      lat = mPlatformPtr->GetPlatform().Position().GetLatitude();
   UtLonPos      lon = mPlatformPtr->GetPlatform().Position().GetLongitude();
   UtLengthValue alt = mPlatformPtr->GetPlatform().Altitude().GetValue();
   if (mPlatformPtr->GetPlatform().AltitudeReferenceValue() == wsf::proxy::AltitudeReferenceEnum::AGL)
   {
      alt = alt + mTerrainQuery.GetElevationData(lat, lon);
   }
   double hpr[3];
   hpr[0] = mPlatformPtr->GetPlatform().Heading().GetValue();
   hpr[1] = mPlatformPtr->GetPlatform().Pitch().GetValue();
   hpr[2] = mPlatformPtr->GetPlatform().Roll().GetValue();
   UtEntity ent;
   ent.SetLocationLLA(lat, lon, alt);
   ent.SetOrientationNED(hpr[0], hpr[1], hpr[2]);
   double wcs[3];
   double wcso[3];
   ent.GetLocationWCS(wcs);
   ent.GetOrientationWCS(wcso[0], wcso[1], wcso[2]);

   mPlatformPtr->SetPositionOrientation(wcs, wcso[0], wcso[1], wcso[2]);
}

void MapRoute::Route::OnProxyModified(const wizard::ProxyChange& aProxyChange) {}

void MapRoute::Route::BuildOrbit(bool aVisible, const UtColor& aOrbitColor)
{
   wkf::AttachmentOrbit* orbit = BuildOrbit(aOrbitColor);
   if (orbit)
   {
      orbit->SetStateVisibility(aVisible);
   }
   // orbits
}

UtColor MapRoute::Route::GetOrbitColor(const WsfPM_Mover& aMover)
{
   UtColor orbitColor;
   if (!aMover.GetValue().GetAttr("orbit_color").GetAttr("red").IsInherited())
   {
      int  red, green, blue;
      auto orbitColorAttr = aMover.GetValue().GetAttr("orbit_color");
      orbitColorAttr.GetAttr("red").GetIntegerValue(red);
      orbitColorAttr.GetAttr("green").GetIntegerValue(green);
      orbitColorAttr.GetAttr("blue").GetIntegerValue(blue);
      orbitColor.Set(static_cast<unsigned char>(red), static_cast<unsigned char>(green), static_cast<unsigned char>(blue));
   }
   else
   {
      orbitColor = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
   }
   return orbitColor;
}

wkf::AttachmentOrbit* MapRoute::Route::BuildOrbit(const UtColor& aOrbitColor)
{
   wkf::AttachmentOrbit* orbit = mPlatformPtr->FindFirstAttachmentOfType<wkf::AttachmentOrbit>();
   if (!orbit)
   {
      orbit = vespa::make_attachment<wkf::AttachmentOrbit>(*mPlatformPtr, vaEnv.GetStandardViewer());
      vespa::VaAttachment::LoadAttachment(*orbit);

      orbit->SetFadeout(3000000);
   }
   orbit->Clear();

   orbit->SetDefinedColor(aOrbitColor);
   orbit->SetColor(aOrbitColor);
   orbit->SetLineWidth(wkfEnv.GetPreferenceObject<wkf::OrbitPrefObject>()->GetLineWidth()); // should be from the options
   orbit->SetPeriods(wkfEnv.GetPreferenceObject<wkf::OrbitPrefObject>()->GetPeriods());
   double simTime = 0.0;
   orbit->Add(simTime,
              mOrbit.GetSemiMajorAxis(),
              mOrbit.GetEccentricity(),
              mOrbit.GetRAAN(),
              mOrbit.GetInclination(),
              mOrbit.GetArgumentOfPeriapsis(),
              mOrbit.GetTrueAnomaly(),
              false);

   orbit->UpdateTimeAngle(mEpoch.GetEarthAngle(), simTime);
   return orbit;
}

void MapRoute::Route::SetTargetAltitude(WsfPathTarget& aTarget, const WsfPM_Route& aProxyRoute, size_t aIndex) noexcept
{
   if (aProxyRoute.IsValid())
   {
      // Get the "aIndex"th waypoint
      auto proxyWaypoint = aProxyRoute.GetWaypoint(aIndex);
      if (proxyWaypoint.IsValid())
      {
         if (!proxyWaypoint.Altitude().IsUnset())
         {
            aTarget.mAltitude = proxyWaypoint.Altitude().GetValue();
            // Set the current waypoint's altitude
            if (proxyWaypoint.AltitudeReferenceValue() == wsf::proxy::AltitudeReferenceEnum::AGL)
            {
               aTarget.mAltitude += mTerrainQuery.GetElevationData(proxyWaypoint.Position().GetLatitude(),
                                                                   proxyWaypoint.Position().GetLongitude());
            }
            mLastWaypointWithDefinedAltitude = aIndex;
         }
         else
         {
            // Set the current waypoint's altitude as the previous waypoint's altitude
            auto previousProxyWaypoint = aProxyRoute.GetWaypoint(mLastWaypointWithDefinedAltitude);
            aTarget.mAltitude          = previousProxyWaypoint.Altitude().GetValue();
            auto previousAltRef        = previousProxyWaypoint.AltitudeReferenceValue();
            // This is to capture the case of a global waypoint where the altitude reference is unset
            if (previousAltRef == wsf::proxy::AltitudeReferenceEnum::Default)
            {
               previousAltRef = mProxyMover.DefaultAltitudeReference();
            }
            // Accumulate the ground level
            if (previousAltRef == wsf::proxy::AltitudeReferenceEnum::AGL)
            {
               // Add the current waypoint's ground level to the altitude
               aTarget.mAltitude += mTerrainQuery.GetElevationData(aTarget.mLatitude, aTarget.mLongitude);
            }
         }
         aTarget.mTargetType |= WsfPathTarget::cALTITUDE;
      }
      else
      {
         aTarget.mAltitude = 0;
      }
   }
}

void MapRoute::Route::SetStateAltitude(WsfPathState& aState, WsfPM_Waypoint aWaypoint) noexcept
{
   if (!aWaypoint.Altitude().IsUnset())
   {
      aState.mAltitude = aWaypoint.Altitude().GetValue();
      if (aWaypoint.AltitudeReferenceValue() == wsf::proxy::AltitudeReferenceEnum::AGL)
      {
         aState.mAltitude += mTerrainQuery.GetElevationData(aState.mLatitude, aState.mLongitude);
      }
   }
   else
   {
      aState.mAltitude = 0;
   }
}


void MapRoute::Route::ResetCallbacks()
{
   mCallbacks.Clear();
}

int MapRoute::Route::FindGotoIndex(const WsfPM_Route& aRoute, const WsfPM_Waypoint& aWaypoint) const
{
   if (!aWaypoint.GotoLabel().empty())
   {
      WsfPM_Waypoint labelwp;
      size_t         goTo = 0;
      if (FindWaypoint(aRoute, aWaypoint.GotoLabel(), labelwp, goTo))
      {
         return ut::cast_to_int(goTo);
      }
   }
   return -1;
}
