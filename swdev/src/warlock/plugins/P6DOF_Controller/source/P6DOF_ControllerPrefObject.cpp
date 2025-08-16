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

#include "P6DOF_ControllerPrefObject.hpp"

#include <QSettings>

WkP6DOF_Controller::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "P6DOF_ControllerOptions")
{
}

void WkP6DOF_Controller::PrefObject::SetShowHud(bool aShow)
{
   mCurrentPrefs.mShowHud = aShow;
   emit ShowHudChanged(aShow);
}

void WkP6DOF_Controller::PrefObject::SetFirstPerson(bool aFirstPerson)
{
   mCurrentPrefs.mFirstPerson = aFirstPerson;
   emit ShowFirstPersonChanged(aFirstPerson);
}

void WkP6DOF_Controller::PrefObject::SetHDD(bool aHDD)
{
   mCurrentPrefs.mHDD = aHDD;
   emit ShowHDDChanged(aHDD);
}

void WkP6DOF_Controller::PrefObject::SetAutopilotMode(bool aAutopilotMode)
{
   mCurrentPrefs.mAutopilot = aAutopilotMode;
   emit AutopilotModeChanged(aAutopilotMode);
}

void WkP6DOF_Controller::PrefObject::SetManualPilotMode(bool aManualPilotMode)
{
   mCurrentPrefs.mManualPilot = aManualPilotMode;
   emit ManualPilotModeChanged(aManualPilotMode);
}

void WkP6DOF_Controller::PrefObject::SetSyntheticPilotMode(bool aSyntheticPilotMode)
{
   mCurrentPrefs.mSyntheticPilot = aSyntheticPilotMode;
   emit SyntheticPilotModeChanged(aSyntheticPilotMode);
}

void WkP6DOF_Controller::PrefObject::SetGuidancePilotMode(bool aGuidancePilotMode)
{
   mCurrentPrefs.mGuidancePilot = aGuidancePilotMode;
   emit GuidancePilotModeChanged(aGuidancePilotMode);
}

void WkP6DOF_Controller::PrefObject::SetResolution(unsigned int aResX, unsigned int aResY)
{
   mCurrentPrefs.mResX = aResX;
   mCurrentPrefs.mResY = aResY;
   emit ResolutionChanged(aResX, aResY);
}

void WkP6DOF_Controller::PrefObject::EmitAllSignals()
{
   emit ShowHudChanged(mCurrentPrefs.mShowHud);
   emit ShowFirstPersonChanged(mCurrentPrefs.mFirstPerson);
   emit ShowHDDChanged(mCurrentPrefs.mHDD);
   emit AutopilotModeChanged(mCurrentPrefs.mAutopilot);
   emit ManualPilotModeChanged(mCurrentPrefs.mManualPilot);
   emit SyntheticPilotModeChanged(mCurrentPrefs.mSyntheticPilot);
   emit GuidancePilotModeChanged(mCurrentPrefs.mGuidancePilot);
   emit ResolutionChanged(mCurrentPrefs.mResX, mCurrentPrefs.mResY);
}

void WkP6DOF_Controller::PrefObject::Apply()
{
   EmitAllSignals();
}

WkP6DOF_Controller::PrefData WkP6DOF_Controller::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mShowHud        = aSettings.value("showHud", mDefaultPrefs.mShowHud).toBool();
   pData.mFirstPerson    = aSettings.value("firstperson", mDefaultPrefs.mFirstPerson).toBool();
   pData.mHDD            = aSettings.value("hdd", mDefaultPrefs.mHDD).toBool();
   pData.mAutopilot      = aSettings.value("autopilot", mDefaultPrefs.mAutopilot).toBool();
   pData.mManualPilot    = aSettings.value("manualpilot", mDefaultPrefs.mManualPilot).toBool();
   pData.mSyntheticPilot = aSettings.value("syntheticpilot", mDefaultPrefs.mSyntheticPilot).toBool();
   pData.mGuidancePilot  = aSettings.value("guidancepilot", mDefaultPrefs.mGuidancePilot).toBool();
   pData.mResX           = aSettings.value("resx", mDefaultPrefs.mResX).toUInt();
   pData.mResY           = aSettings.value("resy", mDefaultPrefs.mResY).toUInt();
   return pData;
}

void WkP6DOF_Controller::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("showHud", mCurrentPrefs.mShowHud);
   aSettings.setValue("firstperson", mCurrentPrefs.mFirstPerson);
   aSettings.setValue("hdd", mCurrentPrefs.mHDD);
   aSettings.setValue("autopilot", mCurrentPrefs.mAutopilot);
   aSettings.setValue("manualpilot", mCurrentPrefs.mManualPilot);
   aSettings.setValue("syntheticpilot", mCurrentPrefs.mSyntheticPilot);
   aSettings.setValue("guidancepilot", mCurrentPrefs.mGuidancePilot);
   aSettings.setValue("resx", mCurrentPrefs.mResX);
   aSettings.setValue("resy", mCurrentPrefs.mResY);
}
