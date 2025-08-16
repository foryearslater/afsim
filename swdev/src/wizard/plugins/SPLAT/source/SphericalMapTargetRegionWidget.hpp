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

#ifndef SphericalMapTargetRegionWidget_HPP
#define SphericalMapTargetRegionWidget_HPP

#include <QWidget>

#include "PlotOptionsWidget.hpp"
#include "UtUnitTypes.hpp"
#include "ui_SphericalMapTargetRegionWidget.h"

namespace SPLAT
{
class SphericalMapTargetRegionWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   struct TargetRegion
   {
      UtLengthValue mAltitude;
      UtLengthValue mRange;

      UtAngleValue mMinAzimuthAngle;
      UtAngleValue mMaxAzimuthAngle;
      UtAngleValue mAzimuthAngleStep;

      UtAngleValue mMinElevationAngle;
      UtAngleValue mMaxElevationAngle;
      UtAngleValue mElevationAngleStep;

      TargetRegion()
         : mAltitude(UtLengthValue(25000.0, UtUnitLength::cFEET))
         , mRange(UtLengthValue(0.0, UtUnitLength::cMETERS))
         , mMinAzimuthAngle(UtAngleValue(-180.0, UtUnitAngle::cDEGREES))
         , mMaxAzimuthAngle(UtAngleValue(180.0, UtUnitAngle::cDEGREES))
         , mAzimuthAngleStep(UtAngleValue(1.0, UtUnitAngle::cDEGREES))
         , mMinElevationAngle(UtAngleValue(-90.0, UtUnitAngle::cDEGREES))
         , mMaxElevationAngle(UtAngleValue(90.0, UtUnitAngle::cDEGREES))
         , mElevationAngleStep(UtAngleValue(1.0, UtUnitAngle::cDEGREES))
      {
      }
   };

   SphericalMapTargetRegionWidget(PlotOptionsDialog* aParent);
   ~SphericalMapTargetRegionWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

private:
   void Initialize();
   void Populate();
   void Connect();
   void SetTargetRegion();
   bool TargetRegionValid();
   void AltitudeRangeButtonToggled(int aId, bool aChecked);

   Ui::SphericalMapTargetRegionWidget mUi;
   TargetRegion                       mTargetRegion;
};
} // namespace SPLAT
#endif
