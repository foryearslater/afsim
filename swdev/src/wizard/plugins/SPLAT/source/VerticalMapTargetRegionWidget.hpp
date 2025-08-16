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

#ifndef VERTICALMAPTARGETREGIONWIDGET_HPP
#define VERTICALMAPTARGETREGIONWIDGET_HPP

#include <QWidget>

#include "PlotOptionsWidget.hpp"
#include "UtLength.hpp"
#include "UtMath.hpp"
#include "UtUnitTypes.hpp"
#include "ui_VerticalMapTargetRegionWidget.h"

namespace SPLAT
{
class VerticalMapTargetRegionWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   struct TargetRegion
   {
      UtLengthValue mMinGroundRange;
      UtLengthValue mMaxGroundRange;
      UtLengthValue mStepGroundRange;

      UtLengthValue mMinAltitude;
      UtLengthValue mMaxAltitude;
      UtLengthValue mStepAltitude;

      TargetRegion()
         : mMinGroundRange(UtLength(0, UtLength::FmtNM))
         , mMaxGroundRange(UtLength(120 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mStepGroundRange(UtLength(0.5 * UtMath::cM_PER_NM, UtLength::FmtNM))
         , mMinAltitude(UtLength(0, UtLength::FmtFT))
         , mMaxAltitude(UtLength(250000 * UtMath::cM_PER_FT, UtLength::FmtFT))
         , mStepAltitude(UtLength(500 * UtMath::cM_PER_FT, UtLength::FmtFT))
      {
      }
   };

   VerticalMapTargetRegionWidget(PlotOptionsDialog* aParent);
   ~VerticalMapTargetRegionWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

private:
   void Initialize();
   void Populate();
   void SetTargetRegion();
   bool TargetRegionValid();

   Ui::VerticalMapTargetRegionWidget mUi;
   TargetRegion                      mTargetRegion;
};
} // namespace SPLAT

#endif
