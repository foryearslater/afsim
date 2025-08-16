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

#include "BackupPrefWidget.hpp"

wizard::BackupPrefWidget::BackupPrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<BackupPrefObject>(parent)
{
   mUi.setupUi(this);
}

void wizard::BackupPrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void wizard::BackupPrefWidget::ReadPreferenceData(const BackupPrefData& aPrefData)
{
   mUi.mBackups->setChecked(aPrefData.mEnable);
}

void wizard::BackupPrefWidget::WritePreferenceData(BackupPrefData& aPrefData)
{
   aPrefData.mEnable = mUi.mBackups->isChecked();
}
