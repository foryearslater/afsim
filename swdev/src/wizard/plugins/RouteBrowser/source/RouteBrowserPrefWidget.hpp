// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ROUTEBROWSERPREFWIDGET_HPP
#define ROUTEBROWSERPREFWIDGET_HPP

#include "RouteBrowserPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_RouteBrowserPrefWidget.h"

namespace RouteBrowser
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget(QWidget* parent = nullptr);
   ~PrefWidget() override = default;

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::RouteBrowserPrefWidget mUI;
};
} // namespace RouteBrowser

#endif
