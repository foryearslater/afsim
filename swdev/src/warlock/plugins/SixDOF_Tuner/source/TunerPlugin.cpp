// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TunerPlugin.hpp"

#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>

#include "TunerCommandDialog.hpp"
#include "TunerMainWidget.hpp"
#include "TunerPrefWidget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkSixDOF_Tuner::Plugin,
                          "SixDOF Tuner",
                          "The SixDOF Tuner plugin adds a Tuner menu to the menu bar that provides interactive tools "
                          "to assist in the tuning of PIDs used in autopilot control of SixDOF platforms.  This plugin "
                          "has a specialized use-case and thus is not loaded by default.",
                          "warlock",
                          false)

WkSixDOF_Tuner::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPreferencesWidget(new PrefWidget())
{
   mInterfacePtr->SetEnabled(false);

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   QMenu*   tunerMenu         = mainWindowPtr->FindOrCreateMenu("SixDOF Tuner");
   QAction* exportAction      = new QAction("Export Settings...", tunerMenu);
   QAction* showTuningAction  = new QAction("Tuning...", tunerMenu);
   QAction* showCommandAction = new QAction("Commands...", tunerMenu);

   connect(mPreferencesWidget->GetPreferenceObject(),
           &WkSixDOF_Tuner::PrefObject::MoverCreatorFileSet,
           this,
           [this]()
           {
              // If launched from Mover Creator and the dialogs have been created yet
              if (!mTunerDockWidget && mPreferencesWidget->GetPreferenceObject()->GetLaunchedFromMoverCreatorFile() != "")
              {
                 CreateWidgets();
              }
           });

   connect(exportAction,
           &QAction::triggered,
           this,
           [this]
           {
              if (!mTunerMainWidget)
              {
                 CreateWidgets();
              }
              mTunerMainWidget->Export();
           });

   connect(showTuningAction,
           &QAction::triggered,
           this,
           [this]
           {
              if (!mTunerDockWidget)
              {
                 CreateWidgets();
              }
              mTunerDockWidget->show();
           });

   connect(showCommandAction,
           &QAction::triggered,
           this,
           [this]
           {
              if (!mCommandDialog)
              {
                 CreateWidgets();
              }
              mCommandDialog->show();
           });

   tunerMenu->addAction(showTuningAction);
   tunerMenu->addAction(showCommandAction);
   tunerMenu->addAction(exportAction);
}

QList<wkf::PrefWidget*> WkSixDOF_Tuner::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> list;
   list.push_back(mPreferencesWidget);
   return list;
}

void WkSixDOF_Tuner::Plugin::CreateWidgets()
{
   auto mainWindowPtr = wkfEnv.GetMainWindow();

   // The Sim interface is disabled until needed, now enable it so the Tuner is usable
   mInterfacePtr->SetEnabled(true);

   // Create Command Dialog
   mCommandDialog = new CommandDialog(GetInterface());
   mCommandDialog->hide();

   // Create Tuner Dock Widget
   mTunerDockWidget = new wkf::DockWidget("TunerDockWidget", mainWindowPtr->centralWidget(), Qt::WindowFlags());
   mTunerDockWidget->setWindowTitle("SixDOF Tuner");

   // Create the main Widget to put in the Dock Widget
   mTunerMainWidget = new MainWidget(GetInterface());
   mTunerDockWidget->setWidget(mTunerMainWidget);
   mTunerDockWidget->hide();

   auto file = mPreferencesWidget->GetPreferenceObject()->GetLaunchedFromMoverCreatorFile();
   // If launched from mover creator
   if (file != "")
   {
      // Set the export file
      mTunerMainWidget->SetExportFile(file);

      // Add a callback when platforms are added so that a platform can be selected automatically.
      mCallbacks.Add(wkf::Observer::PlatformsAdded.Connect(&WkSixDOF_Tuner::Plugin::PlatformsAddedHandler, this));
   }

   connect(mTunerMainWidget, &MainWidget::InitializeAutoTune, this, &Plugin::InitialAutoTuneSettings);
}

void WkSixDOF_Tuner::Plugin::InitialAutoTuneSettings(int aPid)
{
   mInterfacePtr->SetAutoTunePopSize(mPreferencesWidget->GetPopSize((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneMaxGenerations(mPreferencesWidget->GetMaxGenerations((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneCarryOverFrac(mPreferencesWidget->GetCarryOverFrac((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneCrossoverProb(mPreferencesWidget->GetCrossoverProb((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneMutationProb(mPreferencesWidget->GetMutationProb((wsf::six_dof::Pid::Type)aPid));

   double temp1, temp2;
   mPreferencesWidget->GetKpRange((wsf::six_dof::Pid::Type)aPid, temp1, temp2);
   mInterfacePtr->SetAutoTuneKpRange(temp1, temp2);

   mPreferencesWidget->GetKiRange((wsf::six_dof::Pid::Type)aPid, temp1, temp2);
   mInterfacePtr->SetAutoTuneKiRange(temp1, temp2);

   mPreferencesWidget->GetKdRange((wsf::six_dof::Pid::Type)aPid, temp1, temp2);
   mInterfacePtr->SetAutoTuneKdRange(temp1, temp2);

   mInterfacePtr->SetAutoTuneRunTime(mPreferencesWidget->GetRunTime((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneErrorWeight(mPreferencesWidget->GetErrorWeight((wsf::six_dof::Pid::Type)aPid));
   mInterfacePtr->SetAutoTuneOvershootWeight(mPreferencesWidget->GetOvershootWeight((wsf::six_dof::Pid::Type)aPid));

   mInterfacePtr->RunAutoTuneCommand();
}

void WkSixDOF_Tuner::Plugin::GuiUpdate()
{
   if (mInterfacePtr->IsEnabled())
   {
      if (mInterfacePtr->GetSimInitializing())
      {
         // If mTunerDockWidget is defined, mTunerMainWidget is too.
         //  mTunerDockWidget is a PluginUiPointer and thus safer to check than mTunerMainWidget
         if (mTunerDockWidget)
         {
            mTunerMainWidget->SimulationInitializing();
         }
      }

      mInterfacePtr->GuiUpdate();
      // If mTunerDockWidget is defined, mTunerMainWidget is too.
      //  mTunerDockWidget is a PluginUiPointer and thus safer to check than mTunerMainWidget
      if (mTunerDockWidget)
      {
         mTunerMainWidget->UpdateData();
      }
   }
}

void WkSixDOF_Tuner::Plugin::PlatformsAddedHandler(const wkf::PlatformList& aPlatforms)
{
   if (mInterfacePtr->IsEnabled())
   {
      for (const auto& platform : aPlatforms)
      {
         // This name has to match the default name used by Mover Creator.
         if (platform->GetName() == "six_dof_test_platform")
         {
            wkfEnv.SetPlatformsSelected({platform});
         }
      }
   }
}
