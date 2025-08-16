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

#include "SimulationManagerPlugin.hpp"

#include <QApplication>
#include <QMenu>
#include <QTextBrowser>
#include <QTextCursor>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "EditorPrefObject.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ScriptBreakpointControl.hpp"
#include "Signals.hpp"
#include "UiResources.hpp"
#include "WkfMainWindow.hpp"
#include "WsfConnector.hpp"
#include "WsfExe.hpp"
#include "WsfExeManager.hpp"
#include "WsfExecution.hpp"
#include "WsfScriptDebugger.hpp"
#include "WsfScriptDebuggerCallstackControl.hpp"
#include "WsfScriptDebuggerWatchControl.hpp"
#include "logging/WkfLogConsolePrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(SimulationManager::Plugin,
                          "Simulation Manager",
                          "Executes the simulation, provides interfaces to identify simulations",
                          "wizard")

SimulationManager::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mPath("")
   , mDebuggerPtr(nullptr)
{
   mPrefWidgets << new PrefWidget;                  // Sim Execution
   mPrefWidgets << new wkf::log::ConsolePrefWidget; // Output Window

   mRestart = false;

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mRunToolbar                    = new Toolbar(mainWindowPtr);
   mainWindowPtr->AddToolBar(mRunToolbar);

   mOutputPanel = new OutputDock(wkfEnv.GetMainWindow());
   mainWindowPtr->addDockWidget(Qt::BottomDockWidgetArea, mOutputPanel);

   connect(wkfEnv.GetPreferenceObject<PrefObject>(), &PrefObject::ActiveSimChanged, this, &Plugin::ActiveSimChangedCB);
   connect(mRunToolbar, &Toolbar::LinkActivated, this, &Plugin::EvokePreferences);

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &Plugin::ProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &Plugin::ProjectClosed);

   mUtCallbacks += wizSignals->DebugSessionActive.Connect(&Plugin::DebugSessionState, this);

   mRunAction = new wkf::Action(QIcon::fromTheme("run"), QString("Run"), this, QKeySequence("F5"));
   mRunAction->setEnabled(false);
   mDebugAction = new wkf::Action(QIcon::fromTheme("run_debug"), QString("Run Debug"), this, QKeySequence("Ctrl+F5"));
   mDebugAction->setEnabled(false);

   mContinueRealtimeAction =
      new wkf::Action(QIcon::fromTheme("play"), QString("Continue Realtime"), this, QKeySequence("Ctrl+F5"));
   mContinueRealtimeAction->setEnabled(false);
   mContinueRealtimeAction->setVisible(false);
   mContinueNonRealtimeAction =
      new wkf::Action(QIcon::fromTheme("fast_forward"), QString("Continue Non-Realtime"), this, QKeySequence("F5"));
   mContinueNonRealtimeAction->setEnabled(false);
   mContinueNonRealtimeAction->setVisible(false);
   mSeparator1 = mRunToolbar->addSeparator();
   mSeparator1->setVisible(false);

   mPauseAction = new wkf::Action(QIcon::fromTheme("pause"), QString("Break All"), this, QKeySequence("Pause"));
   mPauseAction->setEnabled(false);
   mPauseAction->setVisible(false);
   mStopAction = new wkf::Action(QIcon::fromTheme("stop"), QString("Stop Debugging"), this, QKeySequence("Shift+F5"));
   mStopAction->setEnabled(false);
   mStopAction->setVisible(false);
   mRestartAction =
      new wkf::Action(QIcon::fromTheme("restart"), QString("Restart Run"), this, QKeySequence("Ctrl+Shift+F5"));
   mRestartAction->setEnabled(false);
   mRestartAction->setVisible(false);
   mSeparator2 = mRunToolbar->addSeparator();
   mSeparator2->setVisible(false);

   mStepIntoAction = new wkf::Action(QIcon::fromTheme("step_into"), QString("Step Into"), this, QKeySequence("F11"));
   mStepIntoAction->setEnabled(false);
   mStepIntoAction->setVisible(false);
   mStepOverAction = new wkf::Action(QIcon::fromTheme("step_over"), QString("Step Over"), this, QKeySequence("F10"));
   mStepOverAction->setEnabled(false);
   mStepOverAction->setVisible(false);
   mStepOutAction = new wkf::Action(QIcon::fromTheme("step_out"), QString("Step Out"), this, QKeySequence("Shift+F11"));
   mStepOutAction->setEnabled(false);
   mStepOutAction->setVisible(false);

   mToggleBreakpointAction =
      new wkf::Action(QIcon::fromTheme("stop_sign"), QString("Toggle Breakpoint"), this, QKeySequence("F9"));
   mDeleteAllBreakpointsAction = new wkf::Action(QString("Delete All Breakpoints"), this, QKeySequence("Ctrl+Shift+F9"));
   mDisableAllBreakpointsAction = new wkf::Action(QString("Disable All Breakpoints"), this, Qt::WindowShortcut);

   mDebugSettings = new wkf::Action(QString("Debugger Settings..."), this, Qt::WindowShortcut);

   connect(mRunAction, &QAction::triggered, [this]() { Run(false); });
   connect(mDebugAction, &QAction::triggered, [this]() { Run(true); });
   connect(mContinueRealtimeAction, &QAction::triggered, [this]() { ContinueRealtime(); });
   connect(mContinueNonRealtimeAction, &QAction::triggered, [this]() { ContinueNonRealtime(); });
   connect(mPauseAction, &QAction::triggered, [this]() { Pause(); });
   connect(mStopAction, &QAction::triggered, [this]() { Stop(); });
   connect(mRestartAction, &QAction::triggered, [this]() { Restart(); });
   connect(mStepIntoAction, &QAction::triggered, [this]() { StepInto(); });
   connect(mStepOutAction, &QAction::triggered, [this]() { StepOut(); });
   connect(mStepOverAction, &QAction::triggered, [this]() { StepOver(); });
   connect(mToggleBreakpointAction, &QAction::triggered, [this]() { ToggleBreakpoint(); });
   connect(mDeleteAllBreakpointsAction, &QAction::triggered, [this]() { DeleteAllBreakpoints(); });
   connect(mDebugSettings, &QAction::triggered, [this]() { DebugSettings(); });

   QMenu* runMenu = mainWindowPtr->FindOrCreateMenu("Run");

   runMenu->addAction(mRunAction);
   runMenu->addAction(mDebugAction);

   runMenu->addAction(mContinueRealtimeAction);
   runMenu->addAction(mContinueNonRealtimeAction);
   runMenu->addAction(mPauseAction);
   runMenu->addAction(mStopAction);
   runMenu->addAction(mRestartAction);

   runMenu->addAction(mStepIntoAction);
   runMenu->addAction(mStepOverAction);
   runMenu->addAction(mStepOutAction);
   runMenu->addAction(mToggleBreakpointAction);
   runMenu->addAction(mDeleteAllBreakpointsAction);
   runMenu->addAction(mDisableAllBreakpointsAction);

   runMenu->addAction(mDebugSettings);

   mRunToolbar->addAction(mRunAction);
   mRunToolbar->addAction(mDebugAction);
   mRunToolbar->addAction(mContinueRealtimeAction);
   mRunToolbar->addAction(mContinueNonRealtimeAction);
   mRunToolbar->addAction(mSeparator1);
   mRunToolbar->addAction(mPauseAction);
   mRunToolbar->addAction(mStopAction);
   mRunToolbar->addAction(mRestartAction);
   mRunToolbar->addAction(mSeparator2);
   mRunToolbar->addAction(mStepIntoAction);
   mRunToolbar->addAction(mStepOverAction);
   mRunToolbar->addAction(mStepOutAction);

   mProgressBar       = new QProgressBar(mRunToolbar);
   mProgressBarAction = mRunToolbar->addWidget(mProgressBar);
   mProgressBarAction->setVisible(false);

   mBreakpointControlPtr = new ScriptBreakpointControl(wkfEnv.GetMainWindow());
   mainWindowPtr->AddDockWidget(Qt::BottomDockWidgetArea, mBreakpointControlPtr, "Script Tools");
   wkfEnv.GetMainWindow()->tabifyDockWidget(mOutputPanel, mBreakpointControlPtr);
   mBreakpointControlPtr->setVisible(false);

   mDebugWatchControlPtr = new WsfScriptDebuggerWatchControl(wkfEnv.GetMainWindow());
   mainWindowPtr->AddDockWidget(Qt::BottomDockWidgetArea, mDebugWatchControlPtr, "Script Tools");
   wkfEnv.GetMainWindow()->tabifyDockWidget(mOutputPanel, mDebugWatchControlPtr);
   mDebugWatchControlPtr->setVisible(false);

   mDebugCallstackControlPtr = new WsfScriptDebuggerCallstackControl(wkfEnv.GetMainWindow());
   mainWindowPtr->AddDockWidget(Qt::BottomDockWidgetArea, mDebugCallstackControlPtr, "Script Tools");
   wkfEnv.GetMainWindow()->tabifyDockWidget(mOutputPanel, mDebugCallstackControlPtr);
   mDebugCallstackControlPtr->setVisible(false);

   const auto& sim = wkfEnv.GetPreferenceObject<PrefObject>()->GetPreferences();
   ActiveSimChangedCB(sim.GetActiveSimName(), sim.GetActiveSimPath(), sim.ResolveActiveSimCompleteArguments());
}

SimulationManager::Plugin::~Plugin()
{
   ClearDebugger();

   for (auto&& pref : mPrefWidgets)
   {
      if (pref)
      {
         delete pref;
      }
   }
}

QList<wkf::PrefWidget*> SimulationManager::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   for (auto&& p : mPrefWidgets)
   {
      prefs << p;
   }
   return prefs;
}

QList<wkf::Action*> SimulationManager::Plugin::GetActions() const
{
   QList<wkf::Action*> list;
   list << mRunAction << mDebugAction;
   list << mContinueRealtimeAction << mContinueNonRealtimeAction << mPauseAction << mStopAction << mRestartAction;
   list << mStepIntoAction << mStepOverAction << mStepOutAction;
   list << mToggleBreakpointAction << mDeleteAllBreakpointsAction << mDisableAllBreakpointsAction;
   list << mDebugSettings;
   return list;
}

void SimulationManager::Plugin::MarginClick(Qt::MouseButton aButton, const QString& aSourcePath, size_t aLineNumber)
{
   if (mDebuggerPtr)
   {
      if (aButton == Qt::LeftButton)
      {
         mDebuggerPtr->ToggleBreakpoint(wizard::ProjectWorkspace::Instance()->FindSource(aSourcePath.toStdString(), false, true),
                                        aLineNumber);
      }
      else if (aButton == Qt::RightButton)
      {
         mDebuggerPtr->MarginRightClick(wkfEnv.GetMainWindow(),
                                        wizard::ProjectWorkspace::Instance()->FindSource(aSourcePath.toStdString(), false, true),
                                        aLineNumber);
      }
   }
}

void SimulationManager::Plugin::MarginDraw(const QString&              aSourcePath,
                                           QPainter&                   aPainter,
                                           const QMap<size_t, QPoint>& aVisibleLineMap)
{
   if (mDebuggerPtr)
   {
      std::vector<WsfScriptDebugger::Breakpoint*> points = mDebuggerPtr->BreakpointsInFile(
         wizard::ProjectWorkspace::Instance()->FindSource(aSourcePath.toStdString(), false, true));
      size_t activeLine = mDebuggerPtr->ArrowDrawHelper(
         wizard::ProjectWorkspace::Instance()->FindSource(aSourcePath.toStdString(), false, true));
      auto                                  activeLineVLM = aVisibleLineMap.find(activeLine);
      std::pair<unsigned int, unsigned int> imageValues   = wizard::UiResources::GetInstance().ResizeImageWithFont();
      for (auto&& pit : points)
      {
         WsfScriptDebugger::BreakpointState bps = mDebuggerPtr->GetBreakpointState(pit);
         auto                               it  = aVisibleLineMap.find(pit->GetLine());
         if (it != aVisibleLineMap.end())
         {
            switch (bps)
            {
            case WsfScriptDebugger::cBP_NORMAL:
               aPainter.drawImage(it.value().x(),
                                  it.value().y() + imageValues.second,
                                  wizard::UiResources::GetInstance()
                                     .GetImageFromTheme(QString("stop_sign_red"))
                                     .scaled(imageValues.first, imageValues.first, Qt::KeepAspectRatio));
               break;
            case WsfScriptDebugger::cBP_INVALID:
               aPainter.drawImage(it.value().x(),
                                  it.value().y() + imageValues.second,
                                  wizard::UiResources::GetInstance()
                                     .GetImageFromTheme(QString("close_hover"))
                                     .scaled(imageValues.first, imageValues.first, Qt::KeepAspectRatio));
               break;
            case WsfScriptDebugger::cBP_DISABLED:
               aPainter.drawImage(it.value().x(),
                                  it.value().y() + imageValues.second,
                                  wizard::UiResources::GetInstance()
                                     .GetImageFromTheme(QString("stop_sign"))
                                     .scaled(imageValues.first, imageValues.first, Qt::KeepAspectRatio));
               break;
            }
         }
      }
      if (activeLine != ut::npos && activeLineVLM != aVisibleLineMap.end())
      {
         aPainter.drawImage(activeLineVLM.value().x(),
                            activeLineVLM.value().y() + imageValues.second,
                            wizard::UiResources::GetInstance()
                               .GetImageFromTheme(QString("debug_arrow"))
                               .scaled(imageValues.first, imageValues.first, Qt::KeepAspectRatio));
      }
   }
}

void SimulationManager::Plugin::GetHighlightLines(const QString& aSourcePath, QList<size_t>& aList)
{
   if (mDebuggerPtr)
   {
      std::vector<size_t> myList = mDebuggerPtr->ActiveLines(
         wizard::ProjectWorkspace::Instance()->FindSource(aSourcePath.toStdString(), false, true));
      for (auto&& it : myList)
      {
         aList.push_back(it);
      }
   }
}

void SimulationManager::Plugin::ShowDebugWindows(bool aState)
{
   if (mDebuggerPtr)
   {
      mDebugWatchControlPtr->ActiveDebuggerChanged(mDebuggerPtr);
   }
   mDebugCallstackControlPtr->setVisible(aState);
   mDebugWatchControlPtr->setVisible(aState);
}

void SimulationManager::Plugin::ProjectOpened(wizard::Project* aProjectPtr)
{
   if (aProjectPtr)
   {
      SetupDebugger(aProjectPtr);
      if (!mPath.isEmpty())
      {
         mRunAction->setEnabled(true);
         mDebugAction->setEnabled(true);
         SetupExe(mPath);
      }
   }
}

void SimulationManager::Plugin::ProjectClosed()
{
   ClearDebugger();
   mRunAction->setEnabled(false);
   mDebugAction->setEnabled(false);
}

void SimulationManager::Plugin::ActiveSimChangedCB(const QString& aExec,
                                                   const QString& aPath,
                                                   const QString& aAdditionalCommandLineArgs)
{
   if (aPath.isEmpty())
   {
      mRunToolbar->SetLabel("");
      mRunAction->setEnabled(false);
      mDebugAction->setEnabled(false);
   }
   else
   {
      mRunToolbar->SetLabel(aExec);
      if (mPath != aPath)
      {
         if (!aExec.isEmpty())
         {
            if (wizard::Project::Instance())
            {
               SetupExe(aPath);
            }
         }
         mPath = aPath;
         mRunAction->setEnabled(!mPath.isEmpty());
         mDebugAction->setEnabled(!mPath.isEmpty());
      }

      mAdditionalCommandLineArgs = aAdditionalCommandLineArgs;
   }
}

void SimulationManager::Plugin::EvokePreferences()
{
   wkfEnv.GetMainWindow()->ShowPreferencePage(QString("Sim Execution"));
}

void SimulationManager::Plugin::Run(bool aDebug, bool aRestart)
{
   if (wizard::Project::Instance())
   {
      if (!wizard::ProjectWorkspace::Instance()->GetExecutions().empty() && aRestart == false)
      {
         if (QMessageBox::Ok !=
             QMessageBox::warning(
                nullptr,
                "Stop Current Run",
                "To run the scenario, the current run must be terminated.  Terminate the current run?",
                QMessageBox::Ok,
                QMessageBox::Cancel))
         {
            return;
         }
      }
      wizard::ProjectWorkspace::Instance()->StopAllExecutions();
      wizard::Project::Instance()->SaveAll();

      if (aDebug && mAdditionalCommandLineArgs.contains("-profiling-output"))
      {
         QMessageBox::warning(nullptr, "Debug Profiling", "Profiling results may be inaccurate when using the debugger.");
      }

      QString cmdLine = mAdditionalCommandLineArgs + " " + wizard::Project::Instance()->GetCmdLineArgs();

      wizard::WsfExecution* exe =
         wizard::WsfExecution::NewExecution(GetExe(),
                                            wizard::Project::Instance()->WorkingDirectory(),
                                            wizard::Project::Instance()->GetStartupFiles(),
                                            cmdLine,
                                            wizard::Project::Instance()->Name(),
                                            wizard::ProjectWorkspace::Instance()->GetWsfConnector()->XioRecvPort(),
                                            aDebug);
      if (exe)
      {
         wizard::ProjectWorkspace::Instance()->StartExecution(exe);
      }
      else
      {
         if (wizExeMgr.GetExes().size() <= 1)
         {
            QMessageBox::information(nullptr,
                                     "No application configured",
                                     "A WSF application must be configured before the scenario can be run.");
         }
         else
         {
            QMessageBox::information(nullptr,
                                     "No application configured",
                                     "This scenario needs to be configured with an application.");
         }
         wkfEnv.GetMainWindow()->ShowPreferencePage(GetName());
      }
   }
}

void SimulationManager::Plugin::Pause()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->Pause();
   }
}

void SimulationManager::Plugin::Stop()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->Stop();
   }
}

void SimulationManager::Plugin::Restart()
{
   // Check if there are any errors before restarting the run
   if (wizard::Project::Instance()->GetParseResults()->errors().empty())
   {
      mRestart = true; // Enable restart flag
      Run(true, true);
   }
   else
   {
      SimulationManager::Plugin::Stop();
      // Call some code from Run so that an empty .aer file is not generated when restart fails
      // Will keep last successful run in the .aer
      wizard::Project::Instance()->SaveAll();
      QString               cmdLine = mAdditionalCommandLineArgs + " " + wizard::Project::Instance()->GetCmdLineArgs();
      wizard::WsfExecution* exe =
         wizard::WsfExecution::NewExecution(GetExe(),
                                            wizard::Project::Instance()->WorkingDirectory(),
                                            wizard::Project::Instance()->GetStartupFiles(),
                                            cmdLine,
                                            wizard::Project::Instance()->Name(),
                                            wizard::ProjectWorkspace::Instance()->GetWsfConnector()->XioRecvPort(),
                                            true);
      if (exe)
      {
         wizard::ProjectWorkspace::Instance()->StartExecution(exe);
      }
   }
}

void SimulationManager::Plugin::ContinueRealtime()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->ContinueRealtime();
   }
}

void SimulationManager::Plugin::ContinueNonRealtime()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->ContinueNonRealtime();
   }
}

void SimulationManager::Plugin::StepInto()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->StepInto();
   }
}

void SimulationManager::Plugin::StepOut()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->StepOut();
   }
}

void SimulationManager::Plugin::StepOver()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->StepOver();
   }
}

void SimulationManager::Plugin::ToggleBreakpoint()
{
   if (mDebuggerPtr)
   {
      wizard::Editor* editor = wizard::ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
      if (editor)
      {
         wizard::TextSource* source = editor->GetSource();
         if (source)
         {
            int line = editor->textCursor().blockNumber();
            mDebuggerPtr->ToggleBreakpoint(source, line);
         }
      }
   }
}

void SimulationManager::Plugin::DeleteAllBreakpoints()
{
   if (mDebuggerPtr)
   {
      mDebuggerPtr->DeleteAllBreakpoints();
   }
}

void SimulationManager::Plugin::DebugSettings()
{
   QTextBrowser* debugSettingsLabel = new QTextBrowser();
   debugSettingsLabel->setText(QString::fromStdString(
      wizard::WsfExecution::DebugSettingsString(wizard::ProjectWorkspace::Instance()->GetWsfConnector()->XioRecvPort())));
   debugSettingsLabel->show();
}

wizard::WsfExe* SimulationManager::Plugin::GetExe()
{
   wizard::WsfExe* exe = wizExeMgr.FindByPath(mPath.toStdString());

   return exe ? exe : wizExeMgr.GetNoExe();
}

void SimulationManager::Plugin::SetupDebugger(wizard::Project* aProjectPtr)
{
   ClearDebugger();

   mDebuggerPtr = new WsfScriptDebugger(aProjectPtr, wizard::ProjectWorkspace::Instance()->GetWsfConnector(), this);
   connect(wizard::ProjectWorkspace::Instance()->GetWsfConnector(),
           &wizard::WsfConnector::TimeUpdated,
           this,
           &Plugin::TimeUpdated);

   mBreakpointControlPtr->ActiveDebuggerChanged(mDebuggerPtr);
}

void SimulationManager::Plugin::ClearDebugger()
{
   if (mDebuggerPtr)
   {
      delete mDebuggerPtr;
      mDebuggerPtr = nullptr;
   }
}

void SimulationManager::Plugin::DebugSessionState(wizard::ScriptDebugger* aDebugger, bool aActive)
{
   if (!mRestart) // Keeps the toolbar from switching off and back on when restart is clicked
   {
      emit wizard::ProjectWorkspace::Instance()->ChangeDebug(aActive);

      mRunAction->setEnabled(!aActive);
      mRunAction->setVisible(!aActive);
      mDebugAction->setEnabled(!aActive);
      mDebugAction->setVisible(!aActive);

      mContinueRealtimeAction->setVisible(aActive);
      mContinueRealtimeAction->setEnabled(aActive);
      mContinueNonRealtimeAction->setVisible(aActive);
      mContinueNonRealtimeAction->setEnabled(aActive);
      mSeparator1->setVisible(aActive);
      mPauseAction->setVisible(aActive);
      mPauseAction->setEnabled(aActive);
      mStopAction->setVisible(aActive);
      mStopAction->setEnabled(aActive);
      mRestartAction->setVisible(aActive);
      mRestartAction->setEnabled(aActive);
      mSeparator2->setVisible(aActive);
      mStepIntoAction->setVisible(aActive);
      mStepIntoAction->setEnabled(aActive);
      mStepOverAction->setVisible(aActive);
      mStepOverAction->setEnabled(aActive);
      mStepOutAction->setVisible(aActive);
      mStepOutAction->setEnabled(aActive);

      mProgressBarAction->setVisible(aActive);
   }
   else // Disables buttons temporarily when restart is clicked
   {
      mContinueRealtimeAction->setEnabled(aActive);
      mContinueNonRealtimeAction->setEnabled(aActive);
      mPauseAction->setEnabled(aActive);
      mStopAction->setEnabled(aActive);
      mRestartAction->setEnabled(aActive);
      mStepIntoAction->setEnabled(aActive);
      mStepOverAction->setEnabled(aActive);
      mStepOutAction->setEnabled(aActive);
   }
   mRestart = false; // Disable restart flag
}

void SimulationManager::Plugin::TimeUpdated()
{
   wizard::WsfConnector* connector = wizard::ProjectWorkspace::Instance()->GetWsfConnector();
   mProgressBar->setMaximum(1000);
   double endTime = std::max(1.0, std::max(connector->mSimTime, connector->mEndTime));
   mProgressBar->setValue(1000 * (connector->mSimTime / endTime));
   QString text;
   text += QString::number((int)connector->mSimTime);
   text += " s; " + QString::number(connector->mSimTime / 3600.0, 'g', 1) + " hr";
   text += " / " + QString::number(endTime / 3600.0, 'g', 1) + " hr";
   text += "; " + QString::number((int)connector->mCurrentRunNumber) + "/";
   text += QString::number((int)connector->mFinalRunNumber) + " runs";
   mProgressBar->setFormat(text);
   mProgressBar->setTextVisible(true);
}

void SimulationManager::Plugin::SetupExe(const QString& aPath)
{
   if (aPath.isEmpty())
   {
      wizard::Project::Instance()->SetExe(wizExeMgr.GetNoExe(), true);
   }
   else
   {
      wizard::WsfExe* exePtr = wizExeMgr.FindByPath(aPath.toStdString());
      if (!exePtr)
      {
         exePtr = wizExeMgr.Add(aPath.toStdString());
      }
      if (!exePtr)
      {
         QMessageBox::warning(wkfEnv.GetMainWindow(),
                              "Could not find executable",
                              "Could not find the executable at " + aPath);
         wizard::Project::Instance()->SetExe(wizExeMgr.GetNoExe(), true);
         return;
      }

      if (!exePtr->IsLoaded())
      {
         QApplication::setOverrideCursor(Qt::WaitCursor);
         exePtr->LoadFromDisk();
         QApplication::restoreOverrideCursor();
      }
      wizard::Project::Instance()->SetExe(exePtr, true);
   }
}

void SimulationManager::Plugin::GuiUpdate()
{
   mOutputPanel->AppendQueueToConsole();
}
