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

#include "ATA_LaunchComputerGenerator.hpp"

#include <ctime>
#include <sstream>

#include "UtAtmosphere.hpp"
#include "UtInput.hpp"
#include "UtInputFile.hpp"
#include "UtMath.hpp"
#include "UtVec3.hpp"
#include "WeaponObserver.hpp"
#include "WsfATA_LaunchComputer.hpp"
#include "WsfGuidedMover.hpp"
#include "WsfLaunchComputerTypes.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfRouteMover.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfStringId.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"
#include "WsfWeapon.hpp"
#include "WsfWeaponEngagement.hpp"

namespace
{
const char* cATA_LAUNCHCOMPUTER_DEFAULT_PRODUCT = "_ATA_LAUNCH_COMPUTER";
}

ATA_LaunchComputerGenerator::ATA_LaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mTargetCount(0)
   , mTgtIndex(0)
   , mQuantityLaunched(0)
   , mHitCount(0)
   , mMissCount(0)
   , mSpeed(197.035)
   , // Tool defaults altitude to 10k ft.  This speed = Mach 0.6 at 10k alt.
   mSavedMinRange(999999999)
   , mSavedMaxRange(0.0)
   , mSavedMaxAzimuth(0.0)
   , mSavedMaxElevation(-UtMath::cPI)
   , mSavedMinElevation(UtMath::cPI)
   , mSavedMaxRangeRate(-999999999)
   , mSavedMinRangeRate(999999999)
   , mSavedMaxBoresight(0.0)
   , mFirstTime(true)
   , mFirstPass(true)
   , mHitAtShortestRange(false)
   , mHitAtLongestRange(false)
   , mState(cUNINITIALIZED)
   , mOutFile()
   , mGenTime()
   , mATALCPtr(new WsfATA_LaunchComputer())
{
   mToolProduces = cATA_LAUNCHCOMPUTER_DEFAULT_PRODUCT;
   mAlt          = 10000.0 * UtMath::cM_PER_FT; // set default altitude to 10k ft
}

ATA_LaunchComputerGenerator::ATA_LaunchComputerGenerator(const ATA_LaunchComputerGenerator& aSrc)
   : Tool(aSrc)
   , mTargetCount(aSrc.mTargetCount)
   , mTgtIndex(aSrc.mTgtIndex)
   , mQuantityLaunched(aSrc.mQuantityLaunched)
   , mHitCount(aSrc.mHitCount)
   , mMissCount(aSrc.mMissCount)
   , mSpeed(aSrc.mSpeed)
   , mSavedMinRange(aSrc.mSavedMinRange)
   , mSavedMaxRange(aSrc.mSavedMaxRange)
   , mSavedMaxAzimuth(aSrc.mSavedMaxAzimuth)
   , mSavedMaxElevation(aSrc.mSavedMaxElevation)
   , mSavedMinElevation(aSrc.mSavedMinElevation)
   , mSavedMaxRangeRate(aSrc.mSavedMaxRangeRate)
   , mSavedMinRangeRate(aSrc.mSavedMinRangeRate)
   , mSavedMaxBoresight(aSrc.mSavedMaxBoresight)
   , mFirstTime(aSrc.mFirstTime)
   , mFirstPass(aSrc.mFirstPass)
   , mHitAtShortestRange(aSrc.mHitAtShortestRange)
   , mHitAtLongestRange(aSrc.mHitAtLongestRange)
   , mState(aSrc.mState)
   , mOutFile()
   , mGenTime(aSrc.mGenTime)
   , mATALCPtr(dynamic_cast<WsfATA_LaunchComputer*>(aSrc.mATALCPtr->Clone()))
{
}

// virtual
ATA_LaunchComputerGenerator::~ATA_LaunchComputerGenerator()
{
   delete mATALCPtr;
}

// virtual
Tool* ATA_LaunchComputerGenerator::Clone() const
{
   return new ATA_LaunchComputerGenerator(*this);
}

// virtual
bool ATA_LaunchComputerGenerator::Initialize(WsfSimulation& aSimulation)
{
   Tool::Initialize(aSimulation);

   mFirstPass   = true;
   mFirstTime   = true;
   mTargetCount = 0;
   mTgtIndex    = 0;

   time_t aclock;
   time(&aclock);
   struct tm*  newtime = localtime(&aclock);
   std::string stamp1("Written by ATA_LaunchComputerGenerator on ");
   std::string stamp2(asctime(newtime));
   mGenTime = std::string(stamp1 + stamp2);

   // Open the output file.
   std::string fileName = mOutputFileName;
   mOutFile.open(fileName.c_str());
   if (!mOutFile)
   {
      auto logError = ut::log::error() << "Unable to open output File!";
      logError.AddNote() << "File: " << mOutputFileName;
      mInitialized = false;
   }

   if (mInitialized)
   {
      mATALCPtr->ResetAllIndices();
      unsigned int resultsSize = mATALCPtr->SizeOfResults();
      mATALCPtr->AllocateResults(resultsSize);
      mInitialized = mATALCPtr->ToolOnlyInitialize(aSimulation.GetSimTime());
      mState       = cINITIALIZED;
   }

   mSavedMinRange     = 999999999;
   mSavedMaxRange     = 0.0;
   mSavedMaxAzimuth   = 0.0;
   mSavedMaxElevation = -UtMath::cPI;
   mSavedMinElevation = UtMath::cPI;
   mSavedMaxRangeRate = -999999999;
   mSavedMinRangeRate = 999999999;
   mSavedMaxBoresight = 0.0;

   mHitCount  = 0;
   mMissCount = 0;

   mATALCPtr->SetDebugEnabled(mInitialized && DebugEnabled());

   if (!mInitialized)
   {
      Tool::UnInitialize();
   }

   return mInitialized;
}

void ATA_LaunchComputerGenerator::ErrorBailout()
{
   Finish();
   ut::log::error() << "Unknown Error in ATA_LaunchComputerGenerator has caused a premature abort!";
}

// virtual
bool ATA_LaunchComputerGenerator::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());

   if (command == "speed")
   {
      aInput.ReadValueOfType(mSpeed, UtInput::cSPEED);
      aInput.ValueGreaterOrEqual(mSpeed, 0.0);
   }
   else if (command == "altitude_and_mach")
   {
      aInput.ReadValueOfType(mAlt, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(mAlt, 0.0);
      double mach = 0.0;
      aInput.ReadValue(mach);
      aInput.ValueGreaterOrEqual(mach, 0.0);
      mSpeed = mach * GetScenario().GetAtmosphere().SonicVelocity(mAlt);
   }
   else if (mATALCPtr->ProcessInput(aInput))
   {
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}

// virtual
void ATA_LaunchComputerGenerator::Update(double aSimTime)
{
   if (aSimTime > mLastUpdate)
   {
      if (mState == cINITIALIZED)
      {
         mState = cENVELOPE_GENERATION;
         mATALCPtr->ResetAllIndices();
         FireNewWeapon(aSimTime);
      }
      else if (mState == cENVELOPE_GENERATION)
      {
         // Check to see if the previously fired weapon has terminated yet...
         if (mObserver.GetState() == WeaponObserver::cTERMINATED)
         {
            // When we encounter a weapon hit, if it was lethal, save off
            // the result, and also any successful constraint maxima or minima.
            if (mObserver.EngagementLethal())
            {
               ++mHitCount;
               double TOF = mObserver.TimeOfFlight();
               mATALCPtr->SetResult(TOF);

               if (mATALCPtr->RangeIndex() == 0)
               {
                  mHitAtShortestRange = true;
               }

               if (mATALCPtr->RangeIndex() == mATALCPtr->Ranges().GetNumBins() - 1)
               {
                  mHitAtLongestRange = true;
               }

               double currentRange = mATALCPtr->TableTargetRange();
               if (mSavedMaxRange < currentRange)
               {
                  mSavedMaxRange = currentRange;
               }
               if (mSavedMinRange > currentRange)
               {
                  mSavedMinRange = currentRange;
               }

               double az = mATALCPtr->TableTargetAzimuth();

               if (mSavedMaxAzimuth < fabs(az))
               {
                  mSavedMaxAzimuth = fabs(az);
               }

               double el = mATALCPtr->TableTargetElevation();

               if (mSavedMaxElevation < el)
               {
                  mSavedMaxElevation = el;
               }

               if (mSavedMinElevation > el)
               {
                  mSavedMinElevation = el;
               }

               double cosBS     = cos(az) * cos(el);
               double boresight = acos(cosBS);

               if (mSavedMaxBoresight < boresight)
               {
                  mSavedMaxBoresight = boresight;
               }

               double rr = mATALCPtr->TableTargetRangeRate();

               if (mSavedMaxRangeRate < rr)
               {
                  mSavedMaxRangeRate = rr;
               }

               if (mSavedMinRangeRate > rr)
               {
                  mSavedMinRangeRate = rr;
               }
            }
            else
            {
               ++mMissCount;
            }

            if (DebugEnabled())
            {
               auto logDebug = ut::log::debug() << "ATA_LaunchComputerGenerator::Update Data Point: ";
               logDebug.AddNote() << "Aspect: " << mATALCPtr->TableTargetAspect() * UtMath::cDEG_PER_RAD << " deg";
               logDebug.AddNote() << "Azimuth: " << mATALCPtr->TableTargetAzimuth() * UtMath::cDEG_PER_RAD << " deg";
               logDebug.AddNote() << "Elevation: " << mATALCPtr->TableTargetElevation() * UtMath::cDEG_PER_RAD << " deg";
               logDebug.AddNote() << "Range Rate: " << mATALCPtr->TableTargetRangeRate() << " m/s";
               logDebug.AddNote() << "Target Range: " << mATALCPtr->TableTargetRange() << " m";
               (mATALCPtr->GetResult()) ? logDebug.AddNote() << "HIT" : logDebug.AddNote() << "MISS";
            }

            // Previously fired weapon went away.  Remove the old target,
            // if our own weapon didn't already kill it.
            WsfPlatform* oldTgtPtr = GetSimulation()->GetPlatformByIndex(mTgtIndex);
            if (oldTgtPtr != nullptr)
            {
               GetSimulation()->DeletePlatform(aSimTime, oldTgtPtr);
            }
            mTgtIndex = 0;
            mObserver.ResetState();
         }

         if (mObserver.GetState() == WeaponObserver::cIDLE)
         {
            // Increment the engagement conditions for the next shot..
            // Run the matrix with later values changing the fastest (range, in this case):
            if (!mATALCPtr->IncrementRange())
            {
               // Return false means we could not increment the condition... we are done.
               Finish();
            }

            if (mState == cDONE)
            {
               WriteOutputFile();
            }
            else
            {
               FireNewWeapon(aSimTime);
            }
         }
      } // if mState

      Tool::Update(aSimTime);
   }
   else if (aSimTime < mLastUpdate)
   {
      auto logError = ut::log::error() << "Timing error...";
      logError.AddNote() << "T = " << aSimTime;
      logError.AddNote() << "Last Update: " << mLastUpdate;
   }
}

//! Output a data file defining the Launch Computer just generated.
void ATA_LaunchComputerGenerator::WriteOutputFile()
{
   if (mHitAtShortestRange || mHitAtLongestRange)
   {
      // output to the generated file requires a prefix of # for commenting and Warning:
      // to issue a warning. However, The logger util has its own prefixing so i have separated
      // the strings so appropriate use depending on output could occur.
      char        fileCommentChar = '#';
      std::string warningPrefix("Warning: ");
      std::string str1("LAR Generator encountered a weapon hit at the ");
      std::string str2("undefined");
      if (mHitAtShortestRange && mHitAtLongestRange)
      {
         str2 = "longest and shortest";
      }
      else if (mHitAtShortestRange)
      {
         str2 = "shortest";
      }
      else if (mHitAtLongestRange)
      {
         str2 = "longest";
      }

      std::string str3(" specified range.");
      std::string scream1(str1 + str2 + str3);
      std::string scream2("Recommend this weapon tool be re-run, specifying a greater length of target ranges.");

      mOutFile << fileCommentChar << warningPrefix << scream1 << std::endl;
      if (mHitAtLongestRange)
      {
         mOutFile << fileCommentChar << ' ' << scream2 << std::endl;
      }

      auto logWarning = ut::log::warning() << scream1;
      if (mHitAtLongestRange)
      {
         logWarning.AddNote() << scream2;
      }
   }

   // Write out the launch computer stuff.
   if (mOutFile)
   {
      mOutFile << std::endl;
      mOutFile << "launch_computer " << mOutputObjectName << " WSF_ATA_LAUNCH_COMPUTER" << std::endl << std::endl;
      mOutFile << "   # Use caution when hand-editing this file.  Some values are computer generated, and should not"
               << std::endl
               << "   # be overridden.  " << mGenTime; // << endl;
      double mach = mSpeed / GetScenario().GetAtmosphere().SonicVelocity(mAlt);
      double launchDV[3];
      mLaunchWpnPtr->GetLaunchDeltaV(launchDV);
      mOutFile << "   # Valid for altitude = " << mAlt << " m, and speed = " << mSpeed << " m/s ( Mach = " << mach
               << " ), and DeltaV = (" << launchDV[0] << ' ' << launchDV[1] << ' ' << launchDV[2] << ") m/s" << std::endl;
      mOutFile << "   # Global launch constraints (below) will always be checked before a more expensive table lookup "
                  "is done."
               << std::endl
               << std::endl;

      // Output global constraint values into the file...
      mOutFile << "   minimum_slant_range " << mSavedMinRange << " m" << std::endl;
      mOutFile << "   maximum_slant_range " << mSavedMaxRange << " m" << std::endl;
      // mOutFile << "   global_maximum_azimuth " << mSavedMaxAzimuth * UtMath::cDEG_PER_RAD << " deg" << endl;
      // mOutFile << "   global_maximum_elevation " << mSavedMaxElevation * UtMath::cDEG_PER_RAD << " deg" << endl;
      // mOutFile << "   global_minimum_elevation " << mSavedMinElevation * UtMath::cDEG_PER_RAD << " deg" << endl;
      mOutFile << "   maximum_boresight_angle " << mSavedMaxBoresight * UtMath::cDEG_PER_RAD << " deg" << std::endl;
      mOutFile << "   maximum_opening_speed " << mSavedMaxRangeRate << " m/s" << std::endl;
      mOutFile << "   minimum_opening_speed " << mSavedMinRangeRate << " m/s" << std::endl << std::endl;

      mOutFile << "   intercept_results" << std::endl;
      mOutFile << "      # The independent variable ranges below must be defined before the results table..." << std::endl;
      mOutFile << "      # Aspect angle has to cover zero to 180 degrees approach angle," << std::endl;
      mOutFile << "      # and results are assumed symmetric about Left and Right," << std::endl;
      mOutFile << "      # so the only question is how many bins (what resolution)." << std::endl;
      mOutFile << "      number_of_aspect_angle_bins " << mATALCPtr->Aspects().GetNumBins() << std::endl;
      mOutFile << "      # Azimuth results are assumed symmetric about the centerline, so should cover zero to"
               << std::endl;
      mOutFile << "      # <limit value> target azimuth angle, user must specify how many bins (what resolution)."
               << std::endl;
      mOutFile << "      azimuth_bin_count_and_limit " << mATALCPtr->Azimuths().GetNumBins() << " "
               << mATALCPtr->Azimuths().Max() * UtMath::cDEG_PER_RAD << " deg" << std::endl;
      mOutFile << "      # Elevation results are not assumed symmetric about the horizon, so should cover" << std::endl;
      mOutFile
         << "      # <negative limit> to <positive limit> target elevation angle, and how many bins (what resolution)."
         << std::endl;
      mOutFile << "      elevation_bin_count_min_and_max_limits " << mATALCPtr->Elevations().GetNumBins() << " "
               << mATALCPtr->Elevations().Min() * UtMath::cDEG_PER_RAD << " deg "
               << mATALCPtr->Elevations().Max() * UtMath::cDEG_PER_RAD << " deg" << std::endl;
      mOutFile << "      range_rate_bin_count_min_and_max_limits " << mATALCPtr->RangeRates().GetNumBins() << " "
               << mATALCPtr->RangeRates().Min() << " m/s " << mATALCPtr->RangeRates().Max() << " m/s" << std::endl;
      mOutFile << "      firing_range_bin_count_and_values " << mATALCPtr->Ranges().GetNumBins() << std::endl
               << "          ";
      double       value;
      unsigned int numWritten = 0;
      for (unsigned int i = 0; i < mATALCPtr->Ranges().GetNumBins(); ++i)
      {
         if (mATALCPtr->Ranges().BinLower(i, value))
         {
            mOutFile << " " << value << " m ";
            ++numWritten;
         }
         if (numWritten % 8 == 0)
         {
            mOutFile << std::endl << "          ";
         }
      }
      mOutFile << " " << mATALCPtr->Ranges().Max() << " m " << std::endl;

      mOutFile << "      time_of_flight_values" << std::endl;
      mOutFile << "         # Dependent Variable = Time Of Flight (sec.)  [0 => Miss] Hits/Misses = " << mHitCount
               << " / " << mMissCount;

      int resultsPerLine = 30;
      int numResults     = 0;
      mATALCPtr->ResetAllIndices();
      do
      {
         if ((numResults % resultsPerLine) == 0)
            mOutFile << std::endl << "         ";
         mOutFile << mATALCPtr->GetResult() << ' ';
         ++numResults;
      } while (mATALCPtr->IncrementRange());

      mOutFile << std::endl;
      mOutFile << "      end_time_of_flight_values" << std::endl;
      mOutFile << "   end_intercept_results" << std::endl;
      mOutFile << "end_launch_computer" << std::endl;
      mOutFile.close();

#ifdef DEV_DEBUG
      if (false) // used only for debugging
      {
         // Now, feed the resulting text file to the class,
         // to assure that it can be read correctly:
         try
         {
            // Open the input file...
            UtInput testInput;
            testInput.PushInput(ut::make_unique<UtInputFile>(mOutputFileName));
            string command;
            testInput.ReadCommand(command);
            WsfLaunchComputerTypes::Get(GetScenario()).LoadType(testInput);
         }
         catch (UtInput::BadValue& aException)
         {
            auto logDev =
               ut::log::developer()
               << "ATA_LaunchComputerGenerator::WriteOutputFile() - Bad value found during re-reading the output file:";
            logDev.AddNote() << "Exception: " << aException.GetMessage();
         }
         catch (UtInput::EndOfData& aException)
         {
            auto logDev = ut::log::developer() << "ATA_LaunchComputerGenerator::WriteOutputFile() - Early end-of-data "
                                                  "encountered in re-reading output file:";
            logDev.AddNote() << "Exception: " << aException.GetMessage();
         }
         catch (UtException& aException)
         {
            auto logDev = ut::log::developer() << "ATA_LaunchComputerGenerator::WriteOutputFile() - Unknown "
                                                  "UtException encountered in re-reading output file:";
            logDev.AddNote() << "Exception: " << aException.what();
         }
         catch (...)
         {
            ut::log::developer() << "ATA_LaunchComputerGenerator::WriteOutputFile() - Unknown exception encountered in "
                                    "re-reading output file.  Rethrown.";
            throw;
         }
      }
#endif
   }
}

// virtual
bool ATA_LaunchComputerGenerator::PlaceNewTarget(double aSimTime, WsfTrack& aTrack)
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
   mTgtIndex = 0;

   // Use a UtEntity to calculate the relative geometry
   // between the launching platform and the target.
   UtEntity ute;
   ute.SetLocationLLA(mLatDeg, mLonDeg, 0.0);
   ute.SetOrientationNED(mHeadingRad, 0.0, 0.0);

   double launcherWCS[3];
   ute.GetLocationWCS(launcherWCS);

   // Now satisfy all the constraints just set:
   // (Note that we ignore the launching aircraft pitch and roll angles.)

   // 1. Target Position:
   double tgtLocalFRDOffset[3];
   double range         = mATALCPtr->TableTargetRange();
   double azimuth       = mATALCPtr->TableTargetAzimuth();
   double elevation     = mATALCPtr->TableTargetElevation();
   tgtLocalFRDOffset[0] = cos(azimuth) * cos(elevation) * range;
   tgtLocalFRDOffset[1] = sin(azimuth) * cos(elevation) * range;
   tgtLocalFRDOffset[2] = -sin(elevation) * range;
   // Transform FRD position to NED:
   double tgtNED[3];
   tgtNED[0] = cos(mHeadingRad) * tgtLocalFRDOffset[0] - sin(mHeadingRad) * tgtLocalFRDOffset[1];
   tgtNED[1] = sin(mHeadingRad) * tgtLocalFRDOffset[0] + cos(mHeadingRad) * tgtLocalFRDOffset[1];
   tgtNED[2] = -mAlt + tgtLocalFRDOffset[2];
   // Transform NED to WCS:
   double tgtLocWCS[3];
   ute.ConvertNEDToWCS(tgtNED, tgtLocWCS);

   // Target compass bearing is based only on his NED offset from me.
   double tgtBearing = atan2(tgtNED[1], tgtNED[0]);

   // If the target were to have me at his zero aspect (nose on),
   // he would be pointing 180 degrees different from his bearing (from my perspective).
   double headingForZeroAspect = tgtBearing + UtMath::cPI;

   // BUT, we need to rotate him so that _I_ am at his specified aspect angle.
   // Note that there could be two possibilities, toward or away from me.
   double tgtHdg1 = headingForZeroAspect + mATALCPtr->TableTargetAspect();
   double tgtHdg2 = headingForZeroAspect - mATALCPtr->TableTargetAspect();

   // 2. Target Velocity (assume level flight, no maneuvering):
   // We must give him a particular velocity so he is at the specified range rate.

   // We are both moving, and part of his range rate is due to MY velocity toward him.
   double myRR = -mSpeed * cos(mHeadingRad - tgtBearing) * cos(elevation);

   // The rest of the Range Rate must be due to his motion...
   // (He has this amount of inertial range rate along my direction.)
   double tgtRR = mATALCPtr->TableTargetRangeRate() - myRR;

   double hdgDif1   = fabs(tgtHdg1 - mHeadingRad);
   double hdgDif2   = fabs(tgtHdg2 - mHeadingRad);
   bool   useHdg1   = true;
   bool   useHdg2   = true;
   double tgtSpeed1 = 0.0;
   double tgtSpeed2 = 0.0;

   if (fabs((fabs(hdgDif1) - 90.0 * UtMath::cRAD_PER_DEG)) > (5.0 * UtMath::cRAD_PER_DEG))
   {
      tgtSpeed1 = tgtRR / cos(hdgDif1) / cos(elevation);
      if ((tgtSpeed1 < 0.0) || (tgtSpeed1 > 1200.0))
      {
         useHdg1 = false;
      }
   }
   else
   {
      useHdg1 = false;
   }

   if (fabs((fabs(hdgDif2) - 90.0 * UtMath::cRAD_PER_DEG)) > (5.0 * UtMath::cRAD_PER_DEG))
   {
      tgtSpeed2 = tgtRR / cos(hdgDif2) / cos(elevation);
      if ((tgtSpeed2 < 0.0) || (tgtSpeed2 > 1200.0))
      {
         useHdg2 = false;
      }
   }
   else
   {
      useHdg2 = false;
   }

   if (!(useHdg1 || useHdg2))
   {
      // No usable solutions... ignore this data point.
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "ATA_LC could not place target with conditions.";
         logDebug.AddNote() << "Quantity Launched: " << mQuantityLaunched + 1;
         logDebug.AddNote() << "Target Range: " << mATALCPtr->TableTargetRange();
         logDebug.AddNote() << "Target Azimuth: " << mATALCPtr->TableTargetAzimuth() * UtMath::cDEG_PER_RAD;
         logDebug.AddNote() << "Target Elevation: " << mATALCPtr->TableTargetElevation() * UtMath::cDEG_PER_RAD;
      }
      return false;
   }
   else if (useHdg1 && useHdg2)
   {
      if (tgtSpeed1 < tgtSpeed2)
      {
         useHdg2 = false;
      }
      else
      {
         useHdg1 = false;
      }
   }

   double tgtSpeed   = tgtSpeed1;
   double tgtHeading = tgtHdg1;
   if (useHdg2)
   {
      tgtSpeed   = tgtSpeed2;
      tgtHeading = tgtHdg2;
   }

   double tgtLat, tgtLon, tgtAlt;
   UtEntity::ConvertWCSToLLA(tgtLocWCS, tgtLat, tgtLon, tgtAlt);

   if (tgtAlt < 0.0)
   {
      // Target would be placed below ground.  Ignore.
      if (DebugEnabled())
      {
         ut::log::debug() << "ATALaunchComputer could not place a target below ground.  Ignored:  YYYY";
      }
      return false;
   }

   WsfPlatform* tgtPtr = WsfPlatformTypes::Get(GetScenario()).Clone(mTargetPlatformTypeId);

   if (tgtPtr == nullptr)
   {
      auto logError = ut::log::error() << "ATA_LaunchComputerGenerator::FireNewWeapon"
                                       << " could not clone a target platform of Type!";
      logError.AddNote() << "Type: " << mTargetPlatformTypeId;
      return false;
   }

   tgtPtr->SetLocationLLA(tgtLat, tgtLon, 1.0);
   WsfRouteMover* moverPtr = dynamic_cast<WsfRouteMover*>(tgtPtr->GetMover());
   if (moverPtr != nullptr)
   {
      WsfWaypoint wp(tgtLat, tgtLon, tgtAlt, tgtSpeed); // alt & speed = 0
      wp.ExtrapolateAlongHeading(tgtHeading);
      WsfRoute route;
      route.Append(wp);
      moverPtr->SetRoute(aSimTime, route);
   }
   tgtPtr->SetLocationWCS(tgtLocWCS);

   // Orientation of target doesn't matter for our purposes...
   tgtPtr->SetOrientationNED(tgtHeading, 0.0, 0.0);

   // Give the target a name...
   std::ostringstream oss;
   ++mTargetCount;
   oss << mTargetPlatformTypeId << "_ATA_" << mTargetCount;
   std::string tgtName(oss.str());
   tgtPtr->SetName(tgtName);
   GetSimulation()->AddPlatform(aSimTime, tgtPtr);
   mTgtIndex = tgtPtr->GetIndex();

   aTrack.SetLocationWCS(tgtLocWCS);
   aTrack.SetWCS_LocationValid(true);
   double tgtVelWCS[3];

   double tgtVelNED[3];
   tgtVelNED[0] = cos(tgtHeading) * tgtSpeed;
   tgtVelNED[1] = sin(tgtHeading) * tgtSpeed;
   tgtVelNED[2] = 0.0;

   tgtPtr->ConvertNEDVectorToWCS(tgtVelWCS, tgtVelNED);
   aTrack.SetVelocityWCS(tgtVelWCS);
   aTrack.SetVelocityValid(true);
   aTrack.SetTargetIndex(mTgtIndex);
   aTrack.Update(aSimTime);

   if (DebugEnabled())
   {
      auto logDebug = ut::log::debug() << "Successfully placed a target at conditions:";
      logDebug.AddNote() << "Quantity Launched: " << mQuantityLaunched + 1;
      logDebug.AddNote() << "Rng: " << mATALCPtr->TableTargetRange();
      logDebug.AddNote() << "Azimuth: " << mATALCPtr->TableTargetAzimuth() * UtMath::cDEG_PER_RAD << " deg";
      logDebug.AddNote() << "Elevation: " << mATALCPtr->TableTargetElevation() * UtMath::cDEG_PER_RAD << " deg.";
   }

   return true;
}

// virtual
bool ATA_LaunchComputerGenerator::FireNewWeapon(double aSimTime)
{
   bool success = true;

   // Create a new Target in the correct Location:
   WsfTrack tgtTrack;
   if (!PlaceNewTarget(aSimTime, tgtTrack))
   {
      // Could not place a new target, so go to the next point
      return false;
   }

   // Set the initial conditions of the launching platform:
   // This will continuously relocate the platform back to the starting point,
   // even though it is supposed to have a velocity... visually, it will "stutter"
   mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, mAlt);
   mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);
   double launchPlatVelNED[] = {mSpeed * cos(mHeadingRad), mSpeed * sin(mHeadingRad), 0.0};
   mLaunchPlatPtr->SetVelocityNED(launchPlatVelNED);

   success = mLaunchWpnPtr->Fire(aSimTime, &tgtTrack, WsfWeapon::FireOptions());

   if (success)
   {
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "ATA LC Generator successfully firing at placed target at Time.";
         logDebug.AddNote() << "T = " << aSimTime;
      }
      ++mQuantityLaunched;
   }
   else
   {
      mObserver.LaunchAborted(aSimTime);
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "ATA LC Generator failed launching at Time.";
         logDebug.AddNote() << "T = " << aSimTime;
      }
      if (mTerminateOnLaunchFailure)
      {
         ErrorBailout();
      }
   }

   return success;
}

//! Virtual method to resolve the output object name and output file names.
// virtual
void ATA_LaunchComputerGenerator::ResolveNames()
{
   std::string objectName = WeaponPlatformTypeId().GetString() + mToolProduces;

   if (mOutputObjectName.empty())
   {
      // This override inserts the "condition" qualifier into the name...
      double             mach = mSpeed / GetScenario().GetAtmosphere().SonicVelocity(mAlt);
      std::ostringstream oss;
      int                iAlt = static_cast<int>(mAlt);
      oss << "_alt" << iAlt << "_mach" << mach;
      std::string condition(oss.str());
      std::string fullName = objectName + condition;
      mOutputObjectName    = fullName;
   }

   if (mOutputFileName.empty())
   {
      std::string OutputFileName = mOutputObjectName + mFileExtension;
      mOutputFileName            = OutputFileName;
   }
}
