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

#include "SAM_LaunchComputerGenerator.hpp"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <fstream>
#include <sstream>

#include "UtInputBlock.hpp"
#include "UtInputFile.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "UtStringUtil.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfExplicitWeapon.hpp"
#include "WsfLaunchComputerTypes.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfRouteMover.hpp"
#include "WsfSensor.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackManager.hpp"
#include "WsfWeaponEngagement.hpp"

namespace
{
const char*  cSAM_LAUNCH_COMPUTER_DEFAULT_PRODUCT("_SAM_LAUNCH_COMPUTER");
const double cNOT_VALID = -1.0;
} // namespace

SAM_LaunchComputerGenerator::SAM_LaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mTrackerHeight(cNOT_VALID)
   , mTrackerMinEl(-UtMath::cPI_OVER_2)
   , mTrackerMaxEl(UtMath::cPI_OVER_2)
   , mEffectiveEarthRadiusRatio(cNOT_VALID)
   , mTrackerName()
   , mTrackerMode()
   , mOffsetIndex(0)
   , mAltIndex(0)
   , mRngIndex(0)
   , mSpeedIndex(0)
   , mMaxDeltaAltitude(0.0)
   , mMinDeltaAltitude(0.0)
   , mMaxSlantRange(0.0)
   , mMinSlantRange(0.0)
   , mMaxTOF(0.0)
   , mState(cLC_GEN)
   , mFirstPass(true)
   ,
   // mLethalAtCurrentAlt(false),
   // mContinueAfterMiss(false),
   mReReadProducedFile(false)
   , mTargetCount(0)
   , mTgtIndex(0)
   , mTgtLatDeg(0.0)
   , mTgtLonDeg(0.0)
   , mTgtAlt(0.0)
   , mTgtHdgRad(0.0)
   , mTgtSpeed(0.0)
   , mSpecifiedOffsets()
   , mSpecifiedAltitudes()
   , mSpecifiedRanges()
   , mSpecifiedTgtSpeeds()
   , mTestPoints()
   , mLC_Instance()
   , mGenTime()
{
   mToolProduces = cSAM_LAUNCH_COMPUTER_DEFAULT_PRODUCT;
}

SAM_LaunchComputerGenerator::~SAM_LaunchComputerGenerator() {}

// virtual
Tool* SAM_LaunchComputerGenerator::Clone() const
{
   return new SAM_LaunchComputerGenerator(*this);
}

double SAM_LaunchComputerGenerator::CurrentOffset() const
{
   // By default, the mObsoleteInput is always at offset Zero.
   double value = 0.0;
   if (!mSpecifiedOffsets.empty())
   {
      value = mSpecifiedOffsets[mOffsetIndex];
   }
   return value;
}

double SAM_LaunchComputerGenerator::CurrentGrndRng()
{
   return mSpecifiedRanges[mRngIndex];
}

double SAM_LaunchComputerGenerator::CurrentAlt()
{
   return mSpecifiedAltitudes[mAltIndex];
}

double SAM_LaunchComputerGenerator::CurrentSpeed()
{
   return mSpecifiedTgtSpeeds[mSpeedIndex];
}

bool SAM_LaunchComputerGenerator::ProcessInput(UtInput& aInput)
{
   const std::string cCOMPLAINT("Array values must be added in increasing order, or use 'from' a 'to' b 'by' c.");
   bool              myCommand = true;

   std::string command(aInput.GetCommand());

   if (command == "re_parse_produced_input_file")
   {
      mReReadProducedFile = true;
   }
   else if (command == "post_generation_interpolation_tests")
   {
      UtInputBlock block(aInput);
      TestPoint    point;
      while (block.ReadCommand())
      {
         UtInput& in = block.GetInput();
         command     = in.GetCommand();
         if (command == "lateral_offset")
         {
            in.ReadValueOfType(point.latOffset, UtInput::cLENGTH);
         }
         else if (command == "altitude")
         {
            in.ReadValueOfType(point.altitude, UtInput::cLENGTH);
         }
         else if (command == "ground_range")
         {
            in.ReadValueOfType(point.groundRange, UtInput::cLENGTH);
         }
         else if (command == "target_speed")
         {
            in.ReadValueOfType(point.speed, UtInput::cSPEED);
         }
         else if (command == "test")
         {
            mTestPoints.push_back(point);
         }
         else
         {
            throw UtInput::BadValue(aInput, "Unrecognized keyword '" + command + "' in the interpolation test block.");
         }
      }
   }
   else if (command == "target_speed")
   {
      double speedNotUsed;
      aInput.ReadValueOfType(speedNotUsed, UtInput::cSPEED);
      ut::log::warning() << "The'target_speed' ignored by this generator. "
                         << "Use 'test_matrix' 'speeds' instead.";
   }
   else if (command == "tracker_height")
   {
      aInput.ReadValueOfType(mTrackerHeight, UtInput::cLENGTH);
      aInput.ValueGreater(mTrackerHeight, 0.0);
   }
   else if (command == "tracker_elevation_limits")
   {
      aInput.ReadValueOfType(mTrackerMinEl, UtInput::cANGLE);
      aInput.ReadValueOfType(mTrackerMaxEl, UtInput::cANGLE);
      aInput.ValueGreaterOrEqual(mTrackerMinEl, -UtMath::cPI_OVER_2);
      aInput.ValueLessOrEqual(mTrackerMaxEl, UtMath::cPI_OVER_2);
      aInput.ValueGreater(mTrackerMaxEl, mTrackerMinEl);
   }
   else if (command == "tracker_name")
   {
      aInput.ReadValue(mTrackerName);
   }
   else if (command == "tracker_mode")
   {
      aInput.ReadValue(mTrackerMode);
   }
   else if (command == "effective_earth_radius")
   {
      aInput.ReadValue(mEffectiveEarthRadiusRatio);
      aInput.ValueGreater(mEffectiveEarthRadiusRatio, 0.0);
   }
   else if ((command == "test_matrix") || (command == "target_points"))
   {
      UtInputBlock block(aInput);
      while (block.ReadCommand(command))
      {
         if (command == "lateral_offsets")
         {
            std::string terminator("end_" + command);
            block.GetInput().ReadCommand(command);
            if (!ProcessArray(aInput, terminator, UtInput::cLENGTH, mSpecifiedOffsets))
            {
               throw UtInput::BadValue(aInput, cCOMPLAINT);
            }
         }
         else if (command == "altitudes")
         {
            std::string terminator("end_" + command);
            block.GetInput().ReadCommand(command);
            if (!ProcessArray(aInput, terminator, UtInput::cLENGTH, mSpecifiedAltitudes))
            {
               throw UtInput::BadValue(aInput, cCOMPLAINT);
            }
         }
         else if (command == "ranges")
         {
            std::string terminator("end_" + command);
            block.GetInput().ReadCommand(command);
            if (!ProcessArray(aInput, terminator, UtInput::cLENGTH, mSpecifiedRanges))
            {
               throw UtInput::BadValue(aInput, cCOMPLAINT);
            }
         }
         else if (command == "speeds")
         {
            std::string terminator("end_" + command);
            block.GetInput().ReadCommand(command);
            if (!ProcessArray(aInput, terminator, UtInput::cSPEED, mSpecifiedTgtSpeeds))
            {
               throw UtInput::BadValue(aInput, cCOMPLAINT);
            }
         }
         else if (command == "target_speeds") // Deprecated
         {
            ut::log::warning() << "'target_speeds' is deprecated. Use 'speeds ... end_speeds' instead.";
            std::string terminator("end_" + command);
            block.GetInput().ReadCommand(command);
            if (!ProcessArray(aInput, terminator, UtInput::cSPEED, mSpecifiedTgtSpeeds))
            {
               throw UtInput::BadValue(aInput, cCOMPLAINT);
            }
         }
         else
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   // ================================ START DEPRECATED INPUT ===========================
   else if (command == "continue_after_miss")
   {
      // do nothing
   }
   else if (command == "target_altitude")
   {
      double alt;
      aInput.ReadValueOfType(alt, UtInput::cLENGTH);
      mSpecifiedAltitudes.push_back(alt);
   }
   // ================================ END DEPRECATED INPUT ===========================
   else if (mLC_Instance.ProcessInput(aInput))
   {
      // do nothing
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}

//! Parse an input stream and extract from it an array of unit values.
//! @param aInput           The input stream reference.
//! @param aBlockTerminator The string to search for end-of-block.
//! @param aUnitType        The type of unit to be read in and added to the array contents.
//! @param aValues          An array of floating point values to hold the results of parsing.
//! @returns 'True' if the aValues array was added to in any way.
bool SAM_LaunchComputerGenerator::ProcessArray(UtInput&             aInput,
                                               const std::string&   aBlockTerminator,
                                               UtInput::ValueType   aUnitType,
                                               std::vector<double>& aValues)
{
   bool valuesAdded = false;
   bool done        = false;

   // The implementation guarantees that the array is null coming in.
   // The idea is to read in an array of unit values, either with:
   // a) A "from" <value> "to" <value> "by" <value> syntax, or
   // b) A continuous stream of values in increasing order, delimited by white space.
   std::string command = aInput.GetCommand();

   while (!done)
   {
      if (command == aBlockTerminator)
      {
         done = true;
      }
      else if (command == "from")
      {
         double startValue;
         aInput.ReadValueOfType(startValue, aUnitType);

         std::string tokenTo;
         aInput.ReadValue(tokenTo); // throw away required (but unused) token "to"

         double endValue;
         aInput.ReadValueOfType(endValue, aUnitType);
         aInput.ValueGreater(endValue, startValue);

         std::string tokenBy;
         aInput.ReadValue(tokenBy); // throw away required (but unused) token "by"

         double deltaValue;
         aInput.ReadValueOfType(deltaValue, aUnitType);

         unsigned int numVals = static_cast<unsigned int>((endValue - startValue) / deltaValue + 1);

         for (unsigned int iVal = 0; iVal != numVals; ++iVal)
         {
            aValues.push_back(startValue + iVal * deltaValue);
         }
         valuesAdded = true;

         aInput.ReadCommand(command);
      }
      else
      {
         if (UtStringUtil::ContainsNonNumeric(command))
         {
            done = true;
         }
         else
         {
            aInput.PushBack(command);
            double distanceValue;
            aInput.ReadValueOfType(distanceValue, aUnitType);
            aValues.push_back(distanceValue);
            valuesAdded = true;
            aInput.ReadCommand(command);
         }
      }
   }

   return valuesAdded;
}

// virtual
void SAM_LaunchComputerGenerator::ErrorBailout()
{
   ut::log::error() << "SAM_LaunchComputerGenerator has terminated early for some unknown reason.";
   mState = cDONE;
   mDone  = true;
   mObserver.SetAllDone();
}

// virtual
void SAM_LaunchComputerGenerator::Update(double aSimTime)
{
   if (aSimTime > mLastUpdate)
   {
      if (mState == cDONE)
      {
      }
      else if (mState == cLC_GEN)
      {
         LCGenUpdate(aSimTime);
      }

      Tool::Update(aSimTime);
   }
   else if (aSimTime < mLastUpdate)
   {
      auto out = ut::log::error() << "Timing error...";
      out.AddNote() << "T = " << aSimTime;
      out.AddNote() << "Last Update: " << mLastUpdate;
   }
}

// virtual
void SAM_LaunchComputerGenerator::LCGenUpdate(double aSimTime)
{
   if (mFirstPass)
   {
      mOffsetIndex = 0;
      mAltIndex    = 0;
      mRngIndex    = 0;
      mSpeedIndex  = 0;

      FireNewWeapon(aSimTime);
      mFirstPass = false;
   }
   else
   {
      // Check to see if the previously fired weapon has terminated yet...
      if (mObserver.GetState() == WeaponObserver::cTERMINATED)
      {
         double tof = cNOT_VALID;
         if (mObserver.EngagementLethal())
         {
            tof = mObserver.TimeOfFlight();
         }
         else // if (! mObserver.EngagementLethal())
         {
            // Previously fired weapon went away.  Remove the old target,
            // if our own weapon didn't already kill it.
            WsfPlatform* oldTgtPtr = GetSimulation()->GetPlatformByIndex(mTgtIndex);
            if (oldTgtPtr != nullptr)
            {
               GetSimulation()->DeletePlatform(aSimTime, oldTgtPtr);
            }
         }
         if (tof != cNOT_VALID)
         {
            mLC_Instance.GetEnvPtr()->SetTOF_Value(mOffsetIndex, mAltIndex, mRngIndex, mSpeedIndex, tof);
         }
         if (DebugEnabled())
         {
            auto logDebug = ut::log::debug() << "SAM_LaunchComputerGenerator::LCGenUpdate";
            logDebug.AddNote() << "iOff: " << mOffsetIndex;
            logDebug.AddNote() << "iAlt: " << mAltIndex;
            logDebug.AddNote() << "iRng: " << mRngIndex;
            logDebug.AddNote() << "iSpd: " << mSpeedIndex;
            logDebug.AddNote() << "tof: " << tof;
         }
         mTgtIndex = 0;
         mObserver.ResetState();
      }

      if (mObserver.GetState() == WeaponObserver::cIDLE)
      {
         // Run a complete matrix with later values changing the fastest (Offset, Altitude, Range, Speed):
         if (mSpeedIndex + 1 != mLC_Instance.GetEnvPtr()->NumSpeeds())
         {
            ++mSpeedIndex;
         }
         else if (mRngIndex + 1 != mLC_Instance.GetEnvPtr()->NumRanges())
         {
            ++mRngIndex;
            mSpeedIndex = 0;
         }
         else if (mAltIndex + 1 != mLC_Instance.GetEnvPtr()->NumAltitudes())
         {
            ++mAltIndex;
            mSpeedIndex = 0;
            mRngIndex   = 0;
         }
         else if (mOffsetIndex + 1 != mLC_Instance.GetEnvPtr()->NumOffsets())
         {
            ++mOffsetIndex;
            mAltIndex   = 0;
            mSpeedIndex = 0;
            mRngIndex   = 0;
         }
         else
         {
            WriteOutputFile();
            TestOutputFile();
            TestPoints();
            mState = cDONE;
            mDone  = true;
            mObserver.SetAllDone();
         } // Range and Altitude Checks

         if (mState != cDONE)
         {
            FireNewWeapon(aSimTime);
         }
      }
   } // if firstPass
}

bool SAM_LaunchComputerGenerator::Initialize(WsfSimulation& aSimulation)
{
   Tool::Initialize(aSimulation);

   // If a tracker height wasn't specified and tracker name was, resolve the tracker height from its definition.
   if ((mTrackerHeight < 0.0) && (!mTrackerName.empty()))
   {
      WsfSensor* sensorPtr = mLaunchPlatPtr->GetComponent<WsfSensor>(mTrackerName);
      if (sensorPtr != nullptr)
      {
         double sensorOffset[3];
         sensorPtr->GetLocation(sensorOffset);
         mTrackerHeight = -sensorOffset[2];
         for (unsigned int i = 0; i < sensorPtr->GetEM_XmtrCount(); ++i)
         {
            WsfEM_Antenna* antennaPtr = sensorPtr->GetEM_Xmtr(i).GetAntenna();
            if (antennaPtr != nullptr)
            {
               mTrackerHeight = std::max(mTrackerHeight, antennaPtr->GetHeight());
            }
         }
      }
      else
      {
         auto logError = ut::log::error() << "The Sensor does not exist on the Launch Platform!";
         logError.AddNote() << "Sensor: " << mTrackerName;
         logError.AddNote() << "Launch Platform: " << mLaunchPlatPtr->GetName();
         mInitialized = false;
      }
   }
   if (mTrackerHeight > 0.0)
   {
      if (mEffectiveEarthRadiusRatio <= 0)
      {
         mEffectiveEarthRadiusRatio = 4.0 / 3.0;
      }
      auto logInfo = ut::log::info() << "Tracker Data";
      logInfo.AddNote() << "Tracker Height: " << mTrackerHeight << " m";
      logInfo.AddNote() << "Effective Earth radius: " << mEffectiveEarthRadiusRatio;
   }

   mFirstPass = true;
   mTgtIndex  = 0;

   mObserver.ResetState();

   // Go through the LC_Instance, and set all the required array values:
   if (mInitialized)
   {
      if (!(mSpecifiedOffsets.empty() || mSpecifiedAltitudes.empty() || mSpecifiedTgtSpeeds.empty() ||
            mSpecifiedRanges.empty()))
      {
         mLC_Instance.GetEnvPtr()->Resize(mSpecifiedOffsets, mSpecifiedAltitudes, mSpecifiedRanges, mSpecifiedTgtSpeeds);
      }
      else
      {
         auto logError = ut::log::error() << "Inadequate engagement conditions specified.";

         if (mSpecifiedOffsets.size() == 0)
         {
            logError.AddNote() << "Offsets array cannot be empty; must have at least one value.";
         }
         if (mSpecifiedAltitudes.size() == 0)
         {
            logError.AddNote() << "Altitudes array cannot be empty; must have at least one value.";
         }
         if (mSpecifiedTgtSpeeds.size() == 0)
         {
            logError.AddNote() << "Speeds array cannot be empty; must have at least one value.";
         }
         if (mSpecifiedRanges.size() == 0)
         {
            logError.AddNote() << "Ranges array cannot be empty; must have at least one value.";
         }
         mInitialized = false;
      }
   }

   if (!mInitialized)
   {
      Tool::UnInitialize();
   }

   return mInitialized;
}

void SAM_LaunchComputerGenerator::WriteOutputFile()
{
   std::ofstream coutFile(mOutputFileName.c_str());

   if (!coutFile)
   {
      ut::log::error() << "Unable to open output file, results not written.";
      return;
   }
   else
   {
      coutFile << std::endl;

      bool valid = CalcConstraints();

      coutFile << "launch_computer " << mOutputObjectName << " " << WsfSAM_LaunchComputer::BaseTypeName() << std::endl;

      time_t aclock;
      time(&aclock);
      struct tm*  newtime = localtime(&aclock);
      std::string stamp1("   # Created by SAM_LaunchComputerGenerator on ");
      std::string stamp2(asctime(newtime));
      mGenTime = std::string(stamp1 + stamp2);

      coutFile << std::endl << mGenTime;
      if (valid)
      {
         coutFile << "   maximum_delta_altitude " << mMaxDeltaAltitude << " m" << std::endl;
         coutFile << "   minimum_delta_altitude " << mMinDeltaAltitude << " m" << std::endl;
         coutFile << "   maximum_slant_range " << mMaxSlantRange << " m" << std::endl;
         coutFile << "   minimum_slant_range " << mMinSlantRange << " m" << std::endl;
         coutFile << "   maximum_time_of_flight " << mMaxTOF << " sec" << std::endl;
      }
      coutFile << std::endl;

      // Write the independent variable arrays:
      WsfSAM_LaunchComputer::InterceptEnvelope& env(*mLC_Instance.GetEnvPtr());

      coutFile << "   lateral_offsets" << std::endl << "      ";
      for (mOffsetIndex = 0; mOffsetIndex < env.NumOffsets(); ++mOffsetIndex)
      {
         if (mOffsetIndex % 9 == 8)
         {
            coutFile << std::endl << "      ";
         }
         coutFile << env.Offset(mOffsetIndex) << " m ";
      }
      coutFile << std::endl << "   end_lateral_offsets" << std::endl << std::endl;

      coutFile << "   altitudes" << std::endl << "      ";
      for (mAltIndex = 0; mAltIndex < env.NumAltitudes(); ++mAltIndex)
      {
         if (mAltIndex % 9 == 8)
         {
            coutFile << std::endl << "      ";
         }
         coutFile << env.Altitude(mAltIndex) << " m ";
      }
      coutFile << std::endl << "   end_altitudes" << std::endl << std::endl;

      coutFile << "   ranges" << std::endl << "      ";
      for (mRngIndex = 0; mRngIndex < env.NumRanges(); ++mRngIndex)
      {
         if (mRngIndex % 9 == 8)
         {
            coutFile << std::endl << "      ";
         }
         coutFile << env.GroundRange(mRngIndex) << " m ";
      }
      coutFile << std::endl << "   end_ranges" << std::endl << std::endl;

      coutFile << "   speeds" << std::endl << "      ";
      for (mSpeedIndex = 0; mSpeedIndex < env.NumSpeeds(); ++mSpeedIndex)
      {
         if (mSpeedIndex % 9 == 8)
         {
            coutFile << std::endl << "      ";
         }
         coutFile << env.Speed(mSpeedIndex) << " m/s ";
      }
      coutFile << std::endl << "   end_speeds" << std::endl << std::endl;

      coutFile << "   intercept_envelope" << std::endl << std::endl;
      coutFile << "      # Each line contains Index and Value for:" << std::endl;
      coutFile << "      # i, lateral_offset, altitude, ground_range, speed, time_of_flight" << std::endl;
      coutFile << "      # Only successful intercept values are shown, other TOFs set = -1. " << std::endl << std::endl;

      unsigned int interceptCount = 0;
      for (mOffsetIndex = 0; mOffsetIndex < env.NumOffsets(); ++mOffsetIndex)
      {
         double latOffset = env.Offset(mOffsetIndex);

         for (mAltIndex = 0; mAltIndex < env.NumAltitudes(); ++mAltIndex)
         {
            double alt = env.Altitude(mAltIndex);

            for (mRngIndex = 0; mRngIndex < env.NumRanges(); ++mRngIndex)
            {
               double rng = env.GroundRange(mRngIndex);

               for (mSpeedIndex = 0; mSpeedIndex < env.NumSpeeds(); ++mSpeedIndex)
               {
                  double spd = env.Speed(mSpeedIndex);

                  double tof = env.TimeOfFlight(mOffsetIndex, mAltIndex, mRngIndex, mSpeedIndex);

                  if (tof > 0.0)
                  {
                     coutFile << "      intercept " << interceptCount << " ";
                     coutFile << mOffsetIndex << " " << latOffset << " m ";
                     coutFile << mAltIndex << " " << alt << " m ";
                     coutFile << mRngIndex << " " << rng << " m ";
                     coutFile << mSpeedIndex << " " << spd << " m/s ";
                     coutFile << tof << " sec" << std::endl;
                     ++interceptCount;
                  }
               }
            }
         }
      }
      coutFile << "   end_intercept_envelope" << std::endl;
      coutFile << "end_launch_computer" << std::endl << std::endl;
      coutFile.close();
   }
}

void SAM_LaunchComputerGenerator::TestOutputFile()
{
   if (mReReadProducedFile) // This is used for debugging the output file produced.
   {
      auto logInfo = ut::log::info() << "SAM Launch Computer Testing: Attempting to re-read launch computer file.";
      // Now, feed the resulting text file to the class,
      // to assure that it can be read correctly:
      try
      {
         // Open the input file...
         UtInput testInput;
         testInput.PushInput(ut::make_unique<UtInputFile>(mOutputFileName));

         if ((WsfLaunchComputerTypes::Get(GetScenario()).LoadType(testInput)) &&
             (WsfSAM_LaunchComputer::ValidateEnvelope(mLC_Instance.GetEnvPtr(), true)))
         {
            logInfo.AddNote() << "Re-read of the created output file was successful.";
         }
         else
         {
            ut::log::error() << "FAILED to re-read of the output file just created.";
         }
      }
      catch (UtInput::BadValue& aException)
      {
         std::string::size_type foundAt = aException.GetMessage().find("Duplicate");
         if (foundAt == std::string::npos)
         {
            // The string "Duplicate" was NOT found as expected.  Must be some other error.
            auto logError = ut::log::error() << "Bad value found during re-reading the output File!";
            logError.AddNote() << "Exception: " << aException.GetMessage();
         }
         else
         {
            // It is okay that we found a duplicate launch computer type... we should have.
            logInfo.AddNote() << "Re-read was successful.";
         }
      }
      catch (UtInput::EndOfData& aException)
      {
         auto logError = ut::log::error() << "Early end-of-data encountered in re-reading output File!";
         logError.AddNote() << "Exception: " << aException.GetMessage();
      }
      catch (UtException& aException)
      {
         auto logError = ut::log::error() << "Unknown UtException encountered in re-reading output file!";
         logError.AddNote() << "Exception: " << aException.what();
      }
      catch (std::exception& e)
      {
         auto out = ut::log::error() << "Unknown exception encountered in re-reading output file.  Rethrown.";
         out.AddNote() << "Type: " << ut::TypeNameOf(e);
         out.AddNote() << "What: " << e.what();
         throw;
      }
   }
}

void SAM_LaunchComputerGenerator::TestPoints()
{
   if (!mTestPoints.empty())
   {
      auto   logInfo = ut::log::info() << "SAM Launch Computer Testing: Post generation interpolation tests.";
      double simTime = GetSimulation()->GetSimTime();
      if (mLC_Instance.Initialize(simTime, mLaunchPlatPtr->GetComponentEntry<WsfWeapon>(0)))
      {
         for (unsigned int i = 0; i != mTestPoints.size(); ++i)
         {
            auto       logNote = logInfo.AddNote();
            TestPoint& tp      = mTestPoints[i];
            double tof = mLC_Instance.InterpolatedTimeToIntercept(tp.latOffset, tp.altitude, tp.groundRange, tp.speed);
            logNote << "Test Results";
            logNote.AddNote() << "Offset: " << tp.latOffset;
            logNote.AddNote() << "Altitude: " << tp.altitude;
            logNote.AddNote() << "GrndRang: " << tp.groundRange;
            logNote.AddNote() << "Speed: " << tp.speed;
            logNote.AddNote() << "Resulting Time Of Flight: " << tof;
         }
      }
   }
}

bool SAM_LaunchComputerGenerator::CalcConstraints()
{
   // Calculate values for maximum perceived range, altitudes,
   // etc., based upon values already placed in the TOF array.

   double maxAlt = 0.0;
   double minAlt = 99999999.0;
   double maxRng = 0.0;
   double minRng = 99999999.0;
   double maxTOF = 0.0;

   bool constraintValid = false;

   WsfSAM_LaunchComputer::InterceptEnvelope* envPtr = mLC_Instance.GetEnvPtr();
   for (mOffsetIndex = 0; mOffsetIndex < envPtr->NumOffsets(); ++mOffsetIndex)
   {
      for (mAltIndex = 0; mAltIndex < envPtr->NumAltitudes(); ++mAltIndex)
      {
         double alt = envPtr->Altitude(mAltIndex);

         for (mRngIndex = 0; mRngIndex < envPtr->NumRanges(); ++mRngIndex)
         {
            double grdRng   = envPtr->GroundRange(mRngIndex);
            double slantRng = sqrt(grdRng * grdRng + alt * alt);

            for (mSpeedIndex = 0; mSpeedIndex < envPtr->NumSpeeds(); ++mSpeedIndex)
            {
               double timeOfFlight = envPtr->TimeOfFlight(mOffsetIndex, mAltIndex, mRngIndex, mSpeedIndex);

               if (timeOfFlight > 0.0)
               {
                  constraintValid = true;
                  if (alt > maxAlt)
                  {
                     maxAlt = alt;
                  }
                  if (alt < minAlt)
                  {
                     minAlt = alt;
                  }
                  if (slantRng > maxRng)
                  {
                     maxRng = slantRng;
                  }
                  if (slantRng < minRng)
                  {
                     minRng = slantRng;
                  }
                  if (timeOfFlight > maxTOF)
                  {
                     maxTOF = timeOfFlight;
                  }
               }
            }
         }
      }
   }

   if (constraintValid)
   {
      mMaxDeltaAltitude = maxAlt;
      mMinDeltaAltitude = minAlt;
      mMaxSlantRange    = maxRng;
      mMinSlantRange    = minRng;
      mMaxTOF           = maxTOF;
   }

   return constraintValid;
}


bool SAM_LaunchComputerGenerator::PlaceNewTarget(double aSimTime, WsfTrack& aTrack)
{
   // Clone a platform of the desired type

   WsfPlatform* tgtPtr = WsfPlatformTypes::Get(GetScenario()).Clone(mTargetPlatformTypeId);
   if (tgtPtr == nullptr)
   {
      auto logError = ut::log::error() << "Could not clone a target platform of Type!";
      logError.AddNote() << "Type: " << mTargetPlatformTypeId;
      return false;
   }

   double co = CurrentOffset();
   double cr = CurrentGrndRng();
   if (co > cr)
   {
      if (DebugEnabled())
      {
         ut::log::debug() << "IGNORED - Cannot place a target where the offset is greater than ground range.";
      }
      return false;
   }

   if (CurrentDownRng() < 0.0)
   {
      if (DebugEnabled())
      {
         ut::log::debug() << "IGNORED - Cannot place a target where the downrange value is invalid.";
      }
      return false;
   }


   // Set the location, orientation and speed
   // From the launcher, compute the target position down range.
   ComputeTargetTrack();

   // The target heading
   double tgtHeadingRad = UtMath::NormalizeAngle0_TwoPi(mTgtHdgRad);

   WsfRouteMover* moverPtr = dynamic_cast<WsfRouteMover*>(tgtPtr->GetMover());
   if (moverPtr != nullptr)
   {
      WsfWaypoint wp(mTgtLatDeg, mTgtLonDeg, mTgtAlt, mTgtSpeed);
      wp.SetHeading(tgtHeadingRad);
      WsfRoute route;
      route.Append(wp);
      moverPtr->SetRoute(aSimTime, route);
   }
   tgtPtr->SetLocationLLA(mTgtLatDeg, mTgtLonDeg, mTgtAlt);
   tgtPtr->SetOrientationNED(tgtHeadingRad, 0.0, 0.0);
   double tgtVelNED[3] = {mTgtSpeed * cos(tgtHeadingRad), mTgtSpeed * sin(tgtHeadingRad), 0.0};
   tgtPtr->SetVelocityNED(tgtVelNED);

   // Give the target a name...
   std::ostringstream oss;
   ++mTargetCount;
   oss << "target_" << mTargetCount;
   std::string tgtName(oss.str());
   tgtPtr->SetName(tgtName);
   if (!GetSimulation()->AddPlatform(aSimTime, tgtPtr))
   {
      ut::log::error() << "Unable to add target platform to the simulation";
      delete tgtPtr;
      return false;
   }
   mTgtIndex = tgtPtr->GetIndex();

   // Initialize the track.
   WsfTrackId trackId;
   trackId.SetOwningPlatformId(mLaunchPlatPtr->GetNameId());
   trackId.SetLocalTrackNumber(mTargetCount);
   double launchLocWCS[3];
   mLaunchPlatPtr->GetLocationWCS(launchLocWCS);
   aTrack.SetOriginatorLocationWCS(launchLocWCS);
   aTrack.Initialize(aSimTime, trackId, *GetSimulation());
   aTrack.Update(aSimTime);

   double tgtLocWCS[3];
   tgtPtr->GetLocationWCS(tgtLocWCS);
   aTrack.SetLocationWCS(tgtLocWCS);
   aTrack.SetWCS_LocationValid(true);
   double tgtVelWCS[3];
   tgtPtr->GetVelocityWCS(tgtVelWCS);
   aTrack.SetVelocityWCS(tgtVelWCS);
   aTrack.SetVelocityValid(true);
   aTrack.SetTargetIndex(mTgtIndex);
   aTrack.Update(aSimTime);

   return true;
}

void SAM_LaunchComputerGenerator::ComputeTargetTrack()
{
   // debug information FYI:
   // double groundRng = CurrentGrndRng();
   // double offset = CurrentOffset();
   // double downRng = CurrentDownRng();

   double downRangeHeadingDeg = UtMath::NormalizeAngle0_360(mHeadingRad * UtMath::cDEG_PER_RAD);
   double nextHeadingDeg      = downRangeHeadingDeg;

   double offsetCornerLatDeg = mLatDeg;
   double offsetCornerLonDeg = mLonDeg;

   mTgtLatDeg = offsetCornerLatDeg;
   mTgtLonDeg = offsetCornerLonDeg;
   mTgtHdgRad = mHeadingRad;

   bool targetIsDisplacedFromShooter = false;

   if (CurrentOffset() >= 1.0)
   {
      targetIsDisplacedFromShooter = true;

      // Compute the (Lat,Lon) at the nearest pass (offset) corner.
      // (This is 90 degrees right of shooter-to-target LOS.)
      UtSphericalEarth::ExtrapolateGreatCirclePosition(mLatDeg,
                                                       mLonDeg,
                                                       downRangeHeadingDeg + 90,
                                                       CurrentOffset(),
                                                       offsetCornerLatDeg,
                                                       offsetCornerLonDeg);

      // Now find out what the local heading is at the "offset" corner,
      // so we can turn 90 degrees left at the end.  Note that by calling convention,
      // we have to reverse beginning and ending points to get the corner heading
      double checkCurrentOffset;
      // (Incidentally, checkCurrentOffset should be equal to offset.)
      UtSphericalEarth::GreatCircleHeadingAndDistance(offsetCornerLatDeg,
                                                      offsetCornerLonDeg,
                                                      mLatDeg,
                                                      mLonDeg,
                                                      nextHeadingDeg,
                                                      checkCurrentOffset);

      assert(fabs(checkCurrentOffset - CurrentOffset()) < 1.0);

      // Then do a 180 degree turn:
      nextHeadingDeg += 180;

      // Then turn 90 deg left.
      nextHeadingDeg += -90;
   }

   if (CurrentDownRng() >= 1.0)
   {
      targetIsDisplacedFromShooter = true;

      // Translate forward by CurrentDownRng() amount to arrive at the target location.
      UtSphericalEarth::ExtrapolateGreatCirclePosition(offsetCornerLatDeg,
                                                       offsetCornerLonDeg,
                                                       nextHeadingDeg,
                                                       CurrentDownRng(),
                                                       mTgtLatDeg,
                                                       mTgtLonDeg);

      assert(mTgtLatDeg == mTgtLatDeg);
      assert(mTgtLonDeg == mTgtLonDeg);
   }

   if (targetIsDisplacedFromShooter && (CurrentDownRng() > 0.0))
   {
      // Calculate the target heading (needed to achieve the desired offset distance).
      double checkCurrentDownRange;
      double targetHeadingDeg;
      UtSphericalEarth::GreatCircleHeadingAndDistance(mTgtLatDeg,
                                                      mTgtLonDeg,
                                                      offsetCornerLatDeg,
                                                      offsetCornerLonDeg,
                                                      targetHeadingDeg,
                                                      checkCurrentDownRange);
      mTgtHdgRad = UtMath::NormalizeAngleMinusPi_Pi(targetHeadingDeg * UtMath::cRAD_PER_DEG);
   }

   // Now confirm that the result from shooter to target is CurrentGrndRng()
   double losHdgDegNotUsed;
   double checkCurrentGrndRange;
   UtSphericalEarth::GreatCircleHeadingAndDistance(mLatDeg, mLonDeg, mTgtLatDeg, mTgtLonDeg, losHdgDegNotUsed, checkCurrentGrndRange);

   // Now resolve the target state.
   mTgtAlt   = CurrentAlt();
   mTgtSpeed = CurrentSpeed();
}

// virtual
bool SAM_LaunchComputerGenerator::FireNewWeapon(double aSimTime)
{
   WsfTrack tgtTrack;
   bool     result = PlaceNewTarget(aSimTime, tgtTrack);
   if (result)
   {
      bool okToFire = true;

      // Check to see if target is masked from the tracker by the horizon.

      if ((mTrackerHeight > 0.0) &&
          UtSphericalEarth::MaskedByHorizon(mLatDeg, mLonDeg, mTrackerHeight, mTgtLatDeg, mTgtLonDeg, mTgtAlt, mEffectiveEarthRadiusRatio))
      {
         if (DebugEnabled())
         {
            auto logDebug = ut::log::debug() << "Launch suppressed due to tracker terrain masking";
            logDebug.AddNote() << "T = " << aSimTime;
            logDebug.AddNote() << "Range: " << CurrentGrndRng() << " m";
            logDebug.AddNote() << "Altitude: " << CurrentAlt() << " m";
         }
         mObserver.LaunchAborted(aSimTime);
         okToFire = false;
      }

      // Check to see if the target is within the tracker elevation limits.

      if (okToFire)
      {
         double tgtLocNED[3];
         mLaunchPlatPtr->ConvertLLAToNED(mTgtLatDeg, mTgtLonDeg, mTgtAlt, tgtLocNED);
         double xy      = sqrt(tgtLocNED[0] * tgtLocNED[0] + tgtLocNED[1] * tgtLocNED[1]);
         double z       = -tgtLocNED[2];
         double tgtElev = atan2(z, xy);
         if ((tgtElev < mTrackerMinEl) || (tgtElev > mTrackerMaxEl))
         {
            if (DebugEnabled())
            {
               auto logDebug = ut::log::debug() << "Launch suppressed due to tracker elevation masking";
               logDebug.AddNote() << "T = " << aSimTime;
               logDebug.AddNote() << "Range: " << CurrentGrndRng() << " m";
               logDebug.AddNote() << "Altitude: " << CurrentAlt() << " m";
            }
            mObserver.LaunchAborted(aSimTime);
            okToFire = false;
         }
      }

      // Proceed with launch if all conditions met.

      if (okToFire)
      {
         if (DebugEnabled())
         {
            auto logDebug = ut::log::debug() << "Launching Weapon Type.";
            logDebug.AddNote() << "Weapon Type: " << WeaponPlatformTypeId();
            logDebug.AddNote() << "T = " << aSimTime;
            logDebug.AddNote() << "Grnd Range: " << CurrentGrndRng() << " m";
            logDebug.AddNote() << "Lat Offset: " << CurrentOffset() << " m";
            logDebug.AddNote() << "Altitude: " << CurrentAlt() << " m";
         }
         mLaunchWpnPtr->CueToTarget(aSimTime, &tgtTrack);
         result = mLaunchWpnPtr->Fire(aSimTime, &tgtTrack, WsfWeapon::FireOptions());
      }
   }

   /*if (! result)
   {
      if (DebugEnabled())
      {
         cout << "  ... but launch attempt FAILED." << endl;
      }
      ErrorBailout();
   }*/

   return result;
}

double SAM_LaunchComputerGenerator::CurrentDownRng()
{
   // According to the spherical Pythagorean theorem:
   // cos (C/R) = cos (A/R) * cos (B/R)
   // So B = R * acos( cos(C/R) / cos (A/R) )
   double of = CurrentOffset();
   double gr = CurrentGrndRng();
   if (fabs(of) > gr)
   {
      return cNOT_VALID;
   }
   else if (fabs(of) == fabs(gr))
   {
      return 0.0;
   }

   double cosCR = cos(gr / UtSphericalEarth::cEARTH_RADIUS);
   double cosAR = cos(of / UtSphericalEarth::cEARTH_RADIUS);
   return UtSphericalEarth::cEARTH_RADIUS * acos(cosCR / cosAR);
}
