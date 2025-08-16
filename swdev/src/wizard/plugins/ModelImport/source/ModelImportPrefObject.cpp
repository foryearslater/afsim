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

#include "ModelImportPrefObject.hpp"

#include <QDir>

#include "Project.hpp"

ModelImport::PrefObject::PrefObject(QObject* aParent)
   : wkf::PrefObjectT<PreferenceData>(aParent, "ModelImport")
{
}

// ================================================================================================

void ModelImport::PrefObject::Apply()
{
   if (mPreferencesChanged)
   {
      emit Update();
      mPreferencesChanged = false;
   }
   emit UpdateDisplay();
}

// ================================================================================================

void ModelImport::PrefObject::SetPreferenceDataP(const PreferenceData& aPrefData)
{
   if (aPrefData.mPath != mCurrentPrefs.mPath || aPrefData.mMetaFile != mCurrentPrefs.mMetaFile)
   {
      mPreferencesChanged = true;
   }
   mCurrentPrefs = aPrefData;
}

// ================================================================================================

ModelImport::PreferenceData ModelImport::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PreferenceData pData;
   pData.mPath        = aSettings.value("Path", mDefaultPrefs.mPath).toString();
   pData.mMetaFile    = aSettings.value("MetaFile", mDefaultPrefs.mMetaFile).toString();
   pData.mImportsFile = aSettings.value("ImportsFile", mDefaultPrefs.mImportsFile).toString();
   pData.mErrorsFile  = aSettings.value("ErrorsFile", mDefaultPrefs.mErrorsFile).toString();
   pData.mDisplayMode = aSettings.value("ViewMode", mDefaultPrefs.mDisplayMode).toString();
   return pData;
}

// ================================================================================================
void ModelImport::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("Path", mCurrentPrefs.mPath);
   aSettings.setValue("MetaFile", mCurrentPrefs.mMetaFile);
   aSettings.setValue("ImportsFile", mCurrentPrefs.mImportsFile);
   aSettings.setValue("ErrorsFile", mCurrentPrefs.mErrorsFile);
   aSettings.setValue("ViewMode", mCurrentPrefs.mDisplayMode);
}

// ================================================================================================

void ModelImport::PrefObject::SetUpdateFlag()
{
   mPreferencesChanged = true;
}
