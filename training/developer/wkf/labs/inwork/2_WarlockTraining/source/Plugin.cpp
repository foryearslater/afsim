// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Plugin.hpp"

#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

// This line registers our plugin with WKF.
// The arguments are:
// 1) The plugin to register.
// 2) The name of the plugin.
// 3) A description of the plugin.
// 4) Which AFSIM application the plugin is for.
WKF_PLUGIN_DEFINE_SYMBOLS(WarlockTraining::Plugin, "Warlock Training", "Displays information for the platform of interest.", "warlock")

WarlockTraining::Plugin::Plugin(const QString& aPluginName,
                                const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidget(new PrefWidget())
   , mDockWidget(new DockWidget(*mInterfacePtr, mDataContainer, mPrefWidget->GetPreferenceObject()))
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);

   //Create actions and assign them default hot-keys
   //These actions will be stored in mActions and passed by the WKF via the GetPreferencesWidgets() override
   wkf::Action* northActionPtr = new wkf::Action("Turn North", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_Up));
   connect(northActionPtr, &QAction::triggered, this, [this]() { mDockWidget->TurnToHeading(0); });
   mActions.push_back(northActionPtr);

   wkf::Action* eastActionPtr = new wkf::Action("Turn East", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_Right));
   connect(eastActionPtr, &QAction::triggered, this, [this]() { mDockWidget->TurnToHeading(90); });
   mActions.push_back(eastActionPtr);

   wkf::Action* southActionPtr = new wkf::Action("Turn South", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_Down));
   connect(southActionPtr, &QAction::triggered, this, [this]() { mDockWidget->TurnToHeading(180); });
   mActions.push_back(southActionPtr);

   wkf::Action* westActionPtr = new wkf::Action("Turn West", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_Left));
   connect(westActionPtr, &QAction::triggered, this, [this]() { mDockWidget->TurnToHeading(270); });
   mActions.push_back(westActionPtr);
}

void WarlockTraining::Plugin::GuiUpdate()
{
   // EXERCISE 1 TASK 1d
   // Call ProcessEvents on the SimInterface so that we can process the SimEvents we have created
   // Remember the argument to ProcessEvents is the DataContainer
   // PLACE YOUR CODE HERE
}

QList<wkf::PrefWidget*> WarlockTraining::Plugin::GetPreferencesWidgets() const
{
   // EXERCISE 2 TASK 1a
   // return a QList that contains the PrefWidget. This will add the widget to the Preferences display.
   // PLACE YOUR CODE HERE, instead of the following line
   return QList<wkf::PrefWidget*>();
}

QList<wkf::Action*> WarlockTraining::Plugin::GetActions() const
{
   // EXERCISE 2 TASK 1b
   // return the QList of Actions. This will add the actions to the Preferences' KeyBinding menu.
   // PLACE YOUR CODE HERE, instead of the following line
   return QList<wkf::Action*>();
}
