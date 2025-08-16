// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKADHOCSCRIPTBROWSERDIALOG_HPP
#define WKADHOCSCRIPTBROWSERDIALOG_HPP

#include <memory>

#include <QDockWidget>

#include "WkScriptSimInterface.hpp"
#include "WkfPlatform.hpp"
#include "ui_WkAdHocScriptBrowserDockWidget.h"

namespace WkAdHocScriptBrowser
{
class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   DockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

private:
   void showEvent(QShowEvent* aEvent) override;

   void OnExecuteClicked();
   void PlatformOfInterestChanged(wkf::Platform* aPlatform);

   std::shared_ptr<warlock::ScriptSimInterface> mSimInterfacePtr;

   Ui::AdHocScriptDockWidget mUI;

   unsigned int mScriptCount{1};
};
} // namespace WkAdHocScriptBrowser
#endif
