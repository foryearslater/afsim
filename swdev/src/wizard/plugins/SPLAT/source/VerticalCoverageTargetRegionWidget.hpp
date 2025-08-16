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

#ifndef VERTICALCOVERAGETARGETREGIONWIDGET_HPP
#define VERTICALCOVERAGETARGETREGIONWIDGET_HPP

#include <QWidget>

#include "PlotOptionsWidget.hpp"
#include "UtUnitTypes.hpp"
#include "ui_VerticalCoverageTargetRegionWidget.h"

namespace SPLAT
{
class VerticalCoverageTargetRegionWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   struct TargetRegion
   {
      UtAngleValue mMinElevationAngle;
      UtAngleValue mMaxElevationAngle;
      UtAngleValue mElevationAngleStep;

      UtLengthValue mMaxRange;
      UtLengthValue mSmallRangeStep;
      UtLengthValue mLargeRangeStep;

      TargetRegion()
         : mMinElevationAngle(UtAngleValue(-5.0, UtUnitAngle::cDEGREES))
         , mMaxElevationAngle(UtAngleValue(89.9, UtUnitAngle::cDEGREES))
         , mElevationAngleStep(UtAngleValue(0.1, UtUnitAngle::cDEGREES))
         , mMaxRange(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mSmallRangeStep(UtLengthValue(100.0, UtUnitLength::cMETERS))
         , mLargeRangeStep(UtLengthValue(5000.0, UtUnitLength::cMETERS))
      {
      }
   };

   VerticalCoverageTargetRegionWidget(PlotOptionsDialog* aParent);
   ~VerticalCoverageTargetRegionWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

private:
   void Initialize();
   void Populate();
   void SetTargetRegion();
   bool TargetRegionValid();

   Ui::VerticalCoverageTargetRegionWidget mUi;
   TargetRegion                           mTargetRegion;
};
} // namespace SPLAT
#endif
