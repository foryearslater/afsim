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

#include "PrefObject.hpp"

WarlockTraining::PrefObject::PrefObject(QObject* aParent /*= 0*/)
   : wkf::PrefObjectT<PrefData>(aParent, "WarlockTraining")
{}

void WarlockTraining::PrefObject::Apply()
{
   // If Preferences have changed, we need to notify subscribers
   if (mPreferencesChanged)
   {
      emit Changed(mCurrentPrefs);
      mPreferencesChanged = false;
   }
}

void WarlockTraining::PrefObject::SetPreferenceDataP(const PrefData& aPrefData)
{
   // Check to see if the new value for the preferences are different than the current values
   // If so, updated the mPreferencesChanged flag
   if (mCurrentPrefs.mDisplayAltitude != aPrefData.mDisplayAltitude ||
       mCurrentPrefs.mDisplayHeading != aPrefData.mDisplayHeading)
   {
      mPreferencesChanged = true;
      mCurrentPrefs = aPrefData;
   }
}

WarlockTraining::PrefData WarlockTraining::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   //Read the settings from the QSettings file
   pData.mDisplayAltitude = aSettings.value("showAltitude", mDefaultPrefs.mDisplayAltitude).toBool();
   pData.mDisplayHeading  = aSettings.value("showHeading",  mDefaultPrefs.mDisplayHeading).toBool();
   return pData;
}

void WarlockTraining::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   //Write the settings to the QSettings file
   aSettings.setValue("showAltitude", mCurrentPrefs.mDisplayAltitude);
   aSettings.setValue("showHeading",  mCurrentPrefs.mDisplayHeading);
}
