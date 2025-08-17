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

#include "ATG_LAR_And_LC_Generator.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "UtInputFile.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "WsfATG_LaunchComputer.hpp"
#include "WsfGuidedMover.hpp"
#include "WsfLaunchComputer.hpp"
#include "WsfLaunchComputerTypes.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfRouteMover.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfStringId.hpp"
#include "WsfTrack.hpp"
#include "WsfWeapon.hpp"
#include "WsfZone.hpp"
#include "WsfZoneTypes.hpp"

namespace
{
const char* cATG_LAR_AND_LC_DEFAULT_PRODUCT = "_ATG_LAUNCH_COMPUTER";
}

ATG_LAR_And_LC_Generator::ATG_LAR_And_LC_Generator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mNumTgtLateralOffsets(0)
   , mTgtLateralOffset(0.0)
   , mNumRanges(0)
   , mMinRange(0.0)
   , mTgtFwdOffset(0.0)
   , mLaunchCond()
   , mLaunchCondIndex(0)
   , mQuantityLaunched(0)
   , mState(cLAR_GEN)
   , mFirstTime(true)
   , mFirstPass(true)
   , mWithinLAR(false)
   , mWeaponTimedOut(false)
   , mHitAtShortestRange(false)
   , mHitAtLongestRange(false)
   , mHitAtLargestOffset(false)
   , mSwitchSides(false)
   , mCurrentAlt(0.0)
   , mCurrentSpd(0.0)
   , mTargetCount(0)
   , mTgtIndex(0)
   , mIRange(0)
   , mIOffset(0)
   , mCentroidX(0.0)
   , mShrinkFactor(1.0)
   , mBaselineRange(0.0)
   , mPlusAltRange(0.0)
   , mPlusSpeedRange(0.0)
   , mPlusLoftRange(0.0)
   , mOutFile()
   , mConditions()
   , mPoints()
   , mGenTime()
{
   mToolProduces = cATG_LAR_AND_LC_DEFAULT_PRODUCT;
   mAlt          = 10000.0 * UtMath::cM_PER_FT; // set default altitude to 10k ft
}

ATG_LAR_And_LC_Generator::ATG_LAR_And_LC_Generator(const ATG_LAR_And_LC_Generator& aSrc)
   : Tool(aSrc)
   , mNumTgtLateralOffsets(aSrc.mNumTgtLateralOffsets)
   , mTgtLateralOffset(aSrc.mTgtLateralOffset)
   , mNumRanges(aSrc.mNumRanges)
   , mMinRange(aSrc.mMinRange)
   , mTgtFwdOffset(aSrc.mTgtFwdOffset)
   , mLaunchCond(aSrc.mLaunchCond)
   , mLaunchCondIndex(aSrc.mLaunchCondIndex)
   , mQuantityLaunched(aSrc.mQuantityLaunched)
   , mState(aSrc.mState)
   , mFirstTime(aSrc.mFirstTime)
   , mFirstPass(aSrc.mFirstPass)
   , mWithinLAR(aSrc.mWithinLAR)
   , mWeaponTimedOut(aSrc.mWeaponTimedOut)
   , mHitAtShortestRange(aSrc.mHitAtShortestRange)
   , mHitAtLongestRange(aSrc.mHitAtLongestRange)
   , mHitAtLargestOffset(aSrc.mHitAtLargestOffset)
   , mSwitchSides(aSrc.mSwitchSides)
   , mCurrentAlt(aSrc.mCurrentAlt)
   , mCurrentSpd(aSrc.mCurrentSpd)
   , mTargetCount(aSrc.mTargetCount)
   , mTgtIndex(aSrc.mTgtIndex)
   , mIRange(aSrc.mIRange)
   , mIOffset(aSrc.mIOffset)
   , mResults(aSrc.mResults)
   , mCentroidX(aSrc.mCentroidX)
   , mShrinkFactor(aSrc.mShrinkFactor)
   , mBaselineRange(aSrc.mBaselineRange)
   , mPlusAltRange(aSrc.mPlusAltRange)
   , mPlusSpeedRange(aSrc.mPlusSpeedRange)
   , mPlusLoftRange(aSrc.mPlusLoftRange)
   , mOutFile()
   , mConditions(aSrc.mConditions)
   , mPoints(aSrc.mPoints)
   , mGenTime(aSrc.mGenTime)
{
}

// virtual
Tool* ATG_LAR_And_LC_Generator::Clone() const
{
   return new ATG_LAR_And_LC_Generator(*this);
}

// virtual
bool ATG_LAR_And_LC_Generator::ProcessInput(UtInput& aInput)
{
   bool        myCommand = true;
   std::string command(aInput.GetCommand());
   if ((command == "altitude") || (command == "speed"))
   {
      const char*       a = "Since ATG_LAR_And_LC_Generator can process many speeds and altitudes, ";
      const char*       b = "each one must be combined using altitude_and_mach, or altitude_and_speed.";
      const std::string scream(std::string(a) + std::string(b));
      throw UtInput::BadValue(aInput, scream);
   }
   else if (command == "altitude_and_mach")
   {
      double      alt, mach;
      std::string altUnits;
      aInput.ReadValue(alt);
      aInput.ReadValue(altUnits);
      alt = aInput.ConvertValue(alt, altUnits, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(alt, 0.0);
      aInput.ReadValue(mach);
      aInput.ValueGreaterOrEqual(mach, 0.0);
      double speed = mach * WsfScenario::FromInput(aInput).GetAtmosphere().SonicVelocity(alt);
      mLaunchCond.emplace_back(alt, speed, altUnits, std::string("mach"));
   }
   else if (command == "altitude_and_speed")
   {
      double      alt, speed;
      std::string altUnits, spdUnits;
      aInput.ReadValue(alt);
      aInput.ReadValue(altUnits);
      alt = aInput.ConvertValue(alt, altUnits, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(alt, 0.0);
      aInput.ReadValue(speed);
      aInput.ReadValue(spdUnits);
      speed = aInput.ConvertValue(speed, spdUnits, UtInput::cSPEED);
      aInput.ValueGreaterOrEqual(speed, 0.0);
      mLaunchCond.emplace_back(alt, speed, altUnits, spdUnits);
   }
   else if ((command == "target_offsets") || (command == "target_lateral_offsets"))
   {
      aInput.ReadValueOfType(mTgtLateralOffset, UtInput::cLENGTH);
      aInput.ReadValue(mNumTgtLateralOffsets);
      aInput.ValueGreater(mTgtLateralOffset, 0.0);
      aInput.ValueGreater<double>(mNumTgtLateralOffsets, 1.5);
   }
   else if (command == "switch_sides")
   {
      mSwitchSides = true;
   }
   else if ((command == "target_ranges") || (command == "target_forward_offsets"))
   {
      aInput.ReadValueOfType(mMinRange, UtInput::cLENGTH);
      aInput.ReadValueOfType(mTgtFwdOffset, UtInput::cLENGTH);
      aInput.ReadValue(mNumRanges);
      aInput.ValueGreater<double>(mNumRanges, 1.5);
   }
   else if (command == "shrink_factor")
   {
      aInput.ReadValue(mShrinkFactor);
      aInput.ValueInClosedRange(mShrinkFactor, 0.0, 1.0);
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }
   return myCommand;
}

//! All Arrays are Zero-Indexed...
double ATG_LAR_And_LC_Generator::Range(int aRangeIndex) const
{
   return mMinRange + (mTgtFwdOffset * aRangeIndex);
}

double ATG_LAR_And_LC_Generator::Offset(int aOffsetIndex) const
{
   double offset = mTgtLateralOffset * aOffsetIndex;
   if (mSwitchSides)
   {
      offset *= -1.0;
   }
   return offset;
}

double ATG_LAR_And_LC_Generator::CurrentRange() const
{
   return Range(mIRange);
}

double ATG_LAR_And_LC_Generator::CurrentOffset() const
{
   return Offset(mIOffset);
}

void ATG_LAR_And_LC_Generator::SetCurrentResult(bool aValue)
{
   SetHitResult(mIRange, mIOffset, aValue);
}

void ATG_LAR_And_LC_Generator::SetHitResult(int aRangeIndex, int aOffsetIndex, bool aValue)
{
   mResults[(aRangeIndex * mNumTgtLateralOffsets) + aOffsetIndex] = aValue;
}

bool ATG_LAR_And_LC_Generator::HitResult(int aRangeIndex, int aOffsetIndex) const
{
   return mResults[(aRangeIndex * mNumTgtLateralOffsets) + aOffsetIndex];
}


// virtual
void ATG_LAR_And_LC_Generator::Update(double aSimTime)
{
   if (mState == cDONE)
   {
   }
   else if (mState == cLAR_GEN)
   {
      LARGenUpdate(aSimTime);
   }
   else
   {
      SensitivityUpdate(aSimTime);
   }
   Tool::Update(aSimTime);
}


void ATG_LAR_And_LC_Generator::KillGuidance()
{
   if (mObserver.GetState() == WeaponObserver::cIN_FLIGHT)
   {
      WsfPlatform* pPtr = mObserver.GetWeaponPlatform();
      if (pPtr != nullptr)
      {
         WsfMover*       moverPtr = pPtr->GetMover();
         WsfGuidedMover* gmPtr    = dynamic_cast<WsfGuidedMover*>(moverPtr);
         if (gmPtr != nullptr)
         {
            // Turn guidance off, so it flies ballistically for the sensitivities.
            gmPtr->SetGuidance(nullptr);
         }
      }
   }
}


// virtual
void ATG_LAR_And_LC_Generator::SensitivityUpdate(double aSimTime)
{
   bool inFlight = mObserver.GetState() == WeaponObserver::cIN_FLIGHT;

   if (mState == cTRANSITION)
   {
      if (!inFlight)
      {
         // Ready to run sensitivities.
         // First, make a baseline ballistic shot.
         mState = cBASELINE;
         if (FireNewWeapon(aSimTime))
         {
            KillGuidance();
         }
      }
   }
   else if (mState == cBASELINE)
   {
      if (inFlight)
      {
         // Do nothing, just wait until impact...
      }
      else
      {
         mBaselineRange = mObserver.HorizontalRange();

         // Now make an increased altitude shot.
         mState = cALT_DELTA;
         if (FireNewWeapon(aSimTime))
         {
            KillGuidance();
         }
      }
   }
   else if (mState == cALT_DELTA)
   {
      if (inFlight)
      {
         // Do nothing, just wait until impact...
      }
      else
      {
         mPlusAltRange = mObserver.HorizontalRange();

         // Now make an increased speed shot.
         mState = cSPD_DELTA;
         if (FireNewWeapon(aSimTime))
         {
            KillGuidance();
         }
      }
   }
   else if (mState == cSPD_DELTA)
   {
      if (inFlight)
      {
         // Do nothing, just wait until impact...
      }
      else
      {
         mPlusSpeedRange = mObserver.HorizontalRange();

         // Now make an increased loft shot.
         mState = cLOFT_DELTA;
         if (FireNewWeapon(aSimTime))
         {
            KillGuidance();
         }
      }
   }
   else if (mState == cLOFT_DELTA)
   {
      if (inFlight)
      {
         // Do nothing, just wait until impact...
      }
      else
      {
         mPlusLoftRange = mObserver.HorizontalRange();

         // All done with sensitivities.
         WriteZone();

         BeginNewLAR();
      }
   }
}


// virtual
void ATG_LAR_And_LC_Generator::LARGenUpdate(double aSimTime)
{
   if (mFirstPass)
   {
      mCurrentAlt = mLaunchCond[mLaunchCondIndex].mAlt;
      mCurrentSpd = mLaunchCond[mLaunchCondIndex].mSpd;
      mIRange     = 0;
      mIOffset    = 0;

      if (FireNewWeapon(aSimTime))
      {
         mFirstPass = false;
      }
   }
   else
   {
      if (mObserver.GetState() == WeaponObserver::cTERMINATED)
      {
         if (DebugEnabled())
         {
            static int count = 0;
            ++count;
            double altScale =
               1.0 / UtInput::ConvertValueFrom(1.0, mLaunchCond[mLaunchCondIndex].mInputAltUnits, UtInput::cLENGTH);
            double spdScale = 1.0;
            if (mLaunchCond[mLaunchCondIndex].mInputSpdUnits == "mach")
            {
               spdScale = 1.0 / GetScenario().GetAtmosphere().SonicVelocity(mCurrentAlt);
            }
            else
            {
               spdScale =
                  1.0 / UtInput::ConvertValueFrom(1.0, mLaunchCond[mLaunchCondIndex].mInputSpdUnits, UtInput::cSPEED);
            }
            // cout.setf(ios::showpoint);
            auto logDebug = ut::log::debug() << "Weapon Observer Termination Report.";
            logDebug.AddNote() << "Count: " << count;
            logDebug.AddNote() << "Current Altitude: " << mCurrentAlt * altScale << " "
                               << mLaunchCond[mLaunchCondIndex].mInputAltUnits;
            logDebug.AddNote() << "Current Speed: " << mCurrentSpd * spdScale << " "
                               << mLaunchCond[mLaunchCondIndex].mInputSpdUnits;
            logDebug.AddNote() << "Current Offset: " << CurrentOffset();
            logDebug.AddNote() << "Current Range: " << CurrentRange();
            logDebug.AddNote() << "Flight Time: " << mObserver.TimeOfFlight();
            logDebug.AddNote() << "Lethal Engagement: " << mObserver.EngagementLethal();
         }

         // When we encounter a weapon hit, save the point,
         // and disable saving another point until the range has been incremented.
         if (mObserver.EngagementLethal())
         {
            SetHitResult(mIRange, mIOffset, true);
            mWithinLAR = true;

            if (mIRange == 0)
            {
               if (mIOffset == 0)
               {
                  mHitAtShortestRange = true;
               }
            }
            else if (mIRange == (mNumRanges - 1))
            {
               mHitAtLongestRange = true;
            }

            if (mIOffset == (mNumTgtLateralOffsets - 1))
            {
               mHitAtLargestOffset = true;
            }
         }
         else
         {
            // Previously fired weapon went away. Remove the old target,
            // if our own weapon didn't already kill it.
            WsfPlatform* oldTgtPtr = GetSimulation()->GetPlatformByIndex(mTgtIndex);
            if (oldTgtPtr != nullptr)
            {
               GetSimulation()->DeletePlatform(aSimTime, oldTgtPtr);
            }
         }

         mTgtIndex = 0;

         if (mObserver.WeaponTimedOut())
         {
            mWeaponTimedOut = true;
         }

         mObserver.ResetState();
      }

      if (mObserver.GetState() == WeaponObserver::cIDLE)
      {
         // Fire a new one:
         // Run the matrix with later values changing the fastest:
         // Altitudes, Velocities, Ranges, Offsets

         if (mIOffset < (mNumTgtLateralOffsets - 1)) // If not at max offset, increment it.
         {
            ++mIOffset;
            FireNewWeapon(aSimTime);
         }
         else if (mIRange < (mNumRanges - 1)) // If not at max range, increment it.
         {
            ++mIRange;
            mIOffset = 0;
            FireNewWeapon(aSimTime);
         }
         else
         {
            mState = cTRANSITION;
         } // Offset and Range Checks
      }    // if observer is idle

   } // if firstPass
}

// virtual
bool ATG_LAR_And_LC_Generator::Initialize(WsfSimulation& aSimulation)
{
   Tool::Initialize(aSimulation);

   mFirstPass       = true;
   mFirstTime       = true;
   mLaunchCondIndex = 0;
   mTargetCount     = 0;
   mTgtIndex        = 0;

   time_t aclock;
   time(&aclock);
   struct tm*  newtime = localtime(&aclock);
   std::string stamp1("   # Created by ATG_LAR_And_LC_Generator on ");
   std::string stamp2(asctime(newtime));
   mGenTime = std::string(stamp1 + stamp2);

   if (mTargetPlatformTypeId.IsNull())
   {
      ut::log::error() << "ATG_LAR_And_LC_Generator: 'target_type' not specified.";
      mInitialized = false;
   }

   if (mWeaponEffectTypeId.IsNull())
   {
      ut::log::error() << "ATG_LAR_And_LC_Generator: 'weapon_effects' not specified.";
      mInitialized = false;
   }

   if ((mTgtLateralOffset <= 0.0) || (mTgtFwdOffset <= 0.0))
   {
      ut::log::error() << "ATG_LAR_And_LC_Generator: All Delta values must be positive.";
      mInitialized = false;
   }

   if (mLaunchCond.empty())
   {
      ut::log::error() << "ATG_LAR_And_LC_Generator: Must specify at least one launch condition set.";
      mInitialized = false;
   }

   // Open the output file.
   std::string fileName(mOutputFileName);
   mOutFile.open(fileName.c_str());
   if (!mOutFile.is_open())
   {
      auto logError = ut::log::error() << "Unable to open output File!";
      logError.AddNote() << "File: " << fileName;
      mInitialized = false;
   }

   if ((mNumRanges < 2) || (mNumTgtLateralOffsets < 2))
   {
      ut::log::error() << "ATG_LAR_And_LC_Generator: Must specify at least two Ranges and two Offsets.";
      mInitialized = false;
   }
   else if (mInitialized)
   {
      mResults.assign(mNumRanges * mNumTgtLateralOffsets, false);
   }

   if (!mInitialized)
   {
      Tool::UnInitialize();
   }

   return mInitialized;
}

void ATG_LAR_And_LC_Generator::ErrorBailout()
{
   mState = cDONE;
   mDone  = true;
   mObserver.SetAllDone();
   ut::log::error() << "Unknown Error in ATG_LAR_And_LC_Generator has caused a premature abort!";
}

void ATG_LAR_And_LC_Generator::BeginNewLAR()
{
   if ((mLaunchCondIndex + 1) >= static_cast<int>(mLaunchCond.size()))
   {
      // We just did the last LAR, so quit.
      WriteOutputFile();

      mState = cDONE;
      mDone  = true;
      mObserver.SetAllDone();
   }
   else
   {
      mResults.assign(mNumRanges * mNumTgtLateralOffsets, false);
      mState = cLAR_GEN;
      ++mLaunchCondIndex;
      mFirstPass = true;
      mWithinLAR = false;
   }
}


void ATG_LAR_And_LC_Generator::AdjustLAR()
{
   if ((!mPoints.empty()) && (mShrinkFactor < 1.0))
   {
      // First, calculate the centroid of the All-Encompassing
      // (non-conservative) LAR:
      // xAvg = Sum(dA*X)/Sum(dA);
      double sum_dAX = 0;
      double sum_dA  = 0;

      for (LocationPoints::size_type i = 1; i < mPoints.size(); ++i)
      {
         double midX = (mPoints[i].mX + mPoints[i - 1].mX) / 2.0;
         double dX   = mPoints[i].mX - mPoints[i - 1].mX;
         double dA   = dX * fabs(mPoints[i].mY + mPoints[i - 1].mY) / 2.0;

         sum_dAX += dA * midX;
         sum_dA += dA;
      }

      if (sum_dA > 0.0)
      {
         // Calculate X centroid of the Region:
         mCentroidX = sum_dAX / sum_dA;

         // Now, take each edge point, and move it TOWARD the centroid,
         // to shrink the LAR into a higher Pk firing area:
         for (auto& point : mPoints)
         {
            double oldX = point.mX;
            double newX = mCentroidX - mShrinkFactor * (mCentroidX - oldX);

            double oldY = point.mY;
            double newY = mShrinkFactor * oldY;

            point.mX = newX;
            point.mY = newY;
         }
      }
   }
}

// virtual
void ATG_LAR_And_LC_Generator::WriteOutputFile()
{
   // Write out the launch computer stuff.
   if (mOutFile)
   {
      mOutFile << "\nlaunch_computer " << mOutputObjectName << " " << WsfATG_LaunchComputer::BaseTypeName() << std::endl;
      mOutFile << mGenTime;

      for (LAR_Condition& condition : mConditions)
      {
         mOutFile << "   launch_acceptable_region " << condition.mLarName << std::endl;

         double altkft = condition.mAlt / UtMath::cM_PER_FT / 1000.0;
         double spdknt = condition.mSpeed / UtMath::cM_PER_NM * 3600.0;

         int iAltKft   = static_cast<int>(altkft + 0.1);           // addition is to counter any round-off error
         int iAltM     = static_cast<int>(condition.mAlt + 0.1);   // addition is to counter any round-off error
         int iSpeedM   = static_cast<int>(condition.mSpeed + 0.1); // addition is to counter any round-off error
         int iSpeedKnt = static_cast<int>(spdknt + 0.1);           // addition is to counter any round-off error

         double mach = condition.mSpeed / GetScenario().GetAtmosphere().SonicVelocity(condition.mAlt);
         mOutFile << "      delta_altitude " << iAltM << " m # launchAlt - tgtAlt = " << iAltKft << " kft\n";
         mOutFile << "      launch_speed " << iSpeedM << " m/s # = " << iSpeedKnt << " knt, M=" << mach << '\n';
         mOutFile << "      use_zone " << condition.mZoneName << " as myZone" << std::endl;

         if ((condition.mBaselineRange != 0.0) && (condition.mPlusAltRange != 0.0) &&
             (condition.mPlusSpeedRange != 0.0) && (condition.mPlusLoftRange != 0.0))
         {
            mOutFile << "      sensitivity_range_per_10m_altitude "
                     << condition.mPlusAltRange - condition.mBaselineRange << " m\n";
            mOutFile << "      sensitivity_range_per_10ms_velocity "
                     << condition.mPlusSpeedRange - condition.mBaselineRange << " m\n";
            mOutFile << "      sensitivity_range_per_percent_gradient "
                     << condition.mPlusLoftRange - condition.mBaselineRange << " m\n";
         }
         mOutFile << "   end_launch_acceptable_region\n";
      }
      mOutFile << "end_launch_computer\n";
      mOutFile.close();
   }
}

// virtual
void ATG_LAR_And_LC_Generator::WriteZone()
{
   CreateZoneFromBools();

   AdjustLAR();

   double altScale = 1.0 / UtInput::ConvertValueFrom(1.0, mLaunchCond[mLaunchCondIndex].mInputAltUnits, UtInput::cLENGTH);
   int    iAlt     = UtMath::Round((float)(mCurrentAlt * altScale));

   double             spdScale = 1.0;
   std::string        spdStr;
   std::ostringstream stm;
   if (mLaunchCond[mLaunchCondIndex].mInputSpdUnits == "mach")
   {
      spdScale = 1.0 / GetScenario().GetAtmosphere().SonicVelocity(mCurrentAlt);
      stm.setf(std::ios_base::fixed);
      stm << mCurrentSpd * spdScale;
      spdStr = stm.str();
      replace(spdStr.begin(), spdStr.end(), '.', 'p');
      std::size_t found0 = spdStr.find_last_not_of('0');
      std::size_t foundp = spdStr.find_first_of('p');
      if (found0 == foundp)
      {
         found0 += 1;
      }
      spdStr.erase(found0 + 1);
   }
   else
   {
      spdScale = 1.0 / UtInput::ConvertValueFrom(1.0, mLaunchCond[mLaunchCondIndex].mInputSpdUnits, UtInput::cSPEED);
      stm << UtMath::Round((float)(mCurrentSpd * spdScale));
      spdStr = stm.str();
   }
   spdStr += mLaunchCond[mLaunchCondIndex].mInputSpdUnits;
   replace(spdStr.begin(), spdStr.end(), '/', 'p');

   std::ostringstream oss1;
   oss1 << "_" << iAlt << mLaunchCond[mLaunchCondIndex].mInputAltUnits << "_" << spdStr;
   std::string strAltAndSpeed(oss1.str());

   std::ostringstream oss2;
   oss2 << WeaponPlatformTypeId().GetString() << strAltAndSpeed;
   std::string zoneName(oss2.str());

   std::ostringstream oss3;
   oss3 << WeaponPlatformTypeId().GetString() << strAltAndSpeed;
   std::string larName(oss3.str());

   // output to the generated file requires a prefix of # for commenting and Warning:
   // to issue a warning. However, The logger util has its own prefixing so i have separated
   // the strings so appropriate use depending on output could occur.
   char        fileCommentChar = '#';
   std::string warningPrefix(" Warning: ");

   if (mHitAtShortestRange || mHitAtLongestRange)
   {
      std::string str1("LAR Generator encountered a weapon hit at the ");
      std::string str2("longest");
      if (mHitAtShortestRange)
      {
         str2 = "shortest";
      }
      std::string str3(" specified range.");
      std::string scream1(str1 + str2 + str3);
      std::string scream2("Recommend this weapon launch LAR be re-run, specifying a greater length of target ranges.");

      mOutFile << fileCommentChar << warningPrefix << scream1 << "\n";
      mOutFile << fileCommentChar << ' ' << scream2 << std::endl;

      auto logWarning = ut::log::warning() << scream1;
      logWarning.AddNote() << scream2;
   }

   if (mHitAtLargestOffset)
   {
      std::string scream3("LAR Generator encountered a weapon hit at the widest specified lateral offset.");
      std::string scream4(
         "Recommend this weapon launch LAR be re-run, specifying a greater breadth of target offsets.");

      mOutFile << fileCommentChar << warningPrefix << scream3 << "\n";
      mOutFile << fileCommentChar << ' ' << scream4 << std::endl;

      auto logWarning = ut::log::warning() << scream3;
      logWarning.AddNote() << scream4;
   }

   if (mWeaponTimedOut)
   {
      std::string scream5("LAR Generator encountered a weapon timeout, results may not be valid.");
      mOutFile << fileCommentChar << warningPrefix << scream5 << std::endl;
      ut::log::warning() << scream5;
   }

   // ===================================================
   // #1      Write Zone
   // ===================================================

   LocationPoints::size_type i = mPoints.size();

   if (mWithinLAR && (i >= 3))
   {
      mOutFile << "zone " << zoneName << std::endl;
      if (mShrinkFactor < 1.0)
      {
         mOutFile << "\n   # (shrink_factor applied was = " << mShrinkFactor << ".)" << std::endl;
      }
      mOutFile << mGenTime;
      mOutFile << "   polygonal" << std::endl;

      for (i = 0; i < mPoints.size(); ++i)
      {
         mOutFile << "   point " << mPoints[i].mX << " " << mPoints[i].mY << " m" << std::endl;
      }

      // Assume the Zone is symmetric about the X axis.
      // Warning:  An unsigned int will ALWAYS pass the criteria i >= 0, so had to change it a little.
      for (i = mPoints.size() - 2; i != 0; --i)
      {
         mOutFile << "   point " << mPoints[i].mX << " " << -(mPoints[i].mY) << " m" << std::endl;
      }
      // mOutFile << "   point " << mPoints[0].mX << " " << -(mPoints[0].mY) << " m" << endl;

      mOutFile << "end_zone" << std::endl;

      // ===================================================
      // #2           Write LAR
      // ===================================================

      LAR_Condition condition;
      condition.mLarName        = larName;
      condition.mZoneName       = zoneName;
      condition.mSpeed          = mCurrentSpd;
      condition.mAlt            = mCurrentAlt;
      condition.mBaselineRange  = mBaselineRange;
      condition.mPlusAltRange   = mPlusAltRange;
      condition.mPlusSpeedRange = mPlusSpeedRange;
      condition.mPlusLoftRange  = mPlusLoftRange;
      mConditions.push_back(condition);
   }
   else
   {
      std::string message(
         "ATG_LAR_And_LC_Generator.cpp did not encounter enough weapon hits -- *** RESULTS NOT VALID ***");
      mOutFile << message << std::endl;
      ut::log::error() << message;
   }
}


bool ATG_LAR_And_LC_Generator::PlaceNewTarget(double aSimTime, WsfTrack& aTrack)
{
   // Initialize the track.
   WsfTrackId trackId;
   trackId.SetOwningPlatformId("shooter");
   trackId.SetLocalTrackNumber(mQuantityLaunched + 1);
   double launchLocWCS[3];
   mLaunchPlatPtr->GetLocationWCS(launchLocWCS);
   aTrack.SetOriginatorLocationWCS(launchLocWCS);
   aTrack.Initialize(aSimTime, trackId, *GetSimulation());
   aTrack.Update(aSimTime);

   // Place a target in the desired location:

   // Use a UtEntity to process the relative geometry
   // between the launching platform and the target.
   UtEntity ute;
   ute.SetLocationLLA(mLatDeg, mLonDeg, 0.0);
   ute.SetOrientationNED(mHeadingRad, 0.0, 0.0);

   double launcherWCS[3];
   ute.GetLocationWCS(launcherWCS);

   double tgtECSOffset[3];
   tgtECSOffset[0] = CurrentRange();
   tgtECSOffset[1] = CurrentOffset();
   tgtECSOffset[2] = 0.0;

   double tgtWCSOffset[3];
   ute.ConvertECSVectorToWCS(tgtWCSOffset, tgtECSOffset);

   double newTgtLocWCS[3];
   UtVec3d::Add(newTgtLocWCS, launcherWCS, tgtWCSOffset);

   double tgtLat, tgtLon, tgtAlt;
   UtEntity::ConvertWCSToLLA(newTgtLocWCS, tgtLat, tgtLon, tgtAlt);

   // Now just to make sure the target is at the right (1 meter) altitude...
   // This is MSL altitude, and does not account for height of terrain!
   tgtAlt = 1.0;
   UtEntity::ConvertLLAToWCS(tgtLat, tgtLon, tgtAlt, newTgtLocWCS);

   WsfPlatform* tgtPtr = WsfPlatformTypes::Get(GetScenario()).Clone(mTargetPlatformTypeId);

   if (tgtPtr == nullptr)
   {
      auto logError = ut::log::error() << "ATG_LAR_And_LC_Generator::FireNewWeapon"
                                       << " could not clone a target platform of Type!";
      logError.AddNote() << "Type: " << mTargetPlatformTypeId.GetString();
      return false;
   }

   tgtPtr->SetLocationLLA(tgtLat, tgtLon, 1.0);
   WsfRouteMover* moverPtr = dynamic_cast<WsfRouteMover*>(tgtPtr->GetMover());
   if (moverPtr != nullptr)
   {
      WsfWaypoint wp(tgtLat, tgtLon, 0.0, 0.0); // alt & speed = 0
      WsfRoute    route;
      route.Append(wp);
      moverPtr->SetRoute(aSimTime, route);
   }
   tgtPtr->SetLocationWCS(newTgtLocWCS);

   // Orientation of target doesn't matter for our purposes...
   tgtPtr->SetOrientationNED(0.0, 0.0, 0.0);

   // Give the target a name...
   std::ostringstream oss;
   ++mTargetCount;
   oss << mTargetPlatformTypeId << "_ATG_" << mTargetCount;
   std::string tgtName(oss.str());
   tgtPtr->SetName(tgtName);
   GetSimulation()->AddPlatform(aSimTime, tgtPtr);
   mTgtIndex = tgtPtr->GetIndex();

   aTrack.SetLocationWCS(newTgtLocWCS);
   aTrack.SetWCS_LocationValid(true);
   double zero[] = {0.0, 0.0, 0.0};
   aTrack.SetVelocityWCS(zero);
   aTrack.SetVelocityValid(true);
   aTrack.SetTargetIndex(mTgtIndex);

   return true;
}


bool ATG_LAR_And_LC_Generator::FireNewWeapon(double aSimTime)
{
   bool success = true;

   // Create a new Target in the correct Location:
   WsfTrack tgtTrack;
   if (!PlaceNewTarget(aSimTime, tgtTrack))
   {
      success = false;
   }

   // Now initialize the initial conditions of the weapon:
   double alt   = mCurrentAlt;
   double pitch = 0.0;

   if (mState == cALT_DELTA)
   {
      alt += 10.0; // Sensitivity to additional 10 meters altitude.
   }

   if (mState == cLOFT_DELTA)
   {
      pitch += 0.01; // Sensitivity to 0.01 radian, or 0.573 degrees climb angle.
   }

   double vel = mCurrentSpd;

   if (mState == cSPD_DELTA)
   {
      vel += 10.0; // Sensitivity to additional 10 m/s velocity.
   }

   double platVelNED[3];
   platVelNED[0] = vel * cos(mHeadingRad);
   platVelNED[1] = vel * sin(mHeadingRad);
   platVelNED[2] = 0.0;

   if (mState == cLOFT_DELTA)
   {
      // Small angle approximation...
      platVelNED[2] = -vel * pitch;
   }

   // if (DebugEnabled())
   //{
   //    cout << "T=" << aSimTime << " ATG_LAR_And_LC_Generator attempting launch of a weapon of type "
   //         << WeaponPlatformTypeId().GetString() << endl;
   // }

   // Set the initial conditions of the launching platform:
   // This will continuously relocate the platform back to the starting point,
   // even though it is supposed to have a velocity... visually, it will "stutter"
   mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, mCurrentAlt);
   mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);
   mLaunchPlatPtr->SetVelocityNED(platVelNED);

   success = mLaunchWpnPtr->Fire(aSimTime, &tgtTrack, WsfWeapon::FireOptions());

   if (success)
   {
      ++mQuantityLaunched;
   }
   else
   {
      mObserver.LaunchAborted(aSimTime);
      if (mTerminateOnLaunchFailure)
      {
         ErrorBailout();
      }
   }

   return success;
}

//! Populates mPoints with the boundary surrounding the mResults shape, using Moore-Neighborhood Tracing.
//! @returns 'true' if a valid symmetrical half-boundary was created, starting and ending at mIOffset=0, otherwise returns 'false'.
bool ATG_LAR_And_LC_Generator::CreateLARBoundary()
{
   // Stores position checked just before last boundary was created. Starts to the top-left of the first position.
   std::pair<int, int> backtrack(mIRange - 1, mIOffset - 1);
   while (true)
   {
      bool                foundBoundary  = false;
      bool                foundBacktrack = false;
      std::pair<int, int> prevBoundary(mIRange, mIOffset);

      // Look for new hit points clockwise around the previous point, starting after the last point checked previously.
      auto path = GetClockwisePath(backtrack);
      for (const auto& tile : path)
      {
         // Skip adding invalid positions to the backtrack.
         if ((tile.first < 0) || (tile.second < 0) || (tile.first > (mNumRanges - 1)) ||
             (tile.second > (mNumTgtLateralOffsets - 1)))
         {
            continue;
         }

         // If next point in clockwise order is a hit, add to boundary and stop.
         if (AddPoint(tile.first, tile.second))
         {
            foundBoundary = true;
            // Point is at maximum range; adjust the backtrack point
            if (tile.first == (mNumRanges - 1))
            {
               backtrack.first  = mNumRanges;
               backtrack.second = tile.second;
            }
            break;
         }

         // Set the backtrack to the last position that was not chosen.
         if (!mResults[(tile.first * mNumTgtLateralOffsets) + tile.second])
         {
            backtrack      = tile;
            foundBacktrack = true;
         }
      }

      // If first tile checked was added to boundary, set backtrack to previous boundary instead.
      if (!foundBacktrack)
      {
         backtrack = prevBoundary;
      }

      // If new boundary is back to the original offset, boundary has finished being created.
      if (mIOffset == 0)
      {
         return true;
      }

      // If program could not find a next potential boundary point, creation has failed.
      if (!foundBoundary)
      {
         return false;
      }
   }
}

//! Get positions around the current position, ordered clockwise, starting just after the position aBacktrack.
//! This is meant to create the clockwise list of positions to check used in Moore-Neighborhood Tracing.
//! @param aBacktrack a (range, offset) pair representing the final position checked before the last boundary creation.
//! @returns A vector of (range, offset) positions around the current position (mIRange, mIOffset)
ATG_LAR_And_LC_Generator::ClockwisePath ATG_LAR_And_LC_Generator::GetClockwisePath(const std::pair<int, int>& aBacktrack) const
{
   // These represent the offsets of each position around the current position, starting at the top.
   constexpr std::array<int, 8> rangePattern{-1, -1, 0, 1, 1, 1, 0, -1};
   constexpr std::array<int, 8> offsetPattern{0, 1, 1, 1, 0, -1, -1, -1};

   // Find the index of the clock pattern that we should start with.
   size_t pathStart = ut::npos;
   for (size_t i = 0; i < offsetPattern.size(); i++)
   {
      if (rangePattern[i] + mIRange == aBacktrack.first && offsetPattern[i] + mIOffset == aBacktrack.second)
      {
         pathStart = i + 1;
         break;
      }
   }

   if (pathStart == ut::npos)
   {
      throw std::logic_error("Unexpected error occurred in ATG_LAR_And_LC_Generator::GetClockwisePath");
   }

   // Create clockwise path list starting at pathStart, with wraparound.
   std::array<std::pair<int, int>, 8> path;
   for (size_t j = 0; j < path.size(); j++)
   {
      size_t patternIndex = pathStart + j;
      // Wraparound index once it passes the end of the arrays.
      if (patternIndex > 7)
      {
         patternIndex -= 8;
      }
      path[j] = std::make_pair(mIRange + rangePattern[patternIndex], mIOffset + offsetPattern[patternIndex]);
   }

   return path;
}

bool ATG_LAR_And_LC_Generator::AddPoint(int aRange, int aOffset)
{
   // Returns false if point is not a hit result
   if (!HitResult(aRange, aOffset))
   {
      return false;
   }

   mIOffset = aOffset;
   mIRange  = aRange;
   mPoints.emplace_back(CurrentRange(), CurrentOffset());
   if (DebugEnabled())
   {
      auto logDebug = ut::log::debug() << "Added LocationPoint Number " << mPoints.size() - 1;
      logDebug.AddNote() << "Index (range, offset): " << mIRange << ", " << mIOffset;
      logDebug.AddNote() << "Range (m): " << CurrentRange();
      logDebug.AddNote() << "Offset (m): " << CurrentOffset();
   }

   return true;
}

void ATG_LAR_And_LC_Generator::CreateZoneFromBools()
{
   bool error = false;

   mPoints.clear();

   // Algorithm: Examine the array of booleans indicating weapon hits.
   // Beginning at the most aft centerline point, traverse to greater
   // range values until a hit is encountered. Then use this as the
   // first Zone point. After that, examine all points around the current
   // one for a "true" boolean, favoring the one most clockwise from the
   // current point.

   mIRange  = 0;
   mIOffset = 0;

   // Find the first hit at short range THAT ALSO HAS A HIT AT THE NEXT LONGER RANGE:
   while ((mIRange < mNumRanges) && (!HitResult(mIRange, mIOffset)))
   {
      ++mIRange;
      if (!HitResult(mIRange + 1, mIOffset))
      {
         ++mIRange;
      }
   }

   if (mIRange == (mNumRanges - 1))
   {
      ut::log::error() << "In ATG_LAR_And_LC_Generator::CreateZoneFromBools()... mIRange = max range index.";
      error = true;
   }
   else
   {
      mPoints.emplace_back(CurrentRange(), CurrentOffset());
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug();
         logDebug << "Added LocationPoint Number " << mPoints.size() - 1;
         logDebug.AddNote() << "Index (range, offset): " << mIRange << ", " << mIOffset;
         logDebug.AddNote() << "Range (m): " << CurrentRange();
         logDebug.AddNote() << "Offset (m): " << CurrentOffset();
      }
      error |= !CreateLARBoundary();
   }

   // ===============================================================

   if ((!error) && (DebugEnabled()))
   {
      const char cMISS  = '.';
      const char cHIT   = 'O';
      const char cZHIT  = '*';
      const char cZMISS = '?';

      auto logResults = ut::log::debug() << "ATG_LAR_And_LC_Generator::CreateZoneFromBools() Results:";
      logResults.AddNote() << "Key: Hit [" << cHIT << "], Miss [" << cMISS << "], Hit & Zone Point [" << cZHIT
                           << "], Miss & Zone Point [" << cZMISS << "]";

      std::stringstream rangeStr;
      for (mIRange = mNumRanges - 1; mIRange >= 0; --mIRange)
      {
         if (mIRange != (mNumRanges - 1))
         {
            logResults.AddNote() << rangeStr.str() << "Range (m): " << CurrentRange();
            rangeStr.str(std::string());
         }

         for (mIOffset = 0; mIOffset < mNumTgtLateralOffsets; ++mIOffset)
         {
            char result = cMISS;

            if (HitResult(mIRange, mIOffset))
            {
               result = cHIT;
            }

            for (auto& point : mPoints)
            {
               double x  = point.mX;
               double y  = point.mY;
               double cx = CurrentRange();
               double cy = CurrentOffset();

               if ((x == cx) && (y == cy))
               {
                  if (result == cHIT)
                  {
                     result = cZHIT;
                  }
                  else
                  {
                     result = cZMISS;
                  }
                  break;
               }
            }
            rangeStr << result << "  ";
         }
      } // end for mIRange
      logResults.AddNote() << rangeStr.str() << "Range (m): " << CurrentRange();

      std::stringstream zoneStr;
      logResults.AddNote() << "ZONE POINT ORDERING:";
      for (mIRange = mNumRanges - 1; mIRange >= 0; --mIRange)
      {
         for (mIOffset = 0; mIOffset < mNumTgtLateralOffsets; ++mIOffset)
         {
            int iPoint = -1;
            for (LocationPoints::size_type i = 0; i < mPoints.size(); ++i)
            {
               double x  = mPoints[i].mX;
               double y  = mPoints[i].mY;
               double cx = CurrentRange();
               double cy = CurrentOffset();

               if ((x == cx) && (y == cy))
               {
                  iPoint = (int)i;
                  break;
               }
            }

            if (iPoint == -1)
            {
               zoneStr << "   ";
            }
            else
            {
               zoneStr << iPoint + 10 << ' ';
            }
         }
         logResults.AddNote() << zoneStr.str() << "Range (m): " << CurrentRange();
         zoneStr.str(std::string());
      }
   }
}
