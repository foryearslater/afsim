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
#ifndef SIMCONTROLLERPREFWIDGET_HPP
#define SIMCONTROLLERPREFWIDGET_HPP

#include "SimControllerPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_SimControllerPrefWidget.h"

namespace WkSimController
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override { return "Simulation"; }

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::SimControllerPrefWidget mUi;
};
} // namespace WkSimController
#endif // SIMCONTROLLERPREFWIDGET_HPP
