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
#include "DemoModePrefWidget.hpp"

#include "DemoModePrefObject.hpp"

WkDemoMode::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUi.setupUi(this);
}

void WkDemoMode::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.enableGroupBox->setChecked(aPrefData.mEnabled);
   mUi.pauseSpinBox->setValue((int)aPrefData.mPauseLength_sec);
   mUi.runSpeedDoubleSpinBox->setValue(aPrefData.mRunSpeedMultiplier);
   mUi.selectGroupBox->setChecked(aPrefData.mSelectPlatform);
   mUi.platformLineEdit->setText(QString::fromStdString(aPrefData.mSelectPlatformName));
}

void WkDemoMode::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mEnabled            = mUi.enableGroupBox->isChecked();
   aPrefData.mPauseLength_sec    = (double)mUi.pauseSpinBox->value();
   aPrefData.mRunSpeedMultiplier = mUi.runSpeedDoubleSpinBox->value();
   aPrefData.mSelectPlatform     = mUi.selectGroupBox->isChecked();
   aPrefData.mSelectPlatformName = mUi.platformLineEdit->text().toStdString();
}
