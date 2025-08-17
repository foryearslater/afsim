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

#include "BallisticLaunchComputerGenerator.hpp"

#include <cassert>
#include <ctime>
#include <fstream> // Required for File I/O
#include <iostream>

#include "UtInput.hpp"
#include "UtInputFile.hpp"
#include "UtMat3.hpp"
#include "UtMath.hpp"
#include "UtVec3.hpp"
#include "WeaponObserver.hpp"
#include "WsfBallisticLaunchComputer.hpp"
#include "WsfGuidedMover.hpp"
#include "WsfLaunchComputer.hpp"
#include "WsfLaunchComputerTypes.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfStringId.hpp"
#include "WsfTrack.hpp"
#include "WsfWaypointMover.hpp"
#include "WsfWeaponEngagement.hpp"

namespace
{
const char* cBALLISTIC_LAUNCH_COMPUTER_DEFAULT_PRODUCT("_BALLISTIC_LAUNCH_COMPUTER");
}

BallisticLaunchComputerGenerator::BallisticLaunchComputerGenerator(WsfScenario& aScenario)
   : Tool(aScenario)
   , mMyAltIndex(0)
   , mMySpdIndex(0)
   , mTgtAltIndex(0)
   , mMyMinAlt(1000.0)
   , mMyDeltaAlt(1000.0)
   , mNumMyAlts(10)
   , mMyMinSpeed(120.0)
   , mDeltaMySpeed(30.0)
   , mNumMySpeeds(4)
   , mTgtMinAlt(0.0)
   , mTgtDeltaAlt(50.0)
   , mNumTgtAlts(10)
   , mState(cLAR_GEN)
   , mFirstPass(true)
   , mTOFs(nullptr)
   , mDownRanges(nullptr)
   , mGenTime()
{
   mToolProduces = cBALLISTIC_LAUNCH_COMPUTER_DEFAULT_PRODUCT;
}

BallisticLaunchComputerGenerator::BallisticLaunchComputerGenerator(const BallisticLaunchComputerGenerator& aSrc)
   : Tool(aSrc)
   , mMyAltIndex(aSrc.mMyAltIndex)
   , mMySpdIndex(aSrc.mMySpdIndex)
   , mTgtAltIndex(aSrc.mTgtAltIndex)
   , mMyMinAlt(aSrc.mMyMinAlt)
   , mMyDeltaAlt(aSrc.mMyDeltaAlt)
   , mNumMyAlts(aSrc.mNumMyAlts)
   , mMyMinSpeed(aSrc.mMyMinSpeed)
   , mDeltaMySpeed(aSrc.mDeltaMySpeed)
   , mNumMySpeeds(aSrc.mNumMySpeeds)
   , mTgtMinAlt(aSrc.mTgtMinAlt)
   , mTgtDeltaAlt(aSrc.mTgtDeltaAlt)
   , mNumTgtAlts(aSrc.mNumTgtAlts)
   , mState(aSrc.mState)
   , mFirstPass(aSrc.mFirstPass)
   , mTOFs(nullptr)
   , mDownRanges(nullptr)
   , mGenTime(aSrc.mGenTime)
{
   if (aSrc.mTOFs)
   {
      CopyDataFrom(aSrc.mTOFs, aSrc.mDownRanges, aSrc.mNumMyAlts, aSrc.mNumMySpeeds, aSrc.mNumTgtAlts);
   }
}

BallisticLaunchComputerGenerator::~BallisticLaunchComputerGenerator()
{
   if (mTOFs != nullptr)
   {
      DeallocateData();
   }
}

void BallisticLaunchComputerGenerator::AllocAndZeroData()
{
   assert((mNumMySpeeds > 1) && (mNumMyAlts > 1));

   mTOFs       = new double**[mNumMyAlts];
   mDownRanges = new double**[mNumMyAlts];

   // Allocate a 3-D rectangular array to hold floats:
   for (int i = 0; i < mNumMyAlts; ++i)
   {
      mTOFs[i]       = new double*[mNumMySpeeds];
      mDownRanges[i] = new double*[mNumMySpeeds];

      for (int j = 0; j < mNumMySpeeds; ++j)
      {
         mTOFs[i][j]       = new double[mNumTgtAlts];
         mDownRanges[i][j] = new double[mNumTgtAlts];

         for (int k = 0; k < mNumTgtAlts; ++k)
         {
            mTOFs[i][j][k]       = 0.0;
            mDownRanges[i][j][k] = 0.0;
         }
      }
   }
}

void BallisticLaunchComputerGenerator::CopyDataFrom(double*** aTOFs,
                                                    double*** aDownRanges,
                                                    int       aNumMyAlts,
                                                    int       aNumMySpeeds,
                                                    int       aNumTgtAlts)
{
   DeallocateData();

   mNumMyAlts   = aNumMyAlts;
   mNumMySpeeds = aNumMySpeeds;
   mNumTgtAlts  = aNumTgtAlts;

   mTOFs       = new double**[aNumMyAlts];
   mDownRanges = new double**[aNumMyAlts];

   // Allocate a 3-D rectangular array to hold floats:
   for (int i = 0; i < aNumMyAlts; ++i)
   {
      mTOFs[i]       = new double*[aNumMySpeeds];
      mDownRanges[i] = new double*[aNumMySpeeds];

      for (int j = 0; j < aNumMySpeeds; ++j)
      {
         mTOFs[i][j]       = new double[aNumTgtAlts];
         mDownRanges[i][j] = new double[aNumTgtAlts];

         for (int k = 0; k < aNumTgtAlts; ++k)
         {
            mTOFs[i][j][k]       = aTOFs[i][j][k];
            mDownRanges[i][j][k] = aDownRanges[i][j][k];
         }
      }
   }
}

void BallisticLaunchComputerGenerator::DeallocateData()
{
   if (!mTOFs)
   {
      return;
   } // already deallocated...

   for (int i = 0; i < mNumMyAlts; ++i)
   {
      for (int j = 0; j < mNumMySpeeds; ++j)
      {
         delete[] mTOFs[i][j];
         delete[] mDownRanges[i][j];
      }
      delete[] mTOFs[i];
      delete[] mDownRanges[i];
   }
   delete[] mTOFs;
   delete[] mDownRanges;

   mTOFs = nullptr;
}

// virtual
Tool* BallisticLaunchComputerGenerator::Clone() const
{
   return new BallisticLaunchComputerGenerator(*this);
}

bool BallisticLaunchComputerGenerator::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;

   std::string command(aInput.GetCommand());

   if (command == "launch_altitudes")
   {
      aInput.ReadValueOfType(mMyMinAlt, UtInput::cLENGTH);
      aInput.ReadValueOfType(mMyDeltaAlt, UtInput::cLENGTH);
      aInput.ReadValue(mNumMyAlts);
      aInput.ValueGreater(mMyMinAlt, 0.0);
      aInput.ValueGreater(mMyDeltaAlt, 0.0);
      aInput.ValueGreater<double>(mNumMyAlts, 1.5);
   }
   else if (command == "target_altitudes")
   {
      aInput.ReadValueOfType(mTgtMinAlt, UtInput::cLENGTH);
      aInput.ReadValueOfType(mTgtDeltaAlt, UtInput::cLENGTH);
      aInput.ReadValue(mNumTgtAlts);
      aInput.ValueGreater(mTgtDeltaAlt, 0.0);
      aInput.ValueGreater<double>(mNumTgtAlts, 1.5);
   }
   else if (command == "launch_speeds")
   {
      aInput.ReadValueOfType(mMyMinSpeed, UtInput::cSPEED);
      aInput.ReadValueOfType(mDeltaMySpeed, UtInput::cSPEED);
      aInput.ReadValue(mNumMySpeeds);
      aInput.ValueGreaterOrEqual(mMyMinSpeed, 0.0);
      aInput.ValueGreaterOrEqual(mDeltaMySpeed, 0.0);
      aInput.ValueGreater<double>(mNumMySpeeds, 1.5);
   }
   else if ((command == "altitude_and_mach") || (command == "altitude_and_speed") || (command == "altitude") ||
            (command == "speed") || (command == "desired_delta_altitude"))
   {
      const char* a = "The '";
      const char* b = "' input not allowed with this object.";
      throw UtInput::BadValue(aInput, std::string(a) + command + std::string(b));
   }
   else
   {
      myCommand = Tool::ProcessInput(aInput);
   }

   return myCommand;
}


// virtual
void BallisticLaunchComputerGenerator::Update(double aSimTime)
{
   if (mState == cDONE)
   {
   }
   else if (mState == cLAR_GEN)
   {
      LARGenUpdate(aSimTime);
   }

   Tool::Update(aSimTime);
}


void BallisticLaunchComputerGenerator::ErrorBailout()
{
   mState = cDONE;
   mDone  = true;
   mObserver.SetAllDone();
   ut::log::error() << "BallistLARGenerator(): Inability to Fire a New Weapon caused a premature abort.";
}

// virtual
void BallisticLaunchComputerGenerator::LARGenUpdate(double aSimTime)
{
   if (mFirstPass)
   {
      mMySpdIndex  = 0;
      mMyAltIndex  = 0;
      mTgtAltIndex = 0;
      mFirstPass   = false;

      if (!FireNewWeapon(aSimTime) && mTerminateOnLaunchFailure)
      {
         ErrorBailout();
      }
   }
   else
   {
      auto logDebug = ut::log::debug();
      // Check to see if the previously fired weapon has terminated yet...
      if (mObserver.GetState() == WeaponObserver::cTERMINATED)
      {
         double tof                                          = mObserver.TimeOfFlight();
         double rng                                          = mObserver.HorizontalRange();
         mTOFs[mMyAltIndex][mMySpdIndex][mTgtAltIndex]       = tof;
         mDownRanges[mMyAltIndex][mMySpdIndex][mTgtAltIndex] = rng;
         if (DebugEnabled())
         {
            logDebug << "BallisticLaunchComputerGenerator::LARGenUpdate():";
            logDebug.AddNote() << "iLnchAlt: " << mMyAltIndex;
            logDebug.AddNote() << "iSpd: " << mMySpdIndex;
            logDebug.AddNote() << "iTgtAlt: " << mTgtAltIndex;
            logDebug.AddNote() << "tof: " << tof;
            logDebug.AddNote() << "rng: " << rng;
         }

         mObserver.ResetState();
      }

      if (mObserver.GetState() == WeaponObserver::cIDLE)
      {
         // Fire a new one:
         // Run the matrix with later values changing the fastest:
         // Launch Altitude, Launch Speed, Target Altitude

         if (mTgtAltIndex != (mNumTgtAlts - 1))
         {
            ++mTgtAltIndex;
            if (!FireNewWeapon(aSimTime) && mTerminateOnLaunchFailure)
            {
               ErrorBailout();
            }
         }
         else if (mMySpdIndex != (mNumMySpeeds - 1))
         {
            ++mMySpdIndex;
            mTgtAltIndex = 0;
            if (!FireNewWeapon(aSimTime))
            {
               ErrorBailout();
            }
         }
         else if (mMyAltIndex != (mNumMyAlts - 1))
         {
            ++mMyAltIndex;
            mMySpdIndex  = 0;
            mTgtAltIndex = 0;

            if (DebugEnabled())
            {
               double myCurAlt = MyCurrentAlt();
               mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, myCurAlt);
               double locWCS[3];
               mLaunchPlatPtr->GetLocationWCS(locWCS);
               logDebug.AddNote() << "Platform Moved to WGS: " << locWCS[0] << ", " << locWCS[1] << ", " << locWCS[2];
               logDebug.AddNote() << "Platform Moved to LLA: " << mLatDeg << ", " << mLonDeg << ", " << myCurAlt;
            }

            if (!FireNewWeapon(aSimTime) && mTerminateOnLaunchFailure)
            {
               ErrorBailout();
            }
         }
         else
         {
            WriteOutputFile();

            // This is the externally visible flag
            // that terminates the simulation.
            mObserver.SetAllDone();
            mState = cDONE;
            mDone  = true;
         }
      }
   } // if firstPass
}


bool BallisticLaunchComputerGenerator::Initialize(WsfSimulation& aSimulation)
{
   Tool::Initialize(aSimulation);

   mFirstPass = true;

   time_t aclock;
   time(&aclock);
   struct tm*  newtime = localtime(&aclock);
   std::string stamp1("   # Created by BallisticLaunchComputerGenerator on ");
   std::string stamp2(asctime(newtime));
   mGenTime = std::string(stamp1 + stamp2);

   if (mTOFs == nullptr)
   {
      AllocAndZeroData();
   }

   if (!mInitialized)
   {
      Tool::UnInitialize();
   }

   return mInitialized;
}


void BallisticLaunchComputerGenerator::WriteOutputFile()
{
   std::ofstream coutFile(mOutputFileName.c_str());

   if (!coutFile)
   {
      ut::log::error() << "BallisticLaunchComputerGenerator.cpp not able to open output file, results not written.";
      return;
   }
   else
   {
      UtAtmosphere atm(GetScenario().GetAtmosphere());

      coutFile << std::endl;

      // ===================================================
      // #2           Write Time Of Flight Array
      // ===================================================

      coutFile << "launch_computer " << mOutputObjectName << " " << WsfBallisticLaunchComputer::BaseTypeName() << std::endl;
      coutFile << mGenTime;
      coutFile << "   launch_altitudes " << mMyMinAlt << " m " << mMyDeltaAlt << " m " << mNumMyAlts
               << " # Min, Max, Number" << std::endl;
      coutFile << "   launch_speeds " << mMyMinSpeed << " m/s " << mDeltaMySpeed << " m/s " << mNumMySpeeds
               << " # Min, Max, Number" << std::endl;
      coutFile << "   target_altitudes " << mTgtMinAlt << " m " << mTgtDeltaAlt << " m " << mNumTgtAlts
               << " # Min, Max, Number" << std::endl;

      int ii, jj, kk;

      // START Output comment lines listing target altitudes ==================
      coutFile << "   #  target_altitudes (m) = ";
      for (kk = 0; kk < mNumTgtAlts; ++kk)
      {
         coutFile << int(TgtAltitude(kk));
         if (kk != (mNumTgtAlts - 1))
         {
            coutFile << ", ";
         }
      }
      coutFile << std::endl;
      coutFile << "   #  target_altitudes (ft) = ";
      for (kk = 0; kk < mNumTgtAlts; ++kk)
      {
         coutFile << int(TgtAltitude(kk) * UtMath::cFT_PER_M);
         if (kk != (mNumTgtAlts - 1))
         {
            coutFile << ", ";
         }
      }
      // END Output comment lines listing target altitudes ==================

      coutFile << std::endl << std::endl << "   times_of_flight_values # (seconds)" << std::endl << "      ";

      for (ii = 0; ii < mNumMyAlts; ++ii)
      {
         // One-liner listing the current launch altitude ===================
         double alt = MyAltitude(ii);
         double vs  = atm.SonicVelocity(alt);
         coutFile << "      # launch_alt = " << int(alt) << " m, " << int(alt * UtMath::cFT_PER_M) << " ft" << std::endl;

         coutFile << "      ";
         for (jj = 0; jj < mNumMySpeeds; ++jj)
         {
            for (kk = 0; kk < mNumTgtAlts; ++kk)
            {
               coutFile << mTOFs[ii][jj][kk] << " ";
            }
            // Trim Mach to nearest hundredth...
            double mach = static_cast<double>(static_cast<int>(0.005 + 100.0 * MySpeed(jj) / vs) / 100.0);
            coutFile << "# launch_spd = " << int(MySpeed(jj)) << " m/s, ";
            coutFile << int(MySpeed(jj) * UtMath::cFT_PER_M) << " ft/s, Mach=" << mach << std::endl << "      ";
         }
      }

      // Repeat the process for downrange values: =======================================
      coutFile << std::endl << "   downrange_values # (meters)" << std::endl;
      coutFile << "      ";

      for (ii = 0; ii < mNumMyAlts; ++ii)
      {
         double alt = MyAltitude(ii);
         double vs  = atm.SonicVelocity(alt);

         coutFile << "      # launch_alt = " << int(alt) << " m, " << int(alt * UtMath::cFT_PER_M) << " ft" << std::endl;

         coutFile << "      ";
         for (jj = 0; jj < mNumMySpeeds; ++jj)
         {
            for (kk = 0; kk < mNumTgtAlts; ++kk)
            {
               coutFile << mDownRanges[ii][jj][kk] << " ";
            }
            // Trim Mach to nearest hundredth...
            double mach = static_cast<double>(static_cast<int>(0.005 + 100.0 * MySpeed(jj) / vs) / 100.0);
            coutFile << "# launch_spd = " << int(MySpeed(jj)) << " m/s, ";
            coutFile << int(MySpeed(jj) * UtMath::cFT_PER_M) << " ft/s, Mach=" << mach << std::endl << "      ";
         }
      }

      coutFile << std::endl << "end_launch_computer" << std::endl << std::endl;

      coutFile.close();

#ifdef DEV_DEBUG
      if (false) // for debug use only
      {
         // This is used in debugging the generator output only
         // Now, feed the resulting text file to the class,
         // to assure that it can be read correctly:
         try
         {
            UtInput testInput;
            testInput.PushInput(ut::make_unique<UtInputFile>(mOutputFileName));
            if (WsfLaunchComputerTypes::Get(GetScenario()).LoadType(testInput))
            {
               ut::log::developer()
                  << "BallisticLaunchComputer::WriteOutputFile() - Successful re-read of written data file.";
            }
            else
            {
               ut::log::developer()
                  << "BallisticLaunchComputer::WriteOutputFile() - FAILED re-read of written data file.";
            }
         }
         catch (UtInput::BadValue& aException)
         {
            auto out =
               ut::log::developer("ERROR")
               << "BallisticLaunchComputer::WriteOutputFile() - Bad value found during re-reading the output file:";
            out.AddNote() << "Exception: " << aException.GetMessage();
         }
         catch (UtInput::EndOfData& aException)
         {
            auto out = ut::log::developer("ERROR") << "BallisticLaunchComputer::WriteOutputFile() - Early end-of-data "
                                                      "encountered in re-reading output file:";
            out.AddNote() << "Exception: " << aException.GetMessage();
         }
         catch (UtException& aException)
         {
            auto out = ut::log::developer("ERROR") << "BallisticLaunchComputer::WriteOutputFile() - Unknown "
                                                      "UtException encountered in re-reading output file:";
            out.AddNote() << "Exception: " << aException.what();
         }
         catch (std::exception& aException)
         {
            ut::log::developer("ERROR") << "BallisticLaunchComputer::WriteOutputFile() - Unknown std::exception "
                                           "encountered in re-reading output file.  Rethrown.";
            throw;
         }
      }
#endif
   }
}


// virtual
bool BallisticLaunchComputerGenerator::FireNewWeapon(double aSimTime)
{
   bool success = false;

   // Fire a new weapon to fly out
   // Initialize the initial conditions of the weapon:

   double myCurAlt = MyCurrentAlt();

   if ((myCurAlt - TgtCurrentAlt()) <= 0.0)
   {
      // Do not fire, but return true to continue to next point:
      success = true;
   }
   else
   {
      // Set the initial conditions of the launching platform:
      // This will continuously relocate the platform back to the starting point,
      // even though it is supposed to have a velocity... visually, it will "stutter"
      mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, myCurAlt);
      mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);
      double launchPlatVelNED[] = {MyCurrentSpeed() * cos(mHeadingRad), MyCurrentSpeed() * sin(mHeadingRad), 0.0};
      mLaunchPlatPtr->SetVelocityNED(launchPlatVelNED);
      mLaunchWpnPtr->UpdatePosition(aSimTime);
      success = mLaunchWpnPtr->Fire(aSimTime, WsfWeapon::FireTarget(), WsfWeapon::FireOptions());

      if (!success && mTerminateOnLaunchFailure)
      {
         ErrorBailout();
      }
   }

   return success;
}

// virtual
void BallisticLaunchComputerGenerator::PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr)
{
   if (aPlatformPtr != mLaunchPlatPtr)
   {
      // This may be a weapon we just fired... try to
      // set the fuse to the right impact altitude.
      WsfWeaponFuse* fusePtr = GetFuse(aPlatformPtr);
      if (fusePtr == nullptr)
      {
         ErrorBailout();
      }
      else
      {
         fusePtr->SetMSL_Criteria(WsfWeaponFuse::cDETONATE_BELOW, TgtCurrentAlt());
      }
   }
}

//! Obtain a WsfWeaponFuse pointer out of the specified platform.
WsfWeaponFuse* BallisticLaunchComputerGenerator::GetFuse(WsfPlatform* aPlatformPtr) const
{
   WsfWeaponFuse* fPtr = nullptr;
   for (WsfComponentList::RoleIterator<WsfProcessor> iter(*aPlatformPtr); !iter.AtEnd(); ++iter)
   {
      fPtr = dynamic_cast<WsfWeaponFuse*>(*iter);
      if (fPtr != nullptr)
      {
         break;
      }
   }
   return fPtr;
}
