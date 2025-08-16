// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "JoystickPlugin.hpp"

#include "JoystickDataContainer.hpp"
#include "JoystickSimCommands.hpp"
#include "SDL_AudioManager.hpp"
#include "SDL_JoystickManager.hpp"
#include "SDL_Manager.hpp"
#include "WkPermissions.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

namespace
{
constexpr const char* cHUD_VIEW = "HudView";
}

WKF_PLUGIN_DEFINE_SYMBOLS(
   Joystick::Plugin,
   "Joystick",
   "The Joystick plugin allows the user to attach to P6DOF movers and control them with a joystick/keyboard.",
   "warlock",
   false)

Joystick::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId, eSECONDARY_VIEWER)
   , mScriptSimInterfacePtr(simEnv.GetScriptSimInterface())
{
   mAudioManagerPtr    = new sdl::AudioManager(sdl::Manager::Instance());
   mJoystickManagerPtr = new sdl::JoystickManager(sdl::Manager::Instance(), wkfEnv.GetMainWindow());
   mJoystickManagerPtr->UseDefinedDevice(0);

   connect(mScriptSimInterfacePtr.get(),
           &warlock::ScriptSimInterface::ReturnValueReady,
           this,
           &Plugin::ReturnValueReadyCB,
           Qt::QueuedConnection);

   // Helper action to trigger CallVoidScripts from HUD Plugin (HeadsUpDisplay::HUDModeCycle)
   wkf::Action* hudModeVoidScriptAction = new wkf::Action("HUDModeVoidScript", wkfEnv.GetMainWindow());
   connect(hudModeVoidScriptAction, &wkf::Action::triggered, this, &Plugin::HUDModeVoidScript);
   mActions.push_back(hudModeVoidScriptAction);
}

Joystick::Plugin::~Plugin()
{
   // Delete the joystick manager
   if (mJoystickManagerPtr)
   {
      if (mJoystickManagerPtr->HasDevice(0))
      {
         mJoystickManagerPtr->ReleaseDevice(0);
      }
      delete mJoystickManagerPtr;
      mJoystickManagerPtr = nullptr;
   }

   // Delete the audio manager
   if (mAudioManagerPtr)
   {
      delete mAudioManagerPtr;
      mAudioManagerPtr = nullptr;
   }
}

void Joystick::Plugin::ConnectToPlatform(const QString& aName)
{
   // Release the old platform first if it exists
   ReleasePlatform();

   auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(aName);
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Unable to Connect",
                           "User does not have permissions to modify platform " +
                              QString::fromStdString(platformPtr->GetName()) + ".");
   }
   else
   {
      // Create the audio manager
      if (!mAudioManagerPtr)
      {
         mAudioManagerPtr = new sdl::AudioManager(sdl::Manager::Instance());
      }

      // Create the joystick manager
      if (!mJoystickManagerPtr)
      {
         mJoystickManagerPtr = new sdl::JoystickManager(sdl::Manager::Instance(), wkfEnv.GetMainWindow());
         mJoystickManagerPtr->UseDefinedDevice(0);
      }

      // Set the connected platform for the data container
      mDataContainer.SetConnectedPlatform(aName.toStdString());
      mJoystickManagerPtr->GetOpenSDL_DeviceSpy()->installEventFilter(mJoystickManagerPtr);
      LaunchDeviceMonitor(wkfEnv.GetMainWindow());
   }
}

void Joystick::Plugin::ReleasePlatform()
{
   // If we have a connected platform, release it and return to synthetic pilot
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      GetInterface()->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(mDataContainer.GetConnectedPlatform(), ActivatePilotCommand::SYNTHETIC));
      mDataContainer.ReleaseConnectedPlatform();
      mJoystickManagerPtr->GetOpenSDL_DeviceSpy()->removeEventFilter(mJoystickManagerPtr);
      mJoystickManagerPtr->HideSDL_DeviceSpy();
   }
}

void Joystick::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      QString name = aEntityPtr->GetName().c_str();
      if (mDataContainer.IsPlatformP6DOF_Entity(name))
      {
         wkf::Action* connectAction = new wkf::Action(QIcon::fromTheme("controller"), QString("Joystick Control"), aMenu);
         connectAction->setData(name);
         connect(connectAction, &QAction::triggered, this, &Plugin::ConnectToPlatformActionHandler);
      }
   }
}

void Joystick::Plugin::GuiUpdate()
{
   if (mDataContainer.GetSimulationInitialized())
   {
      ReleasePlatform();
      mDataContainer.SetSimulationInitialized(false);
   }

   mInterfacePtr->ProcessEvents(mDataContainer); // Process all the new SimEvents

   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      // Get the time (locally) for use with periodic data
      double currentTime_sec = GetInterface()->GetSimTime();

      if ((mJoystickManagerPtr != nullptr) && (mJoystickManagerPtr->HasDevice(0)))
      {
         mJoystickManagerPtr->Update();

         ControlCommand::ControlData controlData;
         controlData.roll   = mJoystickManagerPtr->GetValue(0, "roll");
         controlData.pitch  = mJoystickManagerPtr->GetValue(0, "pitch");
         controlData.rudder = mJoystickManagerPtr->GetValue(0, "rudder");

         controlData.throttle = mJoystickManagerPtr->GetValue(0, "throttle");
         float afterBurner    = mJoystickManagerPtr->GetValue(0, "afterburner");

         if (afterBurner > 0.01)
         {
            if (controlData.throttle > 0.99)
            {
               controlData.throttle = 1 + afterBurner;
            }
         }

         float spd_brake_extend  = mJoystickManagerPtr->GetValue(0, "spd_brake_extend");
         float spd_brake_retract = mJoystickManagerPtr->GetValue(0, "spd_brake_retract");
         controlData.spdBrake    = 0.0f;
         if (spd_brake_extend > 0.5)
         {
            controlData.spdBrake = 1.0f;
         }
         else if (spd_brake_retract > 0.5)
         {
            controlData.spdBrake = -1.0f;
         }

         float landing_gear_down = mJoystickManagerPtr->GetValue(0, "landing_gear_down");
         controlData.landingGear = 0.0f;
         ;
         if (landing_gear_down > 0.5)
         {
            controlData.landingGear = 1.0f;
         }

         // FUTURE:  Add flaps support in the future:
         // float flaps_down = mJoystickManagerPtr->GetValue(0, "flaps_down");

         float trimInputPitch = mJoystickManagerPtr->GetValue(0, "trim_el");
         if (trimInputPitch > 0.001)
         {
            controlData.trimNoseUp = -1.0;
         }
         else if (trimInputPitch < -0.001)
         {
            controlData.trimNoseUp = 1.0;
         }

         float trimInputRoll = mJoystickManagerPtr->GetValue(0, "trim_az");
         if (trimInputRoll > 0.001)
         {
            controlData.trimRollRight = 1.0;
         }
         else if (trimInputRoll < -0.001)
         {
            controlData.trimRollRight = -1.0;
         }

         // Send the control data to the AFSIM sim via the JoystickSimInterface
         // Be sure that no additional flight controls data additions to controlData
         // are performed below this point or they will not be communicated back to sim.
         GetInterface()->AddSimCommand(ut::make_unique<ControlCommand>(mDataContainer.GetConnectedPlatform(), controlData));

         // Execute joystick manager scripts
         for (const std::string& scriptName : mJoystickManagerPtr->GetScriptQueue())
         {
            warlock::script::Instance scriptInstance;
            scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
            std::cout << "*** Execute script (" << scriptName << ")" << std::endl;
            mScriptSimInterfacePtr->GetPlatformScriptData(mDataContainer.GetConnectedPlatform(),
                                                          scriptName,
                                                          scriptInstance.script);
            mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
         }
         mJoystickManagerPtr->ClearScriptQueue();

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
         mWeaponHudModeSelectedExists         = ScriptExists("WeaponHudModeSelected");
         mNavHudModeSelectedExists            = ScriptExists("NavHudModeSelected");
         mNonWeaponHudModeSelectedExists      = ScriptExists("NonWeaponHudModeSelected");
         mJettisonExternalTanksExists         = ScriptExists("JettisonExternalTanks");
         mClearMasterWarningExists            = ScriptExists("ClearMasterWarning");
         // mLockTargetExists = ScriptExists("LockTarget");

         ReadWeaponInputs();
         ReadSensorInputs();
         ReadEW_Inputs();
         ReadControlsAndDisplaysInputs();
         ReadOtherInputs();

         // This requests various data via script at periodic rates
         RequestDataViaScriptCalls(currentTime_sec);

         // Adjusting where to "look" (deprecated)
         // float pinkie_paddle = mJoystickManagerPtr->GetValue(0, "pinkie_paddle");
         // float azAccumInput = mJoystickManagerPtr->GetValue(0, "view_az_accum") * 10.0;
         // float elAccumInput = mJoystickManagerPtr->GetValue(0, "view_el_accum") * 10.0;
         // float viewAzInput = mJoystickManagerPtr->GetValue(0, "view_az");
         // float viewElInput = mJoystickManagerPtr->GetValue(0, "view_el");
         // AdjustViewDirection(pinkie_paddle, azAccumInput, elAccumInput, viewAzInput, viewElInput);
      }

      // Set various audio tones/warnings based on various controls and data
      SetAudioTones(currentTime_sec);
   }
}

void Joystick::Plugin::ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue)
{
   // NOTE:  We place all incoming script data into a temp joystick data container.
   JoystickDataContainer::JoystickData joystickData;


   // If aScriptId is in the set of scripts I have commanded
   if (mScriptsWaitingForReturnValues.count(aScriptId) > 0)
   {
      mScriptsWaitingForReturnValues.erase(aScriptId);

      if (aReturnValue.platformName == mDataContainer.GetConnectedPlatform())
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
               joystickData.radarNumScanBars = myQStringList.at(0).toInt();
               joystickData.radarCurrentBar  = myQStringList.at(1).toInt();
               joystickData.radarHighPRF     = myQStringList.at(2).toInt();
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
               joystickData.radarMinAz_deg = myQStringList.at(0).toDouble();
               joystickData.radarMaxAz_deg = myQStringList.at(1).toDouble();
               joystickData.radarMinEl_deg = myQStringList.at(2).toDouble();
               joystickData.radarMaxEl_deg = myQStringList.at(3).toDouble();

               // double val1 = myQStringList.at(0).toDouble(); // FOV_MinimumAzimuth());
               // double val2 = myQStringList.at(1).toDouble(); // FOV_MaximumAzimuth());
               // double val3 = myQStringList.at(2).toDouble(); // FOV_MinimumElevation());
               // double val4 = myQStringList.at(3).toDouble(); // FOV_MaximumElevation());
               // double val5 = myQStringList.at(4).toDouble(); // CurrentCuedAzimuth());
               // double val6 = myQStringList.at(5).toDouble(); // CurrentCuedElevation());
               // double val7 = myQStringList.at(6).toDouble(); // FOV_MaximumRange());
               //
               // UpdateDataForHud
               // joystickData.radarMinAz_deg   = 0.0;
               // joystickData.radarMaxAz_deg   = 0.0;
               // joystickData.radarMinEl_deg   = 0.0;
               // joystickData.radarMaxEl_deg   = 0.0;
               // joystickData.radarCuedAz_deg  = 0.0;
               // joystickData.radarCuedEl_deg  = 0.0;
               // joystickData.radarMaxRange_nm = 0.0;
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
               // joystickData.activeWeapon    = myQStringList.at(0).toDouble();
               // joystickData.numActiveWeapon = myQStringList.at(0).toDouble();
               // gWeaponSelected
               // gWeaponTypeSelected

               joystickData.activeWeapon      = myQStringList.at(0).toStdString();
               std::string weaponTypeSelected = myQStringList.at(1).toStdString();
            }
         }
         else if (aReturnValue.scriptName == "WeaponSelectToggled")
         {
            joystickData.activeWeapon = aReturnValue.returnValue.GetString();
         }
         else if (aReturnValue.scriptName == "GetBingoFuelQty")
         {
            double bingoFuel_kg                = aReturnValue.returnValue.GetDouble();
            joystickData.bingoFuelQuantity_lbs = bingoFuel_kg * UtMath::cLB_PER_KG;
         }
         else if (aReturnValue.scriptName == "ReturnBitPackedFlags")
         {
            int pack = aReturnValue.returnValue.GetInt();

            if (pack & 0x0001)
            {
               joystickData.shootFlagActive = true;
            }
            else
            {
               joystickData.shootFlagActive = false;
            }

            if (pack & 0x0002)
            {
               joystickData.breakX_Flag = true;
            }
            else
            {
               joystickData.breakX_Flag = false;
            }

            if (pack & 0x0004)
            {
               joystickData.ecmXmit = true;
            }
            else
            {
               joystickData.ecmXmit = false;
            }

            if (pack & 0x0008)
            {
               joystickData.jammingDetected = true;
            }
            else
            {
               joystickData.jammingDetected = false;
            }

            if (pack & 0x0010)
            {
               joystickData.masterArmEnabled = true;
            }
            else
            {
               joystickData.masterArmEnabled = false;
            }

            if (pack & 0x0020)
            {
               joystickData.masterWarningEnabled = true;
            }
            else
            {
               joystickData.masterWarningEnabled = false;
            }
         }
         else if (aReturnValue.scriptName == "NumChaffRemaining")
         {
            joystickData.numChaff = aReturnValue.returnValue.GetInt();
         }
         else if (aReturnValue.scriptName == "NumFlaresRemaining")
         {
            joystickData.numFlares = aReturnValue.returnValue.GetInt();
         }
         else if (aReturnValue.scriptName == "ToggleECM")
         {
            joystickData.ecmXmit = aReturnValue.returnValue.GetBool();
            if (joystickData.ecmXmit)
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
            joystickData.masterArmEnabled = aReturnValue.returnValue.GetBool();
            if (joystickData.masterArmEnabled)
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

bool Joystick::Plugin::ScriptExists(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      return mScriptSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, aScriptName, scriptInstance.script);
   }

   return mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script);
}

void Joystick::Plugin::CallScript(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mDataContainer.GetConnectedPlatform();

   bool valid = false;
   if (!mDataContainer.GetConnectedPlatform().empty())
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

void Joystick::Plugin::CallVoidScript(const std::string& aScriptName)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mDataContainer.GetConnectedPlatform();

   bool valid = false;
   if (!mDataContainer.GetConnectedPlatform().empty())
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

void Joystick::Plugin::CallVoidScriptWithStrings(const std::string& aScriptName, std::string aString1, std::string aString2)
{
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mDataContainer.GetConnectedPlatform();

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
   if (!mDataContainer.GetConnectedPlatform().empty())
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

void Joystick::Plugin::ProcessButtonState(bool&             aIsBeingPressed,
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

void Joystick::Plugin::ProcessButtonStatePressed(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aPressedCallback)
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

void Joystick::Plugin::ProcessButtonStateReleased(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aReleasedCallback)
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

void Joystick::Plugin::SetAudioTones(double aCurrentTime_sec)
{
   if (mTriggerIsBeingPulled)
   {
      if (mAudioManagerPtr != nullptr)
      {
         mAudioManagerPtr->PlaySound(1, aCurrentTime_sec);
      }
   }
   else if (mWeaponReleaseIsBeingPressed)
   {
      if (mAudioManagerPtr != nullptr)
      {
         mAudioManagerPtr->PlaySound(2, aCurrentTime_sec);
      }
   }
   else
   {
      if (mAudioManagerPtr != nullptr)
      {
         mAudioManagerPtr->PauseSound();
      }
   }
}

void Joystick::Plugin::HUDModeVoidScript()
{
   QAction*                        sender = dynamic_cast<QAction*>(QObject::sender());
   JoystickDataContainer::eHudMode mode   = static_cast<JoystickDataContainer::eHudMode>(sender->data().toInt());

   // Send a SimCommand for logging the hud data to the event pipe
   HUD_DataCommand::HUD_Data hudData;
   hudData.hudMode = mode;
   GetInterface()->AddSimCommand(ut::make_unique<HUD_DataCommand>(mDataContainer.GetConnectedPlatform(), hudData));

   // Send out HUD mode via script
   if (mode == JoystickDataContainer::eHUD_WPN_MODE)
   {
      if (mWeaponHudModeSelectedExists)
      {
         CallVoidScript("WeaponHudModeSelected");
      }
   }
   else if (mode == JoystickDataContainer::eHUD_NAV_MODE)
   {
      if (mWeaponHudModeSelectedExists)
      {
         CallVoidScript("NavHudModeSelected");
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

void Joystick::Plugin::LaunchDeviceMonitor(QWidget* aParent)
{
   if (mJoystickManagerPtr != nullptr)
   {
      mJoystickManagerPtr->OpenSDL_DeviceSpy(aParent);
      connect(mJoystickManagerPtr->GetOpenSDL_DeviceSpy(),
              &sdl::JoystickManagerSpy::WindowIsClosing,
              this,
              &Plugin::ReleasePlatform);
   }
}

void Joystick::Plugin::ReadWeaponInputs()
{
   // Trigger
   float triggerPulledJoystickInput = mJoystickManagerPtr->GetValue(0, "trigger_fire");
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
   float weaponReleasePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "weapon_release");
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
   float weaponSelectPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "weapon_select");
   bool  weaponSelectPressed              = false;
   if (weaponSelectPressedJoystickInput > 0.5)
   {
      weaponSelectPressed = true;
   }
   ProcessButtonStateReleased(mWeaponSelectIsBeingPressed,
                              weaponSelectPressed,
                              [this]() { WeaponSelectReleasedHandler(); });

   // Master Arm Switch
   float masterArmPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "master_arm");
   bool  masterArmPressed              = false;
   if (masterArmPressedJoystickInput > 0.5)
   {
      masterArmPressed = true;
   }
   ProcessButtonStateReleased(mMasterArmButtonIsBeingPressed,
                              masterArmPressed,
                              [this]() { MasterArmButtonReleasedHandler(); });

   // Weapon Mode (A/A or A/G)
   float weaponModePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "weapon_mode");
   bool  weaponModePressed              = false;
   if (weaponModePressedJoystickInput > 0.5)
   {
      weaponModePressed = true;
   }
   ProcessButtonStateReleased(mWeaponModeButtonIsBeingPressed,
                              weaponModePressed,
                              [this]() { WeaponModeButtonReleasedHandler(); });
}

void Joystick::Plugin::ReadSensorInputs()
{
   // Radar Power
   float radarPowerTogglePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "radar_power");
   bool  radarPowerTogglePressed              = false;
   if (radarPowerTogglePressedJoystickInput > 0.5)
   {
      radarPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mRadarPowerTogglePressed,
                              radarPowerTogglePressed,
                              [this]() { RadarPowerToggleReleasedHandler(); });

   // Radar Mode Cycle
   float radarModeCyclePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "radar_mode_cycle");
   bool  radarModeCyclePressed              = false;
   if (radarModeCyclePressedJoystickInput > 0.5)
   {
      radarModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mRadarModeCyclePressed,
                              radarModeCyclePressed,
                              [this]() { RadarModeCycleReleasedHandler(); });

   // Radar Submode Cycle
   float radarSubmodeCyclePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "radar_submode_cycle");
   bool  radarSubmodeCyclePressed              = false;
   if (radarSubmodeCyclePressedJoystickInput > 0.5)
   {
      radarSubmodeCyclePressed = true;
   }
   ProcessButtonStateReleased(mRadarSubmodeCyclePressed,
                              radarSubmodeCyclePressed,
                              [this]() { RadarSubmodeCycleReleasedHandler(); });

   // IRST Power
   float irstPowerTogglePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "irst_power");
   bool  irstPowerTogglePressed              = false;
   if (irstPowerTogglePressedJoystickInput > 0.5)
   {
      irstPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mIrstPowerTogglePressed,
                              irstPowerTogglePressed,
                              [this]() { IrstPowerToggleReleasedHandler(); });

   // IRST Mode Cycle
   float irstModeCyclePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "irst_mode_cycle");
   bool  irstModeCyclePressed              = false;
   if (irstModeCyclePressedJoystickInput > 0.5)
   {
      irstModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstModeCyclePressed, irstModeCyclePressed, [this]() { IrstModeCycleReleasedHandler(); });

   // IRST Cue to Radar
   float irstCueToRadarPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "irst_cue_to_radar");
   bool  irstCueToRadarCyclePressed         = false;
   if (irstCueToRadarPressedJoystickInput > 0.5)
   {
      irstCueToRadarCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstCueToRadarPressed,
                              irstCueToRadarCyclePressed,
                              [this]() { IrstCueToRadarReleasedHandler(); });

   // IRST Cue to (Pilot) Head
   float irstCueToHeadPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "irst_cue_to_radar");
   bool  irstCueToHeadCyclePressed         = false;
   if (irstCueToHeadPressedJoystickInput > 0.5)
   {
      irstCueToHeadCyclePressed = true;
   }
   ProcessButtonStateReleased(mIrstCueToHeadPressed,
                              irstCueToHeadCyclePressed,
                              [this]() { IrstCueToHeadReleasedHandler(); });

   // FLIR Power
   float flirPowerTogglePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "flir_power");
   bool  flirPowerTogglePressed              = false;
   if (flirPowerTogglePressedJoystickInput > 0.5)
   {
      flirPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mFlirPowerTogglePressed,
                              flirPowerTogglePressed,
                              [this]() { FlirPowerToggleReleasedHandler(); });

   // FLIR Mode Cycle
   float flirModeCyclePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "flir_mode_cycle");
   bool  flirModeCyclePressed              = false;
   if (flirModeCyclePressedJoystickInput > 0.5)
   {
      flirModeCyclePressed = true;
   }
   ProcessButtonStateReleased(mFlirModeCyclePressed, flirModeCyclePressed, [this]() { FlirModeCycleReleasedHandler(); });

   // RWR Power
   float rwrPowerTogglePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "rwr_power");
   bool  rwrPowerTogglePressed              = false;
   if (rwrPowerTogglePressedJoystickInput > 0.5)
   {
      rwrPowerTogglePressed = true;
   }
   ProcessButtonStateReleased(mRwrPowerTogglePressed,
                              rwrPowerTogglePressed,
                              [this]() { RwrPowerToggleReleasedHandler(); });

   // RWR Filter
   float rwrFilterTogglePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "rwr_filter");
   bool  rwrFilterTogglePressed              = false;
   if (rwrFilterTogglePressedJoystickInput > 0.5)
   {
      rwrFilterTogglePressed = true;
   }
   ProcessButtonStateReleased(mRwrFilterTogglePressed,
                              rwrFilterTogglePressed,
                              [this]() { RwrFilterToggleReleasedHandler(); });
}

void Joystick::Plugin::ReadEW_Inputs()
{
   // Chaff Button
   float chaffPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "chaff_release");
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
   float flaresPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "flare_release");
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
   float cmPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "cm_release");
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
   float ecmPowerPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "ecm_power");
   bool  ecmPowerPressed              = false;
   if (ecmPowerPressedJoystickInput > 0.5)
   {
      ecmPowerPressed = true;
   }
   ProcessButtonStateReleased(mEcmPowerButtonIsBeingPressed,
                              ecmPowerPressed,
                              [this]() { EcmPowerButtonReleasedHandler(); });

   // ECM Mode
   float ecmAutoPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "ecm_auto");
   bool  ecmAutoPressed              = false;
   if (ecmAutoPressedJoystickInput > 0.5)
   {
      ecmAutoPressed = true;
   }
   ProcessButtonStateReleased(mEcmAutoButtonIsBeingPressed, ecmAutoPressed, [this]() { EcmAutoButtonReleasedHandler(); });
}

void Joystick::Plugin::ReadControlsAndDisplaysInputs()
{
   // HUD Mode Cycle
   float hudModePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "hud_mode_cycle");
   bool  hudModePressed              = false;
   if (hudModePressedJoystickInput > 0.5)
   {
      hudModePressed = true;
   }
   ProcessButtonStateReleased(mHudModeButtonIsBeingPressed, hudModePressed, [this]() { HudModeButtonReleasedHandler(); });

   // MFDs are disabled as of now, they will be moved to HDD.
   //// MFD Lft Mode Cycle
   // float mfdLftModePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "mfd_mode_lft_cycle");
   // bool mfdLftModePressed = false;
   // if (mfdLftModePressedJoystickInput > 0.5) { mfdLftModePressed = true; }
   // ProcessButtonStateReleased(mMfdLftModeButtonIsBeingPressed, mfdLftModePressed, [this]() {
   // MfdLftModeButtonReleasedHandler(); });

   //// MFD Rgt Mode Cycle
   // float mfdRgtModePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "mfd_mode_rgt_cycle");
   // bool mfdRgtModePressed = false;
   // if (mfdRgtModePressedJoystickInput > 0.5) { mfdRgtModePressed = true; }
   // ProcessButtonStateReleased(mMfdRgtModeButtonIsBeingPressed, mfdRgtModePressed, [this]() {
   // MfdRgtModeButtonReleasedHandler(); });

   //// MFD Active Cycle
   // float mfdActivePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "mfd_active_cycle");
   // bool mfdActivePressed = false;
   // if (mfdActivePressedJoystickInput > 0.5) { mfdActivePressed = true; }
   // ProcessButtonStateReleased(mMfdActiveButtonIsBeingPressed, mfdActivePressed, [this]() {
   // MfdActiveButtonReleasedHandler(); });

   //// Lock on to target
   // float lockTargetPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "lock_target");
   // bool lockTargetPressed = false;
   // if (lockTargetPressedJoystickInput > 0.5) { lockTargetPressed = true; }
   // ProcessButtonStatePressed(mLockTargetIsBeingPressed, lockTargetPressed, [this]() { LockTargetPressedHandler(); });

   // Sensor Az Control
   // float sensorAzRgtLft = mJoystickManagerPtr->GetValue(0, "sensor_az_rgt_lft");
   // bool buttonPressed = false; // Reused flag

   // Sensor El Control
   // float sensorElUpDn = mJoystickManagerPtr->GetValue(0, "sensor_el_up_dn");

   // Planned Improvements
   //// Increase Sensor Range
   // float increaseSensorRangePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "sensor_rng_incr");
   // bool increaseSensorRangePressed = false;
   // if (increaseSensorRangePressedJoystickInput > 0.5) { increaseSensorRangePressed = true; }
   // ProcessButtonStatePressed(mIncreaseSensorRangePressed, increaseSensorRangePressed, [this]() {
   // IncreaseSensorRangePressedHandler(); });
   //
   //// Decrease Sensor Range
   // float decreaseSensorRangePressedJoystickInput = mJoystickManagerPtr->GetValue(0, "sensor_rng_decr");
   // bool decreaseSensorRangePressed = false;
   // if (decreaseSensorRangePressedJoystickInput > 0.5) { decreaseSensorRangePressed = true; }
   // ProcessButtonStatePressed(mDecreaseSensorRangePressed, decreaseSensorRangePressed, [this]() {
   // DecreaseSensorRangePressedHandler(); });
   //
   // buttonPressed = false;
   // if (sensorAzRgtLft > 0.001) { buttonPressed = true; }
   // ProcessButtonStatePressed(mSensorRightIsBeingPressed, buttonPressed, [this]() { SensorAzRgtPressedHandler(); });
   //
   // buttonPressed = false;
   // if (sensorAzRgtLft < -0.001) { buttonPressed = true; }
   // ProcessButtonStatePressed(mSensorLeftIsBeingPressed, buttonPressed, [this]() { SensorAzLftPressedHandler(); });
   //
   // buttonPressed = false;
   // if (sensorElUpDn > 0.001) { buttonPressed = true; }
   // ProcessButtonStatePressed(mSensorUpIsBeingPressed, buttonPressed, [this]() { SensorElUpPressedHandler(); });
   //
   // buttonPressed = false;
   // if (sensorElUpDn < -0.001) { buttonPressed = true; }
   // ProcessButtonStatePressed(mSensorDownIsBeingPressed, buttonPressed, [this]() { SensorElDnPressedHandler(); });
   //
   //// Center the Sensor
   // float centerSensorPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "center_sensor");
   // bool centerSensorPressed = false;
   // if (centerSensorPressedJoystickInput > 0.5) { centerSensorPressed = true; }
   // ProcessButtonStatePressed(mCenterSensorPressed, centerSensorPressed, [this]() { CenterSensorPressedHandler(); });
   //
   //// Unlock the target
   // float unlockTargetPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "unlock_target");
   // bool unlockTargetPressed = false;
   // if (unlockTargetPressedJoystickInput > 0.5) { unlockTargetPressed = true; }
   // ProcessButtonStatePressed(mUnlockTargetIsBeingPressed, unlockTargetPressed, [this]() { UnlockTargetPressedHandler(); });

   // Target Designation Control - This is handled locally (no script support)

   float td_rgt_lft = mJoystickManagerPtr->GetValue(0, "td_rgt_lft");
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

   float td_up_dn = mJoystickManagerPtr->GetValue(0, "td_up_dn");
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

void Joystick::Plugin::ReadOtherInputs()
{
   // Jettison Tanks Button
   float jettTanksPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "jett_ext_tanks");
   bool  jettTanksPressed              = false;
   if (jettTanksPressedJoystickInput > 0.5)
   {
      jettTanksPressed = true;
   }
   ProcessButtonStateReleased(mJettisonTanksButtonIsBeingPressed,
                              jettTanksPressed,
                              [this]() { JettisonTanksButtonReleasedHandler(); });

   // Master Warn Silence
   float masterWarnPressedJoystickInput = mJoystickManagerPtr->GetValue(0, "master_warn");
   bool  masterWarnPressed              = false;
   if (masterWarnPressedJoystickInput > 0.5)
   {
      masterWarnPressed = true;
   }
   ProcessButtonStateReleased(mMasterWarnButtonIsBeingPressed,
                              masterWarnPressed,
                              [this]() { MasterWarnButtonReleasedHandler(); });
}

void Joystick::Plugin::TriggerPulledHandler()
{
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mDataContainer.GetConnectedPlatform(),
                                                    "TriggerPulled",
                                                    scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void Joystick::Plugin::TriggerReleasedHandler()
{
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mDataContainer.GetConnectedPlatform(),
                                                    "TriggerReleased",
                                                    scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void Joystick::Plugin::WeaponReleasePressedHandler()
{
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mDataContainer.GetConnectedPlatform(),
                                                    "WeaponReleaseButtonPushed",
                                                    scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void Joystick::Plugin::WeaponReleaseReleasedHandler()
{
   if (!mDataContainer.GetConnectedPlatform().empty())
   {
      warlock::script::Instance scriptInstance;
      // scriptInstance.args;
      scriptInstance.platformName = mDataContainer.GetConnectedPlatform();
      // scriptInstance.returnValue;
      mScriptSimInterfacePtr->GetPlatformScriptData(mDataContainer.GetConnectedPlatform(),
                                                    "WeaponReleaseButtonReleased",
                                                    scriptInstance.script);
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
}

void Joystick::Plugin::WeaponSelectReleasedHandler()
{
   if (mWeaponSelectToggledExists)
   {
      CallScript("WeaponSelectToggled");
   }
}

// MFDs are disabled as of now, they will be moved to HDD.
// This will need hooked up to a wkf::Action to send the command to HDD plugin.
// void Joystick::Plugin::LockTargetPressedHandler()
//{
//   //wkf::HUD* hud = mHuds[QString::fromStdString(mDataContainer.GetConnectedPlatform())]->GetHUD();
//   //wkf::HUD* hud = mDockWidget->GetHUD();
//
//   // mScriptSimInterfacePtr->GetConnectedPlatform
//   // double TD_X = 0.0;
//   // double TD_Y = 0.0;
//   // hud->RadarTargetDesignationCoords(TD_X, TD_Y);
//   // double range_nm = hud->RadarDisplayRange_nm();
//   // WsfTrack* track = mSimInterface->LockRadarTarget(TD_X, TD_Y, range_nm);
//
//   std::string lockedTargetPlatformName;
//   if (hud->LockRadarTarget(lockedTargetPlatformName))
//   {
//      if (mLockTargetExists)
//      {
//         CallVoidScriptWithStrings("LockTarget", "radar", lockedTargetPlatformName);
//      }
//   }
//}


void Joystick::Plugin::ChaffButtonPressedHandler()
{
   if (mChaffButtonPressedExists)
   {
      CallVoidScript("ChaffButtonPressed");
   }
}

void Joystick::Plugin::ChaffButtonReleasedHandler()
{
   if (mChaffButtonReleasedExists)
   {
      CallVoidScript("ChaffButtonReleased");
   }
}

void Joystick::Plugin::FlaresButtonPressedHandler()
{
   if (mFlaresButtonPressedExists)
   {
      CallVoidScript("FlaresButtonPressed");
   }
}

void Joystick::Plugin::FlaresButtonReleasedHandler()
{
   if (mFlaresButtonReleasedExists)
   {
      CallVoidScript("FlaresButtonReleased");
   }
}

void Joystick::Plugin::CountermeasuresButtonPressedHandler()
{
   if (mCountermeasuresButtonPressedExists)
   {
      CallVoidScript("CountermeasuresButtonPressed");
   }
}

void Joystick::Plugin::CountermeasuresButtonReleasedHandler()
{
   if (mCountermeasuresButtonReleasedExists)
   {
      CallVoidScript("CountermeasuresButtonReleased");
   }
}

void Joystick::Plugin::MasterArmButtonReleasedHandler()
{
   if (mMasterArmToggleExists)
   {
      CallScript("MasterArmToggle");
   }
}

void Joystick::Plugin::WeaponModeButtonReleasedHandler()
{
   if (mAirGroundModeToggleExists)
   {
      CallScript("AirGroundModeToggle"); // This can be A/G or A/G
   }
}

void Joystick::Plugin::EcmPowerButtonReleasedHandler()
{
   if (mEcmPowerToggleExists)
   {
      CallScript("EcmPowerToggle");
   }
}

void Joystick::Plugin::EcmAutoButtonReleasedHandler()
{
   if (mEcmAutoToggleExists)
   {
      CallScript("EcmAutoToggle");
   }
}

void Joystick::Plugin::RadarPowerToggleReleasedHandler()
{
   if (mRadarPowerToggleExists)
   {
      CallVoidScript("RadarPowerToggle");
   }
}

void Joystick::Plugin::RadarModeCycleReleasedHandler()
{
   if (mRadarModeCycleExists)
   {
      CallVoidScript("RadarModeCycle");
   }
}

void Joystick::Plugin::RadarSubmodeCycleReleasedHandler()
{
   if (mRadarSubmodeCycleExists)
   {
      CallVoidScript("RadarSubmodeCycle");
   }
}

void Joystick::Plugin::IrstPowerToggleReleasedHandler()
{
   if (mIrstPowerToggleExists)
   {
      CallVoidScript("IrstPowerToggle");
   }
}

void Joystick::Plugin::IrstModeCycleReleasedHandler()
{
   if (mIrstModeCycleExists)
   {
      CallVoidScript("IrstModeCycle");
   }
}

void Joystick::Plugin::IrstCueToRadarReleasedHandler()
{
   if (mIrstCueToRadarExists)
   {
      CallVoidScript("IrstCueToRadar");
   }
}

void Joystick::Plugin::IrstCueToHeadReleasedHandler()
{
   if (mIrstCueToHeadExists)
   {
      CallVoidScript("IrstCueToHead");
   }
}

void Joystick::Plugin::FlirPowerToggleReleasedHandler()
{
   if (mFlirPowerToggleExists)
   {
      CallVoidScript("FlirPowerToggle");
   }
}

void Joystick::Plugin::FlirModeCycleReleasedHandler()
{
   if (mFlirModeCycleExists)
   {
      CallVoidScript("FlirModeCycle");
   }
}

void Joystick::Plugin::RwrPowerToggleReleasedHandler()
{
   if (mRwrPowerToggleExists)
   {
      CallVoidScript("RwrPowerToggle");
   }
}

void Joystick::Plugin::RwrFilterToggleReleasedHandler()
{
   if (mRwrFilterToggleExists)
   {
      CallVoidScript("RwrFilterToggle");
   }
}

// Planned Improvements
// void Joystick::Plugin::SensorAzLftPressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorAzLft", false);  Needs extra argument for which sensor
//}
//
// void Joystick::Plugin::SensorAzRgtPressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorAzRgt", false);  Needs extra argument for which sensor
//}
//
// void Joystick::Plugin::SensorElUpPressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorElUp", false);  Needs extra argument for which sensor
//}
//
// void Joystick::Plugin::SensorElDnPressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorElDn", false);  Needs extra argument for which sensor
//}
//
// void Joystick::Plugin::CenterSensorPressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("CenterSensor", false);  Needs extra argument for which sensor
//}
//
// void Joystick::Plugin::IncreaseSensorRangePressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorRangeIncreased", false);
//}
//
// void Joystick::Plugin::DecreaseSensorRangePressedHandler()
//{
//   // Planned improvement: Flesh this out zCallScript("SensorRangeDecreased", false);
//}
//
// void Joystick::Plugin::UnlockTargetPressedHandler()
//{
//   // Place a CallVoidScript() or CallScript() function here for desired actions
//}

void Joystick::Plugin::HudModeButtonReleasedHandler()
{
   wkf::Action* hudAction =
      wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->GetActionByUniqueName("HeadsUpDisplay.hud_mode_cycle");
   if (hudAction != nullptr) // If the other plugin isn’t loaded, nothing happens
   {
      // Set the connected platform name before triggering the action.
      hudAction->setData(QVariant(QString::fromStdString(mDataContainer.GetConnectedPlatform())));
      hudAction->trigger();
   }
}
// MFDs are disabled as of now, they will be moved to HDD.
// void Joystick::Plugin::MfdLftModeButtonReleasedHandler()
//{
//   JoystickDataContainer::JoystickData joystickData = mDataContainer.GetJoystickData();
//
//   switch (joystickData.mfdLftMode)
//   {
//   case JoystickDataContainer::eMFD_RADAR:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_RWR;
//      break;
//   case JoystickDataContainer::eMFD_RWR:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_IRST;
//      break;
//   case JoystickDataContainer::eMFD_IRST:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_FLIR;
//      break;
//   case JoystickDataContainer::eMFD_FLIR:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_DATALINK;
//      break;
//   case JoystickDataContainer::eMFD_DATALINK:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_WEAPONS;
//      break;
//   case JoystickDataContainer::eMFD_WEAPONS:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_FUEL;
//      break;
//   case JoystickDataContainer::eMFD_FUEL:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_ENGINES;
//      break;
//   case JoystickDataContainer::eMFD_ENGINES:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_RADAR;
//      break;
//   default:
//      joystickData.mfdLftMode = JoystickDataContainer::eMFD_RADAR;
//      break;
//   }
//   mDataContainer.SetJoystickData(joystickData);
//}
//
// void Joystick::Plugin::MfdRgtModeButtonReleasedHandler()
//{
//   JoystickDataContainer::JoystickData joystickData = mDataContainer.GetJoystickData();
//
//   switch (joystickData.mfdRgtMode)
//   {
//   case JoystickDataContainer::eMFD_RADAR:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_RWR;
//      break;
//   case JoystickDataContainer::eMFD_RWR:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_IRST;
//      break;
//   case JoystickDataContainer::eMFD_IRST:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_FLIR;
//      break;
//   case JoystickDataContainer::eMFD_FLIR:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_DATALINK;
//      break;
//   case JoystickDataContainer::eMFD_DATALINK:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_WEAPONS;
//      break;
//   case JoystickDataContainer::eMFD_WEAPONS:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_FUEL;
//      break;
//   case JoystickDataContainer::eMFD_FUEL:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_ENGINES;
//      break;
//   case JoystickDataContainer::eMFD_ENGINES:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_RADAR;
//      break;
//   default:
//      joystickData.mfdRgtMode = JoystickDataContainer::eMFD_RADAR;
//      break;
//   }
//
//   mDataContainer.SetJoystickData(joystickData);
//}
//
// void Joystick::Plugin::MfdActiveButtonReleasedHandler()
//{
//   JoystickDataContainer::JoystickData joystickData = mDataContainer.GetJoystickData();
//
//   if (joystickData.mfdActive == 0)
//   {
//      joystickData.mfdActive = 1;
//      mDataContainer.SetJoystickData(joystickData);
//   }
//   else if (joystickData.mfdActive == 1)
//   {
//      joystickData.mfdActive = 0;
//      mDataContainer.SetJoystickData(joystickData);
//   }
//   else
//   {
//      std::cout << "ERROR handling active MFD in Joystick::Plugin::MfdActiveButtonReleasedHandler()" << std::endl;
//   }
//}

void Joystick::Plugin::JettisonTanksButtonReleasedHandler()
{
   if (mJettisonExternalTanksExists)
   {
      CallVoidScript("JettisonExternalTanks");
   }
}

void Joystick::Plugin::MasterWarnButtonReleasedHandler()
{
   if (mClearMasterWarningExists)
   {
      CallVoidScript("ClearMasterWarning");
   }
}

void Joystick::Plugin::RequestDataViaScriptCalls(double aCurrentTime_sec)
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

void Joystick::Plugin::ConnectToPlatformActionHandler()
{
   QAction* sender       = dynamic_cast<QAction*>(QObject::sender());
   QString  platformName = sender->data().toString();

   if (sender)
   {
      // Connect to the platform and set manual pilot
      ConnectToPlatform(platformName);
      GetInterface()->AddSimCommand(
         ut::make_unique<ActivatePilotCommand>(platformName.toStdString(), ActivatePilotCommand::MANUAL));
   }
}
