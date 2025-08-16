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

#ifndef OUTPUTVARIABLESWIDGET_HPP
#define OUTPUTVARIABLESWIDGET_HPP

#include <QWidget>

#include "PlotOptionsDialog.hpp"
#include "PlotOptionsWidget.hpp"
#include "ui_OutputVariablesWidget.h"

namespace SPLAT
{
class OutputVariablesWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   OutputVariablesWidget(PlotOptionsDialog* aParent);
   ~OutputVariablesWidget() override = default;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

   QStringList& GetSelectedVariables() { return mSelectedVariables; }

signals:
   void RequiredJammingPowerPressed(bool aChecked);

private:
   void Initialize();
   void Connect();
   void SetVariables();

   Ui::OutputVariablesWidget mUi;
   QStringList               mSelectedVariables;
};
} // namespace SPLAT
#endif
