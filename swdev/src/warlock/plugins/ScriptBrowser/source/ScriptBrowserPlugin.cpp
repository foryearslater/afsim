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
#include "ScriptBrowserPlugin.hpp"

#include <QLabel>
#include <QMenu>

#include "ScriptBrowserDockWidget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkScriptBrowser::Plugin,
   "Script Browser",
   "The Script Browser plugin provides a dialog for executing scripts prefixed with the string \"WARLOCK_\" in an "
   "AFSIM simulation. Scripts can be defined either on a platform or in the global script context.",
   "warlock")

WkScriptBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   // Create a dock widget and add it to the main window
   mDockWidget = new DockWidget(mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();
}

void WkScriptBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*mDockWidget);
}
