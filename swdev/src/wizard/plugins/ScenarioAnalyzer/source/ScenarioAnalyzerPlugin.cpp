// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: ScenarioAnalyzerPlugin.cpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/

#include <cctype>
#include <algorithm>
#include <string>

#include <QDockWidget>
#include <QMenu>
#include <QPushButton>

#include "UtPath.hpp"
#include "UtQt.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "Plugin.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "RunEnvManager.hpp"
#include "Gui.hpp"
#include "Signals.hpp"
#include "WsfExe.hpp"

#include "ScenarioAnalyzerPlugin.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(ScenarioAnalyzer::ScenarioAnalyzerPluginRegistration, "Scenario Analyzer", "Initializes the Scenario Analyzer", "wizard", false)

#define ArrayLength(a) (sizeof(a) / sizeof((a)[0]))

namespace // {anonymous}
{
void setWsfExecPath(ScenarioAnalyzer::ScenarioAnalyzerGui& gui,
                    ScenarioAnalyzer::ScenarioAnalyzerPlugin& plugin,
                    wizard::WsfExe* exe)
{
   // if the path is empty, the user has selected "No application"
   if (exe == nullptr ||
       exe->GetPath().GetNormalizedPath().empty() ||
       exe->GetApplication() != "mission" || //Scenario Analyzer assumes the application is mission and only works with mission.
       exe->GetVersion() < "2.3")
   {
      plugin.wsfExecExe = ScenarioAnalyzer::AbsoluteFile{QString()};
      gui.wsfExecExe = nullptr;
   }
   else
   {
      plugin.wsfExecExe = ScenarioAnalyzer::utPathToAbsoluteFile(exe->GetPath());
      gui.wsfExecExe = &plugin.wsfExecExe;
   }
}

void setMissionExecutableSelected(ScenarioAnalyzer::ScenarioAnalyzerGui& gui,
                                  ScenarioAnalyzer::ScenarioAnalyzerPlugin& plugin,
                                  wizard::WsfExe* exe)
{
   std::string applicationName = exe->GetApplication();
   std::transform(applicationName.begin(), applicationName.end(), applicationName.begin(), ::tolower);
   auto pos = applicationName.find("mission");
   gui.missionExecutableSelected = (pos != std::string::npos);
}

void setStartupFiles(ScenarioAnalyzer::ScenarioAnalyzerGui& gui,
                     const std::vector<UtPath>& files)
{
   gui.scenarioFiles.clear();
   gui.scenarioFiles.reserve(static_cast<int>(files.size()));
   for (size_t i = 0; i < files.size(); ++i)
   {
      gui.scenarioFiles.push_back(ScenarioAnalyzer::utPathToAbsoluteFile(files[i]));
   }
}

void loadPlugin(ScenarioAnalyzer::ScenarioAnalyzerGui& gui,
                ScenarioAnalyzer::ScenarioAnalyzerPlugin& plugin)
{
   using ScenarioAnalyzer::AbsoluteDir;
   AbsoluteDir ideInstallDir = ScenarioAnalyzer::utPathToAbsoluteDir(wizard::RunEnvManager::GetInstallRootDir());
   AbsoluteDir scenarioAnalyzerDir =
      ScenarioAnalyzer::createAbsoluteDir(appendDir(ideInstallDir, "resources/data/wsf_scenario_analyzer"));
   AbsoluteDir suiteDir =
     ScenarioAnalyzer::createAbsoluteDir(appendDir(scenarioAnalyzerDir, "check_suites"));
   plugin.suiteDir = suiteDir;
   AbsoluteDir sessionNoteDir =
     ScenarioAnalyzer::createAbsoluteDir(appendDir(scenarioAnalyzerDir, "session_note_suites"));
   plugin.sessionNoteDir = sessionNoteDir;

   AbsoluteDir debugScenarioAnalyzerDir = ScenarioAnalyzer::createAbsoluteDir(appendDir(ideInstallDir, "Debug/wsf_scenario_analyzer_data"));
   AbsoluteDir debugSuiteDir =
     ScenarioAnalyzer::createAbsoluteDir(appendDir(debugScenarioAnalyzerDir, "check_suites"));
   plugin.debugSuiteDir = debugSuiteDir;
   AbsoluteDir debugSessionNoteDir =
     ScenarioAnalyzer::createAbsoluteDir(appendDir(debugScenarioAnalyzerDir, "session_note_suites"));
   plugin.debugSessionNoteDir = debugSessionNoteDir;

   wkf::MainWindow* mainWindow = wkfEnv.GetMainWindow();

   setupDockWidgetLayouts(gui);

   gui.selectDock->setObjectName("ScenarioAnalyzerChecksDock");
   gui.resultsDock->setObjectName("ScenarioAnalyzerResultsDock");

   // If running from the installed location (normal use), the directories defined by
   // plugin.suiteDir and plugin.sessionNoteDir. Load SAT checks and session notes from
   // this directories.
   if (QDir(plugin.suiteDir.d).exists() && QDir(plugin.sessionNoteDir.d).exists())
   {
      gui.suiteDir = &plugin.suiteDir;
      gui.sessionNoteDir = &plugin.sessionNoteDir;
   }
   // If the debug directories exist, Wizard is being run under a Visual Studio debugger. Load
   // SAT checks and session notes from the debug directory.
   else if (QDir(plugin.debugSuiteDir.d).exists() && QDir(plugin.debugSessionNoteDir.d).exists())
   {
      gui.suiteDir = &plugin.debugSuiteDir;
      gui.sessionNoteDir = &plugin.debugSessionNoteDir;
   }
   // If neither pair of directories exist, assume that Wizard is running form the installed
   // location (normal use) and use the paths defined by plugin.suiteDir and plugin.sessionNoteDir
   // so that any error messages refer to the expected locations.
   else
   {
      gui.suiteDir = &plugin.suiteDir;
      gui.sessionNoteDir = &plugin.sessionNoteDir;
   }

   mainWindow->addDockWidget(Qt::RightDockWidgetArea, gui.selectDock);
   gui.selectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

   mainWindow->addDockWidget(Qt::BottomDockWidgetArea, gui.resultsDock);

   // Hide the Scenario Analyzer widgets until needed
   gui.selectDock->hide();
   gui.resultsDock->hide();

   auto showChecksDockAction = [&gui]()
   {
      gui.selectDock->show();
      UtQtRaiseWidget(gui.selectDock);
   };

   auto showResultsDockAction = [&gui]()
   {
      gui.resultsDock->show();
      UtQtRaiseWidget(gui.resultsDock);
   };

   auto loadChecksAction = [&gui]()
   {
      initiateLoadChecks(gui);
   };

   auto loadChecksCancelAction = [&gui]()
   {
      initiateCancelLoadChecks(gui);
   };

   auto runChecksAction = [&gui]()
   {
      initiateRunChecks(gui);
   };

   auto runChecksCancelAction = [&gui]()
   {
      initiateCancelRunChecks(gui);
   };

   // NOTE: some of the IDE events set environment state after the event is
   // emitted. For example, the project is not changed in the workspace until
   // after the ProjectOpened/ProjectClosed event is emitted. This means we
   // must rely on the event parameters to check the current value of something
   // rather than querying the environment.


   auto projectOpenedAction = [&gui, &plugin](wizard::Project * project)
   {
      assert(project != nullptr);
      setStartupFiles(gui, project->GetStartupFiles());
      setWsfExecPath(gui, plugin, project->GetExe());
      initiateLoadChecks(gui);
      buttonsSetEnabledState(gui);
   };

   auto projectClosedAction = [&gui, &plugin](wizard::Project*)
   {
      gui.scenarioFiles.clear();
      plugin.wsfExecExe = ScenarioAnalyzer::AbsoluteFile{QString()};
      gui.wsfExecExe = nullptr;
      buttonsSetEnabledState(gui);
   };


   auto exeChangedAction = [&gui, &plugin](wizard::WsfExe * exe)
   {
      setMissionExecutableSelected(gui, plugin, exe);
      setWsfExecPath(gui, plugin, exe);
      initiateLoadChecks(gui);
      buttonsSetEnabledState(gui);
   };

   // NOTE: startup files are the files passed as arguments to mission
   auto startupFilesChangedAction = [&gui](const std::vector<UtPath>& files)
   {
      setStartupFiles(gui, files);
      buttonsSetEnabledState(gui);
   };

   wizard::ProjectWorkspace& workspace = *(wizard::ProjectWorkspace::Instance());
   // If plugin was previously loaded during this session, an executable and an active project may
   // already be selected. In this case, exeChangedAction and projectOpenedAction / startupFilesChanged
   // callbacks won't get triggered, so we must manually call setExePath and setStartupFiles.
   // (If gui.wsfExecExe and gui.scenarioFiles haven't been set, the Run Checks button will be inactive.)

   wizard::Project* project = workspace.GetProject();
   if (project)
   {
      if (project->GetExe())
      {
         setWsfExecPath(gui, plugin, project->GetExe());
      }
      setStartupFiles(gui, project->GetStartupFiles());
   }

   // gotta connect 'em all
   QObject::connect(gui.loadChecksButton, &QPushButton::clicked, loadChecksAction);
   QObject::connect(gui.loadChecksCancelButton, &QPushButton::clicked, loadChecksCancelAction);
   QObject::connect(gui.runChecksButton, &QPushButton::clicked, runChecksAction);
   QObject::connect(gui.runChecksCancelButton, &QPushButton::clicked, runChecksCancelAction);
   plugin.connections[0] = QObject::connect(&workspace,
                                            &wizard::ProjectWorkspace::ProjectOpened,
                                            projectOpenedAction);
   plugin.connections[1] = QObject::connect(&workspace,
                                            &wizard::ProjectWorkspace::ProjectClosed,
                                            projectClosedAction);
   plugin.connections[2] = QObject::connect(wizSignals,
                                            &wizard::Signals::WsfExeChanged,
                                            exeChangedAction);
   plugin.connections[3] = QObject::connect(wizSignals,
                                            &wizard::Signals::ProjectStartupFilesChanged,
                                            startupFilesChangedAction);

   plugin.viewChecksAction = new wkf::Action("Scenario Analyzer Checks", wkfEnv.GetMainWindow());

   plugin.viewResultsAction = new wkf::Action("Scenario Analyzer Results", wkfEnv.GetMainWindow());

   QObject::connect(plugin.viewChecksAction, &QAction::triggered, std::function<void()>(showChecksDockAction));
   QObject::connect(plugin.viewResultsAction, &QAction::triggered, std::function<void()>(showResultsDockAction));
   mainWindow->addAction(plugin.viewChecksAction);
   mainWindow->addAction(plugin.viewResultsAction);

   // preload the available checks
   initiateLoadChecks(gui);
}

void unloadPlugin(ScenarioAnalyzer::ScenarioAnalyzerGui& gui,
                  ScenarioAnalyzer::ScenarioAnalyzerPlugin& plugin)
{
   cancelActiveTasks(gui);

   for (size_t i = 0; i < ArrayLength(plugin.connections); ++i)
   {
      QMetaObject::Connection conn = plugin.connections[i];
      if (conn)
      {
         QObject::disconnect(conn);
      }
   }
}
} // {anonymous}

ScenarioAnalyzer::ScenarioAnalyzerPluginRegistration::ScenarioAnalyzerPluginRegistration(const QString& aPluginName, const size_t aUniqueId) : wizard::Plugin(aPluginName, aUniqueId)
{
   loadPlugin(this->gui, this->plugin);
}

ScenarioAnalyzer::ScenarioAnalyzerPluginRegistration::~ScenarioAnalyzerPluginRegistration()
{
   unloadPlugin(this->gui, this->plugin);
}
