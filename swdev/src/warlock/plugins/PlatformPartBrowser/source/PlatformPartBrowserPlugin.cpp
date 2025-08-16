// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserPlugin.hpp"

#include "PlatformPartBrowserDockWidget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkPlatformPartBrowser::Plugin,
                          "Platform Part Browser",
                          "The Platform Part Browser plugin creates a dialog that displays a list of platform parts "
                          "for the selected platform and provides basic controls for the desired part.",
                          "warlock")

WkPlatformPartBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   // Create a dock widget and add it to the main window
   mDockWidget = new WkPlatformPartBrowser::DockWidget(GetInterface(), mData, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();
}

void WkPlatformPartBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mData);
}