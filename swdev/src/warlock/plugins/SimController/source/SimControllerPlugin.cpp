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
#include "SimControllerPlugin.hpp"

#include <QMenu>
#include <QStatusBar>

#include "SimControllerSimCommands.hpp"
#include "SimControllerUpdater.hpp"
#include "UtMemory.hpp"
#include "WkRunManager.hpp"
#include "WkStartupDialog.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkSimController::Plugin,
   "Simulation Controller",
   "The Simulation Controller plugin provides a toolbar for controlling the advancement of time in an AFSIM "
   "simulation, including the ability to pause/resume, terminate, restart and set the clock rate of the simulation. "
   "Also displays the status of the simulation in the status bar.",
   "warlock")

WkSimController::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new PrefWidget)
{
   mStatusWidgetPtr = new StatusWidget(mSimulationState, mPrefWidgetPtr->GetPreferenceObject(), aUniqueId);

   // Create and add toolbar to main window
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mToolbarPtr = new Toolbar(GetInterface(), mSimulationState, mPrefWidgetPtr->GetPreferenceObject());
   mainWindowPtr->AddToolBar(mToolbarPtr);

   // Add status bar widget
   mainWindowPtr->statusBar()->addPermanentWidget(mStatusWidgetPtr);

   AddLoadScenarioActionsToFileMenu();

   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &PrefObject::SendDIS_PDUsChanged,
           mToolbarPtr.data(),
           &Toolbar::SetSendDIS_PDUs);
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &PrefObject::FallingBehindThresholdChanged,
           mInterfacePtr.get(),
           &SimInterface::OnFallingBehindThresholdChanged);
   connect(&mSimulationState, &SimControllerDataContainer::RemoteSimulationBehind, this, &Plugin::RemoteSimulationBehindHandler);
   connect(mToolbarPtr.data(), &Toolbar::RequestStartupDialog, this, &Plugin::StartupDialogRequested);
   connect(&warlock::RunManager::GetInstance(),
           &warlock::RunManager::RecentScenarioListChanged,
           this,
           [=]()
           {
              mRecentScenariosMenu->clear();
              PopulateRecentMenu();
           });
}

QList<wkf::PrefWidget*> WkSimController::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

QList<wkf::Action*> WkSimController::Plugin::GetActions() const
{
   QList<wkf::Action*> actions;
   foreach (QAction* a, mToolbarPtr->actions())
   {
      wkf::Action* wkAction = qobject_cast<wkf::Action*>(a);
      if (wkAction)
      {
         actions.push_back(wkAction);
      }
   }
   return actions;
}

void WkSimController::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mSimulationState);
   mStatusWidgetPtr->Update();
}

void WkSimController::Plugin::LoadActionHandler()
{
   QStringList files = wkf::getOpenFileNames(nullptr,
                                             "Select Scenario File(s)",
                                             warlock::RunManager::GetInstance().GetMostRecentDirectory(),
                                             "AFSIM (*.txt);; All Files (*.*)");
   if (!files.empty())
   {
      warlock::RunManager::GetInstance().LoadScenario(files);
   }
}

void WkSimController::Plugin::StartupDialogRequested()
{
   WsfStandardApplication::Options options;
   QStringList                     inputFiles;
   warlock::StartupDialog          dialog(inputFiles, wkfEnv.IsFileBrowsingLocked(), nullptr);
   if (dialog.exec() == QDialog::Accepted)
   {
      for (const QString& file : inputFiles)
      {
         options.mInputFiles.push_back(file.toStdString());
      }
      warlock::RunManager::GetInstance().LoadScenario(options);
   }
}

void WkSimController::Plugin::RecentActionHandler()
{
   auto* action = qobject_cast<QAction*>(sender());
   if (action)
   {
      warlock::RunManager::GetInstance().LoadScenario(action->data().toStringList());
   }
}

void WkSimController::Plugin::RemoteSimulationBehindHandler()
{
   auto prefObject = mPrefWidgetPtr->GetPreferenceObject();
   if (prefObject->GetSendAutoPause())
   {
      mInterfacePtr->AddSimCommand(ut::make_unique<PauseCommand>(prefObject->GetSendDIS_PDUs()));
      QString errorMsg = "A networked simulation has fallen behind and pause command has automatically been generated "
                         "to allow the simulation to catch up.";
      QMessageBox::warning(mToolbarPtr, "Paused", errorMsg);
   }
}

void WkSimController::Plugin::AddLoadScenarioActionsToFileMenu()
{
   // Find the File Menu
   QMenu* fileMenu = wkfEnv.GetMainWindow()->GetMenuByName("File");
   if (fileMenu != nullptr)
   {
      wkf::Action* loadAction = new wkf::Action(QIcon::fromTheme("folder"),
                                                "Load Scenario...",
                                                wkfEnv.GetMainWindow(),
                                                QKeySequence(QKeySequence::Open));
      connect(loadAction, &QAction::triggered, this, &Plugin::LoadActionHandler);

      mRecentScenariosMenu = new QMenu("Recent Scenarios", fileMenu);
      mRecentScenariosMenu->setToolTipsVisible(true);
      PopulateRecentMenu();

      QList<QAction*> actions = fileMenu->actions();
      if (!actions.empty())
      {
         fileMenu->insertAction(actions.front(), loadAction);
         fileMenu->insertMenu(actions.front(), mRecentScenariosMenu);
         fileMenu->insertSeparator(actions.back());
      }
      else // File menu is empty, something is wrong, should at least be "Exit" in the menu
      {
         fileMenu->addAction(loadAction);
         fileMenu->addSeparator();
         fileMenu->addMenu(mRecentScenariosMenu);
      }
   }
}

void WkSimController::Plugin::PopulateRecentMenu()
{
   const warlock::RunManager& runMgr  = warlock::RunManager::GetInstance();
   const auto&                recents = runMgr.GetRecentScenarios();

   foreach (QStringList files, recents)
   {
      QAction* action = new QAction(runMgr.GetScenarioDisplayName(files, true), mRecentScenariosMenu);
      if (files.size() > 1)
      {
         action->setToolTip(files.join('\n'));
      }

      action->setData(files);
      connect(action, &QAction::triggered, this, &Plugin::RecentActionHandler);
      mRecentScenariosMenu->addAction(action);
   }
   if (recents.isEmpty())
   {
      auto* action = mRecentScenariosMenu->addAction("(no recent scenarios)");
      action->setEnabled(false);
   }
}

std::unique_ptr<wkf::Updater> WkSimController::Plugin::GetUpdater(const QString&     aPlatformName,
                                                                  const QString&     aDatum,
                                                                  const unsigned int aContext) const
{
   if (aDatum == "mSimTimeWidget")
   {
      return wkf::make_updater<SimTimeUpdater>();
   }
   else if (aDatum == "mWallTimeWidget")
   {
      return wkf::make_updater<WallTimeUpdater>();
   }
   else if (aDatum == "mClockRateWidget")
   {
      return wkf::make_updater<ClockRateUpdater>();
   }
   return nullptr;
}
