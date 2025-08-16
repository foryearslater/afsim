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

#ifndef PLOTOPTIONSWIDGET_HPP
#define PLOTOPTIONSWIDGET_HPP

#include <QWidget>

namespace SPLAT
{
class PlotOptionsDialog;

class PlotOptionsWidget : public QWidget
{
   Q_OBJECT
public:
   PlotOptionsWidget(PlotOptionsDialog* aParent);
   ~PlotOptionsWidget() override = default;
   virtual void WriteData(std::ostream& aStream) {}
   virtual bool OkClicked() { return true; }
   virtual bool ApplyChanges()    = 0;
   virtual void RestoreDefaults() = 0;

   enum PlotType
   {
      cHORIZONTAL_COVERAGE,
      cHORIZONTAL_MAP,
      cSPHERICAL_MAP,
      cVERTICAL_COVERAGE,
      cVERTICAL_MAP
   };

protected:
   PlotOptionsDialog* mParentDialog;
};
} // namespace SPLAT

#endif
