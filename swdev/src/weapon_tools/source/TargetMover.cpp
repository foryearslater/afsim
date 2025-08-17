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

#include "TargetMover.hpp"

#include <iostream>

#include "UtEarth.hpp"
#include "UtInput.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace
{
const double cDEFAULT_LATERAL_ACCEL_LIMIT = 9.0 * UtEarth::cACCEL_OF_GRAVITY;
}

TargetMover::TargetMover(WsfScenario& aScenario)
   : WsfMover(aScenario)
   , mMissileIndex(0)
   , mLateralAccelLimit(cDEFAULT_LATERAL_ACCEL_LIMIT)
   , mBreakRight(true) //,
                       // mTurnDetermined(false)
{
}

TargetMover::TargetMover(const TargetMover& aSrc)
   : WsfMover(aSrc)
   , mMissileIndex(aSrc.mMissileIndex)
   , mLateralAccelLimit(aSrc.mLateralAccelLimit)
   , mBreakRight(aSrc.mBreakRight) //,
                                   // mTurnDetermined(aSrc.mTurnDetermined)
{
}

TargetMover::~TargetMover() {}

bool TargetMover::Initialize(double aSimTime)
{
   return WsfMover::Initialize(aSimTime);
}

void TargetMover::Update(double aSimTime)
{
   if (aSimTime > mLastUpdateTime)
   {
      double dt = aSimTime - mLastUpdateTime;
      double targetLat, targetLon, targetAlt;
      GetPlatform()->GetLocationLLA(targetLat, targetLon, targetAlt);

      double tgtVel[3];
      GetPlatform()->GetVelocityNED(tgtVel);
      double targetCourseRad     = atan2(tgtVel[1], tgtVel[0]);
      double targetCourseDeg     = targetCourseRad * UtMath::cDEG_PER_RAD;
      double targetHdgChgRadians = 0;
      double bankAngle           = 0.0;
      double wingLeveler         = 1.0;
      double targetSpeed         = sqrt(tgtVel[0] * tgtVel[0] + tgtVel[1] * tgtVel[1]);

      // For this mover, if the missile index is defined, the target
      // should turn and run away at the pre-set maximum G limit.
      WsfPlatform* missilePtr = GetSimulation()->GetPlatformByIndex(mMissileIndex);
      if (missilePtr != nullptr)
      {
         double mslLat, mslLon, mslAlt;
         missilePtr->GetLocationLLA(mslLat, mslLon, mslAlt);
         double tgtToMslLOS_deg, distNotUsed;
         UtSphericalEarth::GreatCircleHeadingAndDistance(targetLat, targetLon, mslLat, mslLon, tgtToMslLOS_deg, distNotUsed);

         // 1.  Turn away from the missile.
         double desiredCourseDeg = tgtToMslLOS_deg - 180;

         // Break direction:
         mBreakRight = UtMath::NormalizeAngleMinus180_180(desiredCourseDeg - targetCourseDeg) > 0.0;

         // 2.  ONLY IF the missile is at zero aspect from the target (nose on),
         //     then turn AWAY from the direction of his velocity vector.
         double aspectDeg = UtMath::NormalizeAngleMinus180_180(tgtToMslLOS_deg - targetCourseRad * UtMath::cDEG_PER_RAD);
         bool   isNoseOn  = fabs(aspectDeg) < 1.0;
         if (isNoseOn)
         {
            double mslVelNED[3];
            missilePtr->GetVelocityNED(mslVelNED);
            double mslCourseRad = atan2(mslVelNED[1], mslVelNED[0]);
            double mslCourseDeg = mslCourseRad * UtMath::cDEG_PER_RAD;
            mBreakRight         = UtMath::NormalizeAngleMinus180_180(mslCourseDeg - targetCourseDeg) > 0.0;
         }

         double desiredTurnRad = targetCourseRad - desiredCourseDeg * UtMath::cRAD_PER_DEG;
         targetHdgChgRadians   = dt * mLateralAccelLimit / targetSpeed;
         if (cos(desiredTurnRad) > .5)
         {
            // The wing leveler function levels the wings back to zero as the aspect
            // to the missile approaches 180 degrees.
            wingLeveler = fabs(sin(desiredTurnRad));
         }
         bankAngle = wingLeveler * atan2(mLateralAccelLimit, UtEarth::cACCEL_OF_GRAVITY);
         if (!mBreakRight)
         {
            targetHdgChgRadians *= -1.0;
            bankAngle *= -1.0;
         }
      }

      double newHeadingRad = targetCourseRad + targetHdgChgRadians * wingLeveler;
      double endLat, endLon;
      UtSphericalEarth::ExtrapolateGreatCirclePosition(targetLat,
                                                       targetLon,
                                                       newHeadingRad * UtMath::cDEG_PER_RAD,
                                                       dt * targetSpeed,
                                                       endLat,
                                                       endLon);

      // Assume level flight
      GetPlatform()->SetLocationLLA(endLat, endLon, targetAlt);
      GetPlatform()->SetOrientationNED(newHeadingRad, 0.0, bankAngle);

      tgtVel[0] = cos(newHeadingRad) * targetSpeed;
      tgtVel[1] = sin(newHeadingRad) * targetSpeed;
      tgtVel[2] = 0.0;
      GetPlatform()->SetVelocityNED(tgtVel);

      WsfMover::Update(aSimTime);
   }
}

// virtual
WsfMover* TargetMover::Clone() const
{
   return new TargetMover(*this);
}

bool TargetMover::ProcessInput(UtInput& aInput)
{
   bool               myCommand = true;
   const std::string& command   = aInput.GetCommand();
   if (command == "lateral_accel_limit")
   {
      aInput.ReadValueOfType(mLateralAccelLimit, UtInput::cACCELERATION);
   }
   else
   {
      myCommand = WsfMover::ProcessInput(aInput);
   }
   return myCommand;
}
