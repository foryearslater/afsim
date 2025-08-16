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

#include "HorizontalMapTargetRegionWidget.hpp"

#include <QMessageBox>


SPLAT::HorizontalMapTargetRegionWidget::HorizontalMapTargetRegionWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
   , mTargetRegionType(TargetRegionType::cRANGE)
{
   mUi.setupUi(this);
   Initialize();
   Connect();
   Populate();
}

void SPLAT::HorizontalMapTargetRegionWidget::WriteData(std::ostream& aStream)
{
   if (mTargetRegionType == TargetRegionType::cRANGE)
   {
      aStream << "   cross_range_limits " << mTargetRegionRange.mMinCross << " " << mTargetRegionRange.mMaxCross << "\n";
      aStream << "   cross_range_step " << mTargetRegionRange.mStepCross << "\n";
      aStream << "   down_range_limits " << mTargetRegionRange.mMinDown << " " << mTargetRegionRange.mMaxDown << "\n";
      aStream << "   down_range_step " << mTargetRegionRange.mStepDown << "\n";
   }
   else if (mTargetRegionType == TargetRegionType::cLATLON)
   {
      aStream << "   latitude_limits " << mTargetRegionLatLon.mMinLat << " " << mTargetRegionLatLon.mMaxLat << "\n";
      aStream << "   latitude_step " << mTargetRegionLatLon.mStepLat << "\n";
      aStream << "   longitude_limits " << mTargetRegionLatLon.mMinLon << " " << mTargetRegionLatLon.mMaxLon << "\n";
      aStream << "   longitude_step " << mTargetRegionLatLon.mStepLon << "\n";
      aStream << "   sensor_relative_orientation " << (mTargetRegionLatLon.mRelative ? "true" : "false") << "\n";
   }
}

bool SPLAT::HorizontalMapTargetRegionWidget::ApplyChanges()
{
   // Target Region
   mTargetRegionType = TargetRegionType::cINVALID;

   if (mUi.mButtonCRDR->isChecked() && TargetRegionRangeValid()) // if the Range option is selected
   {
      mTargetRegionType = TargetRegionType::cRANGE;
      SetTargetRegion();
   }
   else if (mUi.mButtonLatLon->isChecked() && TargetRegionLatLonValid()) // if the Lat/Lon option is selected
   {
      mTargetRegionType = TargetRegionType::cLATLON;
      SetTargetRegion();
   }

   if (mTargetRegionType == TargetRegionType::cINVALID)
   {
      QMessageBox::critical(this, "Error", "Target Region Invalid");
   }
   return (mTargetRegionType != TargetRegionType::cINVALID);
}

void SPLAT::HorizontalMapTargetRegionWidget::Initialize()
{
   mTargetRegionType   = TargetRegionType::cRANGE;
   mTargetRegionRange  = TargetRegionRange();
   mTargetRegionLatLon = TargetRegionLatLon();

   mUi.mButtonCRDR->setChecked(true);
   mUi.mButtonLatLon->setChecked(false);
   mUi.mRelativeCheckBox->setChecked(false);
}

void SPLAT::HorizontalMapTargetRegionWidget::Populate()
{
   // Down Range / Cross Range
   mUi.mCrossRangeMinEdit->SetValue(mTargetRegionRange.mMinCross);
   mUi.mCrossRangeMaxEdit->SetValue(mTargetRegionRange.mMaxCross);
   mUi.mCrossRangeStepEdit->SetValue(mTargetRegionRange.mStepCross);
   mUi.mDownRangeMinEdit->SetValue(mTargetRegionRange.mMinCross);
   mUi.mDownRangeMaxEdit->SetValue(mTargetRegionRange.mMaxCross);
   mUi.mDownRangeStepEdit->SetValue(mTargetRegionRange.mStepCross);

   // Latitude / Longitude
   mUi.mLatMinEdit->SetValue(mTargetRegionLatLon.mMinLat);
   mUi.mLatMaxEdit->SetValue(mTargetRegionLatLon.mMaxLat);
   mUi.mLatStepEdit->SetValue(mTargetRegionLatLon.mStepLat);
   mUi.mLonMinEdit->SetValue(mTargetRegionLatLon.mMinLon);
   mUi.mLonMaxEdit->SetValue(mTargetRegionLatLon.mMaxLon);
   mUi.mLonStepEdit->SetValue(mTargetRegionLatLon.mStepLon);
}

void SPLAT::HorizontalMapTargetRegionWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::HorizontalMapTargetRegionWidget::Connect()
{
   connect(mUi.mButtonCRDR,
           &QRadioButton::toggled,
           this,
           [this](bool aChecked) { mUi.mRangeGroupBox->setEnabled(aChecked); });
   connect(mUi.mButtonLatLon,
           &QRadioButton::toggled,
           this,
           [this](bool aChecked) { mUi.mLatLonGroupBox->setEnabled(aChecked); });
}

void SPLAT::HorizontalMapTargetRegionWidget::SetTargetRegion()
{
   mTargetRegionRange.mMinCross  = mUi.mCrossRangeMinEdit->GetValue();
   mTargetRegionRange.mMaxCross  = mUi.mCrossRangeMaxEdit->GetValue();
   mTargetRegionRange.mStepCross = mUi.mCrossRangeStepEdit->GetValue();
   mTargetRegionRange.mMinDown   = mUi.mDownRangeMinEdit->GetValue();
   mTargetRegionRange.mMaxDown   = mUi.mDownRangeMaxEdit->GetValue();
   mTargetRegionRange.mStepDown  = mUi.mDownRangeStepEdit->GetValue();

   mTargetRegionLatLon.mMinLat   = mUi.mLatMinEdit->GetValue();
   mTargetRegionLatLon.mMaxLat   = mUi.mLatMaxEdit->GetValue();
   mTargetRegionLatLon.mStepLat  = mUi.mLatStepEdit->GetValue();
   mTargetRegionLatLon.mMinLon   = mUi.mLonMinEdit->GetValue();
   mTargetRegionLatLon.mMaxLon   = mUi.mLonMaxEdit->GetValue();
   mTargetRegionLatLon.mStepLon  = mUi.mLonStepEdit->GetValue();
   mTargetRegionLatLon.mRelative = mUi.mRelativeCheckBox->isChecked();
}

bool SPLAT::HorizontalMapTargetRegionWidget::TargetRegionRangeValid()
{
   return mUi.mCrossRangeMinEdit->IsValid() && mUi.mCrossRangeMaxEdit->IsValid() && mUi.mCrossRangeStepEdit->IsValid() &&
          mUi.mDownRangeMinEdit->IsValid() && mUi.mDownRangeMaxEdit->IsValid() && mUi.mDownRangeStepEdit->IsValid();
}

bool SPLAT::HorizontalMapTargetRegionWidget::TargetRegionLatLonValid()
{
   return mUi.mLatMinEdit->IsValid() && mUi.mLatMaxEdit->IsValid() && mUi.mLatStepEdit->IsValid() &&
          mUi.mLonMinEdit->IsValid() && mUi.mLonMaxEdit->IsValid() && mUi.mLonStepEdit->IsValid();
}