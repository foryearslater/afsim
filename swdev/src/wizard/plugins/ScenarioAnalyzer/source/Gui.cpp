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
*   File: Gui.cpp
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

/*
Some notes on concurrency:

This application uses delegates a few tasks to another thread to keep the GUI
responsive. These tasks are assigned to `QThreadPool::globalInstance()` rather
than using a dedicated thread. Using `QThreadPool` reduces the amount of custom
synchronization code the application needs to contain, compared to a raw
thread. Also, using the global thread pool keeps the application from
overallocating threads.

Tasks are encapsulated in a subclass of `QRunnable`. Each task contains two
fields called `inputs` and `outputs`, which contain the inputs and outputs of
the task, respectively. Each task also contains a field called `mutex` which is
used to ensure visibility of inputs and outputs. One should protect all reads
and writes to the `inputs` and `outputs` fields with the mutex. The mutex
ensures that reads/writes are visible to all CPU cores.

Some input and output fields are *reentrant* types (see
http://doc.qt.io/qt-5/threads-reentrancy.html). A reentrant type can be used
across multiple threads as long as each thread has its own copy of the type. Qt
also defines a number of *implicitly-shared* types (see
http://doc.qt.io/qt-5/implicit-sharing.html). Types that are implicitly shared
increment a reference counter during a copy, rather than copying the entire
object. These two types of objects have to be carefully handled when they are
shared among multiple threads. For example, say we want to share a
`QVector<QString>`. Both `QVector` and `QString` are reentrant and implicitly
shared. Copying the `QVector` alone would not copy all of the `QString`s it
contains. But, since `QString` is reentrant, we are not supposed to operate on
it on another thread. Thus, we must copy each `QString` inside the `QVector` in
addition to copying the `QVector` itself. Note that this problem is recursive.
If we have a `QVector<QVector<QString>>`, we have to do three levels of
copying: copy the outermost vector, copy each vector contained in the vector,
and copy the strings in each of these vectors. This file defines a `deepCopy`
function which copies a `QVector` and each of its elements (a two-level deep
copy), which is sufficient for the task inputs/outputs in this file.

Deep copies are the responsibility of the receiver of some data. Thus, tasks
need to deep copy inputs in their `run` function, and the GUI thread needs to
deep copy outputs in a slot function.

Each task input set contains an atomic boolean called `cancelSignal`. This
field is set to `false` prior to starting a task. The GUI contains a control to
cancel tasks, which sets this field to `true`. This field is checked
periodically by each task, and if its value is true, the task is aborted. Note
that If a task is close to completion, it may finish even if it is canceled.

Each `QRunnable` task includes two signals: `done` and `canceled`. `done` is
emitted when a task finished uninterrupted. `canceled` is emitted when the task
is canceled. If a task is canceled, all intermediate results are discarded.
These signals use `QObject::connect` just like any other signal. However, it is
important that these signals use a `Qt::QueuedConnection`. Qt can probably
infer this from the sender and receiver objects, but since this fact is
critical to the correctness of the application, it is better to explicitly
state the connection type in the `connect` function.
*/

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QMutex>
#include <QProcess>
#include <QString>
#include <QVector>
#include <QTemporaryFile>
#include <QThreadPool>
#include <QFontDatabase>
#include <QTextTable>
#include <QBoxLayout>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegExp>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTreeView>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "WsfApplication.hpp"

#include "Gui.hpp"
#include "ScenarioAnalyzerMessage.hpp"
#include "ResultsModel.hpp"

namespace ScenarioAnalyzer
{
int extractScriptFunctionNames(const QVector<AbsoluteFile>& suiteFiles,
                               QVector<AbsoluteFile>& validSuiteFiles,
                               QVector<QString>& errors,
                               QVector<SuiteParseErrorWithContext>& parseErrors,
                               QVector<QVector<QString>>& scriptFunctionNamesBySuite,
                               QVector<QVector<QString>>& scriptFunctionTitlesBySuite,
                               std::vector<int>& validIndices, // indices for which dependencies are loaded
                               const std::string& missionOutputOrig,
                               QMap<QString, QString>& missingDependencies)

{
   int totalFunctions = 0;
   for (int i = 0; i < suiteFiles.size(); ++i)
   {
      // read the whole file into memory
      QFile suiteFile(suiteFiles[i].f);
      std::string suiteFileStr = suiteFiles[i].f.toLocal8Bit().data();
      if (!suiteFile.open(QIODevice::ReadOnly))
      {
         QString error =
            QString("Failed to open file: '%1'")
            .arg(suiteFile.fileName());
         errors.push_back(error);
         continue;
      }
      QByteArray fileContents = suiteFile.readAll();
      if (suiteFile.error() != QFileDevice::NoError)
      {
         QString error =
            QString("Failed to read file: '%1'")
            .arg(suiteFile.fileName());
         errors.push_back(error);
         continue;
      }
      suiteFile.close();

      QVector<DependencyName> dependencyNames;
      QVector<char> scriptFunctionText;
      QVector<ScriptFunctionName> scriptFunctionNames;

      SuiteFileParseResult parseResult = parseSuiteFile(
                                            fileContents.size(), fileContents.data(), dependencyNames, scriptFunctionText, scriptFunctionNames);

      if (parseResult.error == SuiteFileParseError::None)
      {
         bool allDependenciesLoaded = true;

         // Only check for dependencies if we were able to find and run mission to determine which plug-ins are present.
         if (!missionOutputOrig.empty() && missionOutputOrig.find("Plugins:") != std::string::npos)
         {
            // If missionOutput lists missing dependencies, we don't want to compare that part of the output against
            // our list of dependencies.
            size_t endDeps = missionOutputOrig.find("ERROR: Extension Dependency missing");
            std::string missionOutput = (endDeps == std::string::npos) ? missionOutputOrig : missionOutputOrig.substr(0, endDeps);

            // unpack the required plugin names into individual QString's
            std::vector<std::string> dependencies;
            dependencies.reserve(dependencyNames.size());
            for (DependencyName dependencyName : dependencyNames)
            {
               std::string nameStr(scriptFunctionText.data() + dependencyName.offset, dependencyName.length);
               dependencies.push_back(nameStr);
            }

            QString missionOutputQStr(missionOutput.c_str());
            for (const auto& dep : dependencies)
            {
               const QString depQStr(dep.c_str());
               std::string regexStr = "\\b\\*?(lib)?" + dep + "(_registration)?(_d)?(.dll)?(_ln\\S*\\.so)?(\\*|,)?\\b";
               const QRegExp dependencyRegExp(regexStr.c_str());

               if (dependencyRegExp.indexIn(missionOutputQStr) != -1)
               {
                  std::string match = dependencyRegExp.cap(0).toStdString();
               }
               else
               {
                  QString fileName = fileNameFromAbsoluteFile(suiteFiles[i]);
                  missingDependencies.insertMulti(fileName, QString(dep.c_str()));
                  allDependenciesLoaded = false;
               }
            }
         }
         // unpack the function names into individual QString's
         QVector<QString> checkNames;
         checkNames.resize(scriptFunctionNames.size());
         QVector<QString> checkTitles;
         checkTitles.resize(scriptFunctionNames.size());
         for (int k = 0; k < scriptFunctionNames.size(); ++k)
         {
            ScriptFunctionName name = scriptFunctionNames[k];
            checkNames[k] = QString::fromLocal8Bit(
                               scriptFunctionText.data() + name.offset, name.length);
            checkTitles[k] = QString(checkNames[k]).replace(QChar('_'), QChar(' '));
         }
         scriptFunctionNamesBySuite[i] = deepCopy(checkNames);
         scriptFunctionTitlesBySuite[i] = deepCopy(checkTitles);
         totalFunctions += checkNames.size();

         if (allDependenciesLoaded)
         {
            validSuiteFiles.push_back(suiteFiles[i]);
            validIndices.push_back(i);
         }

         continue;
      }


      // assemble the parse error information
      size_t textLength = fileContents.size();
      const char* textBegin = fileContents.data();
      const char* textEnd = textBegin + textLength;

      // find beginning of erroneous line
      const char* lineBegin = parseResult.errorBegin.cursor;
      for (;;)
      {
         if (lineBegin == textBegin)
         {
            break;
         }
         char c = *lineBegin;
         if (c == '\r' || c == '\n')
         {
            ++lineBegin;
            break;
         }
         --lineBegin;
      }

      // find end of erroneous line
      const char* lineEnd = parseResult.errorBegin.cursor;
      for (;;)
      {
         if (lineEnd == textEnd)
         {
            break;
         }
         char c = *lineEnd;
         if (c == '\r' || c == '\n')
         {
            break;
         }
         ++lineEnd;
      }

      int lineLength = static_cast<int>(lineEnd - lineBegin);

      // find end of error within line
      size_t errorLength = parseResult.errorEnd - parseResult.errorBegin.cursor;
      size_t endOfLineLength = lineEnd - parseResult.errorBegin.cursor;
      if (errorLength > endOfLineLength)
      {
         errorLength = endOfLineLength;
      }
      char* errorEnd = parseResult.errorBegin.cursor + errorLength;

      SuiteParseErrorWithContext error;
      error.type = parseResult.error;
      error.fileName = suiteFile.fileName();
      error.lineNumber = parseResult.errorBegin.lineNumber;
      error.context = QString::fromLocal8Bit(lineBegin, lineLength);
      error.offset = static_cast<int>(parseResult.errorBegin.cursor - lineBegin);
      error.length = static_cast<int>(errorEnd - parseResult.errorBegin.cursor);

      parseErrors.push_back(error);
   }
   return totalFunctions;
}
}

void ScenarioAnalyzer::loadChecks(LoadChecksTask& task)
{
   task.fields->mutex.lock();
   std::atomic<bool>* cancelSignal = &(task.fields->inputs.cancelSignal);
   AbsoluteDir checkSuiteDir = task.fields->inputs.checkSuiteDir;
   AbsoluteDir sessionNoteDir = task.fields->inputs.sessionNoteDir;
   AbsoluteFile wsfExecExe = task.fields->inputs.wsfExecExe;
   task.fields->mutex.unlock();

   QVector<AbsoluteFile> suiteFiles;
   QVector<AbsoluteFile> validSuiteFiles;
   std::vector<int> validCheckIndices;
   QVector<QVector<QString>> checkNamesBySuite;
   QVector<QVector<QString>> checkTitlesBySuite;
   QVector<QString> errors;
   QVector<SuiteParseErrorWithContext> parseErrors;

   QString dirQStr = checkSuiteDir.d;
   std::string suiteDirStr = dirQStr.toLocal8Bit().data();
   QDir qSuiteDir = QDir(dirQStr);
   QStringList entries = qSuiteDir.entryList(QDir::Files);

   suiteFiles.resize(entries.size());
   checkNamesBySuite.resize(entries.size());
   checkTitlesBySuite.resize(entries.size());

   validSuiteFiles.reserve(entries.size());
   validCheckIndices.reserve(entries.size());

   // extract script functions from all suite files
   for (int i = 0; i < entries.size(); ++i)
   {
      if (cancelSignal->load())
      {
         emit task.canceled();
         return;
      }
      suiteFiles[i] = AbsoluteFile{appendFile(checkSuiteDir, entries[i])};
   }

   QVector<AbsoluteFile> sessionNoteFiles;
   QVector<AbsoluteFile> validSessionNoteFiles;
   std::vector<int> validSessionNoteIndices;
   QVector<QVector<QString>> sessionNoteNamesBySuite;
   QVector<QVector<QString>> sessionNoteTitlesBySuite;
   QVector<QString> SNerrors;
   QVector<SuiteParseErrorWithContext> SNparseErrors;

   QString SNdirQStr = sessionNoteDir.d;
   std::string sessionNoteDirStr = SNdirQStr.toLocal8Bit().data();
   QDir qSessionNoteDir = QDir(SNdirQStr);
   QStringList SNentries = qSessionNoteDir.entryList(QDir::Files);

   sessionNoteFiles.resize(SNentries.size());
   sessionNoteNamesBySuite.resize(SNentries.size());
   sessionNoteTitlesBySuite.resize(SNentries.size());

   validSessionNoteFiles.reserve(SNentries.size());
   validSessionNoteIndices.reserve(SNentries.size());

   for (int i = 0; i < SNentries.size(); ++i)
   {
      if (cancelSignal->load())
      {
         emit task.canceled();
         return;
      }
      sessionNoteFiles[i] = AbsoluteFile{ appendFile(sessionNoteDir, SNentries[i]) };
   }

   // Run mission with no arguments just to scrape output for loaded plug-ins and
   // included libraries.
   std::string missionOutputString;
   if (!wsfExecExe.f.isEmpty())
   {
      QProcess process;
      process.setProgram(toNativePath(wsfExecExe));
      process.setProcessChannelMode(QProcess::MergedChannels);
      process.start();

      if (process.waitForStarted(-1))
      {
         if (process.waitForFinished(5000))
         {
            QByteArray missionOutput = process.readAllStandardOutput();
            missionOutputString = missionOutput.toStdString();
         }
         else
         {
            process.kill();
         }
      }
   }

   QMap<QString, QString> missingCheckDependencies;
   ScenarioAnalyzer::extractScriptFunctionNames(suiteFiles, validSuiteFiles, errors, parseErrors,
                                                checkNamesBySuite, checkTitlesBySuite, validCheckIndices,
                                                missionOutputString, missingCheckDependencies);

   QMap<QString, QString> missingSessionNoteDependencies;
   ScenarioAnalyzer::extractScriptFunctionNames(sessionNoteFiles, validSessionNoteFiles, SNerrors, SNparseErrors,
                                                sessionNoteNamesBySuite, sessionNoteTitlesBySuite, validSessionNoteIndices,
                                                missionOutputString, missingSessionNoteDependencies);

   task.fields->mutex.lock();
   task.fields->outputs.suiteFiles = suiteFiles;
   task.fields->outputs.validSuiteFiles = validSuiteFiles;
   task.fields->outputs.validCheckIndices = validCheckIndices;
   task.fields->outputs.checkNames = checkNamesBySuite;
   task.fields->outputs.checkTitles = checkTitlesBySuite;
   task.fields->outputs.missingCheckDependencies = missingCheckDependencies;

   task.fields->outputs.sessionNoteFiles = sessionNoteFiles;
   task.fields->outputs.validSessionNoteFiles = validSessionNoteFiles;
   task.fields->outputs.validSessionNoteIndices = validSessionNoteIndices;
   task.fields->outputs.sessionNoteNames = sessionNoteNamesBySuite;
   task.fields->outputs.missingSessionNoteDependencies = missingSessionNoteDependencies;

   task.fields->outputs.errors = errors;
   task.fields->outputs.parseErrors = parseErrors;

   task.fields->mutex.unlock();

   emit task.done();

}

namespace ScenarioAnalyzer
{
void generateUberfile(const QVector<AbsoluteFile>& scenarioFiles,
                      const QVector<AbsoluteFile>& suiteFiles,
                      const QVector<AbsoluteFile>& sessionNoteFiles,
                      const QVector<QString>& checkNames,
                      const QVector<QString>& sessionNoteNames,
                      std::vector<char>& uberfileText,
                      unsigned int executeTimeSeconds)
{
   uberfileText.reserve(4096);
   appendCString(
      uberfileText,
      "# This file was automatically generated by the AFSIM Scenario Analyzer. If you\n");
   appendCString(
      uberfileText,
      "# are not actively running this tool, you may delete this file.\n\n");

   for (const auto& scenarioFile : scenarioFiles)
   {
      std::string scenarioFileFullPath = (scenarioFile.f).toLocal8Bit().constData();

      auto lastSlash = scenarioFileFullPath.find_last_of("/\\");
      std::string path = scenarioFileFullPath.substr(0, lastSlash + 1);
      std::string fileName = scenarioFileFullPath.substr(lastSlash + 1);
      appendCString(uberfileText, "file_path \"");
      appendQString(uberfileText, QString::fromLocal8Bit(path.c_str()));

      appendCString(uberfileText, "\"\n");

      appendCString(uberfileText, "include \"");
      appendQString(uberfileText, QString::fromLocal8Bit(fileName.c_str()));
      appendCString(uberfileText, "\"\n");
   }
   appendCString(uberfileText, "\n");

   for (AbsoluteFile includeFilePath : suiteFiles)
   {
      appendCString(uberfileText, "include \"");
      appendQString(uberfileText, includeFilePath.f);
      appendCString(uberfileText, "\"\n");
   }
   appendCString(uberfileText, "\n");

   // Include all files in the Session Notes directory
   for (AbsoluteFile includeFilePath : sessionNoteFiles)
   {
      appendCString(uberfileText, "include \"");
      appendQString(uberfileText, includeFilePath.f);
      appendCString(uberfileText, "\"\n");
   }
   appendCString(uberfileText, "\n");

   std::string executeCommand = "execute at_time ";
   if (executeTimeSeconds > 0)
   {
      executeCommand += std::to_string(executeTimeSeconds);
   }
   else
   {
      executeCommand += "1.0e-10";
   }
   executeCommand += " s absolute\n";
   appendCString(uberfileText, executeCommand.c_str());

   for (const auto& checkName : checkNames)
   {
      appendCString(uberfileText, "   ");
      appendQString(uberfileText, checkName);
      appendCString(uberfileText, "();\n");
   }

   appendCString(uberfileText, "\n");

   for (const auto& sessionNoteName : sessionNoteNames)
   {
      std::string snn = sessionNoteName.toLocal8Bit().data();
      appendCString(uberfileText, "   ");
      appendQString(uberfileText, sessionNoteName);
      appendCString(uberfileText, "();\n");
   }

   appendCString(uberfileText, "end_execute\n\n");

   std::string endTimeCommand = "end_time ";
   if (executeTimeSeconds > 0)
   {
      endTimeCommand += std::to_string((double)executeTimeSeconds + 1e-9);
   }
   else
   {
      endTimeCommand += "1.0e-9";
   }
   endTimeCommand += " s\n\n";
   appendCString(uberfileText, endTimeCommand.c_str());
   appendCString(uberfileText, "event_output\n   disable all\nend_event_output\n");
}
}

void ScenarioAnalyzer::runChecks(RunChecksTask& task)
{
   task.fields->mutex.lock();
   std::atomic<bool>* cancelSignal = &(task.fields->inputs.cancelSignal);
   QVector<AbsoluteFile> suiteFiles = deepCopy(task.fields->inputs.suiteFiles);
   QVector<AbsoluteFile> sessionNoteFiles = deepCopy(task.fields->inputs.sessionNoteFiles);
   QVector<QString> checkNames = deepCopy(task.fields->inputs.checkNames);
   QVector<QString> sessionNoteNames = deepCopy(task.fields->inputs.sessionNoteNames);
   AbsoluteFile wsfExecExe = task.fields->inputs.wsfExecExe;
   QVector<AbsoluteFile> scenarioFiles = deepCopy(task.fields->inputs.scenarioFiles);
   int executeTimeSeconds = task.fields->inputs.executeTimeSeconds;
   task.fields->mutex.unlock();

   std::vector<ScenarioAnalyzerMessage> noteMessages;
   std::vector<ScenarioAnalyzerMessage> resultMessages;
   QString uberfileTextQString;
   QString wsfExecOutputQString;
   QVector<QString> errors;

   // NOTE: introducing scope suppresses a compiler warning
   {
      std::vector<char> uberfileText;
      ScenarioAnalyzer::generateUberfile(scenarioFiles, suiteFiles, sessionNoteFiles, checkNames, sessionNoteNames, uberfileText, executeTimeSeconds);
      uberfileTextQString = QString::fromLocal8Bit(uberfileText.data(), static_cast<int>(uberfileText.size()));

      if (cancelSignal->load())
      {
         emit task.canceled();
         return;
      }

      // NOTE: QTemporaryFile instances do not delete the file backing them until
      // they go out of scope
      // NOTE: Qt deletes this file automatically if the this process exits
      // normally. If this process is killed forecfully (e.g. with SIGKILL),
      // this file will not be deleted.
      QTemporaryFile uberfile("masterFileXXXXXX.txt");
      if (!uberfile.open())
      {
         errors.push_back(QString("Could not open generated scenario file."));
         goto emitDoneSignal;
      }
      QString uberfilePath = uberfile.fileName();

      // The docs for QIODevice::write say it "writes at most *maxSize* bytes of
      // data from *data*". "At most" implies that it may not write everything, so
      // we have to write the bytes in a loop to be safe.
      qint64 dataLength = uberfileText.size();
      char* pData = uberfileText.data();
      while (dataLength > 0)
      {
         qint64 writeLength = uberfile.write(pData, dataLength);
         if (writeLength == -1)
         {
            errors.push_back(QString("Failed to write generated scenario file."));
            goto emitDoneSignal;
         }
         dataLength -= writeLength;
      }

      uberfile.close();

      if (cancelSignal->load())
      {
         emit task.canceled();
         return;
      }

      QStringList wsfExecArgs;
      wsfExecArgs.push_back(uberfilePath);

      QProcess process;
      process.setProgram(toNativePath(wsfExecExe));
      process.setArguments(wsfExecArgs);
      if (!scenarioFiles.empty())
      {
         // Many scenarios depend on the `include` command, which permits
         // relative paths. Most executions provide a single input file to
         // `mission`, and expect the working directory to be the file's
         // parent directory. In cases where more than one file is provided, we
         // have to guess the parent directory. We simply choose the one with
         // the shortest path length. It is likely that all files are part of
         // the same directory tree, so this should be the root of the tree.
         AbsoluteDir workingDir = parentDir(scenarioFiles[0]);
         for (const auto& scenarioFile : scenarioFiles)
         {
            AbsoluteDir dir = parentDir(scenarioFile);
            if (dir.d.size() < workingDir.d.size())
            {
               workingDir = dir;
            }
         }
         process.setWorkingDirectory(toNativePath(workingDir));
      }
      process.setProcessChannelMode(QProcess::MergedChannels);
      // NOTE: Qt terminates this process automatically if the parent process
      // exits normally. If the parent process is killed forcefully (e.g. with
      // SIGKILL) it will not be terminated.
      process.start();

      // this call checks if the process started successfully
      if (!process.waitForStarted(-1))
      {
         errors.push_back("Failed to start Mission.");
         goto emitDoneSignal;
      }

      //TODO use the QProcess::finished signal and a QWaitCondition instead of
      // polling to wait for a finished process. This can provide more responsive
      // and efficient cancellation than the polling loop below.
      while (!process.waitForFinished(250))
      {
         if (cancelSignal->load())
         {
            emit task.canceled();
            // NOTE: mission should not have any harmful side-effects if it is killed forcefully
            process.kill();
            return;
         }
      }

      // NOTE: reading more than 268 megabytes from stdout crashes the application
      // (tested on a Windows machine). It is unlikely we will reach this limit.
      QByteArray wsfExecOutput = process.readAllStandardOutput();
      wsfExecOutputQString = QString::fromLocal8Bit(wsfExecOutput.data(), wsfExecOutput.size());

      if (process.exitCode() != 0)
      {
         errors.push_back("mission did not finish running successfully - check its output for errors.");
         goto emitDoneSignal;
      }

      if (cancelSignal->load())
      {
         emit task.canceled();
         return;
      }

      // Parse out scenario analyzer messages from the input stream
      std::stringstream missionOutputStream(wsfExecOutput.data());
      std::vector<ScenarioAnalyzerMessage> messages = ScenarioAnalyzerMessage::ParseFromIStream(missionOutputStream);

      // Organize into "notes" and "results"
      for (const auto& saMessage : messages)
      {
         if (saMessage.GetSeverity() == ScenarioAnalyzerMessage::cNOTE &&
             !saMessage.IsSuccessfulResult())
         {
            // Just a note if tagged "note" and doesn't have a successful result
            noteMessages.push_back(saMessage);
         }
         else
         {
            // This is either a successful result or a warning/error
            resultMessages.push_back(saMessage);
         }
      }
   }

emitDoneSignal:
   task.fields->mutex.lock();
   task.fields->outputs.uberfileText = uberfileTextQString;
   task.fields->outputs.wsfExecOutput = wsfExecOutputQString;
   task.fields->outputs.noteMessages = noteMessages;
   task.fields->outputs.resultMessages = resultMessages;
   task.fields->outputs.errors = errors;
   task.fields->mutex.unlock();

   emit task.done();
}

void ScenarioAnalyzer::LoadChecksTask::run()
{
   loadChecks(*this);
}

void ScenarioAnalyzer::RunChecksTask::run()
{
   runChecks(*this);
}

void ScenarioAnalyzer::printCheckLoadErrors(QFont monospaceFont,
                                            const QVector<QString>& errors,
                                            const QVector<SuiteParseErrorWithContext>& parseErrors,
                                            QTextCursor& cursor)
{
   QTextCharFormat defaultFormat;

   QTextCharFormat scriptFormat;
   scriptFormat.setFont(monospaceFont);

   QColor lightRed = QColor(255, 150, 150);
   QTextCharFormat scriptErrorFormat;
   scriptErrorFormat.setFont(monospaceFont);
   scriptErrorFormat.setBackground(QBrush(lightRed));

   // `beginEditBlock` and `endEditBlock` prevent QTextEdit from
   // recording undo steps for each insertion, which makes inserting
   // text a little more efficient.
   cursor.beginEditBlock();
   QTextTableFormat tableFormat;
   tableFormat.setCellPadding(5.0f);
   tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);

   QTextTable* table = cursor.insertTable(
                          errors.size() + parseErrors.size(), 1, tableFormat);
   int row = 0;

   cursor.setCharFormat(defaultFormat);
   for (int i = 0; i < errors.size(); ++i, ++row)
   {
      QTextCursor cellCursor = table->cellAt(row, 0).firstCursorPosition();
      cellCursor.insertText(errors[i]);
   }

   for (int i = 0; i < parseErrors.size(); ++i, ++row)
   {
      SuiteParseErrorWithContext error = parseErrors[i];

      const char* errorMessage = nullptr;
      switch (error.type)
      {
         case SuiteFileParseError::UnknownCommand:
            errorMessage = "Unknown command";
            break;
         case SuiteFileParseError::UnclosedBlockComment:
            errorMessage = "Missing end of '/*' comment";
            break;
         case SuiteFileParseError::UnclosedStringLiteral:
            errorMessage = "Missing closing quote '\"'";
            break;
         case SuiteFileParseError::UnclosedScriptBlock:
            errorMessage = "Missing corresponding 'end_script' token";
            break;
         case SuiteFileParseError::InvalidScriptReturnType:
            errorMessage = "Script function return type must be 'void'";
            break;
         case SuiteFileParseError::MissingScriptFunctionName:
            errorMessage = "Missing script function name";
            break;
         case SuiteFileParseError::None:
            break;
      }

      QString beforeErrorStr = error.context.left(error.offset);
      QString errorStr = error.context.mid(error.offset, error.length);
      QString afterErrorStr = error.context.mid(error.offset + error.length);

      QTextCursor cellCursor = table->cellAt(row, 0).firstCursorPosition();

      cursor.setCharFormat(defaultFormat);
      cellCursor.insertText(QString("In '"));
      cellCursor.insertText(error.fileName);
      cellCursor.insertText(QString("', line "));
      cellCursor.insertText(QString::number(error.lineNumber));
      cellCursor.insertText(QString(": "));
      if (errorMessage == nullptr)
      {
         assert(false);
         cellCursor.insertText(QString::number((int) error.type));
      }
      else
      {
         cellCursor.insertText(QString(errorMessage));
      }
      cellCursor.insertBlock();

      cellCursor.insertText(beforeErrorStr, scriptFormat);
      cellCursor.insertText(errorStr, scriptErrorFormat);
      cellCursor.insertText(afterErrorStr, scriptFormat);
   }
   cursor.endEditBlock();
}

void ScenarioAnalyzer::buttonsSetEnabledState(ScenarioAnalyzerGui& gui)
{
   bool loadChecksEnabled =
      gui.suiteDir != nullptr &&
      !gui.loadingChecks &&
      !gui.runningChecks;
   gui.loadChecksButton->setEnabled(loadChecksEnabled);

   bool runChecksEnabled =
      gui.suiteDir != nullptr &&
      gui.wsfExecExe != nullptr &&
      !gui.scenarioFiles.empty() &&
      !gui.selectCheckModel->root.children.empty() &&
      !gui.loadingChecks &&
      !gui.runningChecks;
   gui.runChecksButton->setEnabled(runChecksEnabled);
}

void ScenarioAnalyzer::setupDockWidgetLayouts(ScenarioAnalyzerGui& gui)
{
   gui.monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

   gui.selectCheckModel = new SelectCheckGroupModel();
   gui.resultsTreeModel = new ExpandingResultsGroupModel();
   gui.resultsDelegate = new ResultsItemDelegate();

   gui.selectDock = new QDockWidget("Scenario Analyzer Checks");

   gui.selectDockWidget = new QWidget();

   gui.checkStackWidget = new QStackedWidget();

   gui.checkLoadErrorsView = new QTextEdit();
   gui.checkLoadErrorsView->setReadOnly(true);
   gui.checkLoadErrorsView->setLineWrapMode(QTextEdit::NoWrap);

   gui.selectCheckView = new QTreeView();
   gui.selectCheckView->setModel(gui.selectCheckModel);
   gui.selectCheckModel->setParent(gui.selectDock);
   gui.selectCheckView->setEnabled(false);
   gui.selectCheckView->setHeaderHidden(true);

   auto controlsWidget = new QWidget();
   controlsWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

   gui.loadChecksButtonStack = new QStackedWidget();
   gui.loadChecksButtonStack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   gui.loadChecksButton = new QPushButton("Load Checks");
   gui.loadChecksButton->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   gui.loadChecksButton->setEnabled(false);

   gui.loadChecksCancelButton = new QPushButton(QString("Cancel"));
   gui.loadChecksCancelButton->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   gui.loadChecksCancelButton->setEnabled(false);

   gui.runChecksButtonStack = new QStackedWidget();
   gui.runChecksButtonStack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   gui.runChecksButton = new QPushButton("Run Checks");
   gui.runChecksButton->setEnabled(false);
   gui.runChecksButton->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

   gui.runChecksCancelButton = new QPushButton(QString("Cancel"));
   gui.runChecksCancelButton->setEnabled(false);
   gui.runChecksCancelButton->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

   gui.progressAndTimeStack = new QStackedWidget();
   gui.progressAndTimeStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   gui.progressWidget = new QWidget();
   auto progressLayout = new QGridLayout();

   gui.progressBar = new QProgressBar();
   // Even if the text string is empty, some space is reserved for it to the
   // right of the progress bar. Setting this property to false removes that
   // extra space.
   gui.progressBar->setTextVisible(false);
   // set the progress bar to the "indeterminate state" (shows that it is busy
   // without showing a percentage)
   gui.progressBar->setRange(0, 0);

   gui.progressLabel = new QLabel();
   gui.progressLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

   progressLayout->addWidget(gui.progressBar, 0, 0);
   progressLayout->addWidget(gui.progressLabel, 0, 0);
   gui.progressWidget->setLayout(progressLayout);

   gui.displayTimeWidget = false;

   gui.timeWidget = new QWidget();
   auto timeLayout = new QGridLayout();

   gui.enterTimeMessageLabel = new QLabel();
   gui.enterTimeMessageLabel->setText("Enter the simulation time at which selected checks should be executed:");
   gui.enterTimeMessageLabel->setWordWrap(true);
   gui.enterTimeMessageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   gui.enterTimeMessageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

   gui.timeValidator = new TimeValidator();
   gui.setCheckTimeLineEdit = new QLineEdit();
   gui.setCheckTimeLineEdit->setValidator(gui.timeValidator);
   gui.timeValidator->setParent(gui.setCheckTimeLineEdit);
   gui.setCheckTimeLineEdit->setPlaceholderText(QString("[days]:[hrs]:[min]:[sec]"));
   gui.setCheckTimeLineEdit->setEnabled(true);
   gui.setCheckTimeLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);


   gui.formattedTimeLabel = new TimeLabel();
   gui.formattedTimeLabel->setText(QString("00:00:00:00"));
   gui.formattedTimeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

   timeLayout->addWidget(gui.enterTimeMessageLabel, 0, 0, 1, -1);
   timeLayout->addWidget(gui.setCheckTimeLineEdit, 1, 0);

   timeLayout->addWidget(gui.formattedTimeLabel, 1, 1);

   gui.timeWidget->setLayout(timeLayout);

   gui.blankWidget = new QWidget();
   auto blankLayout = new QGridLayout();

   gui.blankWidget->setLayout(blankLayout);

   gui.progressIndex = gui.progressAndTimeStack->addWidget(gui.progressWidget);
   gui.timeIndex = gui.progressAndTimeStack->addWidget(gui.timeWidget);
   gui.blankIndex = gui.progressAndTimeStack->addWidget(gui.blankWidget);

   // When user enters a time in the writable setCheckTimeLineEdit, display the
   // properly formatted representation of their input
   QObject::connect(gui.setCheckTimeLineEdit, &QLineEdit::textChanged,
                    gui.formattedTimeLabel, &TimeLabel::setTextAsValidTime);


   gui.resultsDock = new QDockWidget("Scenario Analyzer Results");

   gui.resultsTabs = new QTabWidget();


   gui.resultsTreeView = new ResultsTreeView();
   gui.resultsTreeView->setModel(gui.resultsTreeModel);
   gui.resultsTreeModel->setParent(gui.resultsDock);
   gui.resultsTreeView->setItemDelegate(gui.resultsDelegate);
   gui.resultsDelegate->setParent(gui.resultsTreeView);
   gui.resultsTreeView->setHeaderHidden(true);
   gui.resultsTreeView->setWordWrap(true);
   gui.resultsTreeView->setUniformRowHeights(false);

   gui.checkLoadErrorsViewIndex = gui.checkStackWidget->addWidget(gui.checkLoadErrorsView);
   gui.selectCheckViewIndex = gui.checkStackWidget->addWidget(gui.selectCheckView);

   gui.uberfileView = new QPlainTextEdit();
   gui.uberfileView->setReadOnly(true);
   gui.uberfileView->setLineWrapMode(QPlainTextEdit::NoWrap);
   gui.uberfileView->document()->setDefaultFont(gui.monospaceFont);

   gui.wsfExecOutputView = new QPlainTextEdit();
   gui.wsfExecOutputView->setReadOnly(true);
   gui.wsfExecOutputView->setLineWrapMode(QPlainTextEdit::NoWrap);
   gui.wsfExecOutputView->document()->setDefaultFont(gui.monospaceFont);

   gui.checkStackWidget->setCurrentIndex(gui.selectCheckViewIndex);

   gui.loadChecksButtonIndex = gui.loadChecksButtonStack->addWidget(gui.loadChecksButton);
   gui.loadChecksCancelButtonIndex = gui.loadChecksButtonStack->addWidget(gui.loadChecksCancelButton);

   gui.runChecksButtonIndex = gui.runChecksButtonStack->addWidget(gui.runChecksButton);
   gui.runChecksCancelButtonIndex = gui.runChecksButtonStack->addWidget(gui.runChecksCancelButton);


   auto controlsLayout = new QHBoxLayout();
   controlsLayout->setAlignment(Qt::AlignLeft);
   controlsLayout->addWidget(gui.loadChecksButtonStack);
   controlsLayout->addWidget(gui.runChecksButtonStack);
   controlsLayout->addWidget(gui.progressAndTimeStack);
   controlsWidget->setLayout(controlsLayout);

   auto checksDockLayout = new QVBoxLayout();
   checksDockLayout->setContentsMargins(0, 0, 0, 0);
   checksDockLayout->setSpacing(0);
   checksDockLayout->addWidget(gui.checkStackWidget);
   checksDockLayout->addWidget(controlsWidget);
   gui.selectDockWidget->setLayout(checksDockLayout);

   auto resultsScrollArea = new QScrollArea;
   resultsScrollArea->setWidget(gui.resultsTreeView);
   resultsScrollArea->setWidgetResizable(true);
   gui.resultsTreeView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
   gui.resultsTreeView->header()->setStretchLastSection(false);
   gui.resultsTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
   gui.resultsTabs->addTab(resultsScrollArea, "Results");
   gui.resultsTabs->addTab(gui.uberfileView, "Generated Scenario File");
   gui.resultsTabs->addTab(gui.wsfExecOutputView, "AFSIM Output");

   gui.selectDock->setWidget(gui.selectDockWidget);
   gui.resultsDock->setWidget(gui.resultsTabs);

   gui.loadChecksFields = std::make_shared<LoadChecksFields>();
   gui.runChecksFields = std::make_shared<RunChecksFields>();

   gui.suiteDir = nullptr;
   gui.wsfExecExe = nullptr;
   gui.loadingChecks = false;
   gui.runningChecks = false;
   gui.missionExecutableSelected = false;
}

void ScenarioAnalyzer::onLoadChecksDone(ScenarioAnalyzerGui& gui)
{
   assert(gui.suiteDir != nullptr);

   gui.loadingChecks = false;

   gui.loadChecksFields->mutex.lock();
   QVector<QVector<QString>> checkTitles;
   QVector<QVector<QString>> checkNames;
   checkTitles.resize(gui.loadChecksFields->outputs.checkTitles.size());
   checkNames.resize(gui.loadChecksFields->outputs.checkNames.size());

   for (int i = 0, sz = gui.loadChecksFields->outputs.checkTitles.size(); i != sz; ++i)
   {
      checkTitles[i] = deepCopy(gui.loadChecksFields->outputs.checkTitles[i]);
   }

   for (int i = 0, sz = gui.loadChecksFields->outputs.checkNames.size(); i != sz; ++i)
   {
      checkNames[i] = deepCopy(gui.loadChecksFields->outputs.checkNames[i]);
   }

   std::vector<int> validCheckIndices = gui.loadChecksFields->outputs.validCheckIndices;
   QVector<QString> errors = deepCopy(gui.loadChecksFields->outputs.errors);
   QVector<SuiteParseErrorWithContext> parseErrors = deepCopy(gui.loadChecksFields->outputs.parseErrors);
   gui.suiteFiles = deepCopy(gui.loadChecksFields->outputs.suiteFiles);
   gui.validSuiteFiles = deepCopy(gui.loadChecksFields->outputs.validSuiteFiles);
   gui.missingCheckDependencies = gui.loadChecksFields->outputs.missingCheckDependencies;

   QVector<QString> validSessionNoteNames;
   std::vector<int> validIndices = gui.loadChecksFields->outputs.validSessionNoteIndices;

   for (size_t i = 0; i != validIndices.size(); ++i)
   {
      QVector<QString> validSessionNoteSuite = deepCopy(gui.loadChecksFields->outputs.sessionNoteNames[validIndices[i]]);
      for (const QString& name : validSessionNoteSuite)
      {
         validSessionNoteNames.push_back(name);
      }
   }
   gui.validSessionNoteNames = validSessionNoteNames;
   gui.sessionNoteFiles = deepCopy(gui.loadChecksFields->outputs.sessionNoteFiles);
   gui.validSessionNoteFiles = deepCopy(gui.loadChecksFields->outputs.validSessionNoteFiles);
   gui.missingSessionNoteDependencies = gui.loadChecksFields->outputs.missingSessionNoteDependencies;

   gui.loadChecksFields->mutex.unlock();

   if (!(errors.empty() && parseErrors.empty()))
   {
      QTextCursor cursor(gui.checkLoadErrorsView->document());
      printCheckLoadErrors(gui.monospaceFont, errors, parseErrors, cursor);
      gui.checkStackWidget->setCurrentIndex(gui.checkLoadErrorsViewIndex);
   }
   else if (checkTitles.empty())
   {
      QTextCursor cursor(gui.checkLoadErrorsView->document());
      cursor.beginEditBlock();
      cursor.insertText(QString("There are no checks available in suite directory '"));
      cursor.insertText(toNativePath(*gui.suiteDir));
      cursor.insertText("'");
      cursor.endEditBlock();
      gui.checkStackWidget->setCurrentIndex(gui.checkLoadErrorsViewIndex);
   }
   else
   {
      assert(checkTitles.size() == gui.suiteFiles.size());

      gui.selectCheckModel->root.children.reserve(checkTitles.size());
      gui.selectCheckModel->root.name = "All checks";
      gui.selectCheckModel->root.title = "All checks";

      CheckTreeItem* root = &(gui.selectCheckModel->root);
      root->children.clear();
      root->children.reserve(checkTitles.size());

      for (int i = 0; i != checkTitles.size(); ++i)
      {
         bool valid = (std::find(validCheckIndices.cbegin(), validCheckIndices.cend(), i) != validCheckIndices.end());

         root->children.push_back(CheckTreeItem());
         CheckTreeItem* suite = &(root->children[i]);
         suite->name = fileNameFromAbsoluteFile(gui.suiteFiles[i]);
         suite->title = fileNameFromAbsoluteFile(gui.suiteFiles[i]);
         suite->selectedCount = 0;
         suite->valid = valid;
         suite->parent = root;

         suite->children.reserve(checkTitles[i].size());
         for (int j = 0; j != checkTitles[i].size(); ++j)
         {
            suite->children.push_back(CheckTreeItem());
            CheckTreeItem* check = &(suite->children[j]);
            check->title = checkTitles[i][j];
            check->name = checkNames[i][j];
            check->selectedCount = 0;
            check->valid = valid;
            check->parent = suite;
         }
      }

      gui.selectCheckModel->refreshRoot();

      gui.selectCheckView->setEnabled(true);
      gui.selectCheckView->expandToDepth(1);
      gui.checkStackWidget->setCurrentIndex(gui.selectCheckViewIndex);
   }

   // Warn of checks that aren't available due to missing dependencies
   if (gui.missionExecutableSelected && !gui.missingCheckDependencies.empty())
   {
      QString warningTitle("Scenario Analyzer: Check Suites Unavailable");
      QString warningMessage;
      QList<QString> unavailableSuites = gui.missingCheckDependencies.uniqueKeys();
      for (int i = 0; i != unavailableSuites.size(); ++i)
      {
         QString suite = unavailableSuites[i];
         warningMessage += QString("Check suite \"") + suite + QString("\" can't be loaded because the following dependencies are missing: ");
         QList<QString> dependencies = gui.missingCheckDependencies.values(suite);
         for (int j = 0; j != dependencies.size(); ++j)
         {
            warningMessage += dependencies[j] + QString(", ");
         }
         warningMessage.remove(warningMessage.size() - 2, 2);
         warningMessage += ".\n\n";
      }
      QMessageBox::information(nullptr, warningTitle, warningMessage);
   }

   buttonsSetEnabledState(gui);
   if (gui.displayTimeWidget)
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.timeIndex);
   }
   else
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.blankIndex);
   }
   gui.loadChecksButtonStack->setCurrentIndex(gui.loadChecksButtonIndex);
}

void ScenarioAnalyzer::onLoadChecksCanceled(ScenarioAnalyzerGui& gui)
{
   gui.loadingChecks = false;

   QTextCursor cursor(gui.checkLoadErrorsView->document());
   cursor.insertText(QString("Check loading canceled"));

   buttonsSetEnabledState(gui);
   gui.checkStackWidget->setCurrentIndex(gui.checkLoadErrorsViewIndex);
   if (gui.displayTimeWidget)
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.timeIndex);
   }
   else
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.blankIndex);
   }
   gui.loadChecksButtonStack->setCurrentIndex(gui.loadChecksButtonIndex);
}

void ScenarioAnalyzer::onRunChecksDone(ScenarioAnalyzerGui& gui)
{
   gui.runningChecks = false;

   gui.runChecksFields->mutex.lock();
   QString uberfileText(gui.runChecksFields->outputs.uberfileText);
   QString wsfExecOutput(gui.runChecksFields->outputs.wsfExecOutput);
   QVector<QString> errors = deepCopy(gui.runChecksFields->outputs.errors);
   gui.runChecksFields->mutex.unlock();

   if (!errors.empty())
   {
      gui.resultsTreeModel->root.name = "All results";
      gui.resultsTreeModel->root.parent = nullptr;
      gui.resultsTreeModel->root.children.clear();
      gui.resultsTreeModel->root.children.reserve(1); // TODO: unnecessary?

      gui.resultsTreeModel->root.children.push_back(ResultsGroupItem());
      ResultsGroupItem* errorsItem = &(gui.resultsTreeModel->root.children[0]);
      errorsItem->children.reserve(errors.size());
      errorsItem->name = QString("ERRORS");
      errorsItem->parent = &(gui.resultsTreeModel->root);

      for (int i = 0, sz = errors.size(); i != sz; ++i)
      {
         errorsItem->children.push_back(ResultsGroupItem());
         ResultsGroupItem* error = &(errorsItem->children.back());
         error->name = errors[i];
         error->parent = errorsItem;
         error->children.clear();
      }

   }
   else
   {
      gui.resultsTreeModel->root.name = "All results";
      gui.resultsTreeModel->root.parent = nullptr;
      gui.resultsTreeModel->root.children.clear();

      gui.resultsTreeModel->constructResultsTree(gui.runChecksFields->outputs.noteMessages,
                                                 gui.runChecksFields->outputs.resultMessages,
                                                 gui.displayTimeWidget);
   }

   buttonsSetEnabledState(gui);
   gui.runChecksButtonStack->setCurrentIndex(gui.runChecksButtonIndex);
   if (gui.displayTimeWidget)
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.timeIndex);
   }
   else
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.blankIndex);
   }
   gui.uberfileView->document()->setPlainText(uberfileText);
   gui.wsfExecOutputView->document()->setPlainText(wsfExecOutput);
   // automatically display down to severity level
   gui.resultsTreeView->customExpand();
   // in case the user closed the dock, make sure it is visible
   gui.resultsDock->show();
   gui.resultsDock->raise();
}

void ScenarioAnalyzer::onRunChecksCanceled(ScenarioAnalyzerGui& gui)
{
   gui.runningChecks = false;

   buttonsSetEnabledState(gui);
   gui.runChecksButtonStack->setCurrentIndex(gui.runChecksButtonIndex);
   if (gui.displayTimeWidget)
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.timeIndex);
   }
   else
   {
      gui.progressAndTimeStack->setCurrentIndex(gui.blankIndex);
   }
};

void ScenarioAnalyzer::initiateLoadChecks(ScenarioAnalyzerGui& gui)
{
   assert(gui.suiteDir != nullptr);

   gui.loadingChecks = true;
   gui.suiteFiles.clear();

   gui.loadChecksFields->mutex.lock();
   gui.loadChecksFields->inputs.cancelSignal.store(false);
   gui.loadChecksFields->inputs.checkSuiteDir = *gui.suiteDir;
   gui.loadChecksFields->inputs.sessionNoteDir = *gui.sessionNoteDir;
   // gui.wsfExecExe will not be valid until a project is opened in the IDE
   gui.loadChecksFields->inputs.wsfExecExe = (gui.wsfExecExe ? *gui.wsfExecExe : AbsoluteFile{ QString() });
   gui.loadChecksFields->mutex.unlock();

   auto loadChecksTask = new LoadChecksTask();
   loadChecksTask->fields = gui.loadChecksFields;

   auto doneAction = [&gui]()
   {
      onLoadChecksDone(gui);
   };

   auto cancelAction = [&gui]()
   {
      onLoadChecksCanceled(gui);
   };

   QObject::connect(
      loadChecksTask,
      &LoadChecksTask::done,
      gui.selectDock,
      doneAction,
      Qt::QueuedConnection);
   QObject::connect(
      loadChecksTask,
      &LoadChecksTask::canceled,
      gui.selectDock,
      cancelAction,
      Qt::QueuedConnection);

   // NOTE: the task's autoDelete flag is true, so the thread pool deletes it
   QThreadPool::globalInstance()->start(loadChecksTask);

   buttonsSetEnabledState(gui);
   gui.loadChecksButtonStack->setCurrentIndex(gui.loadChecksCancelButtonIndex);
   gui.selectCheckModel->clear();
   gui.selectCheckView->setEnabled(false);
   gui.checkLoadErrorsView->setPlainText(QString());
   gui.progressLabel->setText("Loading checks...");
   gui.progressAndTimeStack->setCurrentIndex(gui.progressIndex);
   gui.loadChecksCancelButton->setEnabled(true);
}

void ScenarioAnalyzer::initiateCancelLoadChecks(ScenarioAnalyzerGui& gui)
{
   gui.loadChecksFields->inputs.cancelSignal.store(true);
   gui.loadChecksCancelButton->setEnabled(false);
}

void ScenarioAnalyzer::initiateRunChecks(ScenarioAnalyzerGui& gui)
{
   assert(gui.wsfExecExe != nullptr);

   gui.runningChecks = true;

   QVector<QString> selectedChecks;
   int checkCount = 0;
   for (int i = 0; i != gui.selectCheckModel->root.children.size(); ++i)
   {
      checkCount += gui.selectCheckModel->root.children[i].children.size();
   }
   selectedChecks.reserve(checkCount);

   // NOTE: iterating over the check list must be done in the GUI thread
   // to prevent a data race.
   for (int i = 0; i != gui.selectCheckModel->root.children.size(); ++i)
   {
      for (int j = 0; j != gui.selectCheckModel->root.children[i].children.size(); ++j)
      {
         CheckTreeItem* check = &(gui.selectCheckModel->root.children[i].children[j]);
         if (check->selectedCount == 1)
         {
            selectedChecks.push_back(check->name);
         }
      }
   }

   if (!gui.missingSessionNoteDependencies.empty())
   {
      QString warningTitle("Scenario Analyzer: Session Note Suites Unavailable");
      QString warningMessage;
      QList<QString> unavailableSuites = gui.missingSessionNoteDependencies.uniqueKeys();
      for (int i = 0; i != unavailableSuites.size(); ++i)
      {
         QString suite = unavailableSuites[i];
         warningMessage += QString("Session Note suite \"") + suite
                           + QString("\" will not run because the following dependencies are missing: ");
         QList<QString> dependencies = gui.missingSessionNoteDependencies.values(suite);
         for (int j = 0; j != dependencies.size(); ++j)
         {
            warningMessage += dependencies[j] + QString(", ");
         }
         warningMessage.remove(warningMessage.size() - 2, 2);
         warningMessage += ".\n\n";
      }
      QMessageBox::information(nullptr, warningTitle, warningMessage);
   }


   gui.runChecksFields->mutex.lock();
   gui.runChecksFields->inputs.cancelSignal.store(false);
   // passing only "valid" files ensure that only suites for which all dependencies are present
   // will be #included in the uberfile
   gui.runChecksFields->inputs.suiteFiles = gui.validSuiteFiles;
   gui.runChecksFields->inputs.sessionNoteFiles = gui.validSessionNoteFiles;
   gui.runChecksFields->inputs.checkNames = selectedChecks;
   gui.runChecksFields->inputs.sessionNoteNames = gui.validSessionNoteNames;
   gui.runChecksFields->inputs.wsfExecExe = *gui.wsfExecExe;
   gui.runChecksFields->inputs.scenarioFiles = gui.scenarioFiles;

   // If we've given user the option to set the time checks should be executed, and user has
   // entered valid input, use this time as the time to execute checks.
   if (gui.setCheckTimeLineEdit->hasAcceptableInput())
   {
      unsigned int seconds = TimeLabel::convertTimeToSeconds(gui.setCheckTimeLineEdit->text());
      gui.runChecksFields->inputs.executeTimeSeconds = seconds;
   }
   else
   {
      gui.runChecksFields->inputs.executeTimeSeconds = 0;
   }

   gui.runChecksFields->mutex.unlock();

   auto runChecksTask = new RunChecksTask();
   runChecksTask->fields = gui.runChecksFields;

   auto doneAction = [&gui]()
   {
      onRunChecksDone(gui);
   };

   auto cancelAction = [&gui]()
   {
      onRunChecksCanceled(gui);
   };

   QObject::connect(
      runChecksTask,
      &RunChecksTask::done,
      gui.resultsDock,
      doneAction,
      Qt::QueuedConnection);
   QObject::connect(
      runChecksTask,
      &RunChecksTask::canceled,
      gui.resultsDock,
      cancelAction,
      Qt::QueuedConnection);

   // NOTE: the task's autoDelete flag is true, so the thread pool deletes it
   QThreadPool::globalInstance()->start(runChecksTask);

   buttonsSetEnabledState(gui);
   gui.runChecksButtonStack->setCurrentIndex(gui.runChecksCancelButtonIndex);
   gui.resultsTreeModel->clear();
   gui.uberfileView->document()->setPlainText(QString());
   gui.wsfExecOutputView->document()->setPlainText(QString());
   gui.progressAndTimeStack->setCurrentIndex(gui.progressIndex);
   gui.progressLabel->setText("Running checks...");
   gui.progressAndTimeStack->setCurrentIndex(gui.progressIndex);
   gui.runChecksCancelButton->setEnabled(true);
}

void ScenarioAnalyzer::initiateCancelRunChecks(ScenarioAnalyzerGui& gui)
{
   gui.runChecksFields->inputs.cancelSignal.store(true);
   gui.runChecksCancelButton->setEnabled(false);
}

void ScenarioAnalyzer::cancelActiveTasks(ScenarioAnalyzerGui& gui)
{
   gui.loadChecksFields->inputs.cancelSignal.store(true);
   gui.runChecksFields->inputs.cancelSignal.store(true);
}
