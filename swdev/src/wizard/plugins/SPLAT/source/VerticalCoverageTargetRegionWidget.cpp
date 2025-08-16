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

#include "VerticalCoverageTargetRegionWidget.hpp"

#include <QMessageBox>

SPLAT::VerticalCoverageTargetRegionWidget::VerticalCoverageTargetRegionWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
{
   mUi.setupUi(this);
   Initialize();
   Populate();
}

void SPLAT::VerticalCoverageTargetRegionWidget::WriteData(std::ostream& aStream)
{
   aStream << "   elevation_limits " << mTargetRegion.mMinElevationAngle << " " << mTargetRegion.mMaxElevationAngle << "\n";
   aStream << "   elevation_step " << mTargetRegion.mElevationAngleStep << "\n";
   if (mTargetRegion.mMaxRange > 0.0) // use default if not > 0.0
   {
      aStream << "   maximum_range " << mTargetRegion.mMaxRange << "\n";
   }
   aStream << "   large_range_step " << mTargetRegion.mLargeRangeStep << "\n";
   aStream << "   small_range_step " << mTargetRegion.mSmallRangeStep << "\n";
}

bool SPLAT::VerticalCoverageTargetRegionWidget::ApplyChanges()
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

void SPLAT::VerticalCoverageTargetRegionWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::VerticalCoverageTargetRegionWidget::Initialize()
{
   mTargetRegion = TargetRegion();
}

void SPLAT::VerticalCoverageTargetRegionWidget::Populate()
{
   mUi.mElevationAngleMinEdit->SetValue(mTargetRegion.mMinElevationAngle);
   mUi.mElevationAngleMaxEdit->SetValue(mTargetRegion.mMaxElevationAngle);
   mUi.mElevationAngleStepEdit->SetValue(mTargetRegion.mElevationAngleStep);

   mUi.mRangeMaxEdit->SetValue(mTargetRegion.mMaxRange);
   mUi.mStepSmallEdit->SetValue(mTargetRegion.mSmallRangeStep);
   mUi.mStepLargeEdit->SetValue(mTargetRegion.mLargeRangeStep);
}

void SPLAT::VerticalCoverageTargetRegionWidget::SetTargetRegion()
{
   mTargetRegion.mMinElevationAngle  = mUi.mElevationAngleMinEdit->GetValue();
   mTargetRegion.mMaxElevationAngle  = mUi.mElevationAngleMaxEdit->GetValue();
   mTargetRegion.mElevationAngleStep = mUi.mElevationAngleStepEdit->GetValue();

   mTargetRegion.mMaxRange       = mUi.mRangeMaxEdit->GetValue();
   mTargetRegion.mSmallRangeStep = mUi.mStepSmallEdit->GetValue();
   mTargetRegion.mLargeRangeStep = mUi.mStepLargeEdit->GetValue();
}

bool SPLAT::VerticalCoverageTargetRegionWidget::TargetRegionValid()
{
   return mUi.mElevationAngleMinEdit->IsValid() && mUi.mElevationAngleMaxEdit->IsValid() &&
          mUi.mElevationAngleStepEdit->IsValid() && mUi.mRangeMaxEdit->IsValid() && mUi.mStepSmallEdit->IsValid() &&
          mUi.mStepLargeEdit->IsValid();
}
