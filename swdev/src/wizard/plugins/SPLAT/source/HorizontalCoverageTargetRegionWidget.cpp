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

#include "HorizontalCoverageTargetRegionWidget.hpp"

#include <QMessageBox>

SPLAT::HorizontalCoverageTargetRegionWidget::HorizontalCoverageTargetRegionWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
{
   mUi.setupUi(this);
   Initialize();
   Connect();
   Populate();
}

void SPLAT::HorizontalCoverageTargetRegionWidget::WriteData(std::ostream& aStream)
{
   aStream << "   azimuth_limits " << mTargetRegion.mMinAzimuthAngle << " " << mTargetRegion.mMaxAzimuthAngle << "\n";
   aStream << "   azimuth_step " << mTargetRegion.mAzimuthAngleStep << "\n";

   if (mTargetRegion.mMinRange > 0.0) // use default if not > 0.0
   {
      aStream << "   minimum_range " << mTargetRegion.mMinRange << "\n";
   }

   if (mTargetRegion.mMaxRange > 0.0) // use default if not > 0.0
   {
      aStream << "   maximum_range " << mTargetRegion.mMaxRange << "\n";
   }

   aStream << "   large_range_step " << mTargetRegion.mLargeRangeStep << "\n";
   aStream << "   small_range_step " << mTargetRegion.mSmallRangeStep << "\n";

   if (mUi.mFixedAltitude_RB->isChecked())
   {
      aStream << "   altitude " << mTargetRegion.mAltitude << "\n";
   }
   else if (mUi.mRangeAltitude_RB->isChecked())
   {
      aStream << "   altitude from " << mTargetRegion.mAltitudeStart;
      aStream << " to " << mTargetRegion.mAltitudeEnd;
      aStream << " by " << mTargetRegion.mAltitudeStep << "\n";
   }
}

bool SPLAT::HorizontalCoverageTargetRegionWidget::ApplyChanges()
{
   bool isValid = false;
   if (TargetRegionValid())
   {
      SetTargetRegion();
      isValid = true;
   }

   if (!isValid)
   {
      QMessageBox::critical(this, "Error", "Target Region Invalid");
   }
   return isValid;
}

void SPLAT::HorizontalCoverageTargetRegionWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::HorizontalCoverageTargetRegionWidget::Connect()
{
   connect(mUi.mFixedAltitude_RB, &QRadioButton::toggled, [this](bool aChecked) { AltitudeButtonToggled(0, aChecked); });
   connect(mUi.mRangeAltitude_RB, &QRadioButton::toggled, [this](bool aChecked) { AltitudeButtonToggled(1, aChecked); });
}

void SPLAT::HorizontalCoverageTargetRegionWidget::Initialize()
{
   mTargetRegion = TargetRegion();
}

void SPLAT::HorizontalCoverageTargetRegionWidget::Populate()
{
   mUi.mFixedAltitude_RB->setChecked(true);

   mUi.mAltitudeFixedLineEdit->SetValue(mTargetRegion.mAltitude);
   mUi.mAltitudeFromLineEdit->SetValue(mTargetRegion.mAltitudeStart);
   mUi.mAltitudeToLineEdit->SetValue(mTargetRegion.mAltitudeEnd);
   mUi.mAltitudeByLineEdit->SetValue(mTargetRegion.mAltitudeStep);

   mUi.mAzimuthAngleMinEdit->SetValue(mTargetRegion.mMinAzimuthAngle);
   mUi.mAzimuthAngleMaxEdit->SetValue(mTargetRegion.mMaxAzimuthAngle);
   mUi.mAzimuthAngleStepEdit->SetValue(mTargetRegion.mAzimuthAngleStep);

   mUi.mRangeMinEdit->SetValue(mTargetRegion.mMinRange);
   mUi.mRangeMaxEdit->SetValue(mTargetRegion.mMaxRange);
   mUi.mStepSmallEdit->SetValue(mTargetRegion.mSmallRangeStep);
   mUi.mStepLargeEdit->SetValue(mTargetRegion.mLargeRangeStep);
}

void SPLAT::HorizontalCoverageTargetRegionWidget::SetTargetRegion()
{
   mTargetRegion.mAltitude      = mUi.mAltitudeFixedLineEdit->GetValue();
   mTargetRegion.mAltitudeStart = mUi.mAltitudeFromLineEdit->GetValue();
   mTargetRegion.mAltitudeEnd   = mUi.mAltitudeToLineEdit->GetValue();
   mTargetRegion.mAltitudeStep  = mUi.mAltitudeByLineEdit->GetValue();

   mTargetRegion.mMinAzimuthAngle  = mUi.mAzimuthAngleMinEdit->GetValue();
   mTargetRegion.mMaxAzimuthAngle  = mUi.mAzimuthAngleMaxEdit->GetValue();
   mTargetRegion.mAzimuthAngleStep = mUi.mAzimuthAngleStepEdit->GetValue();

   mTargetRegion.mMinRange       = mUi.mRangeMinEdit->GetValue();
   mTargetRegion.mMaxRange       = mUi.mRangeMaxEdit->GetValue();
   mTargetRegion.mSmallRangeStep = mUi.mStepSmallEdit->GetValue();
   mTargetRegion.mLargeRangeStep = mUi.mStepLargeEdit->GetValue();
}

bool SPLAT::HorizontalCoverageTargetRegionWidget::TargetRegionValid()
{
   return mUi.mAltitudeFixedLineEdit->IsValid() && mUi.mAltitudeFromLineEdit->IsValid() &&
          mUi.mAltitudeToLineEdit->IsValid() && mUi.mAltitudeByLineEdit->IsValid() &&
          mUi.mAzimuthAngleMinEdit->IsValid() && mUi.mAzimuthAngleMaxEdit->IsValid() &&
          mUi.mAzimuthAngleStepEdit->IsValid() && mUi.mRangeMinEdit->IsValid() && mUi.mRangeMaxEdit->IsValid() &&
          mUi.mStepSmallEdit->IsValid() && mUi.mStepLargeEdit->IsValid();
}

void SPLAT::HorizontalCoverageTargetRegionWidget::AltitudeButtonToggled(int aId, bool aChecked)
{
   if (aId == 0)
   {
      if (aChecked)
      {
         mUi.mFixedAltitude_RB->setChecked(true);
         mUi.mRangeAltitude_RB->setChecked(false);

         mUi.mAltitudeFixedLabel->setEnabled(true);
         mUi.mAltitudeFixedLineEdit->setEnabled(true);


         mUi.mAltitudeFromLabel->setEnabled(false);
         mUi.mAltitudeFromLineEdit->setEnabled(false);
         mUi.mAltitudeToLabel->setEnabled(false);
         mUi.mAltitudeToLineEdit->setEnabled(false);
         mUi.mAltitudeByLabel->setEnabled(false);
         mUi.mAltitudeByLineEdit->setEnabled(false);
      }
   }
   else if (aId == 1)
   {
      if (aChecked)
      {
         mUi.mFixedAltitude_RB->setChecked(false);
         mUi.mRangeAltitude_RB->setChecked(true);

         mUi.mAltitudeFixedLabel->setEnabled(false);
         mUi.mAltitudeFixedLineEdit->setEnabled(false);

         mUi.mAltitudeFromLabel->setEnabled(true);
         mUi.mAltitudeFromLineEdit->setEnabled(true);
         mUi.mAltitudeToLabel->setEnabled(true);
         mUi.mAltitudeToLineEdit->setEnabled(true);
         mUi.mAltitudeByLabel->setEnabled(true);
         mUi.mAltitudeByLineEdit->setEnabled(true);
      }
   }
}
