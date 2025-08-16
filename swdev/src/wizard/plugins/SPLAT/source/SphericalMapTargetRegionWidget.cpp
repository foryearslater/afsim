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

#include "SphericalMapTargetRegionWidget.hpp"

#include <QMessageBox>

SPLAT::SphericalMapTargetRegionWidget::SphericalMapTargetRegionWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
{
   mUi.setupUi(this);
   Initialize();
   Connect();
   Populate();
}

void SPLAT::SphericalMapTargetRegionWidget::WriteData(std::ostream& aStream)
{
   aStream << "   azimuth_limits " << mTargetRegion.mMinAzimuthAngle << " " << mTargetRegion.mMaxAzimuthAngle << "\n";
   aStream << "   azimuth_step " << mTargetRegion.mAzimuthAngleStep << "\n";
   aStream << "   elevation_limits " << mTargetRegion.mMinElevationAngle << " " << mTargetRegion.mMaxElevationAngle << "\n";
   aStream << "   elevation_step " << mTargetRegion.mElevationAngleStep << "\n";

   if (mUi.mAltitude_RB->isChecked())
   {
      aStream << "   altitude " << mTargetRegion.mAltitude << "\n";
   }
   else if (mUi.mRange_RB->isChecked())
   {
      aStream << "   range " << mTargetRegion.mRange << "\n";
   }
}

bool SPLAT::SphericalMapTargetRegionWidget::ApplyChanges()
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

void SPLAT::SphericalMapTargetRegionWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::SphericalMapTargetRegionWidget::Connect()
{
   connect(mUi.mAltitude_RB, &QRadioButton::toggled, [this](bool aChecked) { AltitudeRangeButtonToggled(0, aChecked); });
   connect(mUi.mRange_RB, &QRadioButton::toggled, [this](bool aChecked) { AltitudeRangeButtonToggled(1, aChecked); });
}

void SPLAT::SphericalMapTargetRegionWidget::Initialize()
{
   mTargetRegion = TargetRegion();
}

void SPLAT::SphericalMapTargetRegionWidget::Populate()
{
   mUi.mAltitude_RB->setChecked(true);

   mUi.mAltitudeLineEdit->SetValue(mTargetRegion.mAltitude);
   mUi.mRangeLineEdit->SetValue(mTargetRegion.mRange);

   mUi.mAzimuthAngleMinEdit->SetValue(mTargetRegion.mMinAzimuthAngle);
   mUi.mAzimuthAngleMaxEdit->SetValue(mTargetRegion.mMaxAzimuthAngle);
   mUi.mAzimuthAngleStepEdit->SetValue(mTargetRegion.mAzimuthAngleStep);

   mUi.mElevationAngleMinEdit->SetValue(mTargetRegion.mMinElevationAngle);
   mUi.mElevationAngleMaxEdit->SetValue(mTargetRegion.mMaxElevationAngle);
   mUi.mElevationAngleStepEdit->SetValue(mTargetRegion.mElevationAngleStep);
}

void SPLAT::SphericalMapTargetRegionWidget::SetTargetRegion()
{
   mTargetRegion.mAltitude = mUi.mAltitudeLineEdit->GetValue();
   mTargetRegion.mRange    = mUi.mRangeLineEdit->GetValue();

   mTargetRegion.mMinAzimuthAngle  = mUi.mAzimuthAngleMinEdit->GetValue();
   mTargetRegion.mMaxAzimuthAngle  = mUi.mAzimuthAngleMaxEdit->GetValue();
   mTargetRegion.mAzimuthAngleStep = mUi.mAzimuthAngleStepEdit->GetValue();

   mTargetRegion.mMinElevationAngle  = mUi.mElevationAngleMinEdit->GetValue();
   mTargetRegion.mMaxElevationAngle  = mUi.mElevationAngleMaxEdit->GetValue();
   mTargetRegion.mElevationAngleStep = mUi.mElevationAngleStepEdit->GetValue();
}

bool SPLAT::SphericalMapTargetRegionWidget::TargetRegionValid()
{
   return mUi.mAltitudeLineEdit->IsValid() && mUi.mRangeLineEdit->IsValid() && mUi.mAzimuthAngleMinEdit->IsValid() &&
          mUi.mAzimuthAngleMaxEdit->IsValid() && mUi.mAzimuthAngleStepEdit->IsValid() &&
          mUi.mElevationAngleMinEdit->IsValid() && mUi.mElevationAngleMaxEdit->IsValid() &&
          mUi.mElevationAngleStepEdit->IsValid();
}

void SPLAT::SphericalMapTargetRegionWidget::AltitudeRangeButtonToggled(int aId, bool aChecked)
{
   if (aId == 0)
   {
      if (aChecked)
      {
         mUi.mAltitude_RB->setChecked(true);
         mUi.mRange_RB->setChecked(false);

         mUi.mAltitudeLineEdit->setEnabled(true);
         mUi.mRangeLineEdit->setEnabled(false);
      }
   }
   else if (aId == 1)
   {
      if (aChecked)
      {
         mUi.mAltitude_RB->setChecked(false);
         mUi.mRange_RB->setChecked(true);

         mUi.mAltitudeLineEdit->setEnabled(false);
         mUi.mRangeLineEdit->setEnabled(true);
      }
   }
}
