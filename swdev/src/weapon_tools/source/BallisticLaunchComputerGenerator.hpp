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

#ifndef BALLISTICLAUNCHCOMPUTERGENERATOR_HPP
#define BALLISTICLAUNCHCOMPUTERGENERATOR_HPP

#include <string>
#include <vector>

#include "WeaponToolsExport.hpp"

class WsfPlatform;
#include "Tool.hpp"
#include "WsfWeaponFuse.hpp"

//! Ballistic Launch Computer Generator Tool.  The Tool will produce a
//! WsfBallisticLaunchComputer valid for a specified ballistic "iron bomb"
//! weapon type.  It computes the proper release time and distance prior to reaching
//! the target location, even if the target track is moving.  (Note that the lateral
//! placement of the weapon impact depends upon launch aircraft heading, which this
//! object does not control.)  The generator will accept a matrix of launch altitudes,
//! launch speeds, and target altitudes.  For each launch altitude and speed, the bomb
//! type will be dropped to intercept the target altitude (the bomb's fuse will be
//! forced to detonate at the specified target altitude.  The times to intercept, and
//! the down range distances flown during bomb descent will be written into the computer,
//! for later table lookup during launch timing calculations.

class WT_EXPORT BallisticLaunchComputerGenerator : public Tool
{
public:
   BallisticLaunchComputerGenerator(WsfScenario& aScenario);
   BallisticLaunchComputerGenerator(const BallisticLaunchComputerGenerator& aSrc);
   BallisticLaunchComputerGenerator& operator=(const BallisticLaunchComputerGenerator&) = delete;

   ~BallisticLaunchComputerGenerator() override;
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;

   void PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr) override;

protected:
   void           LARGenUpdate(double aSimTime);
   bool           FireNewWeapon(double aSimTime) override;
   void           ErrorBailout() override;
   WsfWeaponFuse* GetFuse(WsfPlatform* aPlatformPtr) const;

   void WriteOutputFile();

private:
   //! Generator States listed in chronological order:
   enum StateType
   {
      cLAR_GEN, //!< Generation in progress
      cDONE     //!< Complete
   };

   double TgtCurrentAlt() const { return TgtAltitude(mTgtAltIndex); }
   double MyCurrentSpeed() const { return MySpeed(mMySpdIndex); }
   double MyCurrentAlt() const { return MyAltitude(mMyAltIndex); }
   double MySpeed(int i) const { return mMyMinSpeed + mDeltaMySpeed * i; }
   double MyAltitude(int i) const { return mMyMinAlt + mMyDeltaAlt * i; }
   double TgtAltitude(int i) const { return mTgtMinAlt + mTgtDeltaAlt * i; }

   void CopyDataFrom(double*** aTOFs, double*** aDownRanges, int aNumMyAlts, int aNumMySpeeds, int aNumTgtAlts);
   void AllocAndZeroData();
   void DeallocateData();

   // -------- Attributes:

   int mMyAltIndex;
   int mMySpdIndex;
   int mTgtAltIndex;

   double mMyMinAlt;
   double mMyDeltaAlt;
   int    mNumMyAlts;

   double mMyMinSpeed;
   double mDeltaMySpeed;
   int    mNumMySpeeds;

   double mTgtMinAlt;
   double mTgtDeltaAlt;
   int    mNumTgtAlts;

   // --- Varying State Values
   StateType mState;
   bool      mFirstPass;

   double***   mTOFs;
   double***   mDownRanges;
   std::string mGenTime;
};

#endif
