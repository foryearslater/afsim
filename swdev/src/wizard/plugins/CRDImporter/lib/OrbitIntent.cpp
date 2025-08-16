// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include "OrbitIntent.hpp"

#include "CrdTransition.hpp"

namespace CrdImporter
{
OrbitIntent::OrbitIntent()  = default;
OrbitIntent::~OrbitIntent() = default;

bool OrbitIntent::GetAutoTimeFlag() const
{
   return mAutoTimeFlag;
}

void OrbitIntent::SetAutoTimeFlag(bool aAutoTimeFlag)
{
   mAutoTimeFlag = aAutoTimeFlag;
}

float OrbitIntent::GetBankAngle() const
{
   return mBankAngle;
}

void OrbitIntent::SetBankAngle(float aBankAngle)
{
   mBankAngle = aBankAngle;
}


const std::string& OrbitIntent::GetClockType() const
{
   return mClockType;
}

void OrbitIntent::SetClockType(const std::string& aClockType)
{
   mClockType = aClockType;
}

double OrbitIntent::GetCommandCompletionTime() const
{
   return mCommandCompletionTime;
}

void OrbitIntent::SetCommandCommandCompletionTime(double aCommandCompletiontime)
{
   mCommandCompletionTime = aCommandCompletiontime;
}

float OrbitIntent::GetInboundTrueCourse() const
{
   return mInboundTrueCourse;
}

void OrbitIntent::SetInboundTrueCourse(float aInboundTrueCourse)
{
   mInboundTrueCourse = aInboundTrueCourse;
}

double OrbitIntent::GetLegDistance() const
{
   return mLegDistance;
}

void OrbitIntent::SetLegDistance(double aLegDistance)
{
   mLegDistance = aLegDistance;
}

const std::string& OrbitIntent::GetReferencePoint() const
{
   return mReferencePoint;
}

void OrbitIntent::SetReferencePoint(const std::string& aReferencePoint)
{
   mReferencePoint = aReferencePoint;
}

bool OrbitIntent::ShouldTurnRight() const
{
   return mTurnRight;
}

void OrbitIntent::SetTurnRight(bool aTurnRight)
{
   mTurnRight = aTurnRight;
}

// Unused, but here for next effort it a more realistic orbit over the current pause.
//   const std::vector<Point>& OrbitIntent::CalculateOrbitPoint(double aCommandLegAirspeedInKnots) const
//   {

//      // Radius Calculation
//      // http://www.flightliteracy.com/load-factors-part-four/
//      float pi = 3.14159274f;
//      double bankAngleRadians = mBankAngle * pi/180;
//      double radiusInFeet = (aCommandLegAirspeedInKnots * aCommandLegAirspeedInKnots) / (11.26 * std::tanf(mBankAngle));
//   }

} // namespace CrdImporter
