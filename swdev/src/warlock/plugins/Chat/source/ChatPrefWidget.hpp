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

#ifndef CHATPREFWIDGET_HPP
#define CHATPREFWIDGET_HPP

#include "ChatPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_ChatPrefWidget.h"

namespace Chat
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT

public:
   PrefWidget();
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override { return "Application"; }

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::ChatPrefWidget mUi;
};
} // namespace Chat
#endif
