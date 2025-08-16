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

#ifndef PREFERENCESDIALOG_HPP
#define PREFERENCESDIALOG_HPP

#include "BackupPrefWidget.hpp"
#include "EditorPrefWidget.hpp"
#include "NewFilePrefWidget.hpp"
#include "ToolPrefWidget.hpp"
#include "WkfPreferencesDialog.hpp"

namespace wizard
{
class PreferencesDialog : public wkf::PreferencesDialog
{
public:
   PreferencesDialog(wkf::MainWindow* parent, Qt::WindowFlags f = Qt::WindowFlags())
      : wkf::PreferencesDialog(parent, f)
   {
      mStandardWidgets << new BackupPrefWidget(this) << new ToolPrefWidget(this);
   }

private:
   void Initialize() override
   {
      wkf::PreferencesDialog::Initialize();
      // Defer construction of editor pref widget/object until Initialize to allow UiResources to be constructed
      mStandardWidgets << new EditorPrefWidget(this);
      mStandardWidgets << new NewFilePrefWidget(this);
   }
};
} // namespace wizard
#endif
