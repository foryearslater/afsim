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

#include "WkRunManager.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QTextDocument>

#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "UtQtException.hpp"
#include "UtTypeInfo.hpp"
#include "WkSimEnvironment.hpp"
#include "WkSimPrefObject.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

namespace warlock
{
RunManager* warlock::RunManager::mInstancePtr = nullptr;

RunManager::RunManager(WsfStandardApplication& aApp)
   : mApp(aApp)
{
   mInstancePtr = this;
   ReadSettings();
}

void RunManager::HandleScenarioLoadFinished()
{
   mProgressDialog->hide();
   if (mLoadThread->WasSuccessful())
   {
      if (wkfEnv.GetMainWindow())
      {
         QStringList list;
         for (const auto& file : mOptions.mInputFiles)
         {
            list.push_back(QString::fromStdString(file));
         }

         wkfEnv.GetMainWindow()->setWindowTitle(QString("[ %1 ]").arg(GetScenarioDisplayName(list, true)));
      }

      AssignScenarioToSimThread(mLoadThread->GetScenario());
   }
   else
   {
      QMessageBox::critical(nullptr, "Load Error", mLoadThread->GetError());

      // Do we want to provide a way to retry the last scenario we attempted to load?
      //  This would be different than the most recent, because recent is the last
      //  successful scenario to be loaded.
   }

   // Clean up the thread
   mLoadThread->deleteLater();
   mLoadThread = nullptr;
}

RunManager& warlock::RunManager::GetInstance()
{
   assert(mInstancePtr);
   return *mInstancePtr;
}

void RunManager::Create(WsfStandardApplication& aApp)
{
   assert(!mInstancePtr);
   if (!mInstancePtr)
   {
      new RunManager(aApp);
   }
}

void RunManager::RemoveRecentScenario(const QStringList& aFileList)
{
   QStringList scenario = StandardizeFilePaths(aFileList);
   if (mRecentScenarios.removeAll(scenario) > 0)
   {
      WriteHistory();
   }
}

void RunManager::ClearRecentScenarios()
{
   mRecentScenarios.clear();
   WriteHistory();
}

QString RunManager::GetMostRecentDirectory() const
{
   QString retVal;
   if (!mRecentScenarios.empty())
   {
      QFileInfo info(mRecentScenarios.front().front());
      if (info.exists())
      {
         retVal = info.absolutePath();
      }
   }
   return retVal;
}

void RunManager::ReloadScenario()
{
   LoadScenario(mOptions);
}

void RunManager::LoadScenario(const QStringList& aFileList)
{
   WsfStandardApplication::Options options;
   for (const QString& file : aFileList)
   {
      options.mInputFiles.push_back(file.toStdString());
   }
   LoadScenario(options);
}

void RunManager::LoadScenario(const WsfStandardApplication::Options& aOptions)
{
   if (mLoadThread)
   {
      QMessageBox::warning(nullptr,
                           "Load Scenario Error",
                           "Could not load the scenario.  Another scenario is in the process of being loaded.");
      return;
   }

   // Create the Progress Dialog if it has not been created yet.
   if (!mProgressDialog)
   {
      // Set up Progress Dialog
      mProgressDialog =
         new QProgressDialog(QString{},
                             QString{},
                             0,
                             0,
                             wkfEnv.GetMainWindow(),
                             Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::MSWindowsFixedSizeDialogHint);
      mProgressDialog->setWindowModality(Qt::ApplicationModal);
      mProgressDialog->setSizeGripEnabled(false);
   }

   // Store the options to execute scenarios later.
   mOptions = aOptions;

   if (IsSimulationActive())
   {
      // Display Progress Dialog to the user to let them know we are terminating the current simulation.
      mProgressDialog->setLabelText("Terminating Simulation...");
      mProgressDialog->show();

      // Set the Pending Scenario Load flag, so that we know when the simulation terminated, to complete the rest of the
      // scenario loading.
      mPendingLoading = true;

      // Send request to the simulation thread to terminate the simulation, once the simulation is terminated the
      // simulation thread will complete.
      mSimThread->TerminateSimulation();
   }
   else
   {
      StartLoading();
   }
}

void RunManager::StartLoading()
{
   QStringList files;
   if (mOptions.mInputFiles.empty())
   {
      QMessageBox::warning(nullptr, "Load Scenario Error", "Scenario file list is empty. No scenario files to load.");
      return;
   }
   else
   {
      // Loop over all input files and convert relative paths to absolute paths.
      // This allows us to change the current working directory and still load the files.
      for (const auto& scenarioFile : mOptions.mInputFiles)
      {
         QFileInfo info(QString::fromStdString(scenarioFile));
         if (info.exists())
         {
            // store all paths as absolute file paths
            files << info.absoluteFilePath();
         }
         else
         {
            // if the file was not found, store as is, it will be handled later in ProcessInputFiles()
            files << QString::fromStdString(scenarioFile);
         }
      }

      // Update mOptions.mInputFiles with the absolute file paths instead
      mOptions.mInputFiles.clear();
      for (const auto& file : files)
      {
         mOptions.mInputFiles.emplace_back(file.toStdString());
      }

      // The main scenario file is the first scenario file listed.
      QString mainScenarioFile = files.front();

      // Initially: I don't like changing our working directory when loading a scenario but AFSIM
      //  assumes the current working directory is the same as the directory the scenario
      //  file is located in. Once this is fixed in AFSIM, we can remove this.
      // However: This (QDir::setCurrent) also affects the QFileDialog initial browse directory
      //  so we benefit from it beyond its use in AFSIM.
      // Conclusion: We will continue to set the current working directory until we find an issue
      //  that would prevent this from being a viable solution.
      QFileInfo info(mainScenarioFile);
      if (!QDir::setCurrent(info.dir().absolutePath()))
      {
         QMessageBox::warning(nullptr,
                              "Change Directory Error",
                              QString("Changing the current working directory to ") + info.dir().absolutePath() +
                                 " failed.\nScenario may not be execute correctly.");
      }

      // Set All scenarios in the list as the Recent scenario.
      // Note: this means the last scenario to load will be treated as the "Most Recent"
      SetMostRecentScenario(files);

      // Override the DIS settings if the user specified DIS settings within Warlock
      std::string input;
      const auto& simPrefData = wkfEnv.GetPreferenceObject<SimPrefObject>()->GetPreferences();
      if (simPrefData.enableDIS)
      {
         input = std::string("dis_interface\n") + "multicast " + simPrefData.multicastIp.toStdString() + " " +
                 simPrefData.netId.toStdString() + "\n" + "port " + std::to_string(simPrefData.port) + "\n" +
                 "application " + std::to_string(simPrefData.applicationId) + "\n" + "site " +
                 std::to_string(simPrefData.siteId) + "\n" + "exercise " + std::to_string(simPrefData.exerciseId) + "\n";
         if (!simPrefData.startPaused)
         {
            input += "autostart\n";
         }
         if (simPrefData.enabledDeferredStart)
         {
            input += "deferred_connection_time " + std::to_string(simPrefData.deferredStartTimeSec) + " sec\n";
         }
         input += "end_dis_interface\n";
      }

      // Display the Progress Bar for loading a scenario
      mProgressDialog->setLabelText(QString("Loading Scenario...\n%1").arg(GetScenarioDisplayName(files, false)));
      mProgressDialog->show();

      // Load the scenario on a different thread and connect to finished() signal
      mLoadThread = new LoadThread(this, mApp, mOptions, input);
      connect(mLoadThread, &QThread::finished, this, &RunManager::HandleScenarioLoadFinished);
      mLoadThread->start();
   }
}

void RunManager::CreateSimThread(std::unique_ptr<WsfScenario> aScenarioPtr)
{
   mSimThread = new SimThread(this, mApp, std::move(aScenarioPtr), mOptions);
   simEnv.moveToThread(mSimThread); // Move warlock::SimEnvironment (simEnv) to thread the simulation is executing on

   // Connect the finished signal
   connect(mSimThread,
           &QThread::finished,
           this,
           [this]()
           {
              // Since the simulation thread is no longer executing, there is no need to trigger the UpdateFrame signals
              // from the environment. UpdateFrame will have to be enabled once the simulation thread starts up again
              wkfEnv.SetFrameUpdateEnabled(false);

              // Hide the progress dialog as we are now done terminating the simulation.
              mProgressDialog->hide();

              if (mPendingLoading)
              {
                 mPendingLoading = false;
                 // Proceed with loading the scenario now the simulation is terminated.
                 StartLoading();
              }
           });
}

void RunManager::Shutdown()
{
   if (mSimThread != nullptr)
   {
      // Tell the simulation to terminate
      mSimThread->TerminateSimulation();
      // wait forever for the simulation thread to finish
      mSimThread->wait();
   }

   if (mInstancePtr)
   {
      delete mInstancePtr;
   }
}

bool RunManager::IsSimulationActive() const
{
   return (mSimThread != nullptr && mSimThread->isRunning());
}

QString RunManager::GetScenarioDisplayName(const QStringList& aFileList, bool aDisplayFullPath)
{
   QString text;

   if (!aFileList.empty())
   {
      QFileInfo fi(aFileList.front());
      text = aDisplayFullPath ? fi.absoluteFilePath() : fi.fileName();

      if (aFileList.size() > 1)
      {
         text.append(QString(" + %1 more...").arg(aFileList.size() - 1));
      }
   }

   return text;
}

void RunManager::ReadSettings()
{
   QSettings& settings = wkfEnv.GetAppDataSettingsFile();

   settings.beginGroup("RunManager");

   int size = settings.beginReadArray("History");
   for (int i = 0; i < size; ++i)
   {
      settings.setArrayIndex(i);
      QStringList scenario = settings.value("scenario").toStringList();
      if (!scenario.empty())
      {
         mRecentScenarios.push_back(scenario);
      }
   }
   while (mRecentScenarios.size() > mMaxRecentScenarios)
   {
      mRecentScenarios.removeLast();
   }
   settings.endArray();
   settings.endGroup();

   emit RecentScenarioListChanged();
}

void RunManager::WriteHistory()
{
   int        count    = 0;
   QSettings& settings = wkfEnv.GetAppDataSettingsFile();

   settings.beginGroup("RunManager");
   settings.remove("History");
   settings.beginWriteArray("History", mRecentScenarios.size());
   for (const QStringList& scenario : mRecentScenarios)
   {
      settings.setArrayIndex(count);
      settings.setValue("scenario", scenario);
      count++;
   }
   settings.endArray();
   settings.endGroup();
}

void RunManager::SetMostRecentScenario(const QStringList& aFileList)
{
   // Remove it from the list of recent scenarios, if it was already in the list.
   RemoveRecentScenario(aFileList);

   // Insert at the beginning of the list
   bool        filesExist;
   QStringList scenario = StandardizeFilePaths(aFileList, &filesExist);
   if (filesExist)
   {
      mRecentScenarios.push_front(scenario);
      if (mRecentScenarios.size() > mMaxRecentScenarios)
      {
         mRecentScenarios.removeLast();
      }
      WriteHistory();

      emit RecentScenarioListChanged();
   }
}

void RunManager::AssignScenarioToSimThread(std::unique_ptr<WsfScenario> aScenarioPtr)
{
   // If the sim thread has not been created yet, make it
   if (mSimThread == nullptr)
   {
      CreateSimThread(std::move(aScenarioPtr));
   }
   else
   {
      // This call, will request termination of the simulation thread if it is active.
      mSimThread->SetCurrentScenario(std::move(aScenarioPtr));
   }

   const auto& simPrefData = wkfEnv.GetPreferenceObject<SimPrefObject>()->GetPreferences();
   mSimThread->SetSimulationExternallyStarted(simPrefData.startPaused);

   // When AssignScenarioToThread() was called the Simulation Thread is in one of two states:
   //  Active with a scenario executing, or Inactive with no valid scenario
   // If Active, mSimThread->isRunning() will be false.
   // If Inactive, mSimThread->isRunning() may differ in the following three ways:
   //  1) SimThread was requested to terminate, and it successfully terminate prior to this thread reaching this line.
   //     In this case the SimThread::HandleFinished() will have launched the new scenario, thus mSimThread->isRunning
   //     will be true, because the new scenario is already executing
   //  2) SimThread was requested to terminate, and it has not yet terminated.  When the thread successfully terminates,
   //     SimThread::HandleFinished() will launch the new thread.  mSimThread->isRunning() will be true due to the old
   //     thread still executing, while waiting for the Termination Request to be handled.
   //  3) SimThread was requested to terminate, and it successfully terminate prior to this thread reaching this line.
   //     In this case the SimThread::HandleFinished() will have launched the new scenario, however it is possible the
   //     new scenario has an issue while loading (such as Initialization Failed). In this case it is possible that the
   //     new scenario has already terminated already due to an error, and thus mSimThread->isRunning() will be false.
   //     However in this case, when mSimThread->start() is called, nothing should happen due to mCurrentScenario being invalid.
   if (!mSimThread->isRunning())
   {
      qApp->processEvents();
      mSimThread->start(); // Start the simulation event loop thread
   }
}

QStringList RunManager::StandardizeFilePaths(const QStringList& aFileList, bool* aFilesExist)
{
   // Standardize all file paths through QFileInfo for exact matching
   QStringList files;
   if (aFilesExist)
   {
      *aFilesExist = true;
   }
   for (const QString& filepath : aFileList)
   {
      QFileInfo fi(filepath);
      if (!fi.exists())
      {
         if (aFilesExist)
         {
            *aFilesExist = false;
         }
      }
      files << fi.absoluteFilePath();
   }
   return files;
}


RunManager::LoadThread::LoadThread(QObject*                               aParent,
                                   WsfStandardApplication&                aApplication,
                                   const WsfStandardApplication::Options& aOptions,
                                   const std::string&                     aAdditionalInput)
   : QThread(aParent)
   , mApplication(aApplication)
   , mOptions(aOptions)
   , mAdditionalInput(aAdditionalInput)
{
}

void RunManager::LoadThread::run()
{
   // Create a scenario
   mScenarioPtr = ut::make_unique<WsfScenario>(mApplication);
   try
   {
      // Read WSF input files into the scenario
      mApplication.ProcessInputFiles(*mScenarioPtr, mOptions.mInputFiles);
      mApplication.ProcessCommandLineCommands(*mScenarioPtr, mOptions);

      if (!mAdditionalInput.empty())
      {
         try
         {
            UtInput networkInput;
            networkInput.PushInputString(mAdditionalInput);
            mScenarioPtr->ProcessInput(networkInput);
         }
         catch (UtInput::ExceptionBase&)
         {
            mErrorStr = Qt::convertFromPlainText(
               "An exception occurred while configuring the DIS network settings.\nTo resolve this issue, change your "
               "DIS settings in the Simulation page of the Preferences dialog.\n\n");
            throw; // re-throw this exception to be handled by outer catch block
         }
      }

      mSuccessful = true;
   }
   catch (UtException& e)
   {
      mScenarioPtr.reset(); // Clean up scenario
      std::cout << e.what() << std::endl;

      mErrorStr += Qt::convertFromPlainText(e.what(), Qt::WhiteSpaceNormal);
      // Find ***** and make font red
      mErrorStr.replace(QRegExp("(\\*\\*\\*\\*\\* \\w+: )"), "<span style=\"color: red\">\\1</span>");
      // RegEx to capture where error occurred
      QRegExp rx("(in.'([^']*)'?,.line \\d+,.near column.\\d+)");
      if (mErrorStr.contains(rx))
      {
         if (rx.cap(2).isEmpty()) // no filename captured, error was in additional input string
         {
            mErrorStr.remove(rx); // remove the file, line, column info
         }
         else
         {
            // provide hyper-link for file with error
            mErrorStr.replace(rx.cap(2), QString("<a href=\"file:///%1\">%1</a>").arg(rx.cap(2)));
         }
      }
   }
}

RunManager::SimThread::SimThread(QObject*                               aParent,
                                 WsfStandardApplication&                aApplication,
                                 std::unique_ptr<WsfScenario>           aScenarioPtr,
                                 const WsfStandardApplication::Options& aOptions)
   : QThread(aParent)
   , mApplication(aApplication)
   , mPendingScenarioPtr(std::move(aScenarioPtr))
   , mOptions(aOptions)
{
   connect(this,
           &QThread::finished,
           this,
           [this]()
           {
              if (mPendingScenarioPtr != nullptr)
              {
                 start();
              }
           });
}

void RunManager::SimThread::SetCurrentScenario(std::unique_ptr<WsfScenario> aScenarioPtr)
{
   TerminateSimulation();

   // If we have a pending scenario that has not been run, and someone is requesting to run a different scenario,
   //  go ahead and overwrite the pending scenario as we will not run it. (this should never happen)
   mPendingScenarioPtr = std::move(aScenarioPtr);
}

void RunManager::SimThread::TerminateSimulation()
{
   // RequestTermination() is not a thread safe function, however it is unlikely to cause collisions due to the
   // implementation setting two variables that rarely change.
   // If crashes start occurring here when selecting new scenarios, look at making this section of code thread-safe.
   if (mSimulationPtr != nullptr)
   {
      mSimulationPtr->RequestTermination();
   }
}

void RunManager::SimThread::run()
{
   ut::SetupThreadErrorHandling();

   auto currentScenarioPtr = std::move(mPendingScenarioPtr);

   // It is possible for current scenario to be invalid. Please read lengthy comment in AssignScenarioToThread() for how.
   if (currentScenarioPtr)
   {
      // Determine if scenario is set to run Monte Carlo.
      if (currentScenarioPtr->GetInitalRunNumber() != currentScenarioPtr->GetFinalRunNumber())
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(
            QMessageBox::Warning,
            "Monte Carlo Simulation",
            "Monte Carlo simulation is not supported in Warlock.\nOnly the first run will be executed.");
      }

      bool reset = false;
      do
      {
         mSimulationPtr =
            mApplication.CreateSimulation(*currentScenarioPtr, mOptions, currentScenarioPtr->GetInitalRunNumber());
         // When running in Warlock, set all simulations to real-time.
         mSimulationPtr->SetRealtime(0, true);

         // This may not exist due to user's option to not launch Warlock Environment
         if (warlock::SimEnvironment::Exists())
         {
            simEnv.Attach(*mSimulationPtr); // Attach warlock to the simulation object
         }

         if (!mApplication.InitializeSimulation(mSimulationPtr.get()))
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Failed Initialization",
                                                              "The simulation failed to initialize.");
         }
         // If given no inputs, quit now
         else if (!mOptions.mInputFiles.empty())
         {
            mSimulationPtr->SetIsExternallyStarted(mExternallyStarted);

            try
            {
               auto result = mApplication.RunEventLoop(mSimulationPtr.get(), mOptions);

               // On reset, re-create the same simulation run.
               reset = result.mResetRequested;
            }
            catch (...)
            {
               ut::qt::ReportException("The simulation", std::current_exception());
            }
            // If a stack overflow is caught via ut::HardwareException, the stack guard frames must be reset.
            // This cannot be called within the catch{} block because the stack has not been unwound yet at that point.
            ut::ResetStackOverflow();
         }

         // Clean up the simulation object
         mSimulationPtr = nullptr;
      } while (reset);
   }
}
} // namespace warlock
