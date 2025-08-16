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
#include "SimControllerPrefWidget.hpp"

#include <QTimeZone>

#include "SimControllerPrefObject.hpp"

WkSimController::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUi.setupUi(this);

   // Until the Dynamic toolbar options are implemented, hide the toolbarGroupBox
   mUi.toolbarGroupBox->hide();
}

void WkSimController::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mShowClockRate          = mUi.mShowClockRateCheckBox->isChecked();
   aPrefData.mShowProgressBar        = mUi.mShowProgressBarCheckBox->isChecked();
   aPrefData.mShowSimMode            = mUi.mShowSimModeCheckBox->isChecked();
   aPrefData.mShowSimState           = mUi.mShowSimStateCheckBox->isChecked();
   aPrefData.mShowSimTime            = mUi.mShowSimTimeCheckBox->isChecked();
   aPrefData.mShowTimeBehind         = mUi.mShowTimeBehindCheckBox->isChecked();
   aPrefData.mShowWallTime           = mUi.mShowWallTimeCheckBox->isChecked();
   aPrefData.mSendDIS_PDUs           = mUi.mSendDIS_PDUsCheckBox->isChecked();
   aPrefData.mAutoPause              = mUi.mAutoPauseGroupBox->isChecked();
   aPrefData.mFallingBehindThreshold = mUi.mFallingBehindSpinBox->value();
   aPrefData.mUTC_OffsetHrs          = mUi.mUTC_OffsetHrsBox->value();
   aPrefData.mUTC_OffsetMins         = mUi.mUTC_OffsetMinsBox->value();

   if (mUi.mLocalTimeSpecButton->isChecked())
   {
      aPrefData.mWallTimeSpec = Qt::LocalTime;
   }
   else if (mUi.mUTC_TimeSpecButon->isChecked())
   {
      aPrefData.mWallTimeSpec = Qt::UTC;
   }
   else if (mUi.mUTC_OffsetTimeSpecButton->isChecked())
   {
      aPrefData.mWallTimeSpec = Qt::OffsetFromUTC;
   }
}

void WkSimController::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.mShowClockRateCheckBox->setChecked(aPrefData.mShowClockRate);
   mUi.mShowProgressBarCheckBox->setChecked(aPrefData.mShowProgressBar);
   mUi.mShowSimModeCheckBox->setChecked(aPrefData.mShowSimMode);
   mUi.mShowSimStateCheckBox->setChecked(aPrefData.mShowSimState);
   mUi.mShowSimTimeCheckBox->setChecked(aPrefData.mShowSimTime);
   mUi.mShowTimeBehindCheckBox->setChecked(aPrefData.mShowTimeBehind);
   mUi.mShowWallTimeCheckBox->setChecked(aPrefData.mShowWallTime);
   mUi.mSendDIS_PDUsCheckBox->setChecked(aPrefData.mSendDIS_PDUs);
   mUi.mUTC_OffsetHrsBox->setValue(aPrefData.mUTC_OffsetHrs);
   mUi.mUTC_OffsetMinsBox->setValue(aPrefData.mUTC_OffsetMins);
   mUi.mAutoPauseGroupBox->setChecked(aPrefData.mAutoPause);
   mUi.mFallingBehindSpinBox->setValue(aPrefData.mFallingBehindThreshold);

   switch (aPrefData.mWallTimeSpec)
   {
   case Qt::LocalTime:
      mUi.mLocalTimeSpecButton->setChecked(true);
      break;
   case Qt::UTC:
      mUi.mUTC_TimeSpecButon->setChecked(true);
      break;
   case Qt::OffsetFromUTC:
      mUi.mUTC_OffsetTimeSpecButton->setChecked(true);
      break;
   default:
      break;
   }
}
