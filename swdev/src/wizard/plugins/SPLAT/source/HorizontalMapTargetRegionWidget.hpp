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

#ifndef HORIZONTALMAPTARGETREGIONWIDGET_HPP
#define HORIZONTALMAPTARGETREGIONWIDGET_HPP

#include <QWidget>

#include "PlotOptionsWidget.hpp"
#include "UtLatPos.hpp"
#include "UtLength.hpp"
#include "UtLonPos.hpp"
#include "UtMath.hpp"
#include "UtUnitTypes.hpp"
#include "ui_HorizontalMapTargetRegionWidget.h"

namespace SPLAT
{
class HorizontalMapTargetRegionWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   enum TargetRegionType
   {
      cLATLON,
      cRANGE,
      cINVALID
   };

   struct TargetRegionRange
   {
      UtLengthValue mMinCross;
      UtLengthValue mMaxCross;
      UtLengthValue mStepCross;

      UtLengthValue mMinDown;
      UtLengthValue mMaxDown;
      UtLengthValue mStepDown;

      TargetRegionRange()
         : mMinCross(UtLength(-120 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mMaxCross(UtLength(120 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mStepCross(UtLength(1 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mMinDown(UtLength(-120 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mMaxDown(UtLength(120 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mStepDown(UtLength(1 * UtMath::cM_PER_NM, UtLength::FmtNM))
      {
      }
   };

   struct TargetRegionLatLon
   {
      UtLatPos mMinLat;
      UtLatPos mMaxLat;
      double   mStepLat;

      UtLonPos mMinLon;
      UtLonPos mMaxLon;
      double   mStepLon;

      bool mRelative;

      TargetRegionLatLon()
         : mMinLat(-1)
         , mMaxLat(1)
         , mStepLat(1)
         , mMinLon(-1)
         , mMaxLon(1)
         , mStepLon(1)
         , mRelative(false)
      {
      }
   };

   HorizontalMapTargetRegionWidget(PlotOptionsDialog* aParent);
   ~HorizontalMapTargetRegionWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

private:
   void Initialize();
   void Populate();
   void Connect();
   void SetTargetRegion();
   bool TargetRegionRangeValid();
   bool TargetRegionLatLonValid();

   Ui::HorizontalMapTargetRegionWidget mUi;
   TargetRegionType                    mTargetRegionType;
   TargetRegionLatLon                  mTargetRegionLatLon;
   TargetRegionRange                   mTargetRegionRange;
};
} // namespace SPLAT

#endif // !HORIZONTALMAPTARGETREGIONWIDGET_HPP
