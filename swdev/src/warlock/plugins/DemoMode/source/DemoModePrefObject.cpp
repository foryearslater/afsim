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
#include "DemoModePrefObject.hpp"

#include <QSettings>

WkDemoMode::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "DemoModeOptions")
{
}

void WkDemoMode::PrefObject::SetSelectPlatform(const std::string& aPlatformName, bool aSelect)
{
   mCurrentPrefs.mSelectPlatform = aSelect;
   if (aSelect)
   {
      mCurrentPrefs.mSelectPlatformName = aPlatformName;
   }
}

void WkDemoMode::PrefObject::Apply()
{
   emit EnabledChanged(mCurrentPrefs.mEnabled);
   emit RunSpeedMultiplierChanged(mCurrentPrefs.mRunSpeedMultiplier);
}

WkDemoMode::PrefData WkDemoMode::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mEnabled            = aSettings.value("enabled", mDefaultPrefs.mEnabled).toBool();
   pData.mPauseLength_sec    = aSettings.value("pauseLength", mDefaultPrefs.mPauseLength_sec).toDouble();
   pData.mRunSpeedMultiplier = aSettings.value("runSpeed", mDefaultPrefs.mRunSpeedMultiplier).toDouble();
   pData.mSelectPlatform     = aSettings.value("selectPlatform", mDefaultPrefs.mSelectPlatform).toBool();
   pData.mSelectPlatformName =
      aSettings.value("platformName", QString::fromStdString(mDefaultPrefs.mSelectPlatformName)).toString().toStdString();
   return pData;
}

void WkDemoMode::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("enabled", mCurrentPrefs.mEnabled);
   aSettings.setValue("pauseLength", mCurrentPrefs.mPauseLength_sec);
   aSettings.setValue("runSpeed", mCurrentPrefs.mRunSpeedMultiplier);
   aSettings.setValue("selectPlatform", mCurrentPrefs.mSelectPlatform);
   aSettings.setValue("platformName", QString::fromStdString(mCurrentPrefs.mSelectPlatformName));
}
