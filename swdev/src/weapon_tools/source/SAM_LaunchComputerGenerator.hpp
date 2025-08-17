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

#ifndef SAM_LAUNCHCOMPUTERGENERATOR_HPP
#define SAM_LAUNCHCOMPUTERGENERATOR_HPP

#include <string>
#include <vector>

#include "WeaponToolsExport.hpp"
#include "WsfSAM_LaunchComputer.hpp"
class WsfTrack;

#include "Tool.hpp"

//! Surface To Air Missile (SAM) Launch Computer Generation Tool.
//! An instance of this class fires SAM System missiles repeatedly against targets
//! at specified altitudes and ranges, to compute the Time Of Flight and Range attained.
//! This information is used to feed values to a Launch Computer, setting the earliest
//! time when a missile system can be fired against an inbound threat missile.

class WT_EXPORT SAM_LaunchComputerGenerator : public Tool
{
public:
   SAM_LaunchComputerGenerator(WsfScenario& aScenario);
   ~SAM_LaunchComputerGenerator() override;
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;

protected:
   //!< Enumerated states listed in chronological order:
   enum StateType
   {
      cLC_GEN, //!< Generate.
      cDONE    //!< Done.
   };

   void ErrorBailout() override;
   bool FireNewWeapon(double aSimTime) override;
   void LCGenUpdate(double aSimTime);
   bool PlaceNewTarget(double aSimTime, WsfTrack& aTrack);

   struct TestPoint
   {
      TestPoint()
         : latOffset(0)
         , altitude(0)
         , groundRange(0)
         , speed(0.0){};
      double latOffset;
      double altitude;
      double groundRange;
      double speed;
   };

   void WriteOutputFile();
   void TestOutputFile();
   void TestPoints();

   bool ProcessArray(UtInput&             aInput,
                     const std::string&   aBlockTerminator,
                     UtInput::ValueType   aUnitType,
                     std::vector<double>& aValues);

private:
   double CurrentOffset() const; //{ return mSpecifiedOffsets[mOffsetIndex]; }
   double CurrentAlt();          // { return mSpecifiedAltitudes[mAltIndex]; }

   // The independent variable is ground range, launcher to target.
   // The DOWN range is calculated from cross range and ground range.
   double CurrentGrndRng(); // { return mSpecifiedRanges[mRngIndex]; }
   double CurrentDownRng();

   double CurrentSpeed();

   void ComputeTargetTrack();

   bool CalcConstraints();

   // -------- Attributes:

   double      mTrackerHeight;
   double      mTrackerMinEl;
   double      mTrackerMaxEl;
   double      mEffectiveEarthRadiusRatio;
   std::string mTrackerName;
   std::string mTrackerMode;

   // The indexes change in the order listed, slowest first, fastest-changing last.
   unsigned int mOffsetIndex;
   unsigned int mAltIndex;
   unsigned int mRngIndex;
   unsigned int mSpeedIndex;

   double mMaxDeltaAltitude;
   double mMinDeltaAltitude;
   double mMaxSlantRange;
   double mMinSlantRange;
   double mMaxTOF;

   // --- Varying State Values
   StateType mState;
   bool      mFirstPass;
   // bool         mLethalAtCurrentAlt;
   // bool         mContinueAfterMiss; // Once a hit is registered, keep trying longer ranges after a miss.
   bool   mReReadProducedFile; // If set, re-read the output file to assure it is "parsable".
   int    mTargetCount;
   size_t mTgtIndex;
   double mTgtLatDeg;
   double mTgtLonDeg;
   double mTgtAlt;
   double mTgtHdgRad;
   double mTgtSpeed;

   std::vector<double> mSpecifiedOffsets;
   std::vector<double> mSpecifiedAltitudes;
   std::vector<double> mSpecifiedRanges; // These are GROUND ranges
   std::vector<double> mSpecifiedTgtSpeeds;

   std::vector<TestPoint> mTestPoints;

   WsfSAM_LaunchComputer mLC_Instance;
   std::string           mGenTime;
};

#endif
