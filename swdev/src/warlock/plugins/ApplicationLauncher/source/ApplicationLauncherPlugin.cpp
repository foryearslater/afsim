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

#include "ApplicationLauncherPlugin.hpp"

#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkApplicationLauncher::Plugin,
                          "Application Launcher",
                          "The Application Launcher plugin provides a toolbar for opening Wizard with the current "
                          "scenario, or opening Mystic with the specified AER recording of the current scenario.",
                          "warlock")

WkApplicationLauncher::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   // Create and add toolbar to main window
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mainWindowPtr->AddToolBar(new Toolbar(GetInterface()));
}
