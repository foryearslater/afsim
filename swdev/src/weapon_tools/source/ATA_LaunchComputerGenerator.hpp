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

#ifndef ATA_LAUNCHCOMPUTERGENERATOR_HPP
#define ATA_LAUNCHCOMPUTERGENERATOR_HPP

#include <fstream>
#include <string>
#include <vector>

#include "WeaponToolsExport.hpp"

class WsfATA_LaunchComputer;
#include "Tool.hpp"

//!  Air-To-Air (ATA) Launch Computer Generation Tool.
//!  A launch computer is valid only for a known launch aircraft altitude and speed.
//!  This Tool contains within it a pointer to a WsfATALaunchComputer object,
//!  which is configured with a sizeable matrix of target engagement conditions:
//!  aspect angle, azimuth, elevation, range rate, and range.  The Tool then traverses
//!  this matrix of engagement conditions, placing an airborne target at the desired
//!  range, and giving it a velocity that results in the correct aspect angle, azimuth,
//!  range rate, etc.  A weapon is fired at the target immediately after it is placed,
//!  and given a perfect track to the target.  The missile flies out, and if possible
//!  kinematically, the target is intercepted and killed.  (USER CAUTION:  For this
//!  Tool, a valid kinematic intercept of the target should ALWAYS result in a target
//!  kill, therefore the missile's intercept Pk should temporarily be set to 1.0,
//!  provided that the point of closest approach is within the lethal radius of the
//!  warhead.)  If the engagement results in a target kill, the time of flight is saved
//!  into an array of results.  If the target is not killed, a zero is recorded.  The
//!  process is repeated for each engagement condition.  When all engagement results
//!  have been recorded, the results are written into an WSF-input format text file,
//!  defining the WsfATALaunchComputer appropriate to launch this weapon at
//!  the specified altitude and speed.  (It will then often be the case that the launch
//!  computer instance just defined will be cut-and-pasted back into the original weapon
//!  definition file, so that it may be used as part of the weapon specification.)
//!
//!  Defining an accurate LAR for an air-to-air (ATA) missile, and therefore when to
//!  launch the missile (which is the job of the launch computer), is a
//!  tough job, because so many variables are involved.  The maximum lethal range of
//!  an ATA missile may vary by an order of magnitude or more, from a head-on aspect, high
//!  altitude, high closing speed downhill shot against a non-maneuvering target which
//!  lies along the aircraft boresight (yielding a relatively long engagement range),
//!  versus a tail aspect, low altitude, high opening speed, uphill shot, off boresight,
//!  against a maneuvering target.  Along with this must also be considered a _minimum_
//!  engagement range, below which the missile cannot react quickly enough to intercept.
//!  These constraints may even overlap, such that no successful firing solution is
//!  possible.  It is the job of this Tool to estimate acceptable launch conditions
//!  during the engagement that yield an acceptable probability of kill for a specified
//!  weapon.

class WT_EXPORT ATA_LaunchComputerGenerator : public Tool
{
public:
   ATA_LaunchComputerGenerator(WsfScenario& aScenario);
   ATA_LaunchComputerGenerator(const ATA_LaunchComputerGenerator& aSrc);
   ATA_LaunchComputerGenerator& operator=(const ATA_LaunchComputerGenerator&) = delete;
   Tool*                        Clone() const override;
   ~ATA_LaunchComputerGenerator() override;
   bool Initialize(WsfSimulation& aSimulation) override;
   bool ProcessInput(UtInput& aInput) override;
   void Update(double aSimTime) override;

protected:
   void WriteOutputFile();
   void ResolveNames() override;

private:
   //! Tool States, listed in chronological order:
   enum StateType
   {
      cUNINITIALIZED = 0,   //!< Un-initialized
      cINITIALIZED,         //!< Initialized
      cENVELOPE_GENERATION, //!< Generating the launch envelope
      cDONE                 //!< Complete
   };

   void ErrorBailout() override;

   void Finish()
   {
      mState = cDONE;
      mDone  = true;
      mObserver.SetAllDone();
   }

   bool FireNewWeapon(double aSimTime) override;

   bool PlaceNewTarget(double aSimTime, WsfTrack& aTrack);

   int    mTargetCount;
   size_t mTgtIndex;
   int    mQuantityLaunched;
   int    mHitCount;
   int    mMissCount;

   double mSpeed;
   double mSavedMinRange;
   double mSavedMaxRange;
   double mSavedMaxAzimuth;
   double mSavedMaxElevation;
   double mSavedMinElevation;
   double mSavedMaxRangeRate;
   double mSavedMinRangeRate;
   double mSavedMaxBoresight;

   // --- Varying State Values
   bool mFirstTime;
   bool mFirstPass;
   bool mHitAtShortestRange;
   bool mHitAtLongestRange;

   StateType     mState;
   std::ofstream mOutFile;
   std::string   mGenTime;

   // Most of the functionality required is in the
   // software object that this tool creates:
   WsfATA_LaunchComputer* mATALCPtr;
};

#endif
