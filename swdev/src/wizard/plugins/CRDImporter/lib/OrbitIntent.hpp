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

#ifndef ORBIT_INTENT_HPP
#define ORBIT_INTENT_HPP

#include <string>

#include "ElementBase.hpp"

namespace CrdImporter
{
class CrdTransition;
class OrbitIntent : public ElementBase
{
public:
   OrbitIntent();
   ~OrbitIntent() override;
   bool               GetAutoTimeFlag() const;
   void               SetAutoTimeFlag(bool aAutoTimeFlag);
   float              GetBankAngle() const;
   void               SetBankAngle(float aBankAngle);
   const std::string& GetClockType() const;
   void               SetClockType(const std::string& aClockType);
   double             GetCommandCompletionTime() const;
   void               SetCommandCommandCompletionTime(double aCommandCompletiontime);
   float              GetInboundTrueCourse() const;
   void               SetInboundTrueCourse(float aInboundTrueCourse);
   double             GetLegDistance() const;
   void               SetLegDistance(double aLegDistance);
   const std::string& GetReferencePoint() const;
   void               SetReferencePoint(const std::string& aReferencePoint);
   bool               ShouldTurnRight() const;
   void               SetTurnRight(bool aTurnRight);
   // Unused but will remain for next task of more realistic orbit.
   // const std::vector<Point>& CalculateOrbitPoint(double aCommandLegAirspeedInKnots) const;

private:
   bool        mAutoTimeFlag = false;
   float       mBankAngle    = 0.0f;
   std::string mClockType;
   double      mCommandCompletionTime = 0.0;
   float       mInboundTrueCourse     = 0.0f;
   double      mLegDistance           = 0.0;
   std::string mReferencePoint;
   bool        mTurnRight;
};
} // namespace CrdImporter
#endif
