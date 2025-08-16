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
#ifndef WKDEMOMODEPREFWIDGET_HPP
#define WKDEMOMODEPREFWIDGET_HPP

#include "DemoModePrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_WkDemoModePrefWidget.h"

namespace WkDemoMode
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();
   ~PrefWidget() override = default;

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::WkPrefWidget mUi;
};
} // namespace WkDemoMode
#endif
