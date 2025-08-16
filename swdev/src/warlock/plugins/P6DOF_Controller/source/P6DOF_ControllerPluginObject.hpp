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

#ifndef P6DOF_CONTROLLERPLUGINOBJECT_HPP
#define P6DOF_CONTROLLERPLUGINOBJECT_HPP

#include <QPointer>

namespace wkf
{
class Action;
}

namespace sdl
{
class AudioManager;
class JoystickManager;
} // namespace sdl

#include "P6DOF_ControllerDockWidget.hpp"
#include "P6DOF_ControllerPrefObject.hpp"
#include "WkScriptSimInterface.hpp"

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::PluginObject works with WkP6DOF_Controller::Plugin
// and provides special user input/output, script handling, and contains
// the data container for the plug-in.
class PluginObject : public QObject
{
public:
   explicit PluginObject(SimInterface* aInterface, const PrefObject* aPrefObject);
   ~PluginObject() override;

   // This called to update the plugin object
   void Update();

   // This is called to "connect" to the specified platform, allowing
   // the user to control and monitor the platform
   void ConnectToPlatform(const QString& aName, QWidget* aParent);

   // This is called to disconnect from the specified platform
   void ReleasePlatform();

   // This can be used to show (or hide) the HUD overlay
   void ShowHUD(bool aState);

   // This returns the Wkf actions that are supported
   const QList<wkf::Action*>& GetActions() const { return mActions; }

   // This returns a reference to the data container for the P6DofController
   P6DOF_ControllerDataContainer& GetDataContainer() { return mDataContainer; }

private:
   void DialogDestroyed(QObject*);
   void Reset();
   void RequestDataViaScriptCalls(double aCurrentTime_sec);
   void UpdateDataForHud();
   void BlockPilotActionSignals(bool aState);
   void FirstPerson(bool aState);
   void ActivateAutopilot(bool aState);
   void ActivateManualPilot(bool aState);
   void ActivateSyntheticPilot(bool aState);
   void ActivateGuidance(bool aState);
   void ViewerInitializedHandler();

   // Callback definition
   using CallbackFn = std::function<void()>;

   // This function sets the "is being pressed" value, based on the aPushed state,
   // and calls the pressed and released functions as appropriate
   void ProcessButtonState(bool&             aIsBeingPressed,
                           bool              aPushed,
                           const CallbackFn& aPressedCallback,
                           const CallbackFn& aReleasedCallback);

   // This function sets the "is being pressed" value, based on the aPushed state,
   // and calls the released function as appropriate
   void ProcessButtonStatePressed(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aPressedCallback);

   // This function sets the "is being pressed" value, based on the aPushed state,
   // and calls the released function as appropriate
   void ProcessButtonStateReleased(bool& aIsBeingPressed, bool aPushed, const CallbackFn& aReleasedCallback);

   void ReadWeaponInputs();
   void ReadSensorInputs();
   void ReadEW_Inputs();
   void ReadControlsAndDisplaysInputs();
   void ReadOtherInputs();

   void TriggerPulledHandler();
   void TriggerReleasedHandler();
   void WeaponReleasePressedHandler();
   void WeaponReleaseReleasedHandler();
   void WeaponSelectReleasedHandler();
   void LockTargetPressedHandler();
   void UnlockTargetPressedHandler();
   void ChaffButtonPressedHandler();
   void ChaffButtonReleasedHandler();
   void FlaresButtonPressedHandler();
   void FlaresButtonReleasedHandler();
   void CountermeasuresButtonPressedHandler();
   void CountermeasuresButtonReleasedHandler();
   void MasterArmButtonReleasedHandler();
   void WeaponModeButtonReleasedHandler();
   void EcmPowerButtonReleasedHandler();
   void EcmAutoButtonReleasedHandler();
   void RadarPowerToggleReleasedHandler();
   void RadarModeCycleReleasedHandler();
   void RadarSubmodeCycleReleasedHandler();
   void IrstPowerToggleReleasedHandler();
   void IrstModeCycleReleasedHandler();
   void IrstCueToRadarReleasedHandler();
   void IrstCueToHeadReleasedHandler();
   void FlirPowerToggleReleasedHandler();
   void FlirModeCycleReleasedHandler();
   void RwrPowerToggleReleasedHandler();
   void RwrFilterToggleReleasedHandler();
   void SensorAzLftPressedHandler();
   void SensorAzRgtPressedHandler();
   void SensorElUpPressedHandler();
   void SensorElDnPressedHandler();
   void CenterSensorPressedHandler();
   void IncreaseSensorRangePressedHandler();
   void DecreaseSensorRangePressedHandler();
   void HudModeButtonReleasedHandler();
   void MfdLftModeButtonReleasedHandler();
   void MfdRgtModeButtonReleasedHandler();
   void MfdActiveButtonReleasedHandler();
   void JettisonTanksButtonReleasedHandler();
   void MasterWarnButtonReleasedHandler();

   void SetAudioTones(double aCurrentTime_sec);

   void AdjustViewDirection(float aPinkiePaddleSwitch,
                            float aAzAccumInput,
                            float aElAccumInput,
                            float aViewAzInput,
                            float aViewElInput);

   // This is the callback function that is called when scripts
   // return values. As new script functions that have return
   // values are needed, handler code should be added to this
   // function.
   void ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue);

   // This will return true if an AFSIM script function with the
   // specified name exists
   bool ScriptExists(const std::string& aScriptName);

   // This will call an AFSIM script function with the specified name.
   // If returnValueExpected is true, the function will be added to the
   // list of scripts awaiting a return value. Use false if no return
   // value is needed.
   void CallScript(const std::string& aScriptName);

   // This will call an AFSIM script function with the specified name that
   // returns void
   void CallVoidScript(const std::string& aScriptName);

   // This will call an AFSIM script function with the specified name and
   // arguments that returns void
   void CallVoidScriptWithStrings(const std::string& aScriptName, std::string aString1, std::string aString2);

   // This outputs a list of controllers via cout (used for debugging)
   void PrintListOfControllers();

   P6DOF_ControllerDataContainer                mDataContainer;
   SimInterface*                                mSimInterface = nullptr;
   std::shared_ptr<warlock::ScriptSimInterface> mScriptSimInterfacePtr;
   const PrefObject*                            mPrefObject;

   QPointer<DockWidget>  mDockWidget;
   QList<wkf::Action*>   mActions;
   sdl::AudioManager*    mAudioManager                         = nullptr;
   sdl::JoystickManager* mJoystickManager                      = nullptr;
   wkf::Action*          mShowHud                              = nullptr;
   wkf::Action*          mShowWeapons                          = nullptr;
   wkf::Action*          mShowKnots                            = nullptr;
   wkf::Action*          mFirstPerson                          = nullptr;
   wkf::Action*          mAutopilot                            = nullptr;
   wkf::Action*          mManualPilot                          = nullptr;
   wkf::Action*          mSyntheticPilot                       = nullptr;
   wkf::Action*          mGuidancePilot                        = nullptr;
   bool                  mCrashed                              = false;
   float                 mAccumAz                              = 0.0f;
   float                 mAccumEl                              = 0.0f;
   bool                  mTriggerIsBeingPulled                 = false;
   bool                  mWeaponReleaseIsBeingPressed          = false;
   bool                  mWeaponSelectIsBeingPressed           = false;
   bool                  mLockTargetIsBeingPressed             = false;
   bool                  mUnlockTargetIsBeingPressed           = false;
   bool                  mChaffButtonIsBeingPressed            = false;
   bool                  mFlareButtonIsBeingPressed            = false;
   bool                  mCountermeasureButtonIsBeingPressed   = false;
   bool                  mTargetDesignationRightIsBeingPressed = false;
   bool                  mTargetDesignationLeftIsBeingPressed  = false;
   bool                  mTargetDesignationUpIsBeingPressed    = false;
   bool                  mTargetDesignationDownIsBeingPressed  = false;
   bool                  mSensorRightIsBeingPressed            = false;
   bool                  mSensorLeftIsBeingPressed             = false;
   bool                  mSensorUpIsBeingPressed               = false;
   bool                  mSensorDownIsBeingPressed             = false;
   bool                  mMasterArmButtonIsBeingPressed        = false;
   bool                  mWeaponModeButtonIsBeingPressed       = false;
   bool                  mEcmPowerButtonIsBeingPressed         = false;
   bool                  mEcmAutoButtonIsBeingPressed          = false;
   bool                  mRadarPowerTogglePressed              = false;
   bool                  mRadarModeCyclePressed                = false;
   bool                  mRadarSubmodeCyclePressed             = false;
   bool                  mIrstPowerTogglePressed               = false;
   bool                  mIrstModeCyclePressed                 = false;
   bool                  mIrstCueToRadarPressed                = false;
   bool                  mIrstCueToHeadPressed                 = false;
   bool                  mFlirPowerTogglePressed               = false;
   bool                  mFlirModeCyclePressed                 = false;
   bool                  mRwrPowerTogglePressed                = false;
   bool                  mRwrFilterTogglePressed               = false;
   bool                  mCenterSensorPressed                  = false;
   bool                  mIncreaseSensorRangePressed           = false;
   bool                  mDecreaseSensorRangePressed           = false;
   bool                  mHudModeButtonIsBeingPressed          = false;
   bool                  mMfdLftModeButtonIsBeingPressed       = false;
   bool                  mMfdRgtModeButtonIsBeingPressed       = false;
   bool                  mMfdActiveButtonIsBeingPressed        = false;
   bool                  mJettisonTanksButtonIsBeingPressed    = false;
   bool                  mMasterWarnButtonIsBeingPressed       = false;
   double                mLastRequestTime_1sec                 = 0.0;
   double                mLastRequestTime_50msec               = 0.0;
   bool                  mReturnIntArrayDataScriptExists       = false;
   bool                  mReturnDoubleArrayDataExists          = false;
   bool                  mReturnStringArrayDataExists          = false;
   bool                  mReturnBitPackedFlagsExists           = false;
   bool                  mNumChaffRemainingExists              = false;
   bool                  mNumFlaresRemainingExists             = false;
   bool                  mGetBingoFuelQtyExists                = false;
   bool                  mWeaponSelectToggledExists            = false;
   bool                  mMasterArmToggleExists                = false;
   bool                  mAirGroundModeToggleExists            = false;
   bool                  mEcmPowerToggleExists                 = false;
   bool                  mEcmAutoToggleExists                  = false;
   bool                  mChaffButtonPressedExists             = false;
   bool                  mChaffButtonReleasedExists            = false;
   bool                  mFlaresButtonPressedExists            = false;
   bool                  mFlaresButtonReleasedExists           = false;
   bool                  mCountermeasuresButtonPressedExists   = false;
   bool                  mCountermeasuresButtonReleasedExists  = false;
   bool                  mRadarPowerToggleExists               = false;
   bool                  mRadarModeCycleExists                 = false;
   bool                  mRadarSubmodeCycleExists              = false;
   bool                  mIrstPowerToggleExists                = false;
   bool                  mIrstModeCycleExists                  = false;
   bool                  mIrstCueToRadarExists                 = false;
   bool                  mIrstCueToHeadExists                  = false;
   bool                  mFlirPowerToggleExists                = false;
   bool                  mFlirModeCycleExists                  = false;
   bool                  mRwrPowerToggleExists                 = false;
   bool                  mRwrFilterToggleExists                = false;
   bool                  mMissileHudModeSelectedExists         = false;
   bool                  mGunHudModeSelectedExists             = false;
   bool                  mGroundAttackHudModeSelectedExists    = false;
   bool                  mNonWeaponHudModeSelectedExists       = false;
   bool                  mJettisonExternalTanksExists          = false;
   bool                  mClearMasterWarningExists             = false;
   bool                  mLockTargetExists                     = false;

   std::string                                 mConnectedPlatform;
   P6DOF_ControllerDataContainer::PlatformData mScriptBasedPlatData;
   std::set<int>                               mScriptsWaitingForReturnValues;
};
} // namespace WkP6DOF_Controller

#endif // P6DOF_ControllerPLUGINOBJECT_HPP
