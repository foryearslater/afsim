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

#include "BallisticMissileLaunchComputerGenerator.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#include "Tool.hpp"
#include "UtInput.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfLaunchComputer.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"

namespace
{
// const WsfStringId myTypeId("ballistic_missile_launch_computer_generator");
const char* cBALLISTIC_MISSILE_LAUNCH_COMPUTER_DEFAULT_PRODUCT("_BALLISTIC_MISSILE_LAUNCH_COMPUTER");
} // namespace

// =================================================================================================
BallisticMissileLaunchComputerGenerator::BallisticMissileLaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mAirTargetFileName()
   , mAirTargetOfs()
   , mSurfaceTargetFileName()
   , mSurfaceTargetOfs()
   , mGnuplotFileName()
   , mGnuplotOfs()
   , mLoftAngles()
   , mBurnTimes()
   , mMinOutputTime(0.0)
   , mMaxOutputTime(1.0E+30)
   , mMinOutputRange(0.0)
   , mMaxOutputRange(1.0E+30)
   , mMaxOutputAlt(1.0E+30)
   , mLimitedToPreApogee(false)
   , mPassNumber(3)
   , mPass1_LoftAngleStart(70.0 * UtMath::cRAD_PER_DEG)
   , mPass1_LoftAngleStep(1.0 * UtMath::cRAD_PER_DEG)
   , mPass1_LoftAngleStop(89.99 * UtMath::cRAD_PER_DEG)
   , mPass2_LoftAngleStep(0.05 * UtMath::cRAD_PER_DEG)
   , mPass2_LoftAngleStop(0.0)
   , mLoftAngleIndex(0)
   , mBurnTimeIndex(0)
   , mLoftAngle(0.0)
   , mBurnTime(0.0)
   , mRange(0.0)
   , mMaxAltitude(0.0)
   , mRangeAtMaxAltitude(0.0)
   , mTimeAtMaxAltitude(0.0)
   , mBestLoftAngle(0.0)
   , mBestRange(0.0)
   , mLoftDepressAngles()
   , mUsingDepressedTraj(false)
   , mUsingLoftedTraj(false)
   , mLoftDepressAngleIndex(0)
   , mLoftDepressAnglesInitialized(false)
   , mLoftDepressAnglesMaxAngle(0.0)
   , mLoftDepressAnglesMinAngle(0.0)
   , mLoftDepressAnglesDeltaAngle(0.0)
   , mLoftDepressAnglesStepValue(0.0)
   , mWriteAirTargetData(true)
{
   mToolProduces       = cBALLISTIC_MISSILE_LAUNCH_COMPUTER_DEFAULT_PRODUCT;
   mWeaponEffectTypeId = nullptr; // Don't require weapon effects override
}

// =================================================================================================
BallisticMissileLaunchComputerGenerator::BallisticMissileLaunchComputerGenerator(
   const BallisticMissileLaunchComputerGenerator& aSrc)
   : Tool(aSrc)
   , mAirTargetFileName(aSrc.mAirTargetFileName)
   , mAirTargetOfs()
   , mSurfaceTargetFileName(aSrc.mSurfaceTargetFileName)
   , mSurfaceTargetOfs()
   , mGnuplotFileName(aSrc.mGnuplotFileName)
   , mGnuplotOfs()
   , mLoftAngles(aSrc.mLoftAngles)
   , mBurnTimes(aSrc.mBurnTimes)
   , mMinOutputTime(aSrc.mMinOutputTime)
   , mMaxOutputTime(aSrc.mMaxOutputTime)
   , mMinOutputRange(aSrc.mMinOutputRange)
   , mMaxOutputRange(aSrc.mMaxOutputRange)
   , mMaxOutputAlt(aSrc.mMaxOutputAlt)
   , mLimitedToPreApogee(aSrc.mLimitedToPreApogee)
   , mPassNumber(aSrc.mPassNumber)
   , mPass1_LoftAngleStart(aSrc.mPass1_LoftAngleStart)
   , mPass1_LoftAngleStep(aSrc.mPass1_LoftAngleStep)
   , mPass1_LoftAngleStop(aSrc.mPass1_LoftAngleStop)
   , mPass2_LoftAngleStep(aSrc.mPass2_LoftAngleStep)
   , mPass2_LoftAngleStop(aSrc.mPass2_LoftAngleStop)
   , mLoftAngleIndex(aSrc.mLoftAngleIndex)
   , mBurnTimeIndex(aSrc.mBurnTimeIndex)
   , mLoftAngle(aSrc.mLoftAngle)
   , mBurnTime(aSrc.mBurnTime)
   , mRange(aSrc.mRange)
   , mMaxAltitude(aSrc.mMaxAltitude)
   , mRangeAtMaxAltitude(aSrc.mRangeAtMaxAltitude)
   , mTimeAtMaxAltitude(aSrc.mTimeAtMaxAltitude)
   , mBestLoftAngle(aSrc.mBestLoftAngle)
   , mBestRange(aSrc.mBestRange)
   , mLoftDepressAngles(aSrc.mLoftDepressAngles)
   , mUsingDepressedTraj(aSrc.mUsingDepressedTraj)
   , mUsingLoftedTraj(aSrc.mUsingLoftedTraj)
   , mLoftDepressAngleIndex(aSrc.mLoftDepressAngleIndex)
   , mLoftDepressAnglesInitialized(aSrc.mLoftDepressAnglesInitialized)
   , mLoftDepressAnglesMaxAngle(aSrc.mLoftDepressAnglesMaxAngle)
   , mLoftDepressAnglesMinAngle(aSrc.mLoftDepressAnglesMinAngle)
   , mLoftDepressAnglesDeltaAngle(aSrc.mLoftDepressAnglesDeltaAngle)
   , mLoftDepressAnglesStepValue(aSrc.mLoftDepressAnglesStepValue)
   , mWriteAirTargetData(aSrc.mWriteAirTargetData)
{
   mWeaponEffectTypeId = nullptr; // Don't require weapon effects override
}

// =================================================================================================
BallisticMissileLaunchComputerGenerator::~BallisticMissileLaunchComputerGenerator() {}

// virtual
Tool* BallisticMissileLaunchComputerGenerator::Clone() const
{
   return new BallisticMissileLaunchComputerGenerator(*this);
}

// =================================================================================================
bool BallisticMissileLaunchComputerGenerator::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());

   if (command == "air_target_file")
   {
      aInput.ReadValueQuoted(mAirTargetFileName);
      mAirTargetFileName = aInput.SubstitutePathVariables(mAirTargetFileName);
   }
   else if (command == "surface_target_file")
   {
      aInput.ReadValueQuoted(mSurfaceTargetFileName);
      mSurfaceTargetFileName = aInput.SubstitutePathVariables(mSurfaceTargetFileName);
   }
   else if (command == "gnuplot_file")
   {
      aInput.ReadValueQuoted(mGnuplotFileName);
      mGnuplotFileName = aInput.SubstitutePathVariables(mGnuplotFileName);
   }
   else if (command == "minimum_output_time")
   {
      aInput.ReadValueOfType(mMinOutputTime, UtInput::cTIME);
      aInput.ValueGreaterOrEqual(mMinOutputTime, 0.0);
   }
   else if (command == "maximum_output_time")
   {
      aInput.ReadValueOfType(mMaxOutputTime, UtInput::cTIME);
      aInput.ValueGreater(mMaxOutputTime, mMinOutputTime);
   }
   else if (command == "minimum_output_range")
   {
      aInput.ReadValueOfType(mMinOutputRange, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(mMinOutputRange, 0.0);
   }
   else if (command == "maximum_output_range")
   {
      aInput.ReadValueOfType(mMaxOutputRange, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(mMaxOutputRange, 0.0);
   }
   else if (command == "maximum_output_altitude")
   {
      aInput.ReadValueOfType(mMaxOutputAlt, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(mMaxOutputAlt, 0.0);
   }
   else if (command == "limited_to_preapogee")
   {
      mLimitedToPreApogee = true;
   }
   else if (command == "loft_angle")
   {
      double value;
      aInput.ReadValueOfType(value, UtInput::cANGLE);
      aInput.ValueGreater(value, 0.0);
      aInput.ValueLessOrEqual(value, UtMath::cPI_OVER_2);
      mLoftAngles.push_back(value);
   }
   else if (command == "loft_angles")
   {
      double      minValue;
      double      maxValue;
      double      stepValue;
      std::string word;
      aInput.ReadValue(word);
      aInput.StringEqual(word, "from");
      aInput.ReadValueOfType(minValue, UtInput::cANGLE);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "to");
      aInput.ReadValueOfType(maxValue, UtInput::cANGLE);
      aInput.ValueGreater(maxValue, minValue);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cANGLE);
      aInput.ValueGreater(stepValue, 0.0);
      for (double value = minValue; value <= (maxValue + 1.0E-8); value += stepValue)
      {
         mLoftAngles.push_back(value);
      }
   }
   else if (command == "burn_time")
   {
      double value;
      aInput.ReadValueOfType(value, UtInput::cTIME);
      aInput.ValueGreater(value, 0.0);
      mBurnTimes.push_back(value);
   }
   else if (command == "burn_times")
   {
      double      minValue;
      double      maxValue;
      double      stepValue;
      std::string word;
      aInput.ReadValue(word);
      aInput.StringEqual(word, "from");
      aInput.ReadValueOfType(minValue, UtInput::cTIME);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "to");
      aInput.ReadValueOfType(maxValue, UtInput::cTIME);
      aInput.ValueGreater(maxValue, minValue);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cTIME);
      aInput.ValueGreater(stepValue, 0.0);
      for (double value = minValue; value <= (maxValue + 1.0E-8); value += stepValue)
      {
         mBurnTimes.push_back(value);
      }
   }
   else if (command == "loft_angle_range")
   {
      double      maxAngle  = 0.0;
      double      maxValue  = 0.0;
      double      stepValue = 0.0;
      std::string word;
      aInput.ReadValue(word);

      if (!((word == "max_angle") || (word == "maxAngle"))) // deprecated, needed for backward compatibility.
      {
         throw UtInput::BadValue(aInput, "Keyword not recognized:  " + word + ".");
      }
      aInput.ReadValueOfType(maxAngle, UtInput::cANGLE);
      aInput.ValueGreater(maxAngle, 0.0);

      mLoftDepressAnglesMaxAngle = maxAngle;

      aInput.ReadValue(word);
      if (!((word == "max_loft") || (word == "maxLoft"))) // deprecated, needed for backward compatibility.
      {
         throw UtInput::BadValue(aInput, "Keyword not recognized:  " + word + ".");
      }
      aInput.ReadValueOfType(maxValue, UtInput::cANGLE);
      aInput.ValueGreater(maxValue, 0.0);

      mLoftDepressAnglesDeltaAngle = maxValue;

      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cANGLE);
      aInput.ValueGreater(stepValue, 0.0);

      mLoftDepressAnglesStepValue = stepValue;

      mUsingLoftedTraj = true;
   }
   else if (command == "depress_angle_range")
   {
      double      minAngle  = 0.0;
      double      maxValue  = 0.0;
      double      stepValue = 0.0;
      std::string word;

      aInput.ReadValue(word);
      if (!((word == "min_angle") || (word == "minAngle"))) // deprecated, needed for backward compatibility.
      {
         throw UtInput::BadValue(aInput, "Keyword not recognized:  " + word + ".");
      }
      aInput.ReadValueOfType(minAngle, UtInput::cANGLE);
      aInput.ValueGreater(minAngle, 0.0);
      mLoftDepressAnglesMinAngle = minAngle;

      aInput.ReadValue(word);
      if (!((word == "max_depress") || (word == "maxDepress"))) // deprecated, needed for backward compatibility.
      {
         throw UtInput::BadValue(aInput, "Keyword not recognized:  " + word + ".");
      }
      aInput.ReadValueOfType(maxValue, UtInput::cANGLE);
      aInput.ValueGreater(maxValue, 0.0);

      mLoftDepressAnglesDeltaAngle = maxValue;

      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cANGLE);
      aInput.ValueGreater(stepValue, 0.0);

      mLoftDepressAnglesStepValue = stepValue;

      mUsingDepressedTraj = true;
   }
   else if (command == "pass_1_loft_angle_start")
   {
      aInput.ReadValueOfType(mPass1_LoftAngleStart, UtInput::cANGLE);
      aInput.ValueGreater(mPass1_LoftAngleStart, 0.0);
      aInput.ValueLess(mPass1_LoftAngleStart, 89.0 * UtMath::cRAD_PER_DEG);
   }
   else if (command == "pass_1_loft_angle_stop")
   {
      aInput.ReadValueOfType(mPass1_LoftAngleStop, UtInput::cANGLE);
      aInput.ValueGreater(mPass1_LoftAngleStop, 0.0);
      aInput.ValueLess(mPass1_LoftAngleStop, 89.999 * UtMath::cRAD_PER_DEG);
   }
   else if (command == "pass_1_loft_angle_step")
   {
      aInput.ReadValueOfType(mPass1_LoftAngleStep, UtInput::cANGLE);
      aInput.ValueGreater(mPass1_LoftAngleStep, 0.0);
      aInput.ValueLess(mPass1_LoftAngleStep, 1.0 * UtMath::cRAD_PER_DEG);
   }
   else if (command == "pass_2_loft_angle_stop")
   {
      aInput.ReadValueOfType(mPass2_LoftAngleStop, UtInput::cANGLE);
      aInput.ValueGreater(mPass2_LoftAngleStop, 0.0);
      aInput.ValueLess(mPass2_LoftAngleStop, 89.999 * UtMath::cRAD_PER_DEG);
   }
   else if (command == "pass_2_loft_angle_step")
   {
      aInput.ReadValueOfType(mPass2_LoftAngleStep, UtInput::cANGLE);
      aInput.ValueGreater(mPass2_LoftAngleStep, 0.0);
      aInput.ValueLess(mPass2_LoftAngleStep, 1.0 * UtMath::cRAD_PER_DEG);
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}

// =================================================================================================
// virtual
bool BallisticMissileLaunchComputerGenerator::Initialize(WsfSimulation& aSimulation)
{
   bool ok = Tool::Initialize(aSimulation);

   if (mBurnTimes.empty())
   {
      mBurnTimes.push_back(0.0);
   }
   if (mAirTargetFileName.empty() && mSurfaceTargetFileName.empty())
   {
      ut::log::error() << "Must specify either air_target_file or surface_target_file!";
      ok = false;
   }
   else if ((!mAirTargetFileName.empty()) && (!mSurfaceTargetFileName.empty()))
   {
      ut::log::error() << "Cannot specify both air_target_file and surface_target_file!";
      ok = false;
   }

   if (mLoftAngles.empty())
   {
      if (mSurfaceTargetFileName.empty())
      {
         ut::log::error() << "loft_angle(s) must be provided for surface-to-air mode!";
         ok = false;
      }
      else
      {
         mPassNumber = 0;
         ut::log::warning() << "No loft angles were provided - the best loft angle will be determined.";
         if (mBurnTimes.empty())
         {
            ut::log::error() << "burn_time(s) must be provided for loft angle search mode.";
            ok = false;
         }
      }
   }

   WsfLaunchComputer* computerPtr = new WsfLaunchComputer();
   computerPtr->SetComputerGenerationMode(true);
   if (!computerPtr->Initialize(aSimulation.GetSimTime(), mLaunchWpnPtr))
   {
      delete computerPtr;
      ut::log::error() << "Failed to initialize launch computer!";
      ok = false;
   }
   else
   {
      mLaunchWpnPtr->SetLaunchComputer(computerPtr);
   }

   if (!ok)
   {
      return ok;
   }

   mLoftAngleIndex        = 0;
   mBurnTimeIndex         = 0;
   mLoftDepressAngleIndex = 0;

   mObserver.ResetState();

   std::sort(mLoftAngles.begin(), mLoftAngles.end());
   std::sort(mBurnTimes.begin(), mBurnTimes.end());

   // Open output files and write headers.

   time_t clock;
   time(&clock);
   struct tm* tmPtr = localtime(&clock);

   if (!mAirTargetFileName.empty())
   {
      mAirTargetOfs.open(mAirTargetFileName.c_str());
      if (mAirTargetOfs)
      {
         mAirTargetOfs << "#Air target launch data for " << mLaunchWpnPtr->GetType() << '\n';
         mAirTargetOfs << "#Generated on " << asctime(tmPtr);
         mAirTargetOfs << "#ground_range altitude time_of_flight speed flight_path_angle\n";
         mAirTargetOfs << std::fixed;
      }
      else
      {
         auto logError = ut::log::error() << "Unable to open File!";
         logError.AddNote() << "File: " << mAirTargetFileName;
         ok = false;
      }
   }
   if (!mSurfaceTargetFileName.empty())
   {
      mSurfaceTargetOfs.open(mSurfaceTargetFileName.c_str());
      if (mSurfaceTargetOfs)
      {
         mSurfaceTargetOfs << "#Ground target launch data for " << mLaunchWpnPtr->GetType() << '\n';
         mSurfaceTargetOfs << "#Generated on " << asctime(tmPtr);
         mSurfaceTargetOfs << "#loft_angle burn_time ground_range time_of_flight impact_speed impact_angle";
         mSurfaceTargetOfs << " burnout_time burnout_range burnout_speed burnout_flight_path_angle\n"; // recent addition
         mSurfaceTargetOfs << std::fixed;
      }
      else
      {
         auto logError = ut::log::error() << "Unable to open File!";
         logError.AddNote() << "File: " << mSurfaceTargetFileName;
         ok = false;
      }
   }
   if (!mGnuplotFileName.empty())
   {
      mGnuplotOfs.open(mGnuplotFileName.c_str());
      if (mGnuplotOfs)
      {
         mGnuplotOfs << "#Trajectory data for " << mLaunchWpnPtr->GetType() << '\n';
         mGnuplotOfs << "#Generated on " << asctime(tmPtr);
         mGnuplotOfs << "#time ground_range altitude speed\n";
         mGnuplotOfs << std::fixed;
      }
      else
      {
         auto logError = ut::log::error() << "Unable to open File!";
         logError.AddNote() << "File: " << mGnuplotFileName;
         ok = false;
      }
   }

   return ok;
}

// =================================================================================================
// virtual
void BallisticMissileLaunchComputerGenerator::Update(double aSimTime)
{
   WeaponObserver::EngagementState state = mObserver.GetState();
   if (state == WeaponObserver::cIN_FLIGHT)
   {
      WsfPlatform* weaponPtr = mObserver.GetWeaponPlatform();
      if (weaponPtr != nullptr)
      {
         weaponPtr->Update(aSimTime);
         double weaponLat, weaponLon, weaponAlt;
         weaponPtr->GetLocationLLA(weaponLat, weaponLon, weaponAlt);
         double launchLat, launchLon, launchAlt;
         double heading, groundRange;
         mObserver.GetLaunchLocLLA(launchLat, launchLon, launchAlt);
         UtSphericalEarth::GreatCircleHeadingAndDistance(launchLat, launchLon, weaponLat, weaponLon, heading, groundRange);
         double timeOfFlight = aSimTime - mObserver.GetLaunchTime();
         if (weaponAlt > mMaxAltitude)
         {
            mMaxAltitude        = weaponAlt;
            mRangeAtMaxAltitude = groundRange;
            mTimeAtMaxAltitude  = timeOfFlight;
         }
         else
         {
            if (mLimitedToPreApogee)
            {
               // Stop writing output data once we reach apogee
               mWriteAirTargetData = false;
               mObserver.SetAllDone();
            }
         }
         if (mWriteAirTargetData && (timeOfFlight >= mMinOutputTime) && (groundRange >= mMinOutputRange))
         {
            if ((timeOfFlight <= mMaxOutputTime) && (groundRange <= mMaxOutputRange) && (weaponAlt <= mMaxOutputAlt))
            {
               double velNED[3];
               weaponPtr->GetVelocityNED(velNED);
               double flightPathAngle = atan2(-velNED[2], sqrt((velNED[0] * velNED[0]) + (velNED[1] * velNED[1])));
               mAirTargetOfs << ' ' << std::setprecision(2) << groundRange;
               mAirTargetOfs << ' ' << std::setprecision(2) << weaponAlt;
               mAirTargetOfs << ' ' << std::setprecision(4) << timeOfFlight;
               mAirTargetOfs << ' ' << std::setprecision(2) << weaponPtr->GetSpeed();
               mAirTargetOfs << ' ' << std::setprecision(4) << flightPathAngle * UtMath::cDEG_PER_RAD;
               mAirTargetOfs << '\n';
            }
            else
            {
               mWriteAirTargetData = false;
            }
         }
         if (!mGnuplotFileName.empty())
         {
            mGnuplotOfs << std::setprecision(4) << timeOfFlight;
            mGnuplotOfs << ' ' << std::setprecision(2) << groundRange;
            mGnuplotOfs << ' ' << std::setprecision(2) << weaponAlt;
            mGnuplotOfs << ' ' << weaponPtr->GetSpeed();
            mGnuplotOfs << '\n';
         }
      }
   }
   else if (state == WeaponObserver::cTERMINATED)
   {
      double weaponLat, weaponLon, weaponAlt;
      mObserver.GetImpactLocLLA(weaponLat, weaponLon, weaponAlt);
      double launchLat, launchLon, launchAlt;
      mObserver.GetLaunchLocLLA(launchLat, launchLon, launchAlt);
      double heading, groundRange;
      UtSphericalEarth::GreatCircleHeadingAndDistance(launchLat, launchLon, weaponLat, weaponLon, heading, groundRange);
      mRange              = groundRange; // Save for multi-pass loft angle search
      double timeOfFlight = mObserver.TimeOfFlight();
      if (!mAirTargetFileName.empty())
      {
         mAirTargetOfs << "-1.00 -1.00 -1.0000 -1.00 -1.0000" << std::endl; // Terminate this trajectory
      }
      if ((mPassNumber == 3) && (!mSurfaceTargetFileName.empty()))
      {
         double boTimeOfFlight;
         double boGreatCircleRange;
         double boAltitude;
         double boSpeed;
         double boFlightPathAngle;
         mObserver.GetBurnOutInfo(boTimeOfFlight, boGreatCircleRange, boAltitude, boSpeed, boFlightPathAngle);
         // clang-format off
         mSurfaceTargetOfs << std::setprecision(4) << mLoftAngle * UtMath::cDEG_PER_RAD
                           << ' ' << std::setprecision(4) << mBurnTime
                           << ' ' << std::setprecision(2) << groundRange
                           << ' ' << std::setprecision(4) << timeOfFlight
                           << ' ' << std::setprecision(2) << mObserver.GetFinalSpeed()
                           << ' ' << std::setprecision(4) << mObserver.GetFinalFlightPathAngle() * UtMath::cDEG_PER_RAD
                           << ' ' << std::setprecision(4) << boTimeOfFlight                             // recent addition
                           << ' ' << std::setprecision(4) << boGreatCircleRange                         // recent addition
                           << ' ' << std::setprecision(4) << boSpeed                                    // recent addition
                           << ' ' << std::setprecision(4) << boFlightPathAngle * UtMath::cDEG_PER_RAD   // recent addition
                           << std::endl;
         // clang-format on
      }
      if (!mGnuplotFileName.empty())
      {
         mGnuplotOfs << "\n\n"; // Terminate the dataset (two blank records)
      }
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "Terminated Range!";
         logDebug.AddNote() << "T = " << std::setprecision(3) << aSimTime;
         logDebug.AddNote() << "Terminated Range: " << std::setprecision(2) << groundRange << " m";
         logDebug.AddNote() << "Time of flight: " << std::setprecision(4) << timeOfFlight << " sec";
         logDebug.AddNote() << "Apogee: " << std::setprecision(2) << mMaxAltitude << " m";
         logDebug.AddNote() << "Range @apogee: " << mRangeAtMaxAltitude;
         logDebug.AddNote() << "Time @apogee: " << std::setprecision(4) << mTimeAtMaxAltitude;
      }
      mObserver.ResetState();
   }
   else if (state == WeaponObserver::cIDLE)
   {
      mObserver.ResetState();
      FireNewWeapon(aSimTime);
      if (mDone)
      {
         if (!mAirTargetFileName.empty())
         {
            mAirTargetOfs.close();
         }
         if (!mSurfaceTargetFileName.empty())
         {
            mSurfaceTargetOfs.close();
         }
         if (!mGnuplotFileName.empty())
         {
            mGnuplotOfs.close();
         }
      }
   }
   Tool::Update(aSimTime);
}

// =================================================================================================
// virtual
bool BallisticMissileLaunchComputerGenerator::FireNewWeapon(double aSimTime)
{
   mBurnTime = 0.0; // Assume pass 1 or 2

   if (mPassNumber == 0)
   {
      mLoftAngle     = mPass1_LoftAngleStart - mPass1_LoftAngleStep;
      mPassNumber    = 1;
      mBestRange     = 0.0;
      mBestLoftAngle = 0.0;
      mRange         = -1.0;
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "Starting Pass 1 to find best loft angle.";
         logDebug.AddNote() << "Searching from ";
         logDebug.AddNote() << mPass1_LoftAngleStart * UtMath::cDEG_PER_RAD << " deg to ";
         logDebug.AddNote() << mPass1_LoftAngleStop * UtMath::cDEG_PER_RAD << " deg with step size of ";
         logDebug.AddNote() << mPass1_LoftAngleStep * UtMath::cDEG_PER_RAD << " deg.";
      }
   }

   if (mPassNumber == 1)
   {
      // Note: once the range decreases by a 1km then we'll stop looking.
      //       Slight variations may cause a miss the peak.
      if (mRange < (mBestRange - 1000.0)) // Switch to next phase
      {
         mPass2_LoftAngleStop = mLoftAngle;
         mLoftAngle           = mBestLoftAngle - mPass1_LoftAngleStep - mPass2_LoftAngleStep;
         mPassNumber          = 2;
         mBestRange           = 0.0;
         mBestLoftAngle       = 0.0;
         mRange               = -1.0;
         if (DebugEnabled())
         {
            auto logDebug = ut::log::debug() << "Starting Pass 2 to find best loft angle.";
            logDebug.AddNote() << "Searching from " << (mLoftAngle + mPass2_LoftAngleStep) * UtMath::cDEG_PER_RAD
                               << " deg to ";
            logDebug.AddNote() << mPass2_LoftAngleStop * UtMath::cDEG_PER_RAD << " deg with step size of ";
            logDebug.AddNote() << mPass2_LoftAngleStep * UtMath::cDEG_PER_RAD << " deg.";
         }
      }
      else // Keep looking
      {
         if (mRange >= mBestRange)
         {
            mBestRange     = mRange;
            mBestLoftAngle = mLoftAngle;
         }
         mLoftAngle += mPass1_LoftAngleStep;
         if (mLoftAngle > (mPass1_LoftAngleStop + 1.0E-8))
         {
            mPass2_LoftAngleStop = 89.99 * UtMath::cRAD_PER_DEG;
            mLoftAngle           = mBestLoftAngle - 2.0 * mPass1_LoftAngleStep;
            mPassNumber          = 2;
            mBestRange           = 0.0;
            mBestLoftAngle       = 0.0;
            mRange               = -1.0;
            if (DebugEnabled())
            {
               auto logDebug = ut::log::debug() << "Pass 1 didn't find a maximum.";
               logDebug.AddNote() << "Starting Pass 2 to find best loft angle";
               logDebug.AddNote() << "Searching from ";
               logDebug.AddNote() << (mLoftAngle + mPass2_LoftAngleStep) * UtMath::cDEG_PER_RAD << " deg to ";
               logDebug.AddNote() << mPass2_LoftAngleStop * UtMath::cDEG_PER_RAD << " deg with step size of ";
               logDebug.AddNote() << mPass2_LoftAngleStep * UtMath::cDEG_PER_RAD << " deg.";
            }
         }
      }
   }

   if (mPassNumber == 2)
   {
      // Note: once the range decreases by a 1km then we'll stop looking.
      //       Slight variations may cause a miss the peak.
      if (mRange < (mBestRange - 1000.0)) // Switch to next phase
      {
         mLoftAngles.push_back(mBestLoftAngle);
         mPassNumber = 3;
         if (DebugEnabled())
         {
            auto logDebug = ut::log::debug() << "Starting Pass 3";
            logDebug.AddNote() << "Loft Angle: " << mBestLoftAngle * UtMath::cDEG_PER_RAD << " deg";
         }
      }
      else // keep looking
      {
         if (mRange >= mBestRange)
         {
            mBestRange     = mRange;
            mBestLoftAngle = mLoftAngle;
         }
         mLoftAngle += mPass2_LoftAngleStep;
         if (mLoftAngle > (mPass2_LoftAngleStop + 1.0E-8))
         {
            if (mBestLoftAngle <= 0.0)
            {
               ut::log::fatal() << "Didn't find maximum during pass 2!";
               exit(1);
            }
            mLoftAngles.push_back(mBestLoftAngle);
            mPassNumber   = 3;
            auto logDebug = ut::log::debug();
            if (DebugEnabled())
            {
               logDebug << "Starting Pass 3";
               logDebug.AddNote() << "Using loft angle of " << mBestLoftAngle * UtMath::cDEG_PER_RAD << " deg.";
            }
         }
      }
   }

   auto logDebug = ut::log::debug();
   if (mPassNumber == 3)
   {
      if (mUsingLoftedTraj || mUsingDepressedTraj) // This is only used for lofted or depressed trajectories
      {
         // If not yet initialized, initialize the loft/depress angles
         if (!mLoftDepressAnglesInitialized)
            InitLoftDepressAngles();

         mBurnTimeIndex  = 0;
         mLoftAngleIndex = 0;
         mBurnTime       = mBurnTimes[mBurnTimeIndex];

         mLoftAngle = mLoftDepressAngles[mLoftDepressAngleIndex];

         mLoftDepressAngleIndex++;

         if (DebugEnabled())
         {
            if (mUsingLoftedTraj)
            {
               auto logNote = logDebug.AddNote() << "Lofted Traj: ";
               logNote.AddNote() << "Loft Angle: " << std::setprecision(7) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
               logNote.AddNote() << "Index: " << mLoftDepressAnglesInitialized - 1;
            }
            else if (mUsingDepressedTraj)
            {
               auto logNote = logDebug.AddNote() << "Depressed Traj: ";
               logNote.AddNote() << "Loft Angle: " << std::setprecision(7) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
               logNote.AddNote() << "Index: " << mLoftDepressAnglesInitialized - 1;
            }
         }

         if (mLoftDepressAngleIndex >= mLoftDepressAngles.size())
         {
            mDone = true;
            mObserver.SetAllDone();
            if (DebugEnabled())
            {
               logDebug.AddNote() << "Complete with mLoftDepressAngleIndex out of limits.";
            }
            return false;
         }
      }
      else
      {
         if (mBurnTimeIndex >= mBurnTimes.size())
         {
            mBurnTimeIndex = 0;
            ++mLoftAngleIndex;
         }
         if (mLoftAngleIndex >= mLoftAngles.size())
         {
            mDone = true;
            mObserver.SetAllDone();
            return false;
         }

         mLoftAngle = mLoftAngles[mLoftAngleIndex];
         mBurnTime  = mBurnTimes[mBurnTimeIndex];
         ++mBurnTimeIndex;
      }
   }

   mRange              = 0.0;
   mMaxAltitude        = 0.0;
   mRangeAtMaxAltitude = 0.0;

   // Set the commanded values in the launch computer.
   WsfLaunchComputer* computerPtr = mLaunchWpnPtr->GetLaunchComputer();
   computerPtr->ClearCommandedValues();
   computerPtr->SetCommandedLoftAngle(mLoftAngle);

   if (DebugEnabled())
   {
      logDebug.AddNote() << "Loft Angle: " << std::setprecision(7) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg ";
   }

   if (mBurnTime > 0.0)
   {
      if (DebugEnabled())
      {
         logDebug.AddNote() << "Burn Time: " << mBurnTime << " sec";
      }
      computerPtr->SetCommandedBurnTime(mBurnTime);
   }

   // Form the track needed for this launch.
   WsfTrack tgtTrack;
   tgtTrack.Initialize(aSimTime, mLaunchWpnPtr->GetPlatform()->GetNextTrackId(), *GetSimulation());

   // The base class time-to-intercept call simply propagates the commanded values to the current value.
   computerPtr->EstimatedTimeToIntercept(aSimTime, tgtTrack, 0.0);

   bool ok = mLaunchWpnPtr->Fire(aSimTime, &tgtTrack, WsfWeapon::FireOptions());
   if (ok)
   {
      if (DebugEnabled())
      {
         auto logLaunch = logDebug.AddNote() << "Launch Data: ";
         logLaunch.AddNote() << "T = " << std::setprecision(3) << aSimTime;
         logLaunch.AddNote() << "Loft Angle: " << std::setprecision(6) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
         logLaunch.AddNote() << "Burn Time: " << std::setprecision(6) << mBurnTime << " secs";
      }
   }
   else
   {
      auto logLaunch = logDebug.AddNote() << "Launch Failed!";
      logLaunch.AddNote() << "T = " << std::setprecision(3) << aSimTime;
      logLaunch.AddNote() << "Loft Angle: " << std::setprecision(6) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
      logLaunch.AddNote() << "Burn Time: " << std::setprecision(6) << mBurnTime << " secs";
   }

   mWriteAirTargetData = false;
   if (!mAirTargetFileName.empty())
   {
      mAirTargetOfs << std::setprecision(6) << mLoftAngle * UtMath::cDEG_PER_RAD << "\n";
      mWriteAirTargetData = true;
   }
   if (!mGnuplotFileName.empty())
   {
      mGnuplotOfs << "# loft angle: " << std::setprecision(6) << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
      mGnuplotOfs << ", burn time: " << std::setprecision(6) << mBurnTime << " secs\n";
   }
   return ok;
}

void BallisticMissileLaunchComputerGenerator::InitLoftDepressAngles()
{
   double intialLoftedAngle = mLoftAngles[0];
   auto   logDebug          = ut::log::debug();
   if (DebugEnabled())
   {
      logDebug << "BallisticMissileLaunchComputerGenerator::InitLoftDepressAngles()";
   }

   if (mUsingLoftedTraj)
   {
      double potentialMaxAngle = intialLoftedAngle + mLoftDepressAnglesDeltaAngle;

      if (DebugEnabled())
      {
         auto logLoft = ut::log::debug() << "mUsingLoftedTraj:";
         logLoft.AddNote() << "Initial Lofted Angle: " << std::setprecision(9)
                           << intialLoftedAngle * UtMath::cDEG_PER_RAD << " deg";
         logLoft.AddNote() << "Loft Depress Angles Delta Angle: " << std::setprecision(9)
                           << mLoftDepressAnglesDeltaAngle * UtMath::cDEG_PER_RAD << " deg";
         logLoft.AddNote() << "Loft Depress Angles Max Angle: " << std::setprecision(9)
                           << mLoftDepressAnglesMaxAngle * UtMath::cDEG_PER_RAD << " deg";
         logLoft.AddNote() << "Potential Max Angle: " << std::setprecision(9)
                           << potentialMaxAngle * UtMath::cDEG_PER_RAD << " deg";
      }

      if (potentialMaxAngle > mLoftDepressAnglesMaxAngle)
      {
         potentialMaxAngle = mLoftDepressAnglesMaxAngle;
      }

      auto logNote = logDebug.AddNote();

      if (DebugEnabled())
      {
         logNote << "Potential Max Angle(adjusted): " << std::setprecision(9)
                 << potentialMaxAngle * UtMath::cDEG_PER_RAD << " deg";
      }

      for (double value = potentialMaxAngle; value >= intialLoftedAngle; value -= mLoftDepressAnglesStepValue)
      {
         if (DebugEnabled())
         {
            auto logValueNote = logNote.AddNote() << "Value(push):";
            logValueNote.AddNote() << std::setprecision(9) << value * UtMath::cDEG_PER_RAD << " deg";
         }
         mLoftDepressAngles.push_back(value);
      }
   }
   else if (mUsingDepressedTraj)
   {
      double potentialMinAngle = intialLoftedAngle - mLoftDepressAnglesDeltaAngle;

      if (DebugEnabled())
      {
         auto logNote = logDebug << "Using Lofted Traj:";
         logNote.AddNote() << "Initial Lofted Angle: " << std::setprecision(9)
                           << intialLoftedAngle * UtMath::cDEG_PER_RAD << " deg";
         logNote.AddNote() << "Loft Depress Angles Delta Angle: " << std::setprecision(9)
                           << mLoftDepressAnglesDeltaAngle * UtMath::cDEG_PER_RAD << " deg";
         logNote.AddNote() << "Loft Depress Angles Min Angle: " << std::setprecision(9)
                           << mLoftDepressAnglesMinAngle * UtMath::cDEG_PER_RAD << " deg";
         logNote.AddNote() << "potentialMinAngle: " << std::setprecision(9) << potentialMinAngle * UtMath::cDEG_PER_RAD
                           << " deg";
      }

      if (potentialMinAngle < mLoftDepressAnglesMinAngle)
      {
         potentialMinAngle = mLoftDepressAnglesMinAngle;
      }

      auto logNote = logDebug.AddNote();
      if (DebugEnabled())
      {
         logNote << "Potential Min Angle(adjusted): " << std::setprecision(9)
                 << potentialMinAngle * UtMath::cDEG_PER_RAD << " deg";
      }


      for (double value = potentialMinAngle; value <= intialLoftedAngle; value += mLoftDepressAnglesStepValue)
      {
         if (DebugEnabled())
         {
            auto logValueNote = logNote.AddNote() << "Value(pushed):";
            logValueNote.AddNote() << std::setprecision(9) << value * UtMath::cDEG_PER_RAD << " deg";
         }
         mLoftDepressAngles.push_back(value);
      }
   }

   // Set flag to show intialized
   mLoftDepressAnglesInitialized = true;
}
