// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKFSIMULATIONPREFWIDGET_HPP
#define WKFSIMULATIONPREFWIDGET_HPP

#include "WkSimPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_WkSimPrefWidget.h"

namespace warlock
{
class SimPrefWidget : public wkf::PrefWidgetT<SimPrefObject>
{
public:
   SimPrefWidget(QWidget* aParent = nullptr);

   QString GetCategoryHint() const override { return "Simulation"; }

private:
   bool ValidateInputs();

   void MulticastHelpButton();
   void InterfaceHelpButton();

   void showEvent(QShowEvent*) override;
   void ReadPreferenceData(const SimPrefData& aPrefData) override;
   void WritePreferenceData(SimPrefData& ba) override;

   Ui::WkSimPrefWidget mUi;
};
} // namespace warlock

#endif
