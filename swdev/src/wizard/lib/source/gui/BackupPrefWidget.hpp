// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef BACKUPPREFWIDGET_HPP
#define BACKUPPREFWIDGET_HPP

#include "BackupPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_BackupPrefWidget.h"

namespace wizard
{
class BackupPrefWidget : public wkf::PrefWidgetT<BackupPrefObject>
{
public:
   BackupPrefWidget(QWidget* parent = nullptr);
   ~BackupPrefWidget() override = default;

   QString GetCategoryHint() const override { return "Application"; }

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const BackupPrefData& aPrefData) override;
   void WritePreferenceData(BackupPrefData& aPrefData) override;

   Ui::BackupPrefWidget mUi;
};
} // namespace wizard

#endif
