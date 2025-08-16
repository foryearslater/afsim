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

#include "VerticalMapTargetRegionWidget.hpp"

#include <QMessageBox>

SPLAT::VerticalMapTargetRegionWidget::VerticalMapTargetRegionWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
{
   mUi.setupUi(this);
   Initialize();
   Populate();
}

void SPLAT::VerticalMapTargetRegionWidget::WriteData(std::ostream& aStream)
{
   aStream << "   ground_range_limits " << mTargetRegion.mMinGroundRange << " " << mTargetRegion.mMaxGroundRange << "\n";
   aStream << "   ground_range_step " << mTargetRegion.mStepGroundRange << "\n";
   aStream << "   altitude_limits " << mTargetRegion.mMinAltitude << " " << mTargetRegion.mMaxAltitude << "\n";
   aStream << "   altitude_step " << mTargetRegion.mStepAltitude << "\n";
   aStream << "\n";
}

bool SPLAT::VerticalMapTargetRegionWidget::ApplyChanges()
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

void SPLAT::VerticalMapTargetRegionWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::VerticalMapTargetRegionWidget::Initialize()
{
   mTargetRegion = TargetRegion();
}

void SPLAT::VerticalMapTargetRegionWidget::Populate()
{
   mUi.mGroundRangeMinEdit->SetValue(mTargetRegion.mMinGroundRange);
   mUi.mGroundRangeMaxEdit->SetValue(mTargetRegion.mMaxGroundRange);
   mUi.mGroundRangeStepEdit->SetValue(mTargetRegion.mStepGroundRange);

   mUi.mAltitudeMinEdit->SetValue(mTargetRegion.mMinAltitude);
   mUi.mAltitudeMaxEdit->SetValue(mTargetRegion.mMaxAltitude);
   mUi.mAltitudeStepEdit->SetValue(mTargetRegion.mStepAltitude);
}

void SPLAT::VerticalMapTargetRegionWidget::SetTargetRegion()
{
   mTargetRegion.mMinGroundRange  = mUi.mGroundRangeMinEdit->GetValue();
   mTargetRegion.mMaxGroundRange  = mUi.mGroundRangeMaxEdit->GetValue();
   mTargetRegion.mStepGroundRange = mUi.mGroundRangeStepEdit->GetValue();

   mTargetRegion.mMinAltitude  = mUi.mAltitudeMinEdit->GetValue();
   mTargetRegion.mMaxAltitude  = mUi.mAltitudeMaxEdit->GetValue();
   mTargetRegion.mStepAltitude = mUi.mAltitudeStepEdit->GetValue();
}

bool SPLAT::VerticalMapTargetRegionWidget::TargetRegionValid()
{
   return mUi.mGroundRangeMinEdit->IsValid() && mUi.mGroundRangeMaxEdit->IsValid() &&
          mUi.mGroundRangeStepEdit->IsValid() && mUi.mAltitudeMinEdit->IsValid() && mUi.mAltitudeMaxEdit->IsValid() &&
          mUi.mAltitudeStepEdit->IsValid();
}