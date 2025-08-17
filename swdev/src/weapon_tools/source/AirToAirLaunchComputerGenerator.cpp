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

#include "AirToAirLaunchComputerGenerator.hpp"

#include <ctime>
#include <fstream>
#include <sstream>

// Weapon Tools
#include "TargetMover.hpp"

// WSF
#include "WsfLaunchComputerTypes.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"

// Utilities
#include "UtInputFile.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"

// ****************************************************************************
// Public

AirToAirLaunchComputerGenerator::AirToAirLaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mTargetCount(0)
   , mTargetIndex(0)
   , mQuantityLaunched(0)
   , mHitCount(0)
   , mMissCount(0)
   , mSignificant(0)
   , mNullCount(0)
   , mResultsSize(0)
   , mCurrentIndex(0)
   , mMaxIterations(100)
   , mIterationCount(0)
   , mIncrementRatio(1.5)
   , mFirstPass(false)
   , mAspectRotIsClockwise(true)
   , mAspectRotIsSignedToLead(false)
   , mShooterMach(0.0)
   , mShooterSpeed(0.0)
   , mTargetAlt(0.0)
   , mTargetMach(0.0)
   , mTargetAspect(0.0)
   , mTargetLead(0.0)
   , mState(cNO_STATE)
   , mSearchingFor(cNO_SEARCH)
   , mCurrent(0.0, false)
   , mPrevious(0.0, false)
   , mCurrentGrndRange(0.0)
   , mCurrentHit(false)
   , mCurrentTOF(0.0)
   , mGrndRangeTolerance(10.0)
   , mGeneratedTimeString()
   , mAtmosphere(aScenario.GetAtmosphere())
   , mATA_LaunchComputerPtr(new WsfAirToAirLaunchComputer(aScenario.GetAtmosphere()))
   , mTempResult()
   , mTableFileName()
   , mResultsFileName()
   , mLastTargetAspect(DBL_MAX)
   , mLastTargetLead(DBL_MAX)
   , mLastTargetMach(DBL_MAX)
   , mLastTargetAlt(DBL_MAX)
   , mLastShooterMach(DBL_MAX)
   , mLastShooterAlt(DBL_MAX)
   , mSuccesses(0)
   , mFailures(0)
   , mGlobalMinRmax(DBL_MAX)
   , mGlobalMaxRmax(-DBL_MAX)
   , mGlobalMinRmin(DBL_MAX)
   , mGlobalMaxRmin(-DBL_MAX)
   , mGlobalMinRne(DBL_MAX)
   , mGlobalMaxRne(-DBL_MAX)
   , mGlobalMinRmaxTOF(DBL_MAX)
   , mGlobalMaxRmaxTOF(-DBL_MAX)
   , mGlobalMinRminTOF(DBL_MAX)
   , mGlobalMaxRminTOF(-DBL_MAX)
   , mGlobalMinRneTOF(DBL_MAX)
   , mGlobalMaxRneTOF(-DBL_MAX)
   , mGenerationState(cNONE)
   , mMaxRange(120000.0)
   , mMinRange(1000.0)
{
   mToolProduces = "_" + WsfAirToAirLaunchComputer::BaseTypeName();
}

// =========================================================================
// virtual
AirToAirLaunchComputerGenerator::~AirToAirLaunchComputerGenerator()
{
   delete mATA_LaunchComputerPtr;
}

// =========================================================================
// virtual
Tool* AirToAirLaunchComputerGenerator::Clone() const
{
   AirToAirLaunchComputerGenerator* lcgPtr = new AirToAirLaunchComputerGenerator(*this);

   // Manually clone the pointer to the launch computer (can't copy pointer or memory will be deleted twice).
   lcgPtr->mATA_LaunchComputerPtr = static_cast<WsfAirToAirLaunchComputer*>(mATA_LaunchComputerPtr->Clone());
   return lcgPtr;
}

// ========================================================================
// virtual
bool AirToAirLaunchComputerGenerator::Initialize(WsfSimulation& aSimulation)
{
   // Initialize the base class first
   Tool::Initialize(aSimulation);

   mTargetCount = 0;
   mTargetIndex = 0;

   time_t aclock;
   time(&aclock);
   struct tm*  newtime = localtime(&aclock);
   std::string stamp1("Written by AirToAirLaunchComputerGenerator on ");
   std::string stamp2(asctime(newtime));
   mGeneratedTimeString = std::string(stamp1 + stamp2);

   if (mInitialized)
   {
      if (mLaunchWpnPtr->GetDeferredLaunchDelay() != 0)
      {
         ut::log::error()
            << "AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR does not support deferred launches. Initialization failed.";
         mInitialized = false;
      }
      else
      {
         mResultsSize     = mATA_LaunchComputerPtr->GetTable().SizeOfResultsSet();
         mCurrentIndex    = 0;
         mInitialized     = mATA_LaunchComputerPtr->ToolOnlyInitialize(aSimulation.GetSimTime());
         mState           = cSTATE_INITIALIZED;
         mGenerationState = cBRACKETING;

         std::string            fileName = mOutputFileName;
         std::string::size_type start    = fileName.find(".");
         mTableFileName   = fileName.substr(0, start) + "_table" + fileName.substr(start, std::string::npos);
         mResultsFileName = fileName.substr(0, start) + "_results" + fileName.substr(start, std::string::npos);
         std::fstream resultsFile(mResultsFileName.c_str(), std::ios::in);
         if (resultsFile.is_open())
         {
            // file exists, clear it out
            resultsFile.close();
            resultsFile.open(mResultsFileName.c_str(), std::ios::out | std::ios::trunc);
            resultsFile.close();
         }
      }
   }

   mHitCount       = 0;
   mMissCount      = 0;
   mSignificant    = 0;
   mNullCount      = 0;
   mIterationCount = 0;

   mATA_LaunchComputerPtr->SetDebugEnabled(mInitialized && DebugEnabled());

   // Cause all weapon fusing for MSL and AGL altitudes to be ignored.
   // This is needed, since if we were to try to generate WEZ results for
   // Sea Level, all missiles might continuously hit the ground.
   mObserver.DisableGroundFusing();

   if (!mInitialized)
   {
      Tool::UnInitialize();
   }

   return mInitialized;
}

// ========================================================================
// virtual
bool AirToAirLaunchComputerGenerator::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());
   if (mATA_LaunchComputerPtr->ProcessInput(aInput))
   {
   }
   else if (command == "minimum_range")
   {
      aInput.ReadValueOfType(mMinRange, UtInput::cLENGTH);
      aInput.ValueGreaterOrEqual(mMinRange, 0.0);
   }
   else if (command == "maximum_range")
   {
      aInput.ReadValueOfType(mMaxRange, UtInput::cLENGTH);
      aInput.ValueGreater(mMaxRange, 0.0);
   }
   else if ((command == "range_tolerance") || (command == "engagement_range_tolerance")) // legacy name
   {
      aInput.ReadValueOfType(mGrndRangeTolerance, UtInput::cLENGTH);
      aInput.ValueGreater(mGrndRangeTolerance, 0.0);
   }
   else if (command == "aspect_rotation_clockwise")
   {
      aInput.ReadValue(mAspectRotIsClockwise);
   }
   else if (command == "aspect_rotation_signed_to_lead_angle")
   {
      aInput.ReadValue(mAspectRotIsSignedToLead);
   }
   else if (command == "heading")
   {
      double dummy;
      aInput.ReadValueOfType(dummy, UtInput::cANGLE);
      if (dummy != 0.0)
      {
         ut::log::warning() << "Supplied heading is ignored, and left at Zero.";
      }
   }
   else if ((command == "maximum_iterations") || (command == "maximum_iterations_per_engagement")) // legacy name
   {
      aInput.ReadValue(mMaxIterations);
      aInput.ValueGreater(mMaxIterations, 1U);
   }
   else if (command == "increment_ratio")
   {
      aInput.ReadValue(mIncrementRatio);
      aInput.ValueGreater(mIncrementRatio, 1.0);
      aInput.ValueInClosedRange(mIncrementRatio, 1.0, 2.0);
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}

// ========================================================================
// virtual
void AirToAirLaunchComputerGenerator::Update(double aSimTime)
{
   if (aSimTime > mLastUpdate)
   {
      // Check to see if the previously fired weapon has terminated yet...
      if (mObserver.GetState() != WeaponObserver::cIN_FLIGHT)
      {
         if (mState == cSTATE_INITIALIZED)
         {
            mState = cSTATE_GENERATION;
         }
         Iterate(aSimTime);
      }
      Tool::Update(aSimTime);
   }
}


// ****************************************************************************
// Protected

// ============================================================================
void AirToAirLaunchComputerGenerator::ErrorBailout()
{
   ut::log::error() << "Unknown Error in AirToAirLaunchComputerGenerator has caused a premature abort!";
   Finish();
}

// virtual
bool AirToAirLaunchComputerGenerator::FireNewWeapon(double aSimTime)
{
   bool success = true;

   // Create a new Target in the correct Location:
   WsfTrack     targetTrack;
   WsfPlatform* targetPtr = PlaceNewTarget(aSimTime, targetTrack);

   // if targetPtr == 0 - could not place a new target, so go to the next point
   if (targetPtr != nullptr)
   {
      // Set the initial conditions of the launching platform:
      // This will continuously relocate the platform back to the starting point,
      // even though it is supposed to have a velocity... visually, it will "stutter"
      mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, mAlt);
      mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);

      double launchPlatVelNED[] = {mShooterSpeed * cos(mHeadingRad), mShooterSpeed * sin(mHeadingRad), 0.0};
      mLaunchPlatPtr->SetVelocityNED(launchPlatVelNED);

      if (!mLaunchWpnPtr->Fire(aSimTime, &targetTrack, WsfWeapon::FireOptions()))
      {
         mObserver.LaunchAborted(aSimTime);
         success = false;
      }

      WsfPlatform* mslPtr = mObserver.GetWeaponPlatform();
      if ((mslPtr != nullptr) && success)
      {
         TargetMover* moverPtr = dynamic_cast<TargetMover*>(targetPtr->GetMover());
         if (moverPtr != nullptr)
         {
            moverPtr->mLateralAccelLimit = mATA_LaunchComputerPtr->GetTable().NoEscapeManeuverAcceleration();
            if (mSearchingFor == cSEARCH_R_NE)
            {
               moverPtr->mMissileIndex = mslPtr->GetIndex();
            }
            else
            {
               moverPtr->mMissileIndex = 0;
            }
         }
         else
         {
            ut::log::error() << "Target must be given only a TargetMover as a mover.";
            delete targetPtr;
            targetPtr = nullptr;
            success   = false;
         }
      }
   }
   else
   {
      success = false;
   }

   if (success)
   {
      ++mQuantityLaunched;
   }
   else
   {
      if (DebugEnabled())
      {
         auto logDebug = ut::log::debug() << "ATA LC Generator failed launching at time.";
         logDebug.AddNote() << "T = " << aSimTime;
      }
      if (mTerminateOnLaunchFailure)
      {
         ErrorBailout();
      }
   }

   return success;
}


// ****************************************************************************
// Private

// ============================================================================
void AirToAirLaunchComputerGenerator::Iterate(double aSimTime)
{
   if ((mObserver.GetState() == WeaponObserver::cIDLE) && (!mFirstPass))
   {
      // This is done only on the first pass through
      mFirstPass    = true;
      mCurrentIndex = 0;
      SetLaunchConditions(aSimTime);
      SetSearchRmin();
   }
   else if (mObserver.GetState() == WeaponObserver::cTERMINATED)
   {
      // Clean up the target

      // Previously fired weapon went away. Remove the old target,
      // if our own weapon didn't already kill it.
      WsfPlatform* oldTargetPtr = GetSimulation()->GetPlatformByIndex(mTargetIndex);
      if (oldTargetPtr != nullptr)
      {
         GetSimulation()->DeletePlatform(aSimTime, oldTargetPtr);
      }
      mTargetIndex = 0;

      // Capture observer data and reset
      mCurrentHit = mObserver.EngagementLethal();
      mCurrentTOF = mObserver.TimeOfFlight();
      if (mCurrentHit)
      {
         ++mHitCount;
      }
      else
      {
         ++mMissCount;
      }
      mObserver.ResetState();

      // Evaluate the result
      if (EvaluateResult())
      {
         // done with a launch condition
         ProcessResults();

         unsigned int fraction = (unsigned int)std::max(1.0, mResultsSize / 50.0);
         if (((mCurrentIndex + 1) % fraction == 0))
         {
            auto logInfo = ut::log::info() << " TOOL:AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR:";
            logInfo.AddNote() << 100 * (mCurrentIndex + 1) / mResultsSize << "% complete.";
            logInfo.AddNote() << "Current Index: " << mCurrentIndex + 1;
            logInfo.AddNote() << "Index Size: " << mResultsSize;
         }

         // Check for DONE:
         if ((mCurrentIndex + 1) == mResultsSize)
         {
            WriteOutputFile();
            Finish();
            return;
         }

         // Switch to the next Launch Condition:
         ++mCurrentIndex;

         SetLaunchConditions(aSimTime);
         SetSearchRmin();
      }


   } // end if terminated

   // Intentionally ignore returned bool.
   FireNewWeapon(aSimTime);
}

// ============================================================================
bool AirToAirLaunchComputerGenerator::EvaluateResult()
{
   // Set default return value
   bool done = false;

   // Increment iteration count and check if max exceeded
   ++mIterationCount;
   if (mIterationCount > mMaxIterations)
   {
      mGenerationState = cFAILED;
      done             = true;
   }

   // Iterations not exceeded so evaluate the result
   if (mSearchingFor == cSEARCH_R_MIN)
   {
      if (Evaluate())
      {
         mTempResult.mRmin    = mCurrentGrndRange;
         mTempResult.mRminTOF = mCurrentTOF;
         SetSearchRmax();
      }
   }
   else if (mSearchingFor == cSEARCH_R_MAX)
   {
      if (Evaluate())
      {
         mTempResult.mRmax    = mCurrentGrndRange;
         mTempResult.mRmaxTOF = mCurrentTOF;
         SetSearchRne();
      }
   }
   else
   {
      if (Evaluate())
      {
         mTempResult.mRne    = mCurrentGrndRange;
         mTempResult.mRneTOF = mCurrentTOF;
         done                = true;
      }
   }

   return done;
}

// ============================================================================
bool AirToAirLaunchComputerGenerator::Evaluate()
{
   // Quick return if failure
   if (mGenerationState == cFAILED)
   {
      mCurrentGrndRange = -1.0;
      mCurrentTOF       = -1.0;
      return true; // Done and failed
   }

   // Determine if we are converged or bracketed
   double deltaR    = mCurrentGrndRange - mPrevious.first;
   bool   converged = (fabs(deltaR) <= mGrndRangeTolerance);
   bool   bracketed = (mCurrentHit != mPrevious.second);

   if (mDebug)
   {
      auto logDebug = ut::log::debug() << "A2A_LCG[" << mIterationCount << "]:";
      logDebug.AddNote() << "Current Hit: " << mCurrentHit;
      logDebug.AddNote() << "Ground Range: " << mCurrentGrndRange;
   }

   if (bracketed)
   {
      if (converged)
      {
         // done
         mGenerationState = cDONE;
         return true; // Done and succeeded
      }
      else // Bracketed but not converged so bisect
      {
         // Bisecting
         mGenerationState = cBISECTING;

         mCurrent.first  = mCurrentGrndRange;
         mCurrent.second = mCurrentHit;

         // Calc new range
         mCurrentGrndRange = mPrevious.first + 0.5 * deltaR;
      }
   }
   else
   {
      // Bracketing
      if (mGenerationState == cBRACKETING)
      {
         // Update the last pass
         mPrevious.first  = mCurrentGrndRange;
         mPrevious.second = mCurrentHit;

         if ((mSearchingFor == cSEARCH_R_MAX) || (mSearchingFor == cSEARCH_R_NE))
         {
            mCurrentGrndRange /= mIncrementRatio; // Decrease
         }
         else
         {
            mCurrentGrndRange *= mIncrementRatio; // Increase
         }
      }
      else
      {
         // Bisecting
         mPrevious.first  = mCurrentGrndRange;
         mPrevious.second = mCurrentHit;

         // Calc new range
         mCurrentGrndRange = mPrevious.first + 0.5 * deltaR;
      }
   }

   // Do a bounds check
   if ((mCurrentGrndRange > mMaxRange) || (mCurrentGrndRange < mMinRange))
   {
      // We are done because we have exceeded the bounds
      mGenerationState  = cFAILED;
      mCurrentGrndRange = -1.0;
      mCurrentTOF       = -1.0;
      return true; // Done but failed
   }

   return false;
}

// ============================================================================
void AirToAirLaunchComputerGenerator::DetermineTargetStateValues(double  aShooterLatDeg,
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
                                                                 double  aTargetVelNED[3])
{
   // Place a target in the desired location:
   // Always measure the lead angle positive to the right side of the shooter.
   // Cannot use NED in this context, must switch to UtSphericalEarth values.

   // Assure that lead is normalized:
   aTargetLeadRad = UtMath::NormalizeAngleMinusPi_Pi(aTargetLeadRad);

   // To locate the target, extrapolate the compass heading to the target
   // by the current ground range. Always place the target on our right, assuming symmetry.
   double shooterToTgt_LOS_CompassHeadingRad = aShooterHeadingRad + aTargetLeadRad;
   double shooterToTgt_LOS_CompassHeadingDeg = shooterToTgt_LOS_CompassHeadingRad * UtMath::cDEG_PER_RAD;

   // First, place the target (Lat, Lon), based upon Lead angle:
   UtSphericalEarth::ExtrapolateGreatCirclePosition(aShooterLatDeg,
                                                    aShooterLonDeg,
                                                    shooterToTgt_LOS_CompassHeadingDeg,
                                                    aGroundRange,
                                                    aTargetLatDeg,
                                                    aTargetLonDeg);

   // Now determine the targets compass LOS to shooter:
   // (Roughly, but not quite 180 degrees from shooters LOS to target, due to curvature of earth issues.)
   double targetLOStoShooterCompassDeg, distanceNotUsed;
   UtSphericalEarth::GreatCircleHeadingAndDistance(aTargetLatDeg,
                                                   aTargetLonDeg,
                                                   aShooterLatDeg,
                                                   aShooterLonDeg,
                                                   targetLOStoShooterCompassDeg,
                                                   distanceNotUsed);

   double targetLOStoShooterCompassRad = targetLOStoShooterCompassDeg * UtMath::cRAD_PER_DEG;

   // This was a tough one to figure out, and interpretations differ, so I
   // added configuration options in the input file to set his behavior:
   double firstSign  = mAspectRotIsClockwise ? 1.0 : -1.0;
   double secondSign = 1.0;
   if (mAspectRotIsSignedToLead)
   {
      secondSign = (aTargetLeadRad >= 0.0) ? 1.0 : -1.0;
   }
   double sign = firstSign * secondSign;

   aTargetHeadingRad = UtMath::NormalizeAngleMinusPi_Pi(targetLOStoShooterCompassRad + sign * aTargetAspectRad);

   double targetSpeed = aTargetMach * mAtmosphere.SonicVelocity(aTargetAlt);
   aTargetVelNED[0]   = targetSpeed * cos(aTargetHeadingRad);
   aTargetVelNED[1]   = targetSpeed * sin(aTargetHeadingRad);
   aTargetVelNED[2]   = 0.0;
}

// ============================================================================
// virtual
WsfPlatform* AirToAirLaunchComputerGenerator::PlaceNewTarget(double aSimTime, WsfTrack& aTrack)
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

   mTargetIndex = 0;

   // Place a target in the desired location:
   // Cannot use NED in this context, must switch to UtSphericalEarth values.
   double targetLatDeg, targetLonDeg, targetHeadingRad;
   double targetVelNED[3];

   // The logic for how to place the target got very involved, and was prone
   // to error, so has been moved into its own function:
   DetermineTargetStateValues(mLatDeg,
                              mLonDeg,
                              mAlt,
                              mHeadingRad, // shooter values
                              mCurrentGrndRange,
                              mTargetLead,
                              mTargetAspect,
                              mTargetMach,
                              mTargetAlt,
                              targetLatDeg,
                              targetLonDeg,
                              targetHeadingRad,
                              targetVelNED);

   WsfPlatform* targetPtr = GetScenario().GetPlatformTypes().Clone(mTargetPlatformTypeId);
   if (targetPtr == nullptr)
   {
      auto logError = ut::log::error()
                      << "AirToAirLaunchComputerGenerator::FireNewWeapon could not clone a target platform of Type!";
      logError.AddNote() << "Type: " << mTargetPlatformTypeId.GetString();
      return targetPtr;
   }

   targetPtr->SetLocationLLA(targetLatDeg, targetLonDeg, mTargetAlt);
   targetPtr->SetOrientationNED(targetHeadingRad, 0.0, 0.0);
   targetPtr->SetVelocityNED(targetVelNED);

   // Give the target a name...
   std::ostringstream oss;
   ++mTargetCount;
   oss << mTargetPlatformTypeId << "_ATA_" << mTargetCount;
   std::string targetName(oss.str());
   targetPtr->SetName(targetName);
   GetSimulation()->AddPlatform(aSimTime, targetPtr);
   mTargetIndex = targetPtr->GetIndex();

   aTrack.SetLocationLLA(targetLatDeg, targetLonDeg, mTargetAlt);

   // Velocity Values:
   double targetVelWCS[3];
   targetPtr->ConvertNEDVectorToWCS(targetVelWCS, targetVelNED);
   aTrack.SetVelocityWCS(targetVelWCS);
   aTrack.SetVelocityValid(true);
   aTrack.SetTargetIndex(mTargetIndex);
   aTrack.Update(aSimTime);

   return targetPtr;
}

// ============================================================================
void AirToAirLaunchComputerGenerator::ProcessResults()
{
   // We captured all six desired values (Rmin, Rmax, Rne, and TOFs), save results
   if (DebugEnabled())
   {
      auto logDebug = ut::log::debug() << "AirToAirLaunchComputerGenerator::ProcessResults():";
      logDebug.AddNote() << "SH: " << mAlt * UtMath::cFT_PER_M;
      logDebug.AddNote() << "SM: " << mShooterMach;
      logDebug.AddNote() << "TH: " << mTargetAlt * UtMath::cFT_PER_M;
      logDebug.AddNote() << "TM: " << mTargetMach;
      logDebug.AddNote() << "Aspt: " << mTargetAspect * UtMath::cDEG_PER_RAD;
      logDebug.AddNote() << "Ld: " << mTargetLead * UtMath::cDEG_PER_RAD << " (f,d) ";
      logDebug.AddNote() << "R: " << mCurrentGrndRange << " m ";
   }

   if (mResultsFileName.empty())
   {
      mATA_LaunchComputerPtr->GetTable().SetResult(mCurrentIndex, mTempResult);
   }
   else
   {
      // Open the TABLE output file.
      std::ofstream resultsFile(mResultsFileName.c_str(), std::ios::app);
      if (!resultsFile.is_open())
      {
         auto out = ut::log::error() << "Unable to open output File!";
         out.AddNote() << mTableFileName;
         throw;
      }

      if ((mTargetAspect != mLastTargetAspect) || (mTargetMach != mLastTargetMach) || (mTargetAlt != mLastTargetAlt) ||
          (mShooterMach != mLastShooterMach) || (mAlt != mLastShooterAlt))
      {
         mLastTargetAspect = mTargetAspect;
         mLastTargetMach   = mTargetMach;
         mLastTargetAlt    = mTargetAlt;
         mLastShooterMach  = mShooterMach;
         mLastShooterAlt   = mAlt;
         mLastTargetLead   = mTargetLead;
         resultsFile << "\n# Shooter Alt=" << mAlt * UtMath::cFT_PER_M << " ft";
         resultsFile << ", M=" << mShooterMach;
         resultsFile << ", Tgt Alt=" << mTargetAlt * UtMath::cFT_PER_M << " ft";
         resultsFile << ", M=" << mTargetMach;
         resultsFile << ", Aspect=" << mTargetAspect * UtMath::cDEG_PER_RAD << " deg" << std::endl;
      }
      mTempResult.Stream(resultsFile);
      resultsFile << "   # Lead=" << mTargetLead * UtMath::cDEG_PER_RAD << std::endl;
      resultsFile.close();
   }

   if (mTempResult.IsNull())
   {
      ++mNullCount;
   }
   else
   {
      ++mSignificant;
   }

   if (mTempResult.mRmax == -1.0)
   {
      ++mFailures;
   }
   else
   {
      ++mSuccesses;
   }
   if (mTempResult.mRne == -1.0)
   {
      ++mFailures;
   }
   else
   {
      ++mSuccesses;
   }
   if (mTempResult.mRmin == -1.0)
   {
      ++mFailures;
   }
   else
   {
      ++mSuccesses;
   }

   if ((mTempResult.mRmax != -1.0) && (mTempResult.mRmax > mGlobalMaxRmax))
   {
      mGlobalMaxRmax = mTempResult.mRmax;
   }
   if ((mTempResult.mRmax != -1.0) && (mTempResult.mRmax < mGlobalMinRmax))
   {
      mGlobalMinRmax = mTempResult.mRmax;
   }

   if ((mTempResult.mRmaxTOF != -1.0) && (mTempResult.mRmaxTOF > mGlobalMaxRmaxTOF))
   {
      mGlobalMaxRmaxTOF = mTempResult.mRmaxTOF;
   }
   if ((mTempResult.mRmaxTOF != -1.0) && (mTempResult.mRmaxTOF < mGlobalMinRmaxTOF))
   {
      mGlobalMinRmaxTOF = mTempResult.mRmaxTOF;
   }

   if ((mTempResult.mRmin != -1.0) && (mTempResult.mRmin > mGlobalMaxRmin))
   {
      mGlobalMaxRmin = mTempResult.mRmin;
   }
   if ((mTempResult.mRmin != -1.0) && (mTempResult.mRmin < mGlobalMinRmin))
   {
      mGlobalMinRmin = mTempResult.mRmin;
   }

   if ((mTempResult.mRminTOF != -1.0) && (mTempResult.mRminTOF > mGlobalMaxRminTOF))
   {
      mGlobalMaxRminTOF = mTempResult.mRminTOF;
   }
   if ((mTempResult.mRminTOF != -1.0) && (mTempResult.mRminTOF < mGlobalMinRminTOF))
   {
      mGlobalMinRminTOF = mTempResult.mRminTOF;
   }

   if ((mTempResult.mRne != -1.0) && (mTempResult.mRne > mGlobalMaxRne))
   {
      mGlobalMaxRne = mTempResult.mRne;
   }
   if ((mTempResult.mRne != -1.0) && (mTempResult.mRne < mGlobalMinRne))
   {
      mGlobalMinRne = mTempResult.mRne;
   }

   if ((mTempResult.mRneTOF != -1.0) && (mTempResult.mRneTOF > mGlobalMaxRneTOF))
   {
      mGlobalMaxRneTOF = mTempResult.mRneTOF;
   }
   if ((mTempResult.mRneTOF != -1.0) && (mTempResult.mRneTOF < mGlobalMinRneTOF))
   {
      mGlobalMinRneTOF = mTempResult.mRneTOF;
   }

   mTempResult.Reset();
}

// ============================================================================
void AirToAirLaunchComputerGenerator::SetLaunchConditions(double aSimTime)
{
   // Intentionally ignore the returned boolean...
   mATA_LaunchComputerPtr->GetTable()
      .IndexToValues(mCurrentIndex, mAlt, mShooterMach, mTargetAlt, mTargetMach, mTargetAspect, mTargetLead);

   if (DebugEnabled())
   {
      int    totalSeconds = static_cast<int>(aSimTime);
      int    hours        = totalSeconds / 3600;
      int    minutes      = (totalSeconds - hours * 3600) / 60;
      double seconds      = aSimTime - hours * 3600 - minutes * 60;

      auto logDebug    = ut::log::debug() << "AirToAirLaunchComputerGenerator::SetLaunchConditions";
      auto logLongTime = logDebug.AddNote() << "T = " << aSimTime;
      logLongTime.AddNote() << "Sim Time in HMS: " << hours << ":" << minutes << "." << seconds;
      auto logSwitchedConditions = logDebug.AddNote() << "Switched to conditions: ";
      logSwitchedConditions.AddNote() << "Shooter Hight: " << mAlt * UtMath::cFT_PER_M;
      logSwitchedConditions.AddNote() << "Shooter Mach: " << mShooterMach;
      logSwitchedConditions.AddNote() << "Target Hight: " << mTargetAlt * UtMath::cFT_PER_M;
      logSwitchedConditions.AddNote() << "Target Mach: " << mTargetMach;
      logSwitchedConditions.AddNote() << "Aspect: " << mTargetAspect * UtMath::cDEG_PER_RAD;
      logSwitchedConditions.AddNote() << "Target Lead: " << mTargetLead * UtMath::cDEG_PER_RAD << " (f,d)";
   }

   // The launcher must be moved in altitude as required...
   // Here we ignore the Tool input for altitude, and use our own override:
   mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, mAlt);
   mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);

   mShooterSpeed = mAtmosphere.SonicVelocity(mAlt) * mShooterMach;

   double ecsVel[3] = {mShooterSpeed, 0.0, 0.0};
   mLaunchPlatPtr->SetVelocityECS(ecsVel);
}

// ============================================================================
void AirToAirLaunchComputerGenerator::SetSearchRmin()
{
   mSearchingFor     = cSEARCH_R_MIN;
   mCurrentGrndRange = mMinRange;
   mPrevious.first   = mMinRange;
   mGenerationState  = cBRACKETING;
   mIterationCount   = 0;

   if (mDebug)
   {
      ut::log::debug() << "A2A_LCG: Searching for Rmin.";
   }
}

// ============================================================================
void AirToAirLaunchComputerGenerator::SetSearchRmax()
{
   mSearchingFor     = cSEARCH_R_MAX;
   mCurrentGrndRange = mMaxRange;
   mPrevious.first   = mMaxRange;
   mGenerationState  = cBRACKETING;
   mIterationCount   = 0;

   if (mDebug)
   {
      ut::log::debug() << "A2A_LCG: Searching for Rmax.";
   }
}

// ============================================================================
void AirToAirLaunchComputerGenerator::SetSearchRne()
{
   mSearchingFor     = cSEARCH_R_NE;
   mCurrentGrndRange = mTempResult.mRmax;
   mPrevious.first   = mTempResult.mRmax;
   mGenerationState  = cBRACKETING;
   mIterationCount   = 0;

   if (mDebug)
   {
      ut::log::debug() << "A2A_LCG: Searching for Rne.";
   }
}

// ============================================================================
//! Output a data file defining the Launch Computer just generated.
void AirToAirLaunchComputerGenerator::WriteOutputFile()
{
   // Here we actually read and write two different files.
   // 1. The launch_computer_table data.
   // 2. The launch computer itself that uses the table.

   // Open the TABLE output file.
   std::ofstream outFile(mTableFileName.c_str());
   if (!outFile.is_open())
   {
      auto logError = ut::log::error() << "Unable to open output File!";
      logError.AddNote() << "File: " << mTableFileName;
      return;
   }

   outFile << "# " << mGeneratedTimeString;
   outFile << "# For launch computer type " << WsfAirToAirLaunchComputer::BaseTypeName() << "\n\n";
   outFile << "launch_computer_table" << std::endl;

   outFile << "   # successes = " << mSuccesses << ", failures = " << mFailures;
   outFile << "\n   # significant conditions = " << mSignificant << ", versus null = " << mNullCount;
   outFile << "\n   # minRmin = " << mGlobalMinRmin << ", maxRmin = " << mGlobalMaxRmin;
   outFile << "\n   # minRminTOF = " << mGlobalMinRminTOF << ", maxRminTOF = " << mGlobalMaxRminTOF;
   outFile << "\n   # minRmax = " << mGlobalMinRmax << ", maxRmax = " << mGlobalMaxRmax;
   outFile << "\n   # minRmaxTOF = " << mGlobalMinRmaxTOF << ", maxRmaxTOF = " << mGlobalMaxRmaxTOF;
   outFile << "\n   # minRne = " << mGlobalMinRne << ", maxRne = " << mGlobalMaxRne;
   outFile << "\n   # minRneTOF = " << mGlobalMinRneTOF << ", maxRneTOF = " << mGlobalMaxRneTOF << std::endl;

   outFile << "   # Dimensionality = " << mATA_LaunchComputerPtr->GetTable().ShooterAlts().size();
   outFile << " X " << mATA_LaunchComputerPtr->GetTable().ShooterMachs().size();
   outFile << " X " << mATA_LaunchComputerPtr->GetTable().TargetAlts().size();
   outFile << " X " << mATA_LaunchComputerPtr->GetTable().TargetMachs().size();
   outFile << " X " << mATA_LaunchComputerPtr->GetTable().TargetAspects().size();
   outFile << " X " << mATA_LaunchComputerPtr->GetTable().TargetLeads().size();
   outFile << " = " << mATA_LaunchComputerPtr->GetTable().SizeOfResultsSet() << std::endl;

   mATA_LaunchComputerPtr->GetTable().Stream(outFile, mResultsFileName);

   outFile << "end_launch_computer_table" << std::endl;

   outFile.close();

   // Now output the launch computer itself:
   std::ofstream secondOutFile(mOutputFileName.c_str());
   if (!secondOutFile.is_open())
   {
      auto logError = ut::log::error() << "Unable to open output File!";
      logError.AddNote() << "File: " << mOutputFileName;
      return;
   }

   secondOutFile << "# " << mGeneratedTimeString;
   secondOutFile << "\nlaunch_computer " << mOutputObjectName << ' ' << WsfAirToAirLaunchComputer::BaseTypeName();
   secondOutFile << "\n   load_table " << mTableFileName;
   secondOutFile << "\nend_launch_computer" << std::endl;
   secondOutFile.close();


   // Now turn around a read in what we just wrote:
   try
   {
      WsfScenario* scen = new WsfScenario(GetScenario().GetApplication());
      UtInput      input;
      input.SetAuxData(0, (void*)&scen);
      input.PushInput(ut::make_unique<UtInputFile>(mOutputFileName));
      scen->LoadFromStream(input);

      // Now clone an instance of what we just loaded,
      // and run a diagnostic test on the object:
      // WsfLaunchComputer* lcPtr = GetScenario().GetLaunchComputerTypes().Clone(mOutputObjectName);
      WsfLaunchComputer*         lcPtr  = WsfLaunchComputerTypes::Get(*scen).Clone(mOutputObjectName);
      WsfAirToAirLaunchComputer* ataPtr = dynamic_cast<WsfAirToAirLaunchComputer*>(lcPtr);
      if (ataPtr != nullptr)
      {
         bool testOK = ataPtr->Test();
         if (!testOK)
         {
            ut::log::error() << "A test of the Generated Launch Computer Failed!";
         }
      }
      else
      {
         ut::log::error() << "Generated Launch Computer was not properly inserted into Types package!";
      }
      delete lcPtr;
      delete scen;
   }
   catch (UtInput::BadValue& aException)
   {
      const std::string acceptableError = "Duplicate launch_computer type:";
      std::string       a               = aException.GetMessage().substr(0, acceptableError.length());
      if (a == acceptableError)
      {
         ut::log::warning() << "File re-read of produced LC type was successful, but produced a duplicate LC Type.";
      }
      else
      {
         ut::log::error() << "Unknown File I/O Error Occurred during AirToAirLC Generation.";
      }
   }
}
