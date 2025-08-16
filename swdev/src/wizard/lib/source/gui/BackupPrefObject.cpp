// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "BackupPrefObject.hpp"

wizard::BackupPrefObject::BackupPrefObject(QObject* parent /*= nullptr*/)
   : wkf::PrefObjectT<BackupPrefData>(parent, cNAME)
{
}

void wizard::BackupPrefObject::Apply()
{
   emit Changed();
}

wizard::BackupPrefData wizard::BackupPrefObject::ReadSettings(QSettings& aSettings) const
{
   BackupPrefData pData;
   pData.mEnable = aSettings.value("enabled", mDefaultPrefs.mEnable).toBool();
   return pData;
}

void wizard::BackupPrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("enabled", mCurrentPrefs.mEnable);
}

bool wizard::BackupPrefObject::Enabled() const
{
   return mCurrentPrefs.mEnable;
}
