// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef LOGSERVERPREFWIDGET_HPP
#define LOGSERVERPREFWIDGET_HPP

#include "LogServerPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_LogServerPrefWidget.h"

namespace LogServer
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget(QWidget* parent = nullptr);
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override;

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::LogServerPrefWidget mUI;
};
} // namespace LogServer

#endif
