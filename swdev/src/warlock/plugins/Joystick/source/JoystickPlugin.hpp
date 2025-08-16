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
#ifndef WKHUDPLUGIN_HPP
#define WKHUDPLUGIN_HPP

#include "JoystickDataContainer.hpp"
#include "JoystickSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkScriptSimInterface.hpp"

namespace sdl
{
class AudioManager;
class JoystickManager;
class JoystickManagerSpy;
} // namespace sdl

namespace Joystick
{
/** The Joystick plugin is a Warlock plugin that provides the
 * ability to manually control (fly) P6DOF movers using joysticks/controllers.
 */
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

   /** Build the context menu option for allowing joystick control.
    * @param aMenu pointer to the menu.
    * @param aEntityPtr pointer to the entity.
    */
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   /** Connect to a platform, enabling joystick control.
    * @param aName name of the platform to connect to.
    */
   void ConnectToPlatform(const QString& aName);

   /** Returns the wkf actions that are supported. */
   QList<wkf::Action*> GetActions() const override { return mActions; }

   /** Update the GUI. */
   void GuiUpdate() override;

private:
   /** Calls a script for changing the HUD mode. */
   void HUDModeVoidScript();

   /** Launch the device monitor, showing a window that monitors the joystick controls.
    * @param aParent parent window to child the monitor window to.
    */
   void LaunchDeviceMonitor(QWidget* aParent);

   /** Read weapon inputs. */
   void ReadWeaponInputs();

   /** Read sensor inputs. */
   void ReadSensorInputs();

   /** Read EW inputs. */
   void ReadEW_Inputs();

   /** Read control and display inputs. */
   void ReadControlsAndDisplaysInputs();

   /** Read other inputs. */
   void ReadOtherInputs();

   /** Handles a trigger pull. */
   void TriggerPulledHandler();

   /** Handles a trigger release. */
   void TriggerReleasedHandler();

   /** Handles a weapon release press. */
   void WeaponReleasePressedHandler();

   /** Handles a weapon release release. */
   void WeaponReleaseReleasedHandler();

   /** Handle a weapon selection release. */
   void WeaponSelectReleasedHandler();

   /** Handle a lock target press. */
   void LockTargetPressedHandler();

   /** Handle a chaff button press. */
   void ChaffButtonPressedHandler();

   /** Handle a chaff button release. */
   void ChaffButtonReleasedHandler();

   /** Handle a flares button press. */
   void FlaresButtonPressedHandler();

   /** Handle a flares button release. */
   void FlaresButtonReleasedHandler();

   /** Handle a counter measures button press. */
   void CountermeasuresButtonPressedHandler();

   /** Handle a counter measures button release. */
   void CountermeasuresButtonReleasedHandler();

   /** Handle a master arm button release. */
   void MasterArmButtonReleasedHandler();

   /** Handle a weapon mode button release. */
   void WeaponModeButtonReleasedHandler();

   /** Handle an ecm power button release. */
   void EcmPowerButtonReleasedHandler();

   /** Handle an ecm auto button release. */
   void EcmAutoButtonReleasedHandler();

   /** Handle a radar power toggle button release. */
   void RadarPowerToggleReleasedHandler();

   /** Handle a radar mode cycle button release. */
   void RadarModeCycleReleasedHandler();

   /** Handle a radar submode cycle button release.*/
   void RadarSubmodeCycleReleasedHandler();

   /** Handle an irst power toggle button release. */
   void IrstPowerToggleReleasedHandler();

   /** Handle an irst mode cycle button release.*/
   void IrstModeCycleReleasedHandler();

   /** Handle an irst cue to radar button release. */
   void IrstCueToRadarReleasedHandler();

   /** Handle an irst cue to head button release. */
   void IrstCueToHeadReleasedHandler();

   /** Handle a flir power toggle button release. */
   void FlirPowerToggleReleasedHandler();

   /** Handle a flir power mode cycle button release. */
   void FlirModeCycleReleasedHandler();

   /** Handle a rwr power toggle button release. */
   void RwrPowerToggleReleasedHandler();

   /** Handle a rwr filter toggle button release. */
   void RwrFilterToggleReleasedHandler();

   //// Planned improvements

   //// Handle an unlock target press.
   // void UnlockTargetPressedHandler();

   //// Handle a sensor azimuth left button press.
   // void SensorAzLftPressedHandler();

   //// Handle a sensor azimuth right button press.
   // void SensorAzRgtPressedHandler();

   //// Handle a sensor elevation up button press.
   // void SensorElUpPressedHandler();

   //// Handle a sensor elevation down button press.
   // void SensorElDnPressedHandler();

   //// Handle a centor sensor button press.
   // void CenterSensorPressedHandler();

   //// Handle an increase sensor range button press.
   // void IncreaseSensorRangePressedHandler();

   //// Handle a decrease sensor range button press.
   // void DecreaseSensorRangePressedHandler();

   /** Handle a hud mode button release. */
   void HudModeButtonReleasedHandler();

   // MFDs are disabled as of now, they will be moved to HDD.
   //// Handle a MFD left mode button release.
   // void MfdLftModeButtonReleasedHandler();

   //// Handle a MFD right mode button release.
   // void MfdRgtModeButtonReleasedHandler();

   //// Handle a MFD active button release.
   // void MfdActiveButtonReleasedHandler();

   /** Handle a jettison tanks button release. */
   void JettisonTanksButtonReleasedHandler();

   /** Handle a master warn button release. */
   void MasterWarnButtonReleasedHandler();

   /** Request data from script calls at a time.
    * @param aCurrentTime_sec current time.
    */
   void RequestDataViaScriptCalls(double aCurrentTime_sec);

   /** Set the audio tones at a time.
    * @param aCurrentTime_sec current time.
    */
   void SetAudioTones(double aCurrentTime_sec);

   // Callback definition
   using CallbackFn = std::function<void()>;

   /** This function sets the "is being pressed" value, based on the aPushed state,
    * and calls the pressed and released functions as appropriate.
    * @param aIsBeingPressed
    * @param aPushed
    * @param aPressedCallback
    * @param aReleasedCallback
    */
   void ProcessButtonState(bool&             aIsBeingPressed,
                           bool              aPushed,
                           const CallbackFn& aPressedCallback,
                           const CallbackFn& aReleasedCallback);

   /** This function sets the "is being pressed" value, based on the aPushed state,
    * and calls the released function as appropriate
    * @param aIsBeingPressed
    * @param aPushed
    * @param aPressedCallback
    */
   void ProcessButtonStatePressed(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aPressedCallback);

   /** This function sets the "is being pressed" value, based on the aPushed state,
    * and calls the released function as appropriate.
    * @param aIsBeingPressed
    * @param aPushed
    * @param aReleasedCallback
    */
   void ProcessButtonStateReleased(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aReleasedCallback);

   /** This is the callback function that is called when scripts
    * return values. As new script functions that have return
    * values are needed, handler code should be added to this
    * function.
    * @param aScriptId id of the script.
    * @param aReturnValue return value.
    */
   void ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue);

   /** This will return true if an AFSIM script function with the specified name exists.
    * @param aScriptName name of the script.
    */
   bool ScriptExists(const std::string& aScriptName);

   /** This will call an AFSIM script function with the specified name.
    * If returnValueExpected is true, the function will be added to the
    * list of scripts awaiting a return value. Use false if no return
    * value is needed.
    * @param aScriptName name of the script.
    */
   void CallScript(const std::string& aScriptName);

   /** This will call an AFSIM script function with the specified name that returns void.
    * @param aScriptName name of the script.
    */
   void CallVoidScript(const std::string& aScriptName);

   /** This will call an AFSIM script function with the specified name and
    * arguments that returns void.
    * @param aScriptName name of the script.
    * @param aString1 string to send.
    * @param aString2 string to send.
    */
   void CallVoidScriptWithStrings(const std::string& aScriptName, std::string aString1, std::string aString2);

   /** Handles a platform being connected. */
   virtual void ConnectToPlatformActionHandler();

   /** Release a platform, disabling joystick control. */
   void ReleasePlatform();

   sdl::AudioManager*    mAudioManagerPtr    = nullptr; ///< Pointer to the audio manager.
   sdl::JoystickManager* mJoystickManagerPtr = nullptr; ///< Pointer to the joystick manager.

   QList<wkf::Action*>                          mActions;               ///< Container for actions.
   std::shared_ptr<warlock::ScriptSimInterface> mScriptSimInterfacePtr; ///< Pointer to the script sim interface.
   JoystickDataContainer mDataContainer; ///< The data container where common data will be accessed/written to.
   std::set<int>         mScriptsWaitingForReturnValues; ///< Set of scripts waiting on return values.

   bool mTriggerIsBeingPulled        = false; ///< True if the trigger is being pulled, false otherwise.
   bool mWeaponReleaseIsBeingPressed = false; ///< True if the weapon release button is being pressed, false otherwise.
   bool mWeaponSelectIsBeingPressed  = false; ///< True if the weapon select button is being pressed, false otherwise.
   bool mLockTargetIsBeingPressed    = false; ///< True if the lock target button is being pressed, false otherwise.
   bool mUnlockTargetIsBeingPressed  = false; ///< True if the unlock target button is being pressed, false otherwise.
   bool mChaffButtonIsBeingPressed   = false; ///< True if the chaff button is being pressed, false otherwise.
   bool mFlareButtonIsBeingPressed   = false; ///< True if the flare button is being pressed, false otherwise.
   bool mCountermeasureButtonIsBeingPressed =
      false; ///< True if the counter measure button is being pressed, false otherwise.
   bool mTargetDesignationRightIsBeingPressed =
      false; ///< True if the target designation right button is being pressed, false otherwise.
   bool mTargetDesignationLeftIsBeingPressed =
      false; ///< True if the target designation left button is being pressed, false otherwise.
   bool mTargetDesignationUpIsBeingPressed =
      false; ///< True if the target designation up button is being pressed, false otherwise.
   bool mTargetDesignationDownIsBeingPressed =
      false; ///< True if the target designation down button is being pressed, false otherwise.
   bool mMasterArmButtonIsBeingPressed  = false; ///< True if the master arm button is being pressed, false otherwise.
   bool mWeaponModeButtonIsBeingPressed = false; ///< True if the weapon mode button is being pressed , false otherwise.
   bool mEcmPowerButtonIsBeingPressed   = false; ///< True if the ecm power button is being pressed, false otherwise.
   bool mEcmAutoButtonIsBeingPressed    = false; ///< True if the ecm auto button is being pressed, false otherwise.
   bool mRadarPowerTogglePressed        = false; ///< True if the rader power toggle button is pressed, false otherwise.
   bool mRadarModeCyclePressed          = false; ///< True if the rader mode cycle button is pressed, false otherwise.
   bool mRadarSubmodeCyclePressed    = false; ///< True if the radar submode cycle button is pressed, false otherwise.
   bool mIrstPowerTogglePressed      = false; ///< True if the irst power toggle button is pressed, false otherwise.
   bool mIrstModeCyclePressed        = false; ///< True if the irst mode cycle button is pressed, false otherwise.
   bool mIrstCueToRadarPressed       = false; ///< True if the irst cue to radar button is pressed, false otherwise.
   bool mIrstCueToHeadPressed        = false; ///< True if the irst cue to head button is pressed, false otherwise.
   bool mFlirPowerTogglePressed      = false; ///< True if the flir power toggle button is pressed, false otherwise.
   bool mFlirModeCyclePressed        = false; ///< True if the flir mode cycle button is pressed, false otherwise.
   bool mRwrPowerTogglePressed       = false; ///< True if the rwr power toggle button is pressed, false otherwise.
   bool mRwrFilterTogglePressed      = false; ///< True if the rwr filter toggle button is pressed, false otherwise.
   bool mHudModeButtonIsBeingPressed = false; ///< True if the hud mode button is pressed, false otherwise.
   bool mJettisonTanksButtonIsBeingPressed = false; ///< True if the jettison tanks button is pressed, false otherwise.
   bool mMasterWarnButtonIsBeingPressed    = false; ///< True if the master warn button is pressed, false otherwise.
   double mLastRequestTime_1sec            = 0.0;   ///< Last request time in increments of 1 sec.
   double mLastRequestTime_50msec          = 0.0;   ///< Last request time in increments of 50 msec.
   bool mReturnIntArrayDataScriptExists = false; ///< True if the return int array data script exists, false otherwise.
   bool mReturnDoubleArrayDataExists = false; ///< True if the return double array data script exists, false otherwise.
   bool mReturnStringArrayDataExists = false; ///< True if the return string array data script exists, false otherwise.
   bool mReturnBitPackedFlagsExists  = false; ///< True if the return bit packed flags script exists, false otherwise.
   bool mNumChaffRemainingExists     = false; ///< True if the num chaff remaining script exists, false otherwise.
   bool mNumFlaresRemainingExists    = false; ///< True if the num flares remaining script exists, false otherwise.
   bool mGetBingoFuelQtyExists       = false; ///< True if the get bingo fuel quantity script exists, false otherwise.
   bool mWeaponSelectToggledExists   = false; ///< True if the weapon select toggled script exists, false otherwise.
   bool mMasterArmToggleExists       = false; ///< True if the master arm toggled script exists, false otherwise.
   bool mAirGroundModeToggleExists   = false; ///< True if the air ground mode toggle script exists, false otherwise.
   bool mEcmPowerToggleExists        = false; ///< True if the ecm power toggle script exists, false otherwise.
   bool mEcmAutoToggleExists         = false; ///< True if the ecm auto toggle script exists, false otherwise.
   bool mChaffButtonPressedExists    = false; ///< True if the chaff button pressed script exists, false otherwise.
   bool mChaffButtonReleasedExists   = false; ///< True if the chaff button released script exists, false otherwise.
   bool mFlaresButtonPressedExists   = false; ///< True if the flares button pressed script exists, false otherwise.
   bool mFlaresButtonReleasedExists  = false; ///< True if the flares button released script exists, false otherwise.
   bool mCountermeasuresButtonPressedExists =
      false; ///< True if the counter measures button pressed exists, false otherwise.
   bool mCountermeasuresButtonReleasedExists =
      false;                                  ///< True if the counter measures button pressed exists, false otherwise.
   bool mRadarPowerToggleExists      = false; ///< True if the radar power toggle script exists, false otherwise.
   bool mRadarModeCycleExists        = false; ///< True if the radar mode cycle script exists, false otherwise.
   bool mRadarSubmodeCycleExists     = false; ///< True if the radar submode cycle script exists, false otherwise.
   bool mIrstPowerToggleExists       = false; ///< True if the irst power toggle script exists, false otherwise.
   bool mIrstModeCycleExists         = false; ///< True if the irst mode cycle script exists, false otherwise.
   bool mIrstCueToRadarExists        = false; ///< True if the irst cue to radar script exists, false otherwise.
   bool mIrstCueToHeadExists         = false; ///< True if the irst cue to head script exists, false otherwise.
   bool mFlirPowerToggleExists       = false; ///< True if the flir power toggle script exists, false otherwise.
   bool mFlirModeCycleExists         = false; ///< True if the flir mode cycle script exists, false otherwise.
   bool mRwrPowerToggleExists        = false; ///< True if the rwr power toggle script exists, false otherwise.
   bool mRwrFilterToggleExists       = false; ///< True if the rwr filter toggle script exists, false otherwise.
   bool mWeaponHudModeSelectedExists = false; ///< True if the missile hud mode selected script exists, false otherwise.
   bool mNavHudModeSelectedExists = false; ///< True if the non weapon hud mode selected script exists, false otherwise.
   bool mNonWeaponHudModeSelectedExists =
      false; ///< True if the non weapon hud mode selected script exists, false otherwise.
   bool mJettisonExternalTanksExists = false; ///< True if the jettison external tanks script exists, false otherwise.
   bool mClearMasterWarningExists    = false; ///< True if the clear master warning script exists, false otherwise.

   // MFDs are disabled as of now, they will be moved to HDD.
   // bool                                         mLockTargetExists = false;                      ///< True if the lock
   // target script exists, false otherwise. bool mMfdLftModeButtonIsBeingPressed = false;        ///< True if the MFD
   // left mode button is pressed, false otherwise. bool mMfdRgtModeButtonIsBeingPressed = false;        ///< True if
   // the MFD right mode button is pressed, false otherwise. bool mMfdActiveButtonIsBeingPressed = false;         ///<
   // True if the MFD active button is pressed, false otherwise.

   // Planned Improvements
   // bool                                         mSensorRightIsBeingPressed = false;             ///< True if the
   // sensor right button is being pressed, false otherwise. bool mSensorLeftIsBeingPressed = false;              ///<
   // True if the sensor left button is being pressed, false otherwise. bool mSensorUpIsBeingPressed = false; ///< True
   // if the sensor up button is being pressed, false otherwise. bool mSensorDownIsBeingPressed = false; ///< True if
   // the sensor down button is being pressed, false otherwise. bool mCenterSensorPressed = false; ///< True if the
   // center sensor button is pressed, false otherwise. bool mIncreaseSensorRangePressed = false;            ///< True
   // if the increase sensor range button is pressed, false otherwise. bool mDecreaseSensorRangePressed = false; ///<
   // True if the decrease sensor range button is pressed, false otherwise.
};
} // namespace Joystick
#endif
