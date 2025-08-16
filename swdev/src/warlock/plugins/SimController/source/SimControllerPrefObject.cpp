// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "SimControllerPrefObject.hpp"

#include <QSettings>

WkSimController::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "SimControllerOptions")
{
}

void WkSimController::PrefObject::SetShowClockRate(bool aShow)
{
   mCurrentPrefs.mShowClockRate = aShow;
   emit ShowClockRateChanged(aShow);
}

void WkSimController::PrefObject::SetShowProgressBar(bool aShow)
{
   mCurrentPrefs.mShowProgressBar = aShow;
   emit ShowProgressBarChanged(aShow);
}

void WkSimController::PrefObject::SetShowSimState(bool aShow)
{
   mCurrentPrefs.mShowSimState = aShow;
   emit ShowSimStateChanged(aShow);
}

void WkSimController::PrefObject::SetShowSimMode(bool aShow)
{
   mCurrentPrefs.mShowSimMode = aShow;
   emit ShowSimModeChanged(aShow);
}

void WkSimController::PrefObject::SetShowSimTime(bool aShow)
{
   mCurrentPrefs.mShowSimTime = aShow;
   emit ShowSimTimeChanged(aShow);
}

void WkSimController::PrefObject::SetShowTimeBehind(bool aShow)
{
   mCurrentPrefs.mShowTimeBehind = aShow;
   emit ShowTimeBehindChanged(aShow);
}

void WkSimController::PrefObject::SetShowWallTime(bool aShow)
{
   mCurrentPrefs.mShowWallTime = aShow;
   emit ShowWallTimeChanged(aShow);
}

void WkSimController::PrefObject::SetSendDIS_PDUs(bool aSend)
{
   mCurrentPrefs.mSendDIS_PDUs = aSend;
   emit SendDIS_PDUsChanged(aSend);
}

void WkSimController::PrefObject::SetUTC_OffsetHrs(int aHrs)
{
   mCurrentPrefs.mUTC_OffsetHrs = aHrs;
   emit WallTimeSpecChanged();
}

void WkSimController::PrefObject::SetUTC_OffsetMins(int aMins)
{
   mCurrentPrefs.mUTC_OffsetMins = aMins;
   emit WallTimeSpecChanged();
}

void WkSimController::PrefObject::SetWallTimeSpec(Qt::TimeSpec aTimeSpec)
{
   mCurrentPrefs.mWallTimeSpec = aTimeSpec;
   emit WallTimeSpecChanged();
}

void WkSimController::PrefObject::EmitAllSignals()
{
   emit ShowClockRateChanged(mCurrentPrefs.mShowClockRate);
   emit ShowProgressBarChanged(mCurrentPrefs.mShowProgressBar);
   emit ShowSimModeChanged(mCurrentPrefs.mShowSimMode);
   emit ShowSimStateChanged(mCurrentPrefs.mShowSimState);
   emit ShowSimTimeChanged(mCurrentPrefs.mShowSimTime);
   emit ShowTimeBehindChanged(mCurrentPrefs.mShowTimeBehind);
   emit ShowWallTimeChanged(mCurrentPrefs.mShowWallTime);
   emit SendDIS_PDUsChanged(mCurrentPrefs.mSendDIS_PDUs);
   emit WallTimeSpecChanged();
   emit FallingBehindThresholdChanged(mCurrentPrefs.mFallingBehindThreshold);
}

void WkSimController::PrefObject::Apply()
{
   EmitAllSignals();
}

WkSimController::PrefData WkSimController::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mShowClockRate   = aSettings.value("showClockRate", mDefaultPrefs.mShowClockRate).toBool();
   pData.mShowProgressBar = aSettings.value("showProgressBar", mDefaultPrefs.mShowProgressBar).toBool();
   pData.mShowSimMode     = aSettings.value("showSimMode", mDefaultPrefs.mShowSimMode).toBool();
   pData.mShowSimState    = aSettings.value("showSimState", mDefaultPrefs.mShowSimState).toBool();
   pData.mShowSimTime     = aSettings.value("showSimTime", mDefaultPrefs.mShowSimTime).toBool();
   pData.mShowTimeBehind  = aSettings.value("showTimeBehind", mDefaultPrefs.mShowTimeBehind).toBool();
   pData.mShowWallTime    = aSettings.value("showWallTime", mDefaultPrefs.mShowWallTime).toBool();

   pData.mSendDIS_PDUs           = aSettings.value("sendDIS_PDUs", mDefaultPrefs.mSendDIS_PDUs).toBool();
   pData.mAutoPause              = aSettings.value("sendDIS_PDUs_SimBehind", mDefaultPrefs.mAutoPause).toBool();
   pData.mFallingBehindThreshold = aSettings.value("simBehindThreshold", mDefaultPrefs.mFallingBehindThreshold).toDouble();

   pData.mUTC_OffsetHrs  = aSettings.value("UTC_OffsetHrs", mDefaultPrefs.mUTC_OffsetHrs).toInt();
   pData.mUTC_OffsetMins = aSettings.value("UTC_OffsetMins", mDefaultPrefs.mUTC_OffsetMins).toInt();
   pData.mWallTimeSpec = static_cast<Qt::TimeSpec>(aSettings.value("wallTimeSpec", mDefaultPrefs.mWallTimeSpec).toInt());
   return pData;
}

void WkSimController::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("showClockRate", mCurrentPrefs.mShowClockRate);
   aSettings.setValue("showProgressBar", mCurrentPrefs.mShowProgressBar);
   aSettings.setValue("showSimMode", mCurrentPrefs.mShowSimMode);
   aSettings.setValue("showSimState", mCurrentPrefs.mShowSimState);
   aSettings.setValue("showSimTime", mCurrentPrefs.mShowSimTime);
   aSettings.setValue("showTimeBehind", mCurrentPrefs.mShowTimeBehind);
   aSettings.setValue("showWallTime", mCurrentPrefs.mShowWallTime);

   aSettings.setValue("sendDIS_PDUs", mCurrentPrefs.mSendDIS_PDUs);
   aSettings.setValue("sendDIS_PDUs_SimBehind", mCurrentPrefs.mAutoPause);
   aSettings.setValue("simBehindThreshold", mCurrentPrefs.mFallingBehindThreshold);

   aSettings.setValue("UTC_OffsetHrs", mCurrentPrefs.mUTC_OffsetHrs);
   aSettings.setValue("UTC_OffsetMins", mCurrentPrefs.mUTC_OffsetMins);
   aSettings.setValue("wallTimeSpec", mCurrentPrefs.mWallTimeSpec);
}
