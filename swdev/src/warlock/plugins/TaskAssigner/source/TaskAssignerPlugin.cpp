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

#include "TaskAssignerPlugin.hpp"

#include "TaskAssignerDockWidget.hpp"
#include "TaskAssignerSimInterface.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkTaskAssigner::Plugin,
                          "Task Assigner",
                          "The Task Assigner plugin provides a dialog that allows a user to construct and send task "
                          "messages between platforms.  This dialog can be found under the View menu.",
                          "warlock")

// ============================================================================
WkTaskAssigner::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   // Create a dock widget and add it to the main window
   mDockWidgetPtr = new WkTaskAssigner::DockWidget(GetInterface(), mData, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();
}

// ============================================================================
void WkTaskAssigner::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mData);

   // Don't bother updating dialog if it is not visible
   if (mDockWidgetPtr->isVisible())
   {
      mDockWidgetPtr->Update();
   }
}
