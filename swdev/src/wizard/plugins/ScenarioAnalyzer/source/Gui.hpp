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
*   File: Gui.hpp
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

#ifndef SCENARIO_ANALYZER_GUI_HPP
#define SCENARIO_ANALYZER_GUI_HPP

#include <atomic>
#include <memory>
#include <vector>

#include <QEventLoop>
#include <QObject>
#include <QRunnable>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QMutex>
#include <QTreeView>

#include "GuiUtilities.hpp"
#include "ResultsView.hpp"
#include "SelectCheckModel.hpp"
#include "TimeUtilities.hpp"
#include "ScenarioAnalyzerMessage.hpp"


class QDockWidget;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QSpacerItem;
class QStackedWidget;
class QTextEdit;
class QTreeView;
class QWidget;

namespace ScenarioAnalyzer
{
class ExpandingResultsGroupModel;

struct LoadChecksInputs
{
   std::atomic<bool> cancelSignal;
   AbsoluteDir checkSuiteDir;
   AbsoluteDir sessionNoteDir;
   AbsoluteFile wsfExecExe;
};

struct LoadChecksOutputsOld
{
   QVector<AbsoluteFile> suiteFiles;
   QVector<QString> checkNames;
   QVector<QString> errors;
   QVector<SuiteParseErrorWithContext> parseErrors;
};

struct LoadChecksOutputs
{
   QVector<AbsoluteFile> suiteFiles;
   QVector<AbsoluteFile> validSuiteFiles;
   std::vector<int> validCheckIndices;
   QVector<QVector<QString>> checkNames;
   QVector<QVector<QString>> checkTitles;
   QMap<QString, QString> missingCheckDependencies;

   QVector<AbsoluteFile> sessionNoteFiles;
   QVector<AbsoluteFile> validSessionNoteFiles;
   std::vector<int> validSessionNoteIndices;
   QVector<QVector<QString>> sessionNoteNames;
   QMap<QString, QString> missingSessionNoteDependencies;

   QVector<QString> errors;
   QVector<SuiteParseErrorWithContext> parseErrors;
};

struct LoadChecksFields
{
   LoadChecksInputs inputs;
   LoadChecksOutputs outputs;
   QMutex mutex;
};

struct RunChecksInputs
{
   std::atomic<bool> cancelSignal;
   QVector<AbsoluteFile> suiteFiles;
   QVector<AbsoluteFile> sessionNoteFiles;
   QVector<QString> checkNames;
   QVector<QString> sessionNoteNames;
   AbsoluteFile wsfExecExe;
   QVector<AbsoluteFile> scenarioFiles;
   unsigned int executeTimeSeconds;
};

struct RunChecksOutputs
{
   QString uberfileText;
   QString wsfExecOutput;
   std::vector<ScenarioAnalyzerMessage> noteMessages;
   std::vector<ScenarioAnalyzerMessage> resultMessages;
   QVector<QString> errors;
};

struct RunChecksFields
{
   RunChecksInputs inputs;
   RunChecksOutputs outputs;
   QMutex mutex;
};

class LoadChecksTask : public QObject, public QRunnable
{
      Q_OBJECT

   public:
      // see note above about QEventLoopLocker
      QEventLoopLocker appQuitLocker;

      std::shared_ptr<LoadChecksFields> fields;

      virtual void run();

   signals:
      void done();
      void canceled();
};

class RunChecksTask : public QObject, public QRunnable
{
      Q_OBJECT

   public:
      // see note above about QEventLoopLocker
      QEventLoopLocker appQuitLocker;

      std::shared_ptr<RunChecksFields> fields;

      virtual void run();

   signals:
      void done();
      void canceled();
};

int extractScriptFunctionNames(
   const QVector<AbsoluteFile>& suiteFiles,
   QVector<AbsoluteFile>& validSuiteFiles,
   QVector<QString>& errors,
   QVector<SuiteParseErrorWithContext>& parseErrors,
   QVector<QVector<QString>>& scriptFunctionNamesBySuite,
   QVector<QVector<QString>>& scriptFunctionTitlesBySuite,
   std::vector<int>& validIndices,
   const std::string& missionOutput,
   QMap<QString, QString>& missingDependencies);

void loadChecks(LoadChecksTask& task);

void generateUberfile(
   const QVector<AbsoluteFile>& scenarioFiles,
   const QVector<AbsoluteFile>& suiteFiles,
   const QVector<AbsoluteFile>& sessionNoteFiles,
   const QVector<QString>& checkNames,
   const QVector<QString>& sessionNoteNames,
   std::vector<char>& uberfileText,
   unsigned int executeTimeSeconds);

void runChecks(RunChecksTask& task);

void printCheckLoadErrors(
   QFont monospaceFont,
   const QVector<QString>& errors,
   const QVector<SuiteParseErrorWithContext>& parseErrors,
   QTextCursor& cursor);



struct ScenarioAnalyzerGui
{
   QFont monospaceFont;
   SelectCheckGroupModel* selectCheckModel;
   ExpandingResultsGroupModel* resultsTreeModel;
   QDockWidget* selectDock;
   QWidget* selectDockWidget;
   QStackedWidget* checkStackWidget;
   QTextEdit* checkLoadErrorsView;
   QTreeView* selectCheckView;
   ResultsItemDelegate* resultsDelegate;
   QStackedWidget* loadChecksButtonStack;
   QPushButton* loadChecksButton;
   QPushButton* loadChecksCancelButton;
   int loadChecksButtonIndex;
   int loadChecksCancelButtonIndex;
   QStackedWidget* runChecksButtonStack;
   QPushButton* runChecksButton;
   QPushButton* runChecksCancelButton;
   int runChecksButtonIndex;
   int runChecksCancelButtonIndex;
   QStackedWidget* progressAndTimeStack;
   QWidget* progressWidget;
   QProgressBar* progressBar;
   QLabel* progressLabel;
   QWidget* timeWidget;
   QLabel* enterTimeMessageLabel;
   QLineEdit* setCheckTimeLineEdit;
   TimeLabel* formattedTimeLabel;
   QValidator* timeValidator;
   bool displayTimeWidget;
   QWidget* blankWidget;
   int progressIndex;
   int timeIndex;
   int blankIndex;
   QDockWidget* resultsDock;
   QTabWidget* resultsTabs;
   QPalette* resultsPalette;
   ResultsTreeView* resultsTreeView;
   QPlainTextEdit* uberfileView;
   QPlainTextEdit* wsfExecOutputView;
   int checkLoadErrorsViewIndex;
   int selectCheckViewIndex;

   // Use `shared_ptr`s for these values because both a task thread and the GUI
   // thread could be referencing them at the same time. The cleanup procedure
   // for plugins in the AFSIM IDE is not well-defined, so cleaning up these
   // values in the plugin unload procedure may not provide good behavior.
   // Using a shared_ptr guarantees these values will be remain around if a
   // task is running when the plugin is unloaded.
   std::shared_ptr<LoadChecksFields> loadChecksFields;
   std::shared_ptr<RunChecksFields> runChecksFields;

   QVector<AbsoluteFile> suiteFiles;
   QVector<AbsoluteFile> validSuiteFiles;
   QMap<QString, QString> missingCheckDependencies;
   QVector<AbsoluteFile> sessionNoteFiles;
   QVector<AbsoluteFile> validSessionNoteFiles;
   QVector<QString> validSessionNoteNames;
   QMap<QString, QString> missingSessionNoteDependencies;
   AbsoluteDir* suiteDir;
   AbsoluteDir* sessionNoteDir;
   AbsoluteFile* wsfExecExe;
   QVector<AbsoluteFile> scenarioFiles;
   bool loadingChecks, runningChecks;
   bool missionExecutableSelected; // False if Mission not active in Wizard's Application Manager
};

void setupDockWidgetLayouts(ScenarioAnalyzerGui& gui);
void buttonsSetEnabledState(ScenarioAnalyzerGui& gui);

void onLoadChecksDone(ScenarioAnalyzerGui& gui);
void onLoadChecksCanceled(ScenarioAnalyzerGui& gui);
void onRunChecksDone(ScenarioAnalyzerGui& gui);
void onRunChecksCanceled(ScenarioAnalyzerGui& gui);
void initiateLoadChecks(ScenarioAnalyzerGui& gui);
void initiateCancelLoadChecks(ScenarioAnalyzerGui& gui);
void initiateRunChecks(ScenarioAnalyzerGui& gui);
void initiateCancelRunChecks(ScenarioAnalyzerGui& gui);
void cancelActiveTasks(ScenarioAnalyzerGui& gui);
}
#endif