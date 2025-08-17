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

#include "Tool.hpp"

#include <iostream>

#include "ToolManager.hpp"
#include "ToolUpdateEvent.hpp"
#include "UtInput.hpp"
#include "UtMath.hpp"
#include "WsfExplicitWeapon.hpp"
#include "WsfLaunchComputer.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfProcessor.hpp"
#include "WsfSimulation.hpp"
#include "WsfStringId.hpp"
#include "WsfWeaponEffects.hpp"
#include "WsfWeaponEffectsTypes.hpp"

namespace
{
const char* launcherNameId("launcher");
}

//! This constructor initializes the object.
Tool::Tool(WsfScenario& aScenario)
   : WsfObject()
   , mScenarioPtr(&aScenario)
   , mSimulationPtr(nullptr)
   , mObserver()
   , mLaunchPlatPtr(nullptr)
   , mLaunchWpnPtr(nullptr)
   , mFrameStep(0.5)
   , mLastUpdate(0.0)
   , mLatDeg(0.0)
   , mLonDeg(0.0)
   , mAlt(0.0)
   , mHeadingRad(0.0)
   , mDone(false)
   , mNotifiedDone(false)
   , mInitialized(false)
   , mDebug(false)
   , mTerminateOnLaunchFailure(true)
   , mToolProduces("_UNKNOWN_PRODUCT")
   , mFileExtension(".txt")
   , mWeaponEffectTypeId("WEAPON_TOOL_LETHALITY")
   , mLaunchPlatformTypeId("LAUNCH_PLATFORM_TYPE")
   , mTargetPlatformTypeId("TARGET_PLATFORM_TYPE")
   , mWeaponNameId()
   , mOutputFileName()
   , mOutputObjectName()
{
}

Tool::Tool(const Tool& aSrc)
   : WsfObject(aSrc)
   , mScenarioPtr(aSrc.mScenarioPtr)
   , mSimulationPtr(nullptr)
   , mObserver()
   , mLaunchPlatPtr(nullptr)
   , mLaunchWpnPtr(nullptr)
   , mFrameStep(aSrc.mFrameStep)
   , mLastUpdate(0.0)
   , mLatDeg(aSrc.mLatDeg)
   , mLonDeg(aSrc.mLonDeg)
   , mAlt(aSrc.mAlt)
   , mHeadingRad(aSrc.mHeadingRad)
   , mDone(aSrc.mDone)
   , mNotifiedDone(aSrc.mNotifiedDone)
   , mInitialized(aSrc.mInitialized)
   , mDebug(aSrc.mDebug)
   , mTerminateOnLaunchFailure(aSrc.mTerminateOnLaunchFailure)
   , mToolProduces(aSrc.mToolProduces)
   , mFileExtension(aSrc.mFileExtension)
   , mWeaponEffectTypeId(aSrc.mWeaponEffectTypeId)
   , mLaunchPlatformTypeId(aSrc.mLaunchPlatformTypeId)
   , mTargetPlatformTypeId(aSrc.mTargetPlatformTypeId)
   , mWeaponNameId(aSrc.mWeaponNameId)
   , mOutputFileName(aSrc.mOutputFileName)
   , mOutputObjectName(aSrc.mOutputObjectName)
{
}

// virtual
Tool::~Tool()
{
   // The two pointers in this object are for reference only, and owned elsewhere... do not delete them
}

//! In a typical WSF construct, this method parses the input stream, in order to extract
//! commands and values fed to it. See the Weapon Tools User's Guide for details.
// virtual
bool Tool::ProcessInput(UtInput& aInput)
{
   bool        myCommand = true;
   std::string command(aInput.GetCommand());

   if (command == "tool_debug")
   {
      mDebug = true;
   }
   else if (command == "position")
   {
      aInput.ReadValueOfType(mLatDeg, UtInput::cLATITUDE);
      aInput.ReadValueOfType(mLonDeg, UtInput::cLONGITUDE);
   }
   else if (command == "altitude")
   {
      aInput.ReadValueOfType(mAlt, UtInput::cLENGTH);
   }
   else if (command == "heading")
   {
      aInput.ReadValueOfType(mHeadingRad, UtInput::cANGLE);
   }
   else if (command == "frame_step")
   {
      aInput.ReadValueOfType(mFrameStep, UtInput::cTIME);
      aInput.ValueGreater(mFrameStep, 0.0);
   }
   else if (command == "launch_platform_type")
   {
      std::string lpType;
      aInput.ReadValue(lpType);
      mLaunchPlatformTypeId = WsfStringId(lpType);
   }
   else if (command == "target_platform_type")
   {
      std::string tpType;
      aInput.ReadValue(tpType);
      mTargetPlatformTypeId = WsfStringId(tpType);
   }
   else if (command == "weapon_name")
   {
      std::string weaponName;
      aInput.ReadValue(weaponName);
      mWeaponNameId = WsfStringId(weaponName);
   }
   else if (command == "weapon_effects")
   {
      std::string weaponEffect;
      aInput.ReadValue(weaponEffect);
      mWeaponEffectTypeId = WsfStringId(weaponEffect);
   }
   else if (command == "tool_produces")
   {
      std::string product;
      aInput.ReadValue(product);
      mToolProduces = WsfStringId(product);
   }
   else if (command == "output_object_name")
   {
      std::string outName;
      aInput.ReadValue(outName);
      mOutputObjectName = WsfStringId(outName);
   }
   else if (command == "output_file_extension")
   {
      std::string ext;
      aInput.ReadValue(ext);
      mFileExtension = WsfStringId(ext);
   }
   else if (command == "output_file_name")
   {
      std::string outFile;
      aInput.ReadValueQuoted(outFile);
      outFile         = aInput.SubstitutePathVariables(outFile);
      mOutputFileName = WsfStringId(outFile);
   }
   else if (command == "terminate_on_launch_failure")
   {
      aInput.ReadValue(mTerminateOnLaunchFailure);
   }
   else
   {
      myCommand = false;
   }

   return myCommand;
}

WsfStringId Tool::WeaponPlatformTypeId() const
{
   if (mLaunchWpnPtr != nullptr)
   {
      return WsfStringId(mLaunchWpnPtr->GetLaunchedPlatformType());
   }
   return WsfStringId(nullptr);
}

//! In another typical WSF construct, this method initializes the object prior to run time.
//! If the Initialization fails, the object will not be permitted to transition to the run mode.
//! Derived classes must assure that they are initialized internally, as well as calling the
//! base class Initialize method.
//!   @return Boolean success value.
// virtual
bool Tool::Initialize(WsfSimulation& aSimulation)
{
   bool success   = true;
   mSimulationPtr = &aSimulation;
   mObserver.Initialize(aSimulation);
   if (mWeaponEffectTypeId != 0)
   {
      std::string effectType = mWeaponEffectTypeId.GetString();
      if (WsfWeaponEffectsTypes::Get(GetScenario()).Find(effectType) == nullptr)
      {
         auto logError = ut::log::error() << "Weapon Effects Type Not Found!";
         logError.AddNote() << "Weapon Effects Type: " << effectType;
         success = false;
      }
   }

   // For Tools within Tools see if there is already a launch platform by the name
   // of "launcher" in the sim. If so, do NOT add another.
   mLaunchPlatPtr = aSimulation.GetPlatformByName(launcherNameId);
   if (mLaunchPlatPtr == nullptr)
   {
      mLaunchPlatPtr = WsfPlatformTypes::Get(GetScenario()).Clone(mLaunchPlatformTypeId);
   }

   if (mLaunchPlatPtr == nullptr)
   {
      auto logError = ut::log::error() << "Unable to clone the required Platform Type.";
      logError.AddNote() << "Platform Type: " << mLaunchPlatformTypeId;
      success = false;
   }

   if (success)
   {
      mLaunchPlatPtr->SetLocationLLA(mLatDeg, mLonDeg, mAlt);
      mLaunchPlatPtr->SetOrientationNED(mHeadingRad, 0.0, 0.0);
      double zero[] = {0.0, 0.0, 0.0};
      mLaunchPlatPtr->SetVelocityNED(zero);

      WsfWeapon* weaponPtr = nullptr;
      if (!mWeaponNameId.IsNull())
      {
         weaponPtr = mLaunchPlatPtr->GetComponent<WsfWeapon>(mWeaponNameId);
      }
      else
      {
         weaponPtr = mLaunchPlatPtr->GetComponentEntry<WsfWeapon>(0);
         if (weaponPtr != nullptr)
         {
            mWeaponNameId = weaponPtr->GetNameId();
         }
      }

      mLaunchWpnPtr = dynamic_cast<WsfExplicitWeapon*>(weaponPtr);
      if (mLaunchWpnPtr != nullptr)
      {
         if (mWeaponEffectTypeId != 0)
         {
            // Override the lethality with our own, for the purposes of generating this envelope:
            mLaunchWpnPtr->SetWeaponEffectsType(mWeaponEffectTypeId);
         }

         // The weapon tools will need an unlimited quantity of rapidly firing weapons...
         mLaunchWpnPtr->SetQuantityRemaining(1.0E10);
      }
      else
      {
         auto logError = ut::log::error()
                         << "Launch platform did not contain an explicit weapon "; // end of sentence to be filled in later.
         logError.AddNote() << "Platform: " << mLaunchPlatPtr->GetName();

         if (!mWeaponNameId.IsNull())
         {
            // Appending directly to logError
            logError << "with the expected name.";
            logError.AddNote() << "Name: " << mWeaponNameId;
         }
         else
         {
            logError << "as weapon index zero.";
            logError.AddNote() << "No name was provided.";
         }
         success = false;
      }
   }

   if (success)
   {
      mObserver.SetToolPtr(this);
      mObserver.ResetState();

      mInitialized = true;
      mLastUpdate  = aSimulation.GetSimTime();
      ResolveNames();
      aSimulation.AddEvent(ut::make_unique<ToolUpdateEvent>(aSimulation.GetSimTime(), this));

      // Add the launch platform to the simulation.
      mLaunchPlatPtr->SetName(launcherNameId);

      // Here we must make sure that each launch computer on each launching
      // weapon is aware that it will be in LaunchComputer "Generation" mode.
      for (WsfComponentList::RoleIterator<WsfWeapon> iter(*mLaunchPlatPtr); !iter.AtEnd(); ++iter)
      {
         WsfLaunchComputer* lcPtr = iter->GetLaunchComputer();
         if (lcPtr != nullptr)
         {
            lcPtr->SetComputerGenerationMode(true);
         }
      }

      if (!aSimulation.AddPlatform(aSimulation.GetSimTime(), mLaunchPlatPtr))
      {
         ut::log::error() << "Unable to add launch platform to the simulation!";
         mInitialized = false;
         UnInitialize();
      }
   }

   return mInitialized;
}

//! If Initialize() fails, this method will de-register the Tool from active consideration within the
//! main execution loop, allowing the simulation to complete when all other Tools (if any) are done.
void Tool::UnInitialize()
{
   GetManager()->UnRegister(this);
}

//! Advance the Tool forward in time. This base method shall be overridden in derived classes, and at the
//! end of the derived method, this parent method must be called, to properly maintain the internal state.
// virtual
void Tool::Update(double aSimTime)
{
   if (mDone && !mNotifiedDone)
   {
      GetManager()->UnRegister(this);
      mNotifiedDone = true;
   }
   mLastUpdate = aSimTime;
}

//! Optional tool method called by WeaponObserver to pass on the MoverBurnout simulation observer event.
// virtual
void Tool::MoverBurnout(double /*aSimTime*/, WsfMover* /*aMoverPtr*/) {}

//! Optional tool method called by WeaponObserver to pass on the GuidanceComputerPhaseChanged simulation observer event.
// virtual
void Tool::GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr) {}

//! Optional tool method called by WeaponObserver to pass on the PlatformAdded simulation observer event.
// virtual
void Tool::PlatformAdded(double /*aSimTime*/, WsfPlatform* /*aPlatformPtr*/) {}

//! Virtual method to resolve the output object name and output file names.
// virtual
void Tool::ResolveNames()
{
   std::string OutputObjectName = WeaponPlatformTypeId().GetString() + mToolProduces;

   if (mOutputObjectName.empty())
   {
      mOutputObjectName = WsfStringId(OutputObjectName);
   }

   if (mOutputFileName.empty())
   {
      std::string OutputFileName = mOutputObjectName + mFileExtension;
      mOutputFileName            = WsfStringId(OutputFileName);
   }
}

ToolManager* Tool::GetManager() const
{
   return ToolManager::Find(GetScenario());
}
