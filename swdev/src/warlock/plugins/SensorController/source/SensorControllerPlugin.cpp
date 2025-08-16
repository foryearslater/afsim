// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "SensorControllerPlugin.hpp"

#include "SensorControllerSimInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkSensorController::Plugin,
   "Sensor Controller",
   "The Sensor Controller plugin allows the user to command a sensor to slew to a az/el or track.",
   "warlock")

WkSensorController::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(GetInterface(), mDataContainer, mainWindowPtr);

   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();
}

void WkSensorController::Plugin::GuiUpdate()
{
   GetInterface()->ProcessEvents(mDataContainer);
}
