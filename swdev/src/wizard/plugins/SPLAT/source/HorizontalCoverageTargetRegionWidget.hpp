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

#ifndef HORIZONTALCOVERAGETARGETREGIONWIDGET_HPP
#define HORIZONTALCOVERAGETARGETREGIONWIDGET_HPP

#include <QWidget>

#include "PlotOptionsWidget.hpp"
#include "UtUnitTypes.hpp"
#include "ui_HorizontalCoverageTargetRegionWidget.h"

namespace SPLAT
{
class HorizontalCoverageTargetRegionWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   struct TargetRegion
   {
      UtLengthValue mAltitude;
      UtLengthValue mAltitudeStart;
      UtLengthValue mAltitudeEnd;
      UtLengthValue mAltitudeStep;

      UtAngleValue mMinAzimuthAngle;
      UtAngleValue mMaxAzimuthAngle;
      UtAngleValue mAzimuthAngleStep;

      UtLengthValue mMinRange;
      UtLengthValue mMaxRange;
      UtLengthValue mSmallRangeStep;
      UtLengthValue mLargeRangeStep;

      TargetRegion()
         : mAltitude(UtLengthValue(25000.0, UtUnitLength::cFEET))
         , mAltitudeStart(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mAltitudeEnd(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mAltitudeStep(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mMinAzimuthAngle(UtAngleValue(-180.0, UtUnitAngle::cDEGREES))
         , mMaxAzimuthAngle(UtAngleValue(180.0, UtUnitAngle::cDEGREES))
         , mAzimuthAngleStep(UtAngleValue(0.1, UtUnitAngle::cDEGREES))
         , mMinRange(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mMaxRange(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mSmallRangeStep(UtLengthValue(100.0, UtUnitLength::cMETERS))
         , mLargeRangeStep(UtLengthValue(5000.0, UtUnitLength::cMETERS))
      {
      }
   };

   HorizontalCoverageTargetRegionWidget(PlotOptionsDialog* aParent);
   ~HorizontalCoverageTargetRegionWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

private:
   void Initialize();
   void Populate();
   void Connect();
   void SetTargetRegion();
   bool TargetRegionValid();
   void AltitudeButtonToggled(int aId, bool aChecked);

   Ui::HorizontalCoverageTargetRegionWidget mUi;
   TargetRegion                             mTargetRegion;
};
} // namespace SPLAT
#endif
