// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "P6DOF_ControllerPluginObject.hpp"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>

#include "P6DOF_ControllerDataContainer.hpp"
#include "P6DOF_ControllerDockWidget.hpp"
#include "P6DOF_ControllerPrefObject.hpp"
#include "P6DOF_ControllerSimCommands.hpp"
#include "P6DOF_ControllerSimInterface.hpp"
#include "SDL_AudioManager.hpp"
#include "SDL_JoystickManager.hpp"
#include "SDL_Manager.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkScriptSimInterface.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

WkP6DOF_Controller::PluginObject::PluginObject(SimInterface* aInterface, const PrefObject* aPrefObject)
   : QObject()
   , mSimInterface(aInterface)
   , mScriptSimInterfacePtr(simEnv.GetScriptSimInterface())
   , mPrefObject(aPrefObject)
{
   mShowHud = new wkf::Action("ShowHUD", wkfEnv.GetMainWindow());
   mShowHud->setCheckable(true);
   mShowHud->setChecked(true);
   connect(mShowHud, &wkf::Action::toggled, this, &PluginObject::ShowHUD);
   connect(mPrefObject, &PrefObject::ShowHudChanged, this, &PluginObject::ShowHUD);
   mActions.push_back(mShowHud);

   mFirstPerson = new wkf::Action("FirstPersonView", wkfEnv.GetMainWindow());
   mFirstPerson->setCheckable(true);
   mFirstPerson->setChecked(true);
   connect(mFirstPerson, &wkf::Action::toggled, this, &PluginObject::FirstPerson);
   connect(mPrefObject, &PrefObject::ShowFirstPersonChanged, this, &PluginObject::FirstPerson);
   mActions.push_back(mFirstPerson);

   mAutopilot = new wkf::Action("ActivateAutopilot", wkfEnv.GetMainWindow());
   mAutopilot->setCheckable(true);
   mAutopilot->setChecked(true);
   connect(mAutopilot, &wkf::Action::toggled, this, &PluginObject::ActivateAutopilot);
   mActions.push_back(mAutopilot);

   mManualPilot = new wkf::Action("ActivateManualPilot", wkfEnv.GetMainWindow());
   mManualPilot->setCheckable(true);
   mManualPilot->setChecked(false);
   connect(mManualPilot, &wkf::Action::toggled, this, &PluginObject::ActivateManualPilot);
   mActions.push_back(mManualPilot);

   mSyntheticPilot = new wkf::Action("ActivateSyntheticPilot", wkfEnv.GetMainWindow());
   mSyntheticPilot->setCheckable(true);
   mSyntheticPilot->setChecked(false);
   connect(mSyntheticPilot, &wkf::Action::toggled, this, &PluginObject::ActivateSyntheticPilot);
   mActions.push_back(mSyntheticPilot);

   mGuidancePilot = new wkf::Action("ActivateGuidance", wkfEnv.GetMainWindow());
   mGuidancePilot->setCheckable(true);
   mGuidancePilot->setChecked(false);
   connect(mGuidancePilot, &wkf::Action::toggled, this, &PluginObject::ActivateGuidance);
   mActions.push_back(mGuidancePilot);

   connect(mScriptSimInterfacePtr.get(),
           &warlock::ScriptSimInterface::ReturnValueReady,
           this,
           &WkP6DOF_Controller::PluginObject::ReturnValueReadyCB,
           Qt::QueuedConnection);
}

WkP6DOF_Controller::PluginObject::~PluginObject()
{
   // mDockWidget is a child to the MainWindow, the MainWindow will delete it,
   // therefore we do not need to delete it here
   if (nullptr != mJoystickManager)
   {
      if (mJoystickManager->HasDevice(0))
      {
         mJoystickManager->ReleaseDevice(0);
      }
      delete mJoystickManager;
      mJoystickManager = nullptr;
   }

   if (nullptr != mAudioManager)
   {
      delete mAudioManager;
      mAudioManager = nullptr;
   }
}

void WkP6DOF_Controller::PluginObject::Update()
{
   if (mDataContainer.GetSimulationInitialized())
   {
      Reset();
      mDataContainer.SetSimulationInitialized(false);
   }
   else if (!mCrashed)
   {
      // Set the connected platform, based on the data container
      mConnectedPlatform = mDataContainer.GetConnectedPlatform();

      if (mDataContainer.GetConnectionDestroyed())
      {
         mCrashed = true;
         if (mDockWidget)
         {
            mDockWidget->ActivateKillFrame(true);
         }
      }
      else if (!mConnectedPlatform.empty())
      {
         // Get the sim data for the platform
         const P6DOF_ControllerDataContainer::PlatformData& platformData = mDataContainer.GetPlatformData();

         // Get the time (locally) for use with periodic data
         double currentTime_sec = platformData.simTime;

         if ((nullptr != mJoystickManager) && (mJoystickManager->HasDevice(0)))
         {
            mJoystickManager->Update();

            ControlCommand::ControlData controlData;
            controlData.roll   = mJoystickManager->GetValue(0, "roll");
            controlData.pitch  = mJoystickManager->GetValue(0, "pitch");
            controlData.rudder = mJoystickManager->GetValue(0, "rudder");

            controlData.throttle = mJoystickManager->GetValue(0, "throttle");
            float afterBurner    = mJoystickManager->GetValue(0, "afterburner");

            if (afterBurner > 0.01)
            {
               if (controlData.throttle > 0.99)
               {
                  controlData.throttle = 1 + afterBurner;
               }
            }

            float spd_brake_extend  = mJoystickManager->GetValue(0, "spd_brake_extend");
            float spd_brake_retract = mJoystickManager->GetValue(0, "spd_brake_retract");
            controlData.spdBrake    = 0.0f;
            if (spd_brake_extend > 0.5)
            {
               controlData.spdBrake = 1.0f;
            }
            else if (spd_brake_retract > 0.5)
            {
               controlData.spdBrake = -1.0f;
            }

            float landing_gear_down = mJoystickManager->GetValue(0, "landing_gear_down");
            controlData.landingGear = 0.0f;
            ;
            if (landing_gear_down > 0.5)
            {
               controlData.landingGear = 1.0f;
            }

            // FUTURE:  Add flaps support in the future:
            // float flaps_down = mJoystickManager->GetValue(0, "flaps_down");

            float trimInputPitch = mJoystickManager->GetValue(0, "trim_el");
            if (trimInputPitch > 0.001)
            {
               controlData.trimNoseUp = -1.0;
            }
            else if (trimInputPitch < -0.001)
            {
               controlData.trimNoseUp = 1.0;
            }

            float trimInputRoll = mJoystickManager->GetValue(0, "trim_az");
            if (trimInputRoll > 0.001)
            {
               controlData.trimRollRight = 1.0;
            }
            else if (trimInputRoll < -0.001)
            {
               controlData.trimRollRight = -1.0;
            }

            // Send the control data to the AFSIM sim via the P6DOF_ControllerSimInterface
            // Be sure that no additional flight controls data additions to controlData
            // are performed below this point or they will not be communicated back to sim.
            mSimInterface->AddSimCommand(ut::make_unique<ControlCommand>(mConnectedPlatform, controlData));

            // Execute joystick manager scripts
            for (const std::string& scriptName : mJoystickManager->GetScriptQueue())
            {
               warlock::script::Instance scriptInstance;
               scriptInstance.platformName = mConnectedPlatform;
               std::cout << "*** Execute script (" << scriptName << ")" << std::endl;
               mScriptSimInterfacePtr->GetPlatformScriptData(mConnectedPlatform, scriptName, scriptInstance.script);
               mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
            }
            mJoystickManager->ClearScriptQueue();

            // Uncomment the next line to output a list of controllers via cout (used for debugging)
            // void PrintListOfControllers();

            // Now, we need to get non-flight control joystick/controller/keyboard input

            // Verify which script functions exist
            mReturnIntArrayDataScriptExists      = ScriptExists("ReturnIntArrayData");
            mReturnDoubleArrayDataExists         = ScriptExists("ReturnDoubleArrayData");
            mReturnStringArrayDataExists         = ScriptExists("ReturnStringArrayData");
            mReturnBitPackedFlagsExists          = ScriptExists("ReturnBitPackedFlags");
            mNumChaffRemainingExists             = ScriptExists("NumChaffRemaining");
            mNumFlaresRemainingExists            = ScriptExists("NumFlaresRemaining");
            mGetBingoFuelQtyExists               = ScriptExists("GetBingoFuelQty");
            mWeaponSelectToggledExists           = ScriptExists("WeaponSelectToggled");
            mMasterArmToggleExists               = ScriptExists("MasterArmToggle");
            mAirGroundModeToggleExists           = ScriptExists("AirGroundModeToggle");
            mEcmPowerToggleExists                = ScriptExists("EcmPowerToggle");
            mEcmAutoToggleExists                 = ScriptExists("EcmAutoToggle");
            mChaffButtonPressedExists            = ScriptExists("ChaffButtonPressed");
            mChaffButtonReleasedExists           = ScriptExists("ChaffButtonReleased");
            mFlaresButtonPressedExists           = ScriptExists("FlaresButtonPressed");
            mFlaresButtonReleasedExists          = ScriptExists("FlaresButtonReleased");
            mCountermeasuresButtonPressedExists  = ScriptExists("CountermeasuresButtonPressed");
            mCountermeasuresButtonReleasedExists = ScriptExists("CountermeasuresButtonReleased");
            mRadarPowerToggleExists              = ScriptExists("RadarPowerToggle");
            mRadarModeCycleExists                = ScriptExists("RadarModeCycle");
            mRadarSubmodeCycleExists             = ScriptExists("RadarSubmodeCycle");
            mIrstPowerToggleExists               = ScriptExists("IrstPowerToggle");
            mIrstModeCycleExists                 = ScriptExists("IrstModeCycle");
            mIrstCueToRadarExists                = ScriptExists("IrstCueToRadar");
            mIrstCueToHeadExists                 = ScriptExists("IrstCueToHead");
            mFlirPowerToggleExists               = ScriptExists("FlirPowerToggle");
            mFlirModeCycleExists                 = ScriptExists("FlirModeCycle");
            mRwrPowerToggleExists                = ScriptExists("RwrPowerToggle");
            mRwrFilterToggleExists               = ScriptExists("RwrFilterToggle");
            mMissileHudModeSelectedExists        = ScriptExists("MissileHudModeSelected");
            mGunHudModeSelectedExists            = ScriptExists("GunHudModeSelected");
            mGroundAttackHudModeSelectedExists   = ScriptExists("GroundAttackHudModeSelected");
            mNonWeaponHudModeSelectedExists      = ScriptExists("NonWeaponHudModeSelected");
            mJettisonExternalTanksExists         = ScriptExists("JettisonExternalTanks");
            mClearMasterWarningExists            = ScriptExists("ClearMasterWarning");
            mLockTargetExists                    = ScriptExists("LockTarget");

            ReadWeaponInputs();
            ReadSensorInputs();
            ReadEW_Inputs();
            ReadControlsAndDisplaysInputs();
            ReadOtherInputs();

            // This requests various data via script at periodic rates
            RequestDataViaScriptCalls(currentTime_sec);

            // Adjusting where to "look" (deprecated)
            float pinkie_paddle = mJoystickManager->GetValue(0, "pinkie_paddle");
            float azAccumInput  = mJoystickManager->GetValue(0, "view_az_accum") * 10.0;
            float elAccumInput  = mJoystickManager->GetValue(0, "view_el_accum") * 10.0;
            float viewAzInput   = mJoystickManager->GetValue(0, "view_az");
            float viewElInput   = mJoystickManager->GetValue(0, "view_el");
            AdjustViewDirection(pinkie_paddle, azAccumInput, elAccumInput, viewAzInput, viewElInput);
         }

         // Set various audio tones/warnings based on various controls and data
         SetAudioTones(currentTime_sec);

         // This function will transfer data to the HUD's data structure
         UpdateDataForHud();
      }
   }
}

void WkP6DOF_Controller::PluginObject::ConnectToPlatform(const QString& aName, QWidget* aParent)
{
   auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(aName);
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Unable to Connect",
                           "User does not have permissions to modify platform " +
                              QString::fromStdString(platformPtr->GetName()) + ".");

      mSimInterface->SetConnectedPlatform("");
      mDataContainer.SetConnectedPlatform("");
      return;
   }

   // Set the connected platform names for both sim interface and data container
   mSimInterface->SetConnectedPlatform(aName.toStdString());
   mDataContainer.SetConnectedPlatform(aName.toStdString());

   mCrashed = false;
   if (nullptr != mDockWidget)
   {
      delete mDockWidget;
      mDockWidget = nullptr;
   }
   if (!mDockWidget)
   {
      if (nullptr == mAudioManager)
      {
         mAudioManager = new sdl::AudioManager(sdl::Manager::Instance());
      }

      if (nullptr == mJoystickManager)
      {
         mJoystickManager = new sdl::JoystickManager(sdl::Manager::Instance(), wkfEnv.GetMainWindow());
         mJoystickManager->UseDefinedDevice(0);
      }

      mDockWidget = new DockWidget(mJoystickManager,
                                   mAudioManager,
                                   aName,
                                   mActions,
                                   mSimInterface,
                                   &mDataContainer,
                                   wkfEnv.GetMainWindow()->centralWidget());

      // Docking and immediately floating the window will guarantee our dock widget has a title bar.
      mDockWidget->setFloating(false);
      mDockWidget->setFloating(true);

      // Set the dock widget's geometry to spawn at the same location as the main window.
      mDockWidget->setGeometry(QRect{wkfEnv.GetMainWindow()->geometry().x(), wkfEnv.GetMainWindow()->geometry().y(), 0, 0});

      // Set the width and height of the dock widget to that which is specified in the preferences object.
      unsigned int width  = mPrefObject->GetResolutionX() / QApplication::desktop()->devicePixelRatio();
      unsigned int height = mPrefObject->GetResolutionY() / QApplication::desktop()->devicePixelRatio();
      mDockWidget->SetDefaultWidth(width);
      mDockWidget->SetDefaultHeight(height);

      mDockWidget->installEventFilter(mJoystickManager);
      connect(mDockWidget.data(), &wkf::ImmersiveViewDockWidget::ImmersiveViewClosed, [this]() { ReleasePlatform(); });
      QString title = "P6DOF Controller: ";
      title += aName;
      mDockWidget->setWindowTitle(title);
      connect(mDockWidget.data(), &QObject::destroyed, this, &PluginObject::DialogDestroyed);
   }

   if (mPrefObject->GetHDD())
   {
      wkfEnv.RequestPlatformResourceInstance("hdd", aName);
   }
   if (mDockWidget->IsInitialized())
   {
      ViewerInitializedHandler();
   }
   else
   {
      connect(mDockWidget.data(),
              &WkP6DOF_Controller::DockWidget::ImmersiveViewInitialized,
              this,
              &WkP6DOF_Controller::PluginObject::ViewerInitializedHandler);
   }

   // mDockWidget->raise();
}

void WkP6DOF_Controller::PluginObject::ViewerInitializedHandler()
{
   FirstPerson(mPrefObject->GetFirstPerson());

   ShowHUD(mPrefObject->GetShowHud());
   if (mPrefObject->GetManualPilotMode())
   {
      ActivateManualPilot(true);
   }
   else if (mPrefObject->GetSyntheticPilotMode())
   {
      ActivateSyntheticPilot(true);
   }
   else if (mPrefObject->GetGuidancePilotMode())
   {
      ActivateGuidance(true);
   }
   else
   {
      ActivateAutopilot(true);
   }

   mDockWidget->raise();
}

void WkP6DOF_Controller::PluginObject::ReleasePlatform()
{
   mSimInterface->ReleaseConnectedPlatform();
   mDataContainer.ReleaseConnectedPlatform();
}

void WkP6DOF_Controller::PluginObject::DialogDestroyed(QObject* aObj)
{
   mDockWidget = nullptr;
}

void WkP6DOF_Controller::PluginObject::Reset()
{
   if (mDockWidget != nullptr)
   {
      delete mDockWidget;
      mDockWidget = nullptr;
   }
}

void WkP6DOF_Controller::PluginObject::RequestDataViaScriptCalls(double aCurrentTime_sec)
{
   // Note that we do nothing if times are the same

   // First we check for 20 Hz (50 msec)
   if (aCurrentTime_sec > mLastRequestTime_50msec)
   {
      // This is typical condition. Now check deltaT

      if ((aCurrentTime_sec - mLastRequestTime_50msec) >= 0.05)
      {
         mLastRequestTime_50msec = aCurrentTime_sec;

         // Call various scripts to get script-side data, but only if they exist.
         // The returned data will be handled in ReturnValueReadyCB(). Note that the
         // returned data is asynchronous (typically arriving about 50m msec later).
         if (mReturnIntArrayDataScriptExists)
         {
            CallScript("ReturnIntArrayData");
         }
         if (mReturnDoubleArrayDataExists)
         {
            CallScript("ReturnDoubleArrayData");
         }
         if (mReturnStringArrayDataExists)
         {
            CallScript("ReturnStringArrayData");
         }
         if (mReturnBitPackedFlagsExists)
         {
            CallScript("ReturnBitPackedFlags");
         }
         if (mNumChaffRemainingExists)
         {
            CallScript("NumChaffRemaining");
         }
         if (mNumFlaresRemainingExists)
         {
            CallScript("NumFlaresRemaining");
         }
      }
   }
   else if (aCurrentTime_sec < mLastRequestTime_50msec)
   {
      // This shouldn't happen unless we have switched platforms, so just save the time
      mLastRequestTime_50msec = aCurrentTime_sec;
   }

   // Next, we check for 1 Hz
   if (aCurrentTime_sec > mLastRequestTime_1sec)
   {
      // This is typical condition. Now check deltaT
      if ((aCurrentTime_sec - mLastRequestTime_1sec) >= 1.0)
      {
         mLastRequestTime_1sec = aCurrentTime_sec;
         if (mGetBingoFuelQtyExists)
         {
            CallScript("GetBingoFuelQty");
         }
      }
   }
   else if (aCurrentTime_sec < mLastRequestTime_1sec)
   {
      // This shouldn't happen unless we have switched platforms, so just save the time
      mLastRequestTime_1sec = aCurrentTime_sec;
   }
}

void WkP6DOF_Controller::PluginObject::UpdateDataForHud()
{
   if (mDockWidget)
   {
      if (!mDockWidget->HUD_Visible())
      {
         return;
      }
      else
      {
         // We first transfer data from the simulation data (P6DOF_ControllerSimInterface)
         // to the hud data and then we will over-write using particular script-based data values

         WkP6DOF_Controller::HUD* hud = mDockWidget->GetHUD();

         const P6DOF_ControllerDataContainer::PlatformData& platformData = mDataContainer.GetPlatformData();

         // Set the data inside the HUD
         hud->mPlatData = platformData;

         // Check for BINGO fuel state
         if (platformData.currentTotalFuelTankQuantity_lbs < mScriptBasedPlatData.bingoFuelQuantity_lbs)
         {
            mScriptBasedPlatData.bingoFlagActive = true;
         }
         else
         {
            mScriptBasedPlatData.bingoFlagActive = false;
         }

         // We now over-write using particular script-based data values
         hud->mPlatData.bingoFuelQuantity_lbs = mScriptBasedPlatData.bingoFuelQuantity_lbs;
         hud->mPlatData.bingoFlagActive       = mScriptBasedPlatData.bingoFlagActive;

         hud->mPlatData.activeWeaponType = mScriptBasedPlatData.activeWeaponType;
         hud->mPlatData.activeWeapon     = mScriptBasedPlatData.activeWeapon;

         hud->mPlatData.shootFlagActive      = mScriptBasedPlatData.shootFlagActive;
         hud->mPlatData.breakX_Flag          = mScriptBasedPlatData.breakX_Flag;
         hud->mPlatData.ecmXmit              = mScriptBasedPlatData.ecmXmit;
         hud->mPlatData.jammingDetected      = mScriptBasedPlatData.jammingDetected;
         hud->mPlatData.masterArmEnabled     = mScriptBasedPlatData.masterArmEnabled;
         hud->mPlatData.masterWarningEnabled = mScriptBasedPlatData.masterWarningEnabled;

         // hud->mPlatData.radarMode              = mScriptBasedPlatData.radarMode;
         // hud->mPlatData.rwrMode                = mScriptBasedPlatData.rwrMode;
         // hud->mPlatData.irstMode               = mScriptBasedPlatData.irstMode;
         // hud->mPlatData.flirMode               = mScriptBasedPlatData.flirMode;

         // Perhaps ask for pending data updates, if true get data
         hud->mPlatData.numChaff   = mScriptBasedPlatData.numChaff;
         hud->mPlatData.numFlares  = mScriptBasedPlatData.numFlares;
         hud->mPlatData.numGunRnds = mScriptBasedPlatData.numGunRnds;

         // Set HUD data based on member data
         hud->mPlatData.tgtDesignationRgt = mTargetDesignationRightIsBeingPressed;
         hud->mPlatData.tgtDesignationLft = mTargetDesignationLeftIsBeingPressed;
         hud->mPlatData.tgtDesignationUp  = mTargetDesignationUpIsBeingPressed;
         hud->mPlatData.tgtDesignationDn  = mTargetDesignationDownIsBeingPressed;

         // NOTE:  Be sure to set data in mScriptBasedPlatData as required

         hud->mPlatData.mfdLftMode = mScriptBasedPlatData.mfdLftMode;
         hud->mPlatData.mfdRgtMode = mScriptBasedPlatData.mfdRgtMode;
         hud->mPlatData.mfdActive  = mScriptBasedPlatData.mfdActive;

         hud->mPlatData.radarMinAz_deg = mScriptBasedPlatData.radarMinAz_deg;
         hud->mPlatData.radarMaxAz_deg = mScriptBasedPlatData.radarMaxAz_deg;
         hud->mPlatData.radarMinEl_deg = mScriptBasedPlatData.radarMinEl_deg;
         hud->mPlatData.radarMaxEl_deg = mScriptBasedPlatData.radarMaxEl_deg;
      }
   }
}

void WkP6DOF_Controller::PluginObject::ShowHUD(bool aState)
{
   mShowHud->blockSignals(true);
   mShowHud->setChecked(aState);
   mShowHud->blockSignals(false);

   if (mDockWidget)
   {
      mDockWidget->ShowHUD(aState);
   }
}

void WkP6DOF_Controller::PluginObject::FirstPerson(bool aState)
{
   mFirstPerson->blockSignals(true);
   mFirstPerson->setChecked(aState);
   mFirstPerson->blockSignals(false);

   if (mDockWidget)
   {
      mDockWidget->SetFirstPerson(aState);
   }
}

void WkP6DOF_Controller::PluginObject::ActivateAutopilot(bool aState)
{
   BlockPilotActionSignals(true);
   mAutopilot->setChecked(true);
   mManualPilot->setChecked(false);
   mSyntheticPilot->setChecked(false);
   mGuidancePilot->setChecked(false);
   BlockPilotActionSignals(false);
   if (aState)
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(mDataContainer.GetConnectedPlatform(), ActivatePilotCommand::HARDWARE));
   }
}

void WkP6DOF_Controller::PluginObject::ActivateManualPilot(bool aState)
{
   BlockPilotActionSignals(true);
   mAutopilot->setChecked(false);
   mManualPilot->setChecked(true);
   mSyntheticPilot->setChecked(false);
   mGuidancePilot->setChecked(false);
   BlockPilotActionSignals(false);
   if (aState)
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(mDataContainer.GetConnectedPlatform(), ActivatePilotCommand::MANUAL));
   }
}

void WkP6DOF_Controller::PluginObject::ActivateSyntheticPilot(bool aState)
{
   BlockPilotActionSignals(true);
   mAutopilot->setChecked(false);
   mManualPilot->setChecked(false);
   mSyntheticPilot->setChecked(true);
   mGuidancePilot->setChecked(false);
   BlockPilotActionSignals(false);
   if (aState)
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(mDataContainer.GetConnectedPlatform(), ActivatePilotCommand::SYNTHETIC));
   }
}

void WkP6DOF_Controller::PluginObject::ActivateGuidance(bool aState)
{
   BlockPilotActionSignals(true);
   mAutopilot->setChecked(false);
   mManualPilot->setChecked(false);
   mSyntheticPilot->setChecked(false);
   mGuidancePilot->setChecked(true);
   BlockPilotActionSignals(false);
   if (aState)
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(mDataContainer.GetConnectedPlatform(), ActivatePilotCommand::GUIDANCE));
   }
}

void WkP6DOF_Controller::PluginObject::BlockPilotActionSignals(bool aState)
{
   mAutopilot->blockSignals(aState);
   mManualPilot->blockSignals(aState);
   mSyntheticPilot->blockSignals(aState);
   mGuidancePilot->blockSignals(aState);
}

void WkP6DOF_Controller::PluginObject::ProcessButtonState(bool&             aIsBeingPressed,
                                                          bool              aPushed,
                                                          const CallbackFn& aPressedCallback,
                                                          const CallbackFn& aReleasedCallback)
{
   bool newlyPressed  = false;
   bool newlyReleased = false;

   if (aPushed)
   {
      // Is currently pushed/pressed

      // Was it not already being pressed?
      if (!aIsBeingPressed)
      {
         newlyPressed = true;
      }
      aIsBeingPressed = true;
   }
   else
   {
      // Not currently pushed/pressed

      // Was it being pressed?
      if (aIsBeingPressed)
      {
         newlyReleased = true;
      }
      aIsBeingPressed = false;
   }

   if (newlyPressed)
   {
      aPressedCallback();
   }

   if (newlyReleased)
   {
      aReleasedCallback();
   }
}

void WkP6DOF_Controller::PluginObject::ProcessButtonStatePressed(bool&             aIsBeingPressed,
                                                                 bool              aPushed,
                                                                 const CallbackFn& aPressedCallback)
{
   bool newlyPressed = false;

   if (aPushed)
   {
      // Is currently pushed/pressed

      // Was it not already being pressed?
      if (!aIsBeingPressed)
      {
         newlyPressed = true;
      }
      aIsBeingPressed = true;
   }
   else
   {
      // Not currently pushed/pressed
      aIsBeingPressed = false;
   }

   if (newlyPressed)
   {
      aPressedCallback();
   }
}

void WkP6DOF_Controller::PluginObject::ProcessButtonStateReleased(bool&             aIsBeingPressed,
                                                                  bool              aPushed,
                                                                  const CallbackFn& aReleasedCallback)
{
   bool newlyReleased = false;

   if (aPushed)
   {
      aIsBeingPressed = true;
   }
   else
   {
      // Not currently pushed/pressed

      // Was it being pressed?
      if (aIsBeingPressed)
      {
         newlyReleased = true;
      }
      aIsBeingPressed = false;
   }

   if (newlyReleased)
   {
      aReleasedCallback();
   }
}

void WkP6DOF_Controller::PluginObject::ReadWeaponInputs()
{
   // Trigger
   float triggerPulledJoystickInput = mJoystickManager->GetValue(0, "trigger_fire");
   bool  triggerPulled              = false;
   if (triggerPulledJoystickInput > 0.5)
   {
      triggerPulled = true;
   }
   ProcessButtonState(
      mTriggerIsBeingPulled,
      triggerPulled,
      [this]() { TriggerPulledHandler(); },
      [this]() { TriggerReleasedHandler(); });

   // Weapon release
   float weaponReleasePressedJoystickInput = mJoystickManager->GetValue(0, "weapon_release");
   bool  weaponReleasePressed              = false;
   if (weaponReleasePressedJoystickInput > 0.5)
   {
      weaponReleasePressed = true;
   }
   ProcessButtonState(
      mWeaponReleaseIsBeingPressed,
      weaponReleasePressed,
      [this]() { WeaponReleasePressedHandler(); },
      [this]() { WeaponReleaseReleasedHandler(); });

   // Weapon select
   float weaponSelectPressedJoystickInput = mJoystickManager->GetValue(0, "weapon_select");
   bool  weaponSelectPressed              = false;
   if (weaponSelectPressedJoystickInput > 0.5)
   {
      weaponSelectPressed = true;
   }
   ProcessButtonStateReleased(mWeaponSelectIsBeingPressed,
                              weaponSelectPressed,
                              [this]() { WeaponSelectReleasedHandler(); });

   // Master Arm Switch
   float masterArmPressedJoystickInput = mJoystickManager->GetValue(0, "master_arm");
   bool  masterArmPressed              = false;
   if (masterArmPressedJoystickInput > 0.5)
   {
      masterArmPressed = true;
   }
   ProcessButtonStateReleased(mMasterArmButtonIsBeingPressed,
                              masterArmPressed,
                              [this]() { MasterArmButtonReleasedHandler(); });

   // Weapon Mode (A/A or A/G)
   float weaponModePressedJoystickInput = mJoystickManager->GetValue(0, "weapon_mode");
   bool  weaponModePressed              = false;
   if (weaponModePressedJoystickInput > 0.5)
   {
      weaponModePressed = true;
   }
   ProcessButtonStateReleased(mWeaponModeButtonIsBeingPressed,
                              weaponModePressed,
                              [this]() { WeaponModeButtonReleasedHandler(); });
}

void WkP6DOF_Controller::PluginObject::ReadSensorInputs()
{
   // Radar Power
   float radarPowerTogglePressedJoystickInput = mJoystickManager->GetValue(0, "radar_power");
   bool  radarPowerTogglePressed              = false;
   if (radarPowerTogglePressedJoystickInput > 0.5)
   {
      radarPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mRadarPowerTogglePressed,
                              radarPowerTogglePressed,
                              [this]() { RadarPowerToggleReleasedHandler(); });

   // Radar Mode Cycle
   float radarModeCyclePressedJoystickInput = mJoystickManager->GetValue(0, "radar_mode_cycle");
   bool  radarModeCyclePressed              = false;
   if (radarModeCyclePressedJoystickInput > 0.5)
   {
      radarModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mRadarModeCyclePressed,
                              radarModeCyclePressed,
                              [this]() { RadarModeCycleReleasedHandler(); });

   // Radar Submode Cycle
   float radarSubmodeCyclePressedJoystickInput = mJoystickManager->GetValue(0, "radar_submode_cycle");
   bool  radarSubmodeCyclePressed              = false;
   if (radarSubmodeCyclePressedJoystickInput > 0.5)
   {
      radarSubmodeCyclePressed = true;
   }
   ProcessButtonStateReleased(mRadarSubmodeCyclePressed,
                              radarSubmodeCyclePressed,
                              [this]() { RadarSubmodeCycleReleasedHandler(); });

   // IRST Power
   float irstPowerTogglePressedJoystickInput = mJoystickManager->GetValue(0, "irst_power");
   bool  irstPowerTogglePressed              = false;
   if (irstPowerTogglePressedJoystickInput > 0.5)
   {
      irstPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mIrstPowerTogglePressed,
                              irstPowerTogglePressed,
                              [this]() { IrstPowerToggleReleasedHandler(); });

   // IRST Mode Cycle
   float irstModeCyclePressedJoystickInput = mJoystickManager->GetValue(0, "irst_mode_cycle");
   bool  irstModeCyclePressed              = false;
   if (irstModeCyclePressedJoystickInput > 0.5)
   {
      irstModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstModeCyclePressed, irstModeCyclePressed, [this]() { IrstModeCycleReleasedHandler(); });

   // IRST Cue to Radar
   float irstCueToRadarPressedJoystickInput = mJoystickManager->GetValue(0, "irst_cue_to_radar");
   bool  irstCueToRadarCyclePressed         = false;
   if (irstCueToRadarPressedJoystickInput > 0.5)
   {
      irstCueToRadarCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstCueToRadarPressed,
                              irstCueToRadarCyclePressed,
                              [this]() { IrstCueToRadarReleasedHandler(); });

   // IRST Cue to (Pilot) Head
   float irstCueToHeadPressedJoystickInput = mJoystickManager->GetValue(0, "irst_cue_to_radar");
   bool  irstCueToHeadCyclePressed         = false;
   if (irstCueToHeadPressedJoystickInput > 0.5)
   {
      irstCueToHeadCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstCueToHeadPressed,
                              irstCueToHeadCyclePressed,
                              [this]() { IrstCueToHeadReleasedHandler(); });

   // FLIR Power
   float flirPowerTogglePressedJoystickInput = mJoystickManager->GetValue(0, "flir_power");
   bool  flirPowerTogglePressed              = false;
   if (flirPowerTogglePressedJoystickInput > 0.5)
   {
      flirPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mFlirPowerTogglePressed,
                              flirPowerTogglePressed,
                              [this]() { FlirPowerToggleReleasedHandler(); });

   // FLIR Mode Cycle
   float flirModeCyclePressedJoystickInput = mJoystickManager->GetValue(0, "flir_mode_cycle");
   bool  flirModeCyclePressed              = false;
   if (flirModeCyclePressedJoystickInput > 0.5)
   {
      flirModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mFlirModeCyclePressed, flirModeCyclePressed, [this]() { FlirModeCycleReleasedHandler(); });

   // RWR Power
   float rwrPowerTogglePressedJoystickInput = mJoystickManager->GetValue(0, "rwr_power");
   bool  rwrPowerTogglePressed              = false;
   if (rwrPowerTogglePressedJoystickInput > 0.5)
   {
      rwrPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mRwrPowerTogglePressed,
                              rwrPowerTogglePressed,
                              [this]() { RwrPowerToggleReleasedHandler(); });

   // RWR Filter
   float rwrFilterTogglePressedJoystickInput = mJoystickManager->GetValue(0, "rwr_filter");
   bool  rwrFilterTogglePressed              = false;
   if (rwrFilterTogglePressedJoystickInput > 0.5)
   {
      rwrFilterTogglePressed = true;
   }
   ProcessButtonStateReleased(mRwrFilterTogglePressed,
                              rwrFilterTogglePressed,
                              [this]() { RwrFilterToggleReleasedHandler(); });
}

void WkP6DOF_Controller::PluginObject::ReadEW_Inputs()
{
   // Chaff Button
   float chaffPressedJoystickInput = mJoystickManager->GetValue(0, "chaff_release");
   bool  chaffPressed              = false;
   if (chaffPressedJoystickInput > 0.5)
   {
      chaffPressed = true;
   }
   ProcessButtonState(
      mChaffButtonIsBeingPressed,
      chaffPressed,
      [this]() { ChaffButtonPressedHandler(); },
      [this]() { ChaffButtonReleasedHandler(); });

   // Flares Button
   float flaresPressedJoystickInput = mJoystickManager->GetValue(0, "flare_release");
   bool  flaresPressed              = false;
   if (flaresPressedJoystickInput > 0.5)
   {
      flaresPressed = true;
   }
   ProcessButtonState(
      mFlareButtonIsBeingPressed,
      flaresPressed,
      [this]() { FlaresButtonPressedHandler(); },
      [this]() { FlaresButtonReleasedHandler(); });

   // Countermeasures Button
   float cmPressedJoystickInput = mJoystickManager->GetValue(0, "cm_release");
   bool  cmPressed              = false;
   if (cmPressedJoystickInput > 0.5)
   {
      cmPressed = true;
   }
   ProcessButtonState(
      mCountermeasureButtonIsBeingPressed,
      cmPressed,
      [this]() { CountermeasuresButtonPressedHandler(); },
      [this]() { CountermeasuresButtonReleasedHandler(); });

   // ECM Toggle
   float ecmPowerPressedJoystickInput = mJoystickManager->GetValue(0, "ecm_power");
   bool  ecmPowerPressed              = false;
   if (ecmPowerPressedJoystickInput > 0.5)
   {
      ecmPowerPressed = true;
   }
   ProcessButtonStateReleased(mEcmPowerButtonIsBeingPressed,
                              ecmPowerPressed,
                              [this]() { EcmPowerButtonReleasedHandler(); });

   // ECM Mode
   float ecmAutoPressedJoystickInput = mJoystickManager->GetValue(0, "ecm_auto");
   bool  ecmAutoPressed              = false;
   if (ecmAutoPressedJoystickInput > 0.5)
   {
      ecmAutoPressed = true;
   }
   ProcessButtonStateReleased(mEcmAutoButtonIsBeingPressed, ecmAutoPressed, [this]() { EcmAutoButtonReleasedHandler(); });
}

void WkP6DOF_Controller::PluginObject::ReadControlsAndDisplaysInputs()
{
   // HUD Mode Cycle
   float hudModePressedJoystickInput = mJoystickManager->GetValue(0, "hud_mode_cycle");
   bool  hudModePressed              = false;
   if (hudModePressedJoystickInput > 0.5)
   {
      hudModePressed = true;
   }
   ProcessButtonStateReleased(mHudModeButtonIsBeingPressed, hudModePressed, [this]() { HudModeButtonReleasedHandler(); });

   // MFD Lft Mode Cycle
   float mfdLftModePressedJoystickInput = mJoystickManager->GetValue(0, "mfd_mode_lft_cycle");
   bool  mfdLftModePressed              = false;
   if (mfdLftModePressedJoystickInput > 0.5)
   {
      mfdLftModePressed = true;
   }
   ProcessButtonStateReleased(mMfdLftModeButtonIsBeingPressed,
                              mfdLftModePressed,
                              [this]() { MfdLftModeButtonReleasedHandler(); });

   // MFD Rgt Mode Cycle
   float mfdRgtModePressedJoystickInput = mJoystickManager->GetValue(0, "mfd_mode_rgt_cycle");
   bool  mfdRgtModePressed              = false;
   if (mfdRgtModePressedJoystickInput > 0.5)
   {
      mfdRgtModePressed = true;
   }
   ProcessButtonStateReleased(mMfdRgtModeButtonIsBeingPressed,
                              mfdRgtModePressed,
                              [this]() { MfdRgtModeButtonReleasedHandler(); });

   // MFD Active Cycle
   float mfdActivePressedJoystickInput = mJoystickManager->GetValue(0, "mfd_active_cycle");
   bool  mfdActivePressed              = false;
   if (mfdActivePressedJoystickInput > 0.5)
   {
      mfdActivePressed = true;
   }
   ProcessButtonStateReleased(mMfdActiveButtonIsBeingPressed,
                              mfdActivePressed,
                              [this]() { MfdActiveButtonReleasedHandler(); });

   // Increase Sensor Range
   float increaseSensorRangePressedJoystickInput = mJoystickManager->GetValue(0, "sensor_rng_incr");
   bool  increaseSensorRangePressed              = false;
   if (increaseSensorRangePressedJoystickInput > 0.5)
   {
      increaseSensorRangePressed = true;
   }
   ProcessButtonStatePressed(mIncreaseSensorRangePressed,
                             increaseSensorRangePressed,
                             [this]() { IncreaseSensorRangePressedHandler(); });

   // Decrease Sensor Range
   float decreaseSensorRangePressedJoystickInput = mJoystickManager->GetValue(0, "sensor_rng_decr");
   bool  decreaseSensorRangePressed              = false;
   if (decreaseSensorRangePressedJoystickInput > 0.5)
   {
      decreaseSensorRangePressed = true;
   }
   ProcessButtonStatePressed(mDecreaseSensorRangePressed,
                             decreaseSensorRangePressed,
                             [this]() { DecreaseSensorRangePressedHandler(); });

   // Lock on to target
   float lockTargetPressedJoystickInput = mJoystickManager->GetValue(0, "lock_target");
   bool  lockTargetPressed              = false;
   if (lockTargetPressedJoystickInput > 0.5)
   {
      lockTargetPressed = true;
   }
   ProcessButtonStatePressed(mLockTargetIsBeingPressed, lockTargetPressed, [this]() { LockTargetPressedHandler(); });

   // Unlock the target
   float unlockTargetPressedJoystickInput = mJoystickManager->GetValue(0, "unlock_target");
   bool  unlockTargetPressed              = false;
   if (unlockTargetPressedJoystickInput > 0.5)
   {
      unlockTargetPressed = true;
   }
   ProcessButtonStatePressed(mUnlockTargetIsBeingPressed, unlockTargetPressed, [this]() { UnlockTargetPressedHandler(); });

   // Sensor Az Control
   float sensorAzRgtLft = mJoystickManager->GetValue(0, "sensor_az_rgt_lft");
   bool  buttonPressed  = false; // Reused flag

   buttonPressed = false;
   if (sensorAzRgtLft > 0.001)
   {
      buttonPressed = true;
   }
   ProcessButtonStatePressed(mSensorRightIsBeingPressed, buttonPressed, [this]() { SensorAzRgtPressedHandler(); });

   buttonPressed = false;
   if (sensorAzRgtLft < -0.001)
   {
      buttonPressed = true;
   }
   ProcessButtonStatePressed(mSensorLeftIsBeingPressed, buttonPressed, [this]() { SensorAzLftPressedHandler(); });

   // Sensor El Control
   float sensorElUpDn = mJoystickManager->GetValue(0, "sensor_el_up_dn");

   buttonPressed = false;
   if (sensorElUpDn > 0.001)
   {
      buttonPressed = true;
   }
   ProcessButtonStatePressed(mSensorUpIsBeingPressed, buttonPressed, [this]() { SensorElUpPressedHandler(); });

   buttonPressed = false;
   if (sensorElUpDn < -0.001)
   {
      buttonPressed = true;
   }
   ProcessButtonStatePressed(mSensorDownIsBeingPressed, buttonPressed, [this]() { SensorElDnPressedHandler(); });

   // Center the Sensor
   float centerSensorPressedJoystickInput = mJoystickManager->GetValue(0, "center_sensor");
   bool  centerSensorPressed              = false;
   if (centerSensorPressedJoystickInput > 0.5)
   {
      centerSensorPressed = true;
   }
   ProcessButtonStatePressed(mCenterSensorPressed, centerSensorPressed, [this]() { CenterSensorPressedHandler(); });

   // Target Designation Control - This is handled locally (no script support)

   float td_rgt_lft = mJoystickManager->GetValue(0, "td_rgt_lft");
   if (td_rgt_lft > 0.001)
   {
      mTargetDesignationRightIsBeingPressed = true;
   }
   else
   {
      mTargetDesignationRightIsBeingPressed = false;
   }
   if (td_rgt_lft < -0.001)
   {
      mTargetDesignationLeftIsBeingPressed = true;
   }
   else
   {
      mTargetDesignationLeftIsBeingPressed = false;
   }

   float td_up_dn = mJoystickManager->GetValue(0, "td_up_dn");
   if (td_up_dn > 0.001)
   {
      mTargetDesignationUpIsBeingPressed = true;
   }
   else
   {
      mTargetDesignationUpIsBeingPressed = false;
   }
   if (td_up_dn < -0.001)
   {
      mTargetDesignationDownIsBeingPressed = true;
   }
   else
   {
      mTargetDesignationDownIsBeingPressed = false;
   }
}

void WkP6DOF_Controller::PluginObject::ReadOtherInputs()
{
   // Jettison Tanks Button
   float jettTanksPressedJoystickInput = mJoystickManager->GetValue(0, "jett_ext_tanks");
   bool  jettTanksPressed              = false;
   if (jettTanksPressedJoystickInput > 0.5)
   {
      jettTanksPressed = true;
   }
   ProcessButtonStateReleased(mJettisonTanksButtonIsBeingPressed,
                              jettTanksPressed,
                              [this]() { JettisonTanksButtonReleasedHandler(); });

   // Master Warn Silence
   float masterWarnPressedJoystickInput = mJoystickManager->GetValue(0, "master_warn");
   bool  masterWarnPressed              = false;
   if (masterWarnPressedJoystickInput > 0.5)
   {
      masterWarnPressed = true;
   }
   ProcessButtonStateReleased(mMasterWarnButtonIsBeingPressed,
                              masterWarnPressed,
                              [this]() { MasterWarnButtonReleasedHandler(); });
}

void WkP6DOF_Controller::PluginObject::TriggerPulledHandler()
{
   if (!mConnectedPlatform.empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mConnectedPlatform;
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mConnectedPlatform, "TriggerPulled", scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::TriggerReleasedHandler()
{
   if (!mConnectedPlatform.empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mConnectedPlatform;
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mConnectedPlatform, "TriggerReleased", scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::WeaponReleasePressedHandler()
{
   if (!mConnectedPlatform.empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mConnectedPlatform;
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mConnectedPlatform, "WeaponReleaseButtonPushed", scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::WeaponReleaseReleasedHandler()
{
   if (!mConnectedPlatform.empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mConnectedPlatform;
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mConnectedPlatform, "WeaponReleaseButtonReleased", scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::WeaponSelectReleasedHandler()
{
   if (mWeaponSelectToggledExists)
   {
      CallScript("WeaponSelectToggled");
   }
}

void WkP6DOF_Controller::PluginObject::LockTargetPressedHandler()
{
   WkP6DOF_Controller::HUD* hud = mDockWidget->GetHUD();

   // mScriptSimInterfacePtr->GetConnectedPlatform
   // double TD_X = 0.0;
   // double TD_Y = 0.0;
   // hud->RadarTargetDesignationCoords(TD_X, TD_Y);
   // double range_nm = hud->RadarDisplayRange_nm();
   // WsfTrack* track = mSimInterface->LockRadarTarget(TD_X, TD_Y, range_nm);

   std::string lockedTargetPlatformName;
   if (hud->LockRadarTarget(lockedTargetPlatformName))
   {
      if (mLockTargetExists)
      {
         CallVoidScriptWithStrings("LockTarget", "radar", lockedTargetPlatformName);
      }
   }
}

void WkP6DOF_Controller::PluginObject::UnlockTargetPressedHandler()
{
   // Place a CallVoidScript() or CallScript() function here for desired actions
}

void WkP6DOF_Controller::PluginObject::ChaffButtonPressedHandler()
{
   if (mChaffButtonPressedExists)
   {
      CallVoidScript("ChaffButtonPressed");
   }
}

void WkP6DOF_Controller::PluginObject::ChaffButtonReleasedHandler()
{
   if (mChaffButtonReleasedExists)
   {
      CallVoidScript("ChaffButtonReleased");
   }
}

void WkP6DOF_Controller::PluginObject::FlaresButtonPressedHandler()
{
   if (mFlaresButtonPressedExists)
   {
      CallVoidScript("FlaresButtonPressed");
   }
}

void WkP6DOF_Controller::PluginObject::FlaresButtonReleasedHandler()
{
   if (mFlaresButtonReleasedExists)
   {
      CallVoidScript("FlaresButtonReleased");
   }
}

void WkP6DOF_Controller::PluginObject::CountermeasuresButtonPressedHandler()
{
   if (mCountermeasuresButtonPressedExists)
   {
      CallVoidScript("CountermeasuresButtonPressed");
   }
}

void WkP6DOF_Controller::PluginObject::CountermeasuresButtonReleasedHandler()
{
   if (mCountermeasuresButtonReleasedExists)
   {
      CallVoidScript("CountermeasuresButtonReleased");
   }
}

void WkP6DOF_Controller::PluginObject::MasterArmButtonReleasedHandler()
{
   if (mMasterArmToggleExists)
   {
      CallScript("MasterArmToggle");
   }
}

void WkP6DOF_Controller::PluginObject::WeaponModeButtonReleasedHandler()
{
   if (mAirGroundModeToggleExists)
   {
      CallScript("AirGroundModeToggle"); // This can be A/G or A/G
   }
}

void WkP6DOF_Controller::PluginObject::EcmPowerButtonReleasedHandler()
{
   if (mEcmPowerToggleExists)
   {
      CallScript("EcmPowerToggle");
   }
}

void WkP6DOF_Controller::PluginObject::EcmAutoButtonReleasedHandler()
{
   if (mEcmAutoToggleExists)
   {
      CallScript("EcmAutoToggle");
   }
}

void WkP6DOF_Controller::PluginObject::RadarPowerToggleReleasedHandler()
{
   if (mRadarPowerToggleExists)
   {
      CallVoidScript("RadarPowerToggle");
   }
}

void WkP6DOF_Controller::PluginObject::RadarModeCycleReleasedHandler()
{
   if (mRadarModeCycleExists)
   {
      CallVoidScript("RadarModeCycle");
   }
}

void WkP6DOF_Controller::PluginObject::RadarSubmodeCycleReleasedHandler()
{
   if (mRadarSubmodeCycleExists)
   {
      CallVoidScript("RadarSubmodeCycle");
   }
}

void WkP6DOF_Controller::PluginObject::IrstPowerToggleReleasedHandler()
{
   if (mIrstPowerToggleExists)
   {
      CallVoidScript("IrstPowerToggle");
   }
}

void WkP6DOF_Controller::PluginObject::IrstModeCycleReleasedHandler()
{
   if (mIrstModeCycleExists)
   {
      CallVoidScript("IrstModeCycle");
   }
}

void WkP6DOF_Controller::PluginObject::IrstCueToRadarReleasedHandler()
{
   if (mIrstCueToRadarExists)
   {
      CallVoidScript("IrstCueToRadar");
   }
}

void WkP6DOF_Controller::PluginObject::IrstCueToHeadReleasedHandler()
{
   if (mIrstCueToHeadExists)
   {
      CallVoidScript("IrstCueToHead");
   }
}

void WkP6DOF_Controller::PluginObject::FlirPowerToggleReleasedHandler()
{
   if (mFlirPowerToggleExists)
   {
      CallVoidScript("FlirPowerToggle");
   }
}

void WkP6DOF_Controller::PluginObject::FlirModeCycleReleasedHandler()
{
   if (mFlirModeCycleExists)
   {
      CallVoidScript("FlirModeCycle");
   }
}

void WkP6DOF_Controller::PluginObject::RwrPowerToggleReleasedHandler()
{
   if (mRwrPowerToggleExists)
   {
      CallVoidScript("RwrPowerToggle");
   }
}

void WkP6DOF_Controller::PluginObject::RwrFilterToggleReleasedHandler()
{
   if (mRwrFilterToggleExists)
   {
      CallVoidScript("RwrFilterToggle");
   }
}

void WkP6DOF_Controller::PluginObject::SensorAzLftPressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorAzLft", false);  Needs extra argument for which sensor
}

void WkP6DOF_Controller::PluginObject::SensorAzRgtPressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorAzRgt", false);  Needs extra argument for which sensor
}

void WkP6DOF_Controller::PluginObject::SensorElUpPressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorElUp", false);  Needs extra argument for which sensor
}

void WkP6DOF_Controller::PluginObject::SensorElDnPressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorElDn", false);  Needs extra argument for which sensor
}

void WkP6DOF_Controller::PluginObject::CenterSensorPressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("CenterSensor", false);  Needs extra argument for which sensor
}

void WkP6DOF_Controller::PluginObject::IncreaseSensorRangePressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorRangeIncreased", false);
}

void WkP6DOF_Controller::PluginObject::DecreaseSensorRangePressedHandler()
{
   // Planned improvement: Flesh this out zCallScript("SensorRangeDecreased", false);
}

void WkP6DOF_Controller::PluginObject::HudModeButtonReleasedHandler()
{
   WkP6DOF_Controller::HUD*                hud  = mDockWidget->GetHUD();
   P6DOF_ControllerDataContainer::eHudMode mode = hud->GetHudMode();
   switch (mode)
   {
   case P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT:
      mode = P6DOF_ControllerDataContainer::eHUD_NAV_LAND;
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_LAND:
      mode = P6DOF_ControllerDataContainer::eHUD_NAV_TAXI;
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_TAXI:
      mode = P6DOF_ControllerDataContainer::eHUD_FUEL;
      break;
   case P6DOF_ControllerDataContainer::eHUD_FUEL:
      mode = P6DOF_ControllerDataContainer::eHUD_ENG;
      break;
   case P6DOF_ControllerDataContainer::eHUD_ENG:
      mode = P6DOF_ControllerDataContainer::eHUD_GND;
      break;
   case P6DOF_ControllerDataContainer::eHUD_GND:
      mode = P6DOF_ControllerDataContainer::eHUD_GUN;
      break;
   case P6DOF_ControllerDataContainer::eHUD_GUN:
      mode = P6DOF_ControllerDataContainer::eHUD_MSL;
      break;
   case P6DOF_ControllerDataContainer::eHUD_MSL:
      mode = P6DOF_ControllerDataContainer::eHUD_TEST;
      break;
   case P6DOF_ControllerDataContainer::eHUD_TEST:
      mode = P6DOF_ControllerDataContainer::eHUD_CONTROLS;
      break;
   case P6DOF_ControllerDataContainer::eHUD_CONTROLS:
      mode = P6DOF_ControllerDataContainer::eHUD_WEAPON;
      break;
   case P6DOF_ControllerDataContainer::eHUD_WEAPON:
      mode = P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT;
      break;
   default:
      mode = P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT;
      break;
   }
   hud->SetHudMode(mode);

   // Send out HUD mode via script
   if (mode == P6DOF_ControllerDataContainer::eHUD_MSL)
   {
      if (mMissileHudModeSelectedExists)
      {
         CallVoidScript("MissileHudModeSelected");
      }
   }
   else if (mode == P6DOF_ControllerDataContainer::eHUD_GUN)
   {
      if (mGunHudModeSelectedExists)
      {
         CallVoidScript("GunHudModeSelected");
      }
   }
   else if (mode == P6DOF_ControllerDataContainer::eHUD_GND)
   {
      if (mGroundAttackHudModeSelectedExists)
      {
         CallVoidScript("GroundAttackHudModeSelected");
      }
   }
   else
   {
      if (mNonWeaponHudModeSelectedExists)
      {
         CallVoidScript("NonWeaponHudModeSelected");
      }
   }
}

void WkP6DOF_Controller::PluginObject::MfdLftModeButtonReleasedHandler()
{
   switch (mScriptBasedPlatData.mfdLftMode)
   {
   case P6DOF_ControllerDataContainer::eMFD_RADAR:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_RWR;
      break;
   case P6DOF_ControllerDataContainer::eMFD_RWR:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_IRST;
      break;
   case P6DOF_ControllerDataContainer::eMFD_IRST:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_FLIR;
      break;
   case P6DOF_ControllerDataContainer::eMFD_FLIR:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_DATALINK;
      break;
   case P6DOF_ControllerDataContainer::eMFD_DATALINK:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_WEAPONS;
      break;
   case P6DOF_ControllerDataContainer::eMFD_WEAPONS:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_FUEL;
      break;
   case P6DOF_ControllerDataContainer::eMFD_FUEL:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_ENGINES;
      break;
   case P6DOF_ControllerDataContainer::eMFD_ENGINES:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_RADAR;
      break;
   default:
      mScriptBasedPlatData.mfdLftMode = P6DOF_ControllerDataContainer::eMFD_RADAR;
      break;
   }
}

void WkP6DOF_Controller::PluginObject::MfdRgtModeButtonReleasedHandler()
{
   switch (mScriptBasedPlatData.mfdRgtMode)
   {
   case P6DOF_ControllerDataContainer::eMFD_RADAR:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_RWR;
      break;
   case P6DOF_ControllerDataContainer::eMFD_RWR:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_IRST;
      break;
   case P6DOF_ControllerDataContainer::eMFD_IRST:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_FLIR;
      break;
   case P6DOF_ControllerDataContainer::eMFD_FLIR:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_DATALINK;
      break;
   case P6DOF_ControllerDataContainer::eMFD_DATALINK:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_WEAPONS;
      break;
   case P6DOF_ControllerDataContainer::eMFD_WEAPONS:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_FUEL;
      break;
   case P6DOF_ControllerDataContainer::eMFD_FUEL:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_ENGINES;
      break;
   case P6DOF_ControllerDataContainer::eMFD_ENGINES:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_RADAR;
      break;
   default:
      mScriptBasedPlatData.mfdRgtMode = P6DOF_ControllerDataContainer::eMFD_RADAR;
      break;
   }
}

void WkP6DOF_Controller::PluginObject::MfdActiveButtonReleasedHandler()
{
   if (mScriptBasedPlatData.mfdActive == 0)
   {
      mScriptBasedPlatData.mfdActive = 1;
   }
   else if (mScriptBasedPlatData.mfdActive == 1)
   {
      mScriptBasedPlatData.mfdActive = 0;
   }
   else
   {
      std::cout << "ERROR handling active MFD in WkP6DOF_Controller::PluginObject::MfdActiveButtonReleasedHandler()"
                << std::endl;
   }
}

void WkP6DOF_Controller::PluginObject::JettisonTanksButtonReleasedHandler()
{
   if (mJettisonExternalTanksExists)
   {
      CallVoidScript("JettisonExternalTanks");
   }
}

void WkP6DOF_Controller::PluginObject::MasterWarnButtonReleasedHandler()
{
   if (mClearMasterWarningExists)
   {
      CallVoidScript("ClearMasterWarning");
   }
}

bool WkP6DOF_Controller::PluginObject::ScriptExists(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mConnectedPlatform;
   if (!mConnectedPlatform.empty())
   {
      return mScriptSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, aScriptName, scriptInstance.script);
   }

   return mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script);
}

void WkP6DOF_Controller::PluginObject::CallScript(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mConnectedPlatform;

   bool valid = false;
   if (!mConnectedPlatform.empty())
   {
      valid =
         mScriptSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, aScriptName, scriptInstance.script);
   }
   else
   {
      valid = mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script);
   }

   if (valid)
   {
      mScriptsWaitingForReturnValues.insert(mScriptSimInterfacePtr->ExecuteScript(scriptInstance));
   }
}

void WkP6DOF_Controller::PluginObject::CallVoidScript(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mConnectedPlatform;

   bool valid = false;
   if (!mConnectedPlatform.empty())
   {
      valid =
         mScriptSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, aScriptName, scriptInstance.script);
   }
   else
   {
      valid = mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script);
   }

   if (valid)
   {
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::CallVoidScriptWithStrings(const std::string& aScriptName,
                                                                 std::string        aString1,
                                                                 std::string        aString2)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mConnectedPlatform;

   warlock::script::Argument arg1;
   QVariant                  variant1 = QVariant(aString1.c_str());
   arg1.type                          = "string";
   arg1.data                          = variant1;
   scriptInstance.args.push_back(arg1);

   warlock::script::Argument arg2;
   QVariant                  variant2 = QVariant(aString2.c_str());
   arg2.type                          = "string";
   arg2.data                          = variant2;
   scriptInstance.args.push_back(arg2);

   bool valid = false;
   if (!mConnectedPlatform.empty())
   {
      valid =
         mScriptSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, aScriptName, scriptInstance.script);
   }
   else
   {
      valid = mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script);
   }

   if (valid)
   {
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void WkP6DOF_Controller::PluginObject::ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue)
{
   // NOTE:  We place all incoming script data into mScriptBasedPlatData

   // If aScriptId is in the set of scripts I have commanded
   if (mScriptsWaitingForReturnValues.count(aScriptId) > 0)
   {
      mScriptsWaitingForReturnValues.erase(aScriptId);

      if (aReturnValue.platformName == mConnectedPlatform)
      {
         if (aReturnValue.scriptName == "ReturnIntArrayData")
         {
            // QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            // std::cout << "ReturnIntArrayData returned " << myQStringList.size() << " items as follows:" << std::endl;
            // for (int i = 0; i < myQStringList.size(); ++i)
            // {
            //    int val = myQStringList.at(i).toInt();
            //    std::cout << " " << val << " : ";
            // }
            // std::cout << " " << std::endl;

            QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            int         numItems      = myQStringList.size();
            if (numItems != 3)
            {
               std::cout << "Expected 3 items in ReturnIntArrayData but received " << numItems << std::endl;
            }
            else
            {
               mScriptBasedPlatData.radarNumScanBars = myQStringList.at(0).toInt();
               mScriptBasedPlatData.radarCurrentBar  = myQStringList.at(1).toInt();
               mScriptBasedPlatData.radarHighPRF     = myQStringList.at(2).toInt();
            }
         }
         else if (aReturnValue.scriptName == "ReturnDoubleArrayData")
         {
            // QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            // std::cout << "ReturnDoubleArrayData returned " << myQStringList.size() << " items as follows:" <<
            // std::endl; for (int i = 0; i < myQStringList.size(); ++i)
            // {
            //    double val = myQStringList.at(i).toDouble();
            //    std::cout << " " << val << " : ";
            // }
            // std::cout << " " << std::endl;

            QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            int         numItems      = myQStringList.size();
            if (numItems != 4)
            {
               std::cout << "Expected 4 items in ReturnDoubleArrayData but received " << numItems << std::endl;
            }
            else
            {
               mScriptBasedPlatData.radarMinAz_deg = myQStringList.at(0).toDouble();
               mScriptBasedPlatData.radarMaxAz_deg = myQStringList.at(1).toDouble();
               mScriptBasedPlatData.radarMinEl_deg = myQStringList.at(2).toDouble();
               mScriptBasedPlatData.radarMaxEl_deg = myQStringList.at(3).toDouble();

               // double val1 = myQStringList.at(0).toDouble(); // FOV_MinimumAzimuth());
               // double val2 = myQStringList.at(1).toDouble(); // FOV_MaximumAzimuth());
               // double val3 = myQStringList.at(2).toDouble(); // FOV_MinimumElevation());
               // double val4 = myQStringList.at(3).toDouble(); // FOV_MaximumElevation());
               // double val5 = myQStringList.at(4).toDouble(); // CurrentCuedAzimuth());
               // double val6 = myQStringList.at(5).toDouble(); // CurrentCuedElevation());
               // double val7 = myQStringList.at(6).toDouble(); // FOV_MaximumRange());
               //
               // UpdateDataForHud
               // mScriptBasedPlatData.radarMinAz_deg   = 0.0;
               // mScriptBasedPlatData.radarMaxAz_deg   = 0.0;
               // mScriptBasedPlatData.radarMinEl_deg   = 0.0;
               // mScriptBasedPlatData.radarMaxEl_deg   = 0.0;
               // mScriptBasedPlatData.radarCuedAz_deg  = 0.0;
               // mScriptBasedPlatData.radarCuedEl_deg  = 0.0;
               // mScriptBasedPlatData.radarMaxRange_nm = 0.0;
            }
         }
         else if (aReturnValue.scriptName == "ReturnStringArrayData")
         {
            // QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            // std::cout << "ReturnStringArrayData returned " << myQStringList.size() << " items as follows:" <<
            // std::endl; for (int i = 0; i < myQStringList.size(); ++i)
            // {
            //    std::string val = myQStringList.at(i).toStdString();
            //    std::cout << " " << val << " : ";
            // }
            // std::cout << " " << std::endl;

            QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            int         numItems      = myQStringList.size();
            if (numItems != 2)
            {
               std::cout << "Expected 2 items in ReturnStringArrayData but received " << numItems << std::endl;
            }
            else
            {
               // mScriptBasedPlatData.activeWeapon    = myQStringList.at(0).toDouble();
               // mScriptBasedPlatData.numActiveWeapon = myQStringList.at(0).toDouble();
               // gWeaponSelected
               // gWeaponTypeSelected

               mScriptBasedPlatData.activeWeapon = myQStringList.at(0).toStdString();
               std::string weaponTypeSelected    = myQStringList.at(1).toStdString();
            }
         }
         else if (aReturnValue.scriptName == "WeaponSelectToggled")
         {
            mScriptBasedPlatData.activeWeapon = aReturnValue.returnValue.GetString();
         }
         else if (aReturnValue.scriptName == "GetBingoFuelQty")
         {
            double bingoFuel_kg                        = aReturnValue.returnValue.GetDouble();
            mScriptBasedPlatData.bingoFuelQuantity_lbs = bingoFuel_kg * UtMath::cLB_PER_KG;
         }
         else if (aReturnValue.scriptName == "ReturnBitPackedFlags")
         {
            int pack = aReturnValue.returnValue.GetInt();

            if (pack & 0x0001)
            {
               mScriptBasedPlatData.shootFlagActive = true;
            }
            else
            {
               mScriptBasedPlatData.shootFlagActive = false;
            }

            if (pack & 0x0002)
            {
               mScriptBasedPlatData.breakX_Flag = true;
            }
            else
            {
               mScriptBasedPlatData.breakX_Flag = false;
            }

            if (pack & 0x0004)
            {
               mScriptBasedPlatData.ecmXmit = true;
            }
            else
            {
               mScriptBasedPlatData.ecmXmit = false;
            }

            if (pack & 0x0008)
            {
               mScriptBasedPlatData.jammingDetected = true;
            }
            else
            {
               mScriptBasedPlatData.jammingDetected = false;
            }

            if (pack & 0x0010)
            {
               mScriptBasedPlatData.masterArmEnabled = true;
            }
            else
            {
               mScriptBasedPlatData.masterArmEnabled = false;
            }

            if (pack & 0x0020)
            {
               mScriptBasedPlatData.masterWarningEnabled = true;
            }
            else
            {
               mScriptBasedPlatData.masterWarningEnabled = false;
            }
         }
         else if (aReturnValue.scriptName == "NumChaffRemaining")
         {
            mScriptBasedPlatData.numChaff = aReturnValue.returnValue.GetInt();
         }
         else if (aReturnValue.scriptName == "NumFlaresRemaining")
         {
            mScriptBasedPlatData.numFlares = aReturnValue.returnValue.GetInt();
         }
         else if (aReturnValue.scriptName == "ToggleECM")
         {
            mScriptBasedPlatData.ecmXmit = aReturnValue.returnValue.GetBool();
            if (mScriptBasedPlatData.ecmXmit)
            {
               std::cout << "ECM is emitting!" << std::endl;
            }
            else
            {
               std::cout << "ECM is silent" << std::endl;
            }
         }
         else if (aReturnValue.scriptName == "ToggleMasterArm")
         {
            mScriptBasedPlatData.masterArmEnabled = aReturnValue.returnValue.GetBool();
            if (mScriptBasedPlatData.masterArmEnabled)
            {
               std::cout << "Master Arm is ***ARMED***" << std::endl;
            }
            else
            {
               std::cout << "Master Arm is ---SAFE---" << std::endl;
            }
         }
         else
         {
            std::cout << "Received data from " << aReturnValue.platformName << " for a script: " << aReturnValue.scriptName
                      << "() but there is no handler for the data" << std::endl;

            QStringList myQStringList = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
            std::cout << "   DATA returned: " << myQStringList.size() << " items as follows:" << std::endl;
            for (int i = 0; i < myQStringList.size(); ++i)
            {
               std::string val = myQStringList.at(i).toStdString();
               std::cout << " " << val << " : ";
            }
            std::cout << " " << std::endl;
         }
      }
   }
}

void WkP6DOF_Controller::PluginObject::SetAudioTones(double aCurrentTime_sec)
{
   if (mTriggerIsBeingPulled)
   {
      if (mAudioManager != nullptr)
      {
         mAudioManager->PlaySound(1, aCurrentTime_sec);
      }
   }
   else if (mWeaponReleaseIsBeingPressed)
   {
      if (mAudioManager != nullptr)
      {
         mAudioManager->PlaySound(2, aCurrentTime_sec);
      }
   }
   else
   {
      if (mAudioManager != nullptr)
      {
         mAudioManager->PauseSound();
      }
   }
}

void WkP6DOF_Controller::PluginObject::AdjustViewDirection(float aPinkiePaddleSwitch,
                                                           float aAzAccumInput,
                                                           float aElAccumInput,
                                                           float aViewAzInput,
                                                           float aViewElInput)
{
   bool centerView = false;
   if (aPinkiePaddleSwitch > 0.5)
   {
      centerView = true;
   }

   if (mDockWidget)
   {
      double az;
      double el;
      double dist;
      mDockWidget->GetAzElDist(az, el, dist);
      mAccumAz += aAzAccumInput;
      mAccumEl += aElAccumInput;
      if (!mFirstPerson->isChecked())
      {
         az += 6.0 * (aViewAzInput + aAzAccumInput);
         el += 6.0 * (aViewElInput + aElAccumInput);
      }
      else
      {
         az = mAccumAz + aViewAzInput * 150.0;
         el = mAccumEl + aViewElInput * 75.0;
      }
      mDockWidget->SetAzElDist(az, el, dist);

      if (centerView)
      {
         mAccumAz = 0.0f;
         mAccumEl = 0.0f;
      }
      // Override settings if not zoomed and centerView is true
      if (centerView && !mFirstPerson->isChecked())
      {
         mDockWidget->SetAzElDist(0.0, 0.0, dist);
      }
   }
}

void WkP6DOF_Controller::PluginObject::PrintListOfControllers()
{
   static std::list<std::string> controllerList;
   static bool                   initialized = false;
   static int                    num         = 0;
   if (!initialized)
   {
      mJoystickManager->GetDevices(controllerList);
      initialized = true;
      for (auto& str : controllerList)
      {
         std::cout << "Device " << num << " " << str << std::endl;
         ++num;
      }
   }
}
