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

#ifndef BALLISTICMISSILELAUNCHCOMPUTERGENERATOR_HPP
#define BALLISTICMISSILELAUNCHCOMPUTERGENERATOR_HPP

#include <fstream>
#include <string>
#include <vector>

#include "Tool.hpp"
#include "WeaponToolsExport.hpp"
class WsfBallisticMissileLaunchComputer;

class WT_EXPORT BallisticMissileLaunchComputerGenerator : public Tool
{
public:
   BallisticMissileLaunchComputerGenerator(WsfScenario& aScenario);
   BallisticMissileLaunchComputerGenerator(const BallisticMissileLaunchComputerGenerator& aSrc);
   BallisticMissileLaunchComputerGenerator& operator=(const BallisticMissileLaunchComputerGenerator&) = delete;
   ~BallisticMissileLaunchComputerGenerator() override;
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;

private:
   bool FireNewWeapon(double aSimTime) override;

   void InitLoftDepressAngles();

   std::string   mAirTargetFileName;
   std::ofstream mAirTargetOfs;
   std::string   mSurfaceTargetFileName;
   std::ofstream mSurfaceTargetOfs;
   std::string   mGnuplotFileName;
   std::ofstream mGnuplotOfs;

   std::vector<double> mLoftAngles;
   std::vector<double> mBurnTimes;
   double              mMinOutputTime;
   double              mMaxOutputTime;
   double              mMinOutputRange;
   double              mMaxOutputRange;
   double              mMaxOutputAlt;
   bool                mLimitedToPreApogee;

   int    mPassNumber;
   double mPass1_LoftAngleStart;
   double mPass1_LoftAngleStep;
   double mPass1_LoftAngleStop;
   double mPass2_LoftAngleStep;
   double mPass2_LoftAngleStop;

   size_t mLoftAngleIndex;
   size_t mBurnTimeIndex;
   double mLoftAngle; //!< The current loft angle
   double mBurnTime;  //!< The current burn time

   double mRange;              //!< Ground range for most recent shot
   double mMaxAltitude;        //!< Maximum altitude for most recent shot
   double mRangeAtMaxAltitude; //!< Ground range when at maximum altitude
   double mTimeAtMaxAltitude;  //!< Time of flight at maximum altitude

   double mBestLoftAngle;
   double mBestRange;

   std::vector<double> mLoftDepressAngles;
   bool                mUsingDepressedTraj;
   bool                mUsingLoftedTraj;
   size_t              mLoftDepressAngleIndex;
   bool                mLoftDepressAnglesInitialized;
   double              mLoftDepressAnglesMaxAngle;
   double              mLoftDepressAnglesMinAngle;
   double              mLoftDepressAnglesDeltaAngle;
   double              mLoftDepressAnglesStepValue;

   bool mWriteAirTargetData;
};

#endif
