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

#ifndef ATG_LAR_AND_LC_GENERATOR_HPP
#define ATG_LAR_AND_LC_GENERATOR_HPP

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include "Tool.hpp"
#include "WeaponToolsExport.hpp"

//! Air To Ground (ATG) Launch Acceptable Region (LAR) Generator Tool. ATG_LAR_And_LC_Generator is a
//! Tool to compute the LAR for Air To Ground weapon. An instance will accept a defined
//! glide-bomb-type guided weapon, dropping it at a specified altitude and speed against a
//! ground target location. The target location will be varied with discrete longitudinal
//! and lateral offsets from the launch point, and the hit / miss results will be saved in a
//! matrix of boolean values. The tool will then attempt to produce a LAR in WsfZone format,
//! within which there is a good probability of target intercept. For off-design launch
//! conditions, the tool will also compute ballistic (no guidance) sensitivities for adjusting
//! the LAR as a function of incremental altitude, speed, and loft (gamma climb angle). A
//! shrink factor may be specified to shrink the LAR boundaries to incorporate conservatism
//! for off-design conditions; it pulls the WsfZone boundary inward proportionally from the
//! centroid of the LAR.

class WT_EXPORT ATG_LAR_And_LC_Generator : public Tool
{
public:
   ATG_LAR_And_LC_Generator(WsfScenario& aScenario);
   ATG_LAR_And_LC_Generator(const ATG_LAR_And_LC_Generator& aSrc);
   ATG_LAR_And_LC_Generator& operator=(const ATG_LAR_And_LC_Generator&) = delete;
   ~ATG_LAR_And_LC_Generator() override                                 = default;
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;

protected:
   void LARGenUpdate(double aSimTime);
   void SensitivityUpdate(double aSimTime);
   bool FireNewWeapon(double aSimTime) override;
   void ErrorBailout() override;

   void WriteOutputFile();

private:
   //! All Arrays are Zero-Indexed...
   double Range(int aRangeIndex) const;
   double Offset(int aOffsetIndex) const;

   double CurrentRange() const;
   double CurrentOffset() const;

   void SetCurrentResult(bool aValue);

   void SetHitResult(int aRangeIndex, int aOffsetIndex, bool aValue);
   bool HitResult(int aRangeIndex, int aOffsetIndex) const;

   void CreateZoneFromBools();

   //! States listed in chronological order.
   enum StateType
   {
      cLAR_GEN,
      cTRANSITION,
      cBASELINE,
      cALT_DELTA,
      cSPD_DELTA,
      cLOFT_DELTA,
      cDONE
   };

   class AltAndSpeed
   {
   public:
      AltAndSpeed(double aAlt, double aSpd, const std::string& aInputAltUnits, const std::string& aInputSpdUnits)
         : mAlt(aAlt)
         , mSpd(aSpd)
         , mInputAltUnits(aInputAltUnits)
         , mInputSpdUnits(aInputSpdUnits)
      {
      }

      double      mAlt;           //!< Altitude in meters
      double      mSpd;           //!< Speed in m/s
      std::string mInputAltUnits; //!< The input altitude units from input stream
      std::string mInputSpdUnits; //!< The input speed units from input stream, null is Mach
   };

   class LocationPoint
   {
   public:
      LocationPoint(double aX, double aY)
         : mX(aX)
         , mY(aY)
      {
      }

      double mX;
      double mY;
   };

   struct LAR_Condition
   {
      std::string mLarName;
      std::string mZoneName;
      double      mAlt;
      double      mSpeed;
      double      mBaselineRange;
      double      mPlusAltRange;
      double      mPlusSpeedRange;
      double      mPlusLoftRange;
   };

   using LaunchConditions = std::vector<AltAndSpeed>;
   using LocationPoints   = std::vector<LocationPoint>;
   using LAR_Conditions   = std::vector<LAR_Condition>;

   void BeginNewLAR();
   bool PlaceNewTarget(double aSimTime, WsfTrack& aTrack);
   void AdjustLAR();
   void WriteZone();
   void KillGuidance();

   using ClockwisePath = std::array<std::pair<int, int>, 8>;

   bool          CreateLARBoundary();
   ClockwisePath GetClockwisePath(const std::pair<int, int>& aBacktrack) const;
   bool          AddPoint(int aRange, int aOffset);

   // Attributes:
   int    mNumTgtLateralOffsets; // Offsets always start at Y=0.0
   double mTgtLateralOffset;

   int    mNumRanges;
   double mMinRange;
   double mTgtFwdOffset;

   LaunchConditions mLaunchCond;
   int              mLaunchCondIndex;

   // Varying State Values
   int       mQuantityLaunched;
   StateType mState;
   bool      mFirstTime;
   bool      mFirstPass;
   bool      mWithinLAR;
   bool      mWeaponTimedOut;
   bool      mHitAtShortestRange;
   bool      mHitAtLongestRange;
   bool      mHitAtLargestOffset;
   bool      mSwitchSides;
   double    mCurrentAlt;
   double    mCurrentSpd;
   int       mTargetCount;
   size_t    mTgtIndex;
   int       mIRange;
   int       mIOffset;

   std::vector<bool> mResults;

   double mCentroidX;
   double mShrinkFactor;

   // Sensitivity Values
   double mBaselineRange;
   double mPlusAltRange;
   double mPlusSpeedRange;
   double mPlusLoftRange;

   std::ofstream mOutFile;

   LAR_Conditions mConditions;
   LocationPoints mPoints;
   std::string    mGenTime;
};

#endif
