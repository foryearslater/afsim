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

#ifndef AIRTOAIRLAUNCHCOMPUTERGENERATOR_HPP
#define AIRTOAIRLAUNCHCOMPUTERGENERATOR_HPP


#include "UtAtmosphere.hpp"
#include "WeaponToolsExport.hpp"
#include "WsfAirToAirLaunchComputer.hpp"

// Base Class
#include "Tool.hpp"

// Forward declarations
class WsfAirToAirLaunchComputer;

//!  Air-To-Air (ATA) Launch Computer Generation Tool (second variant).
//!  This Tool contains within it a pointer to a WsfAirToAirLaunchComputer object,
//!  which is configured with a sizable matrix of air-to-air engagement conditions.
//!  The Tool then traverses this matrix of engagement conditions, placing an
//!  airborne shooter and target at selected conditions. Engagement is repeated while
//!  range is iterated upon to yield three values, a minimum, maximum, and a range of
//!  no-escape, and weapon times-of-flight for each. A weapon is fired at the target
//!  immediately after it is placed, and given a perfect track to the target. The
//!  missile flies out, and if kinematically possible, the target is intercepted
//!  and killed. (USER CAUTION:  For this Tool, a valid kinematic intercept of the
//!  target should ALWAYS result in a target kill, therefore the missile's intercept
//!  Pk should temporarily be set to 1.0, provided that the point of closest approach
//!  is within the lethal radius of the warhead.)  If the engagement results in a
//!  target kill, the range and time of flight is saved into an array of results. If
//!  the target is not killed (at any range), a -1 is recorded. The process is
//!  repeated for each engagement condition. When all engagement results have been
//!  recorded, the results are written into an WSF-input format text file, defining
//!  the conditions appropriate to launch this weapon.

class WT_EXPORT AirToAirLaunchComputerGenerator : public Tool
{
public:
   AirToAirLaunchComputerGenerator(WsfScenario& aScenario);
   ~AirToAirLaunchComputerGenerator() override;
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;

protected:
   //! Tool States, listed in chronological order:
   enum StateType
   {
      cNO_STATE = 0 //!< Un-initialized
      ,
      cSTATE_INITIALIZED //!< Initialized
      ,
      cSTATE_GENERATION //!< Generating the results
      ,
      cSTATE_DONE //!< Complete
   };

   enum SearchingFor
   {
      cNO_SEARCH = 0,
      cSEARCH_R_MIN,
      cSEARCH_R_NE,
      cSEARCH_R_MAX,
      cSEARCH_DONE
   };

   enum GenerationStateType
   {
      cNONE = 0,
      cBRACKETING,
      cBISECTING,
      cDONE,
      cFAILED
   };

   void ErrorBailout() override;

   void Finish()
   {
      mState = cSTATE_DONE;
      mDone  = true;
      mObserver.SetAllDone();
   }

   bool FireNewWeapon(double aSimTime) override;

private:
   void Iterate(double aSimTime);
   bool EvaluateResult();
   bool Evaluate();

   void DetermineTargetStateValues(double  aShooterLatDeg,
                                   double  aShooterLonDeg,
                                   double  aShooterAlt,
                                   double  aShooterHeadingRad,
                                   double  aGroundRange,
                                   double  aTargetLeadRad,
                                   double  aTargetAspectRad,
                                   double  aTargetMach,
                                   double  aTargetAlt,
                                   double& aTargetLatDeg,
                                   double& aTargetLonDeg,
                                   double& aTargetHeadingRad,
                                   double  aTargetVelNED[3]);

   WsfPlatform* PlaceNewTarget(double aSimTime, WsfTrack& aTrack);

   void ProcessResults();

   void SetLaunchConditions(double aSimTime);

   void SetSearchRmin();
   void SetSearchRmax();
   void SetSearchRne();

   void WriteOutputFile();

   unsigned int mTargetCount;
   size_t       mTargetIndex;
   unsigned int mQuantityLaunched;
   unsigned int mHitCount;
   unsigned int mMissCount;
   unsigned int mSignificant;
   unsigned int mNullCount;
   size_t       mResultsSize;
   unsigned int mCurrentIndex;
   unsigned int mMaxIterations;
   unsigned int mIterationCount;

   double mIncrementRatio;

   bool mFirstPass;

   bool mAspectRotIsClockwise;
   bool mAspectRotIsSignedToLead;

   double mShooterMach;
   double mShooterSpeed;
   double mTargetAlt;
   double mTargetMach;
   double mTargetAspect;
   double mTargetLead;

   StateType    mState;
   SearchingFor mSearchingFor;

   std::pair<double, bool> mCurrent;
   std::pair<double, bool> mPrevious;

   double mCurrentGrndRange;
   bool   mCurrentHit;
   double mCurrentTOF;
   double mGrndRangeTolerance;

   std::string mGeneratedTimeString;

   UtAtmosphere mAtmosphere;

   // Most of the functionality required is in the
   // software object that this tool creates:
   WsfAirToAirLaunchComputer*                            mATA_LaunchComputerPtr;
   WsfAirToAirLaunchComputer::ATA_InterceptTable::Result mTempResult;

   std::string mTableFileName;
   std::string mResultsFileName;

   double mLastTargetAspect;
   double mLastTargetLead;
   double mLastTargetMach;
   double mLastTargetAlt;
   double mLastShooterMach;
   double mLastShooterAlt;

   unsigned int mSuccesses;
   unsigned int mFailures;
   double       mGlobalMinRmax;
   double       mGlobalMaxRmax;
   double       mGlobalMinRmin;
   double       mGlobalMaxRmin;
   double       mGlobalMinRne;
   double       mGlobalMaxRne;
   double       mGlobalMinRmaxTOF;
   double       mGlobalMaxRmaxTOF;
   double       mGlobalMinRminTOF;
   double       mGlobalMaxRminTOF;
   double       mGlobalMinRneTOF;
   double       mGlobalMaxRneTOF;

   GenerationStateType mGenerationState;
   double              mMaxRange; // meters  (default 1km)
   double              mMinRange; // meters  (default 120km)
};

#endif
