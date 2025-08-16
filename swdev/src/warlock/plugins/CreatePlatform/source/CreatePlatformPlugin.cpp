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

#include "CreatePlatformPlugin.hpp"

#include "CreatePlatformDockWidget.hpp"
#include "CreatePlatformSimInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkCreatePlatform::Plugin,
                          "Create Platform",
                          "The Create Platform plugin provides a simple dialog, accessed through the View menu, to "
                          "create platforms during the execution of a simulation.",
                          "warlock")

WkCreatePlatform::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(GetInterface(), mCreatePlatformData, mainWindowPtr);

   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();
}

void WkCreatePlatform::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mCreatePlatformData);
}