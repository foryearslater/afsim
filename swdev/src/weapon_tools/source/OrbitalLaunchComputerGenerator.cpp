// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company
// ****************************************************************************

#include "OrbitalLaunchComputerGenerator.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#include "Tool.hpp"
#include "UtEarth.hpp"
#include "UtInput.hpp"
#include "UtMath.hpp"
#include "UtScriptContext.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfGuidanceProgram.hpp"
#include "WsfLaunchComputer.hpp"
#include "WsfMover.hpp"
#include "WsfOrbitalLaunchComputer.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfWeaponEngagement.hpp"
#include "script/WsfScriptContext.hpp"

namespace
{
// const WsfStringId myTypeId("orbital_launch_computer_generator");
const char* cORBITAL_LAUNCH_COMPUTER_DEFAULT_PRODUCT("_ORBITAL_LAUNCH_COMPUTER");
} // namespace

// =================================================================================================
OrbitalLaunchComputerGenerator::OrbitalLaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mSpeed(0.0)
   , mPitch(0.0)
   , mLaunchHeadings()
   , mLaunchHeading(0.0)
   , mLaunchHeadingIndex(0)
   , mLoftAngles()
   , mLoftAngle(0.0)
   , mLoftAngleIndex(0)
   , mPitchRates()
   , mPitchRate(0.0)
   , mPitchRateIndex(0)
   , mLEO_Data()
   , mBO_Data()
   , mLEO_Orbits()
   , mBO_Orbits()
   , mPlatformName()
{
   mToolProduces       = cORBITAL_LAUNCH_COMPUTER_DEFAULT_PRODUCT;
   mWeaponEffectTypeId = nullptr; // Don't require weapon effects override
}

// =================================================================================================
OrbitalLaunchComputerGenerator::OrbitalLaunchComputerGenerator(const OrbitalLaunchComputerGenerator& aSrc)
   : Tool(aSrc)
   , mLEO_DataFileName(aSrc.mOutputFileName)
   , mSpeed(aSrc.mSpeed)
   , mPitch(aSrc.mPitch)
   , mLaunchHeadings(aSrc.mLaunchHeadings)
   , mLaunchHeading(aSrc.mLaunchHeading)
   , mLaunchHeadingIndex(aSrc.mLaunchHeadingIndex)
   , mLoftAngles(aSrc.mLoftAngles)
   , mLoftAngle(aSrc.mLoftAngle)
   , mLoftAngleIndex(aSrc.mLoftAngleIndex)
   , mPitchRates(aSrc.mPitchRates)
   , mPitchRate(aSrc.mPitchRate)
   , mPitchRateIndex(aSrc.mPitchRateIndex)
   , mLEO_Data(aSrc.mLEO_Data)
   , mBO_Data(aSrc.mBO_Data)
   , mLEO_Orbits(aSrc.mLEO_Orbits)
   , mBO_Orbits(aSrc.mBO_Orbits)
   , mPlatformName(aSrc.mPlatformName)
{
   mWeaponEffectTypeId = nullptr; // Don't require weapon effects override
}

// =================================================================================================
Tool* OrbitalLaunchComputerGenerator::Clone() const // override
{
   return new OrbitalLaunchComputerGenerator(*this);
}

// =================================================================================================
bool OrbitalLaunchComputerGenerator::ProcessInput(UtInput& aInput) // override
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());
   if (command == "speed")
   {
      aInput.ReadValueOfType(mSpeed, UtInput::cSPEED);
      aInput.ValueGreaterOrEqual(mSpeed, 0.0);
   }
   else if (command == "pitch")
   {
      aInput.ReadValueOfType(mPitch, UtInput::cANGLE);
      aInput.ValueInClosedRange(mPitch, -UtMath::cPI_OVER_2, UtMath::cPI_OVER_2);
   }
   else if (command == "leo_data_file")
   {
      aInput.ReadValueQuoted(mLEO_DataFileName);
      mOutputFileName = aInput.SubstitutePathVariables(mLEO_DataFileName);
   }
   else if (command == "launch_heading")
   {
      double value;
      aInput.ReadValueOfType(value, UtInput::cANGLE);
      aInput.ValueGreater(value, 0.0);
      aInput.ValueLessOrEqual(value, UtMath::cPI_OVER_2);
      mLaunchHeadings.clear();
      mLaunchHeadings.push_back(value);
   }
   else if (command == "launch_headings")
   {
      double      minValue;
      double      maxValue;
      double      stepValue;
      std::string word;
      aInput.ReadValue(word);
      aInput.StringEqual(word, "from");
      aInput.ReadValueOfType(minValue, UtInput::cANGLE);
      aInput.ReadValue(word);
      aInput.ValueGreaterOrEqual(minValue, 0.0);
      aInput.StringEqual(word, "to");
      aInput.ReadValueOfType(maxValue, UtInput::cANGLE);
      aInput.ValueGreater(maxValue, minValue);
      aInput.ValueLess(maxValue, UtMath::cTWO_PI);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cANGLE);
      aInput.ValueGreater(stepValue, 0.0);
      mLaunchHeadings.clear();
      for (double value = minValue; value <= (maxValue + 1.0E-8); value += stepValue)
      {
         mLaunchHeadings.push_back(value);
      }
   }
   else if (command == "loft_angle")
   {
      double value;
      aInput.ReadValueOfType(value, UtInput::cANGLE);
      aInput.ValueGreater(value, 0.0);
      aInput.ValueLessOrEqual(value, UtMath::cPI_OVER_2);
      mLoftAngles.clear();
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
      mLoftAngles.clear();
      for (double value = minValue; value <= (maxValue + 1.0E-8); value += stepValue)
      {
         mLoftAngles.push_back(value);
      }
   }
   else if (command == "pitch_rate")
   {
      double value;
      aInput.ReadValueOfType(value, UtInput::cANGULAR_RATE);
      aInput.ValueGreater(value, 0.0);
      mPitchRates.clear();
      mPitchRates.push_back(value);
   }
   else if (command == "pitch_rates")
   {
      double      minValue;
      double      maxValue;
      double      stepValue;
      std::string word;
      aInput.ReadValue(word);
      aInput.StringEqual(word, "from");
      aInput.ReadValueOfType(minValue, UtInput::cANGULAR_RATE);
      aInput.ReadValue(word);
      aInput.ValueGreater(minValue, 0.0);
      aInput.StringEqual(word, "to");
      aInput.ReadValueOfType(maxValue, UtInput::cANGULAR_RATE);
      aInput.ValueGreater(maxValue, minValue);
      aInput.ReadValue(word);
      aInput.StringEqual(word, "by");
      aInput.ReadValueOfType(stepValue, UtInput::cANGULAR_RATE);
      aInput.ValueGreater(stepValue, 0.0);
      mPitchRates.clear();
      for (double value = minValue; value <= (maxValue + 1.0E-8); value += stepValue)
      {
         mPitchRates.push_back(value);
      }
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}

// =================================================================================================
bool OrbitalLaunchComputerGenerator::Initialize(WsfSimulation& aSimulation) // override
{
   bool ok = Tool::Initialize(aSimulation);

   if (mLEO_DataFileName.empty())
   {
      ut::log::error() << "Must specify leo_data_file!";
      ok = false;
   }

   if (mLaunchHeadings.empty())
   {
      mLaunchHeadings.push_back(mHeadingRad);
   }
   if (mLoftAngles.empty())
   {
      ut::log::error() << "loft_angle(s) must be specified!";
      ok = false;
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

   mObserver.ResetState();

   std::sort(mLaunchHeadings.begin(), mLaunchHeadings.end());
   std::sort(mLoftAngles.begin(), mLoftAngles.end());

   mLaunchHeadingIndex = 0;
   mLoftAngleIndex     = mLoftAngles.size();

   return ok;
}

// =================================================================================================
void OrbitalLaunchComputerGenerator::Update(double aSimTime) // override
{
   WeaponObserver::EngagementState state = mObserver.GetState();
   if (state == WeaponObserver::cIN_FLIGHT)
   {
      WsfPlatform* platformPtr = mObserver.GetWeaponPlatform();
      if (platformPtr != nullptr) // It better exist
      {
         platformPtr->Update(aSimTime);
         // If the mover has burned out and orbit has not been achieved, give it a few seconds to see if it was close
         if ((mBO_Data.mTimeOfFlight > 0.0) && // burnout has occured...
             (mLEO_Data.mTimeOfFlight <= 0.0)) // but burnout orbit not achieved
         {
            if (aSimTime > (mObserver.GetLaunchTime() + mBO_Data.mTimeOfFlight + 30.0))
            {
               // Delete the platform. This will cause the TERMINATE state to be set and the output to be issued.
               platformPtr->GetSimulation()->DeletePlatform(aSimTime, platformPtr);
            }
         }
      }
   }
   else if (state == WeaponObserver::cTERMINATED)
   {
      static const char* upperCase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      static const char* lowerCase = "abcdefghijklmnopqrstuvwxyz";

      double lat, lon, alt;
      mObserver.GetImpactLocLLA(lat, lon, alt);

      // Issue a diagnostic warning if orbit wasn't achieved. This could occur for many reasons...
      // The diagnostic is not issued during a pitch rate sweep...
      if ((mPitchRate == 0.0) && (mLEO_Data.mTimeOfFlight <= 0.0))
      {
         auto out = ut::log::warning() << "ORBIT NOT ACHIEVED";
         out.AddNote() << "T = " << std::setprecision(3) << aSimTime;
         out.AddNote() << "Time of Flight: " << ComputeTimeOfFlight(aSimTime);
         out.AddNote() << "Platform: " << mPlatformName;

         if (alt < 1.0)
         {
            out.AddNote() << "Reason: Ground impact";
         }
         else if (mBO_Data.mTimeOfFlight > 0.0)
         {
            out.AddNote() << "Reason: Burned out";
            out.AddNote() << "Altitude: " << alt;
            out.AddNote() << "Speed: " << mObserver.GetFinalSpeed();
            out.AddNote() << "FPA: " << mObserver.GetFinalFlightPathAngle() * UtMath::cDEG_PER_RAD;
            out.AddNote() << "Orbital Speed: "
                          << sqrt(UtEarth::cGRAVITATIONAL_PARAMETER / (alt + UtSphericalEarth::cEARTH_RADIUS));
         }
         else
         {
            out.AddNote() << "Reason: <UNKNOWN>";
         }
      }
      if ((mPitchRate > 0.0) && (!DebugEnabled()))
      {
         auto logAlt = ut::log::info() << "Weapon Observer Termination Altitude Report: ";
         if (mLEO_Data.mTimeOfFlight > 0.0)
         {
            // Select a character based on the altitude (starting at 100km and going up in 50km increments)
            // 'A'=[100km,150km), 'B'=1[150km,200km), etc.
            // Uppercase characters will be used to signify the altitude is increasing from the last loft angle.
            // Lowercase characters will be used to signify the altitude is decreasing from the last loft angle.

            int altIndex  = static_cast<int>((alt - 100000.0) / 50000.0);
            altIndex      = std::min(std::max(altIndex, 0), 25);
            char altTrend = upperCase[altIndex];
            if (!mLEO_Orbits.empty())
            {
               if (mLEO_Orbits.back().mTimeOfFlight > 0.0)
               {
                  if (alt < mLEO_Orbits.back().mAlt)
                  {
                     altTrend = lowerCase[altIndex];
                  }
               }
            }
            logAlt << altTrend;
         }
         else if (alt < 1.0)
         {
            logAlt << "*";
         }
         else if (mBO_Data.mTimeOfFlight > 0.0)
         {
            logAlt << "-";
         }
         else
         {
            logAlt << "?";
         }
      }
      mLEO_Orbits.push_back(mLEO_Data);
      mBO_Orbits.push_back(mBO_Data);
      mObserver.ResetState();
   }
   else if (state == WeaponObserver::cIDLE)
   {
      mObserver.ResetState();
      FireNewWeapon(aSimTime);
      if (mDone)
      {
         WsfOrbitalLaunchComputer::WriteOrbits(mLaunchWpnPtr, mLEO_DataFileName, mLEO_Orbits);
      }
   }
   Tool::Update(aSimTime);
}

// =================================================================================================
// A callback from WeaponObserver::MoverBurnout.
void OrbitalLaunchComputerGenerator::MoverBurnout(double aSimTime, WsfMover* aMoverPtr)
{
   WsfPlatform* platformPtr = aMoverPtr->GetPlatform();
   if (platformPtr != mObserver.GetWeaponPlatform())
   {
      return;
   }
   CaptureInsertionData(aSimTime, platformPtr);
}

// =================================================================================================
// A callback from WeaponObserver::GuidanceComputerPhaseChanged.
void OrbitalLaunchComputerGenerator::GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr)
{
   WsfPlatform* platformPtr = aComputerPtr->GetPlatform();
   if (platformPtr != mObserver.GetWeaponPlatform())
   {
      return;
   }

   // Capture the orbit insertion data if the guidance computer has signaled orbit insertion.
   if (platformPtr->GetAuxDataConst().AttributeExists(WsfGuidanceProgram::cGC_ORBIT_DECLARED))
   {
      CaptureInsertionData(aSimTime, platformPtr);
      // Once orbit has been achieved the shot can be terminated. Deleting the platform will cause the
      // data for this condition to be saved and the next launch to occur.
      platformPtr->GetSimulation()->DeletePlatform(aSimTime, platformPtr);
   }
}

// =================================================================================================
// Captures data about the orbit insertion.
void OrbitalLaunchComputerGenerator::CaptureInsertionData(double aSimTime, WsfPlatform* aPlatformPtr)
{
   // There are pretty much three ways this is called.
   // a) The orbital insertion program declared orbit and commanded the engines off. This terminates the stage and
   //    will cause a MoverBurnOut if the stage was the last boost stage.
   //    The user typically has an 'next_phase X when boost_complete'.
   // b) The orbital insertion program declared orbit and commanded the throttle to zero - effectively shutting down the
   // engines
   //    without causing a stage ejection (the construction of the rocket might allow for restart).
   //    The user typically has a 'next_phase X when program ORBITAL_INSERTION_PROGRAM complete'.
   // c) The rocket never enters orbit. MoverBurnOut occurs without declaring orbit.

   // Capture the 'burn_out' state. This is when:
   // 1) The boost phase has ended (signaled by a 'MoverBurnout' callback). This could be because:
   //    a) The guidance computer declared orbit insertion, and had the 'on_insertion eject_stage' option.
   //    b) The boost phase ended which orbit being declared.
   // 2) A guidance computer phase change occur that signaled orbit insertion.

   if (mBO_Data.mTimeOfFlight <= 0.0)
   {
      mBO_Data.mTimeOfFlight = ComputeTimeOfFlight(aSimTime);
      aPlatformPtr->GetLocationLLA(mBO_Data.mLat, mBO_Data.mLon, mBO_Data.mAlt);
      mBO_Data.mSpeedECI   = aPlatformPtr->GetSpeedECI();
      mBO_Data.mSpeedFixed = aPlatformPtr->GetSpeedWCS();
      double dummy;
      aPlatformPtr->GetOrientationNED(mBO_Data.mHeading, dummy, dummy);
      mBO_Data.mFlightPathAngle = ComputeFlightPathAngle(aPlatformPtr);
   }

   // Capture the insertion state if it was declared.
   if (aPlatformPtr->HasAuxData() && (mLEO_Data.mTimeOfFlight <= 0.0))
   {
      const WsfAttributeContainer& auxData       = aPlatformPtr->GetAuxDataConst();
      bool                         orbitDeclared = false;
      if (auxData.AttributeExists(WsfGuidanceProgram::cGC_ORBIT_DECLARED))
      {
         auxData.Get(WsfGuidanceProgram::cGC_ORBIT_DECLARED, orbitDeclared);
      }
      if (orbitDeclared)
      {
         mLEO_Data.mTimeOfFlight    = ComputeTimeOfFlight(aSimTime);
         mLEO_Data.mLat             = mBO_Data.mLat;
         mLEO_Data.mLon             = mBO_Data.mLon;
         mLEO_Data.mAlt             = mBO_Data.mAlt;
         mLEO_Data.mSpeedECI        = mBO_Data.mSpeedECI;
         mLEO_Data.mSpeedFixed      = mBO_Data.mSpeedFixed;
         mLEO_Data.mHeading         = mBO_Data.mHeading;
         mLEO_Data.mFlightPathAngle = ComputeFlightPathAngle(aPlatformPtr);
         if (auxData.AttributeExists(WsfGuidanceProgram::cGC_DELTA_V_REMAINING))
         {
            auxData.Get(WsfGuidanceProgram::cGC_DELTA_V_REMAINING, mLEO_Data.mResidualDeltaV);
         }
         if (auxData.AttributeExists(WsfGuidanceProgram::cGC_BURN_TIME_REMAINING))
         {
            auxData.Get(WsfGuidanceProgram::cGC_BURN_TIME_REMAINING, mLEO_Data.mTimeRemaining);
         }
         if (auxData.AttributeExists(WsfGuidanceProgram::cGC_FUEL_REMAINING))
         {
            auxData.Get(WsfGuidanceProgram::cGC_FUEL_REMAINING, mLEO_Data.mFuelRemaining);
         }
      }
   }
}

// =================================================================================================
// A callback from WeaponObserver::PlatformAdded.
void OrbitalLaunchComputerGenerator::PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr)
{
   if ((aPlatformPtr != nullptr) && (aPlatformPtr->GetType() != mLaunchPlatformTypeId.GetString()))
   {
      mObserver.SetInFlight(aSimTime, aPlatformPtr);
      mPlatformName = aPlatformPtr->GetName();
   }
}

// =================================================================================================
//! Compute the orbital speed at a specified altitude.
double OrbitalLaunchComputerGenerator::ComputeFlightPathAngle(WsfPlatform* aPlatformPtr)
{
   double locECI[3];
   double velECI[3];
   aPlatformPtr->GetLocationECI(locECI);
   aPlatformPtr->GetVelocityECI(velECI);
   double cosTheta = UtVec3d::DotProduct(locECI, velECI) / (UtVec3d::Magnitude(locECI) * UtVec3d::Magnitude(velECI));
   return (UtMath::cPI_OVER_2 - acos(UtMath::Limit(cosTheta, 1.0)));
}

// =================================================================================================
//! Compute the orbital speed at a specified altitude.
double OrbitalLaunchComputerGenerator::ComputeOrbitalSpeed(WsfPlatform* aPlatformPtr)
{
   double locECI[3];
   aPlatformPtr->GetLocationECI(locECI);
   double r = UtVec3d::Magnitude(locECI);
   return sqrt(UtEarth::cGRAVITATIONAL_PARAMETER / r);
}

// =================================================================================================
double OrbitalLaunchComputerGenerator::ComputeTimeOfFlight(double aTime) const
{
   return (aTime - mObserver.GetLaunchTime());
}

// =================================================================================================
bool OrbitalLaunchComputerGenerator::FireNewWeapon(double aSimTime) // override
{
   bool ok(false);
   if (mPitchRates.empty())
   {
      ok = FireNextWeapon(aSimTime);
      if ((!ok) && mDone)
      {
         mObserver.SetAllDone();
      }
   }
   else
   {
      if (mPitchRateIndex == 0 && mLEO_Orbits.empty())
      {
         if (!DebugEnabled())
         {
            ut::log::debug() << "Pitch Rate: " << std::fixed << std::setprecision(4)
                             << mPitchRates[mPitchRateIndex] * UtMath::cDEG_PER_RAD << " deg";
         }
      }
      ok = FireNextWeapon(aSimTime);
      if ((!ok) && mDone)
      {
         std::cout << std::endl;
         SummarizePitchRateResults();
         ++mPitchRateIndex;
         if (mPitchRateIndex >= mPitchRates.size())
         {
            mObserver.SetAllDone();
            return false;
         }
         if (!DebugEnabled())
         {
            ut::log::debug() << "Pitch Rate: " << std::fixed << std::setprecision(4)
                             << mPitchRates[mPitchRateIndex] * UtMath::cDEG_PER_RAD << " deg";
         }
         mDone = false;

         mLEO_Orbits.clear();
         mBO_Orbits.clear();

         mLaunchHeadingIndex = 0;
         mLoftAngleIndex     = mLoftAngles.size();
         ok                  = FireNextWeapon(aSimTime);
      }
   }
   return ok;
}

// =================================================================================================
bool OrbitalLaunchComputerGenerator::FireNextWeapon(double aSimTime)
{
   if (mLoftAngleIndex >= mLoftAngles.size())
   {
      if (mLaunchHeadingIndex >= mLaunchHeadings.size())
      {
         mDone = true;
         // mObserver.SetAllDone();
         return false;
      }
      mLaunchHeading = mLaunchHeadings[mLaunchHeadingIndex];
      ++mLaunchHeadingIndex;
      mLoftAngleIndex = 0;
   }
   mLoftAngle = mLoftAngles[mLoftAngleIndex];
   ++mLoftAngleIndex;

   // If requested, set the 'pitch_rate' to be used in the FLIGHT_PATH_ANGLE_PROGRAM
   if (!mPitchRates.empty())
   {
      mPitchRate               = mPitchRates[mPitchRateIndex];
      UtScriptContext& context = GetSimulation()->GetScriptContext().GetContext();
      UtScriptData&    var     = context.Var("OLC_PITCH_RATE");
      var.SetDouble(mPitchRate * UtMath::cDEG_PER_RAD);
   }

   // Initialize the orbit results...
   Orbit zero;
   mLEO_Data                = zero;
   mLEO_Data.mLaunchHeading = mLaunchHeading;
   mLEO_Data.mLoftAngle     = mLoftAngle;

   mBO_Data                = zero;
   mBO_Data.mLaunchHeading = mLaunchHeading;
   mBO_Data.mLoftAngle     = mLoftAngle;

   // Set the launch platform state.
   // - The heading is set to the current launch heading.
   // - If the launch speed was specified as non-zero then:
   //   - The pitch is set to the input value.
   //   - The velocity vector aligns with the X axis of the orientation.
   // The ability to set the launch speed and pitch is to support launch of the Virgin Orbit 'Launcher One' from the
   // 'Cosmic Girl', or any launch from moving airborne platform.
   WsfPlatform* launchPtr = mLaunchWpnPtr->GetPlatform();
   double       heading, pitch, roll;
   mLaunchWpnPtr->GetPlatform()->GetOrientationNED(heading, pitch, roll);
   heading = mLaunchHeading;
   if (mSpeed > 0.0)
   {
      pitch = mPitch;
   }
   launchPtr->SetOrientationNED(heading, pitch, roll);

   // Set the launch speed if it was given.
   if (mSpeed > 0.0)
   {
      double velNE     = mSpeed * cos(pitch);
      double velNED[3] = {velNE * sin(heading), velNE * cos(heading), -mSpeed * sin(pitch)};
      launchPtr->SetVelocityNED(velNED);
   }

   // Set the commanded values in the launch computer.
   WsfLaunchComputer* computerPtr = mLaunchWpnPtr->GetLaunchComputer();
   computerPtr->ClearCommandedValues();
   computerPtr->SetCommandedLoftAngle(mLoftAngle);

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
         auto logDebug = ut::log::debug() << "OrbitalLaunchComputerGenerator::FireNextWeapon";
         logDebug.AddNote() << "T = " << std::setprecision(3) << aSimTime;
         auto logNote = logDebug.AddNote() << "Launched";
         logNote.AddNote() << "Launch Heading: " << std::setprecision(6) << mLaunchHeading * UtMath::cDEG_PER_RAD << " deg";
         logNote.AddNote() << "Loft Angle: " << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
         if (mPitchRate > 0.0)
         {
            logNote.AddNote() << "Pitch Rate: " << std::setprecision(6) << mPitchRate * UtMath::cDEG_PER_RAD << " deg";
         }
      }
   }
   else
   {
      auto logError = ut::log::error() << "OrbitalLaunchComputerGenerator::FireNextWeapon";
      logError.AddNote() << "T = " << std::setprecision(3) << aSimTime;
      auto logNote = logError.AddNote() << "Failed to launch";
      logNote.AddNote() << "Launch Heading: " << std::setprecision(6) << mLaunchHeading * UtMath::cDEG_PER_RAD << " deg";
      logNote.AddNote() << "Loft Angle: " << mLoftAngle * UtMath::cDEG_PER_RAD << " deg";
   }
   return ok;
}

// =================================================================================================
void OrbitalLaunchComputerGenerator::SummarizePitchRateResults()
{
   enum
   {
      cORBIT,
      cBURNED_OUT,
      cCRASHED
   };

   struct Stats
   {
      Stats()
         : mCount(0)
         , mMinAlt(1.0E+30)
         , mMaxAlt(0.0)
         , mSumAlt(0.0)
      {
      }

      int    mCount;
      double mMinAlt;
      double mMaxAlt;
      double mSumAlt;
   };

   std::vector<Stats> stats(3);
   auto               leoIter = mLEO_Orbits.begin();
   auto               boIter  = mBO_Orbits.begin();
   size_t             i       = 0;
   while (leoIter != mLEO_Orbits.end())
   {
      Orbit& leoData = *leoIter;
      Orbit& boData  = *boIter;
      if (leoData.mTimeOfFlight > 0.0)
      {
         i = cORBIT;
         ++stats[i].mCount;
         stats[i].mMinAlt = std::min(stats[i].mMinAlt, leoData.mAlt);
         stats[i].mMaxAlt = std::max(stats[i].mMaxAlt, leoData.mAlt);
         stats[i].mSumAlt += leoData.mAlt;
      }
      else if (boData.mTimeOfFlight > 0.0)
      {
         i = cBURNED_OUT;
         ++stats[i].mCount;
         stats[i].mMinAlt = std::min(stats[i].mMinAlt, boData.mAlt);
         stats[i].mMaxAlt = std::max(stats[i].mMaxAlt, boData.mAlt);
         stats[i].mSumAlt += boData.mAlt;
      }
      else
      {
         i = cCRASHED;
         ++stats[i].mCount;
      }
      ++leoIter;
      ++boIter;
   }
   auto logInfo = ut::log::info() << "Summary for pitch rate: " << mPitchRate * UtMath::cDEG_PER_RAD << " deg";
   i            = cORBIT;
   if (stats[i].mCount > 0)
   {
      logInfo.AddNote() << "Orbit: " << stats[i].mCount;
      logInfo.AddNote() << "Alt Min: " << stats[i].mMinAlt;
      logInfo.AddNote() << "Max: " << stats[i].mMaxAlt;
      logInfo.AddNote() << "Avg: " << stats[i].mSumAlt / stats[i].mCount;
   }
   i = cBURNED_OUT;
   if (stats[i].mCount > 0)
   {
      logInfo.AddNote() << "Burned Out: " << stats[i].mCount;
      logInfo.AddNote() << "Alt Min: " << stats[i].mMinAlt;
      logInfo.AddNote() << "Max: " << stats[i].mMaxAlt;
      logInfo.AddNote() << "Avg: " << stats[i].mSumAlt / stats[i].mCount;
   }
   i = cCRASHED;
   if (stats[i].mCount > 0)
   {
      logInfo.AddNote() << "Crashed: " << stats[i].mCount;
   }
}
