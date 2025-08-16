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
#include "NetworkBrowserPlugin.hpp"

#include <QMenu>

#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkNetworkBrowser::Plugin,
   "Network Browser",
   "The Network Browser plugin creates a dialog, accessed from the View menu, that displays information related to "
   "network connections. It also populates the Network section of the Platform Details dialog.",
   "warlock")

WkNetworkBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mNetworkData()
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   // Create a dock widget and add it to the main window
   mDockWidget = new DockWidget(mNetworkData, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();
}

void WkNetworkBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mNetworkData);
}
