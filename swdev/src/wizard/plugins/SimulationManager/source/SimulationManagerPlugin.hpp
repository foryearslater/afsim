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
#ifndef SIMULATIONMANAGERPLUGIN_HPP
#define SIMULATIONMANAGERPLUGIN_HPP

#include <QProgressBar>

#include "OutputDock.hpp"
#include "Plugin.hpp"
#include "PrefWidget.hpp"
#include "Toolbar.hpp"
#include "WkfAction.hpp"


namespace wizard
{
class ScriptDebugger;
class WsfExe;
} // namespace wizard

namespace SimulationManager
{
class ScriptBreakpointControl;
class WsfScriptDebuggerWatchControl;
class WsfScriptDebuggerCallstackControl;
class WsfScriptDebugger;

class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   QList<wkf::Action*> GetActions() const override;

   void MarginClick(Qt::MouseButton aButton, const QString& aSourcePath, size_t aLineNumber) override;
   void MarginDraw(const QString& aSourcePath, QPainter& aPainter, const QMap<size_t, QPoint>& aVisibleLineMap) override;
   void                           GetHighlightLines(const QString& aSourcePath, QList<size_t>& aList) override;
   void                           ShowDebugWindows(bool aState);
   WsfScriptDebuggerWatchControl* GetDebugWatchControl() { return mDebugWatchControlPtr; }

protected:
protected slots:
   void ProjectOpened(wizard::Project* aProjectPtr);
   void ProjectClosed();

private:
   void ActiveSimChangedCB(const QString& aExec, const QString& aPath, const QString& aAdditionalCommandLineArgs);
   void EvokePreferences();
   void Run(bool aDebug, bool aRestart = false);
   void Pause();
   void Stop();
   void Restart();
   void ContinueRealtime();
   void ContinueNonRealtime();
   void StepInto();
   void StepOut();
   void StepOver();
   void ToggleBreakpoint();
   void DeleteAllBreakpoints();
   void DebugSettings();
   void SetupExe(const QString& aPath);
   wizard::WsfExe* GetExe();
   void            SetupDebugger(wizard::Project* aProjectPtr);
   void            ClearDebugger();
   void            DebugSessionState(wizard::ScriptDebugger* aDebugger, bool aActive);
   void            TimeUpdated();

   void GuiUpdate() override;

   UtCallbackHolder                 mUtCallbacks;
   QList<QPointer<wkf::PrefWidget>> mPrefWidgets;
   Toolbar*                         mRunToolbar;
   QString                          mPath;
   QString                          mAdditionalCommandLineArgs;
   OutputDock*                      mOutputPanel;
   WsfScriptDebugger*               mDebuggerPtr;

   wkf::Action* mRunAction;
   wkf::Action* mDebugAction;

   wkf::Action* mContinueRealtimeAction;
   wkf::Action* mContinueNonRealtimeAction;
   wkf::Action* mPauseAction;
   wkf::Action* mRestartAction;
   QAction*     mSeparator1;
   QAction*     mSeparator2;
   wkf::Action* mStopAction;
   wkf::Action* mStepIntoAction;
   wkf::Action* mStepOutAction;
   wkf::Action* mStepOverAction;

   wkf::Action* mToggleBreakpointAction;
   wkf::Action* mDeleteAllBreakpointsAction;
   wkf::Action* mDisableAllBreakpointsAction;

   wkf::Action* mDebugSettings;

   QProgressBar* mProgressBar;
   QAction*      mProgressBarAction;

   WsfScriptDebuggerWatchControl*     mDebugWatchControlPtr;
   WsfScriptDebuggerCallstackControl* mDebugCallstackControlPtr;
   ScriptBreakpointControl*           mBreakpointControlPtr;

   bool mRestart; // Restart flag that becomes true when restart is clicked
};
} // namespace SimulationManager
#endif
