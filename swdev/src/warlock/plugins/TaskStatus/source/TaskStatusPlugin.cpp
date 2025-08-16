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

#include "TaskStatusPlugin.hpp"

#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkTaskStatus::Plugin,
                          "Task Status",
                          "The Task Status plugin provides a dialog, found under the View menu, for cancelling and "
                          "viewing the status of tasks.",
                          "warlock")

// ============================================================================
WkTaskStatus::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   // Create a status dock widget and add it to the main window
   mDockWidgetPtr = new WkTaskStatus::DockWidget(GetInterface(), mData, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::BottomDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();
}

// ============================================================================
void WkTaskStatus::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mData);
}
