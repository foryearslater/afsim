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

#include "RunManager.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTextDocument>

#include "Environment.hpp"
#include "RunEnvManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::RunManager* wizard::RunManager::mInstancePtr = nullptr;


wizard::RunManager::RunManager()
   : mMaxRecentScenarios(12)
   , mFileList()
   , mApplicationPath(QString::fromStdString(RunEnvManager::GetBinDir().GetNormalizedPath()))
{
   mInstancePtr = this;
   ReadSettings();
}

wizard::RunManager::~RunManager()
{
   mInstancePtr = nullptr;
}

wizard::RunManager& wizard::RunManager::GetInstance()
{
   assert(mInstancePtr);
   return *mInstancePtr;
}

void wizard::RunManager::Create()
{
   assert(!mInstancePtr);
   if (!mInstancePtr)
   {
      new RunManager();
   }
}

void wizard::RunManager::RemoveRecentScenario(const QStringList& aFileList)
{
   QStringList scenario = StandardizeFilePaths(aFileList);
   if (mRecentScenarios.removeAll(scenario) > 0)
   {
      WriteHistory();
   }
}

void wizard::RunManager::ClearRecentScenarios()
{
   mRecentScenarios.clear();
   // Clear the "Open Recent" menu
   wizEnv.PopulateRecents();
   WriteHistory();
}

QString wizard::RunManager::GetMostRecentDirectory()
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

bool wizard::RunManager::ReloadScenario()
{
   return LoadScenario(mFileList);
}

bool wizard::RunManager::LoadScenario(const QStringList& aFileList)
{
   // Store the options to execute scenarios later.

   QStringList files;
   QString     afprojFile = "";

   for (auto&& it : aFileList)
   {
      QFileInfo info(it);
      if (info.suffix() == "afproj")
      {
         if (aFileList.size() > 1)
         {
            QMessageBox::warning(nullptr,
                                 "Attempted to load more than one file with a project file.",
                                 QString("Project files may not be loaded with a list of other files.  Only ") + it +
                                    " will be loaded.");
            // let the user know that we are only going to load the afproj file
         }
         afprojFile = it;
         break;
      }
   }
   if (!afprojFile.isEmpty())
   {
      mFileList = QStringList({afprojFile});
   }
   else
   {
      mFileList = aFileList;
   }

   if (!mFileList.empty())
   {
      // The main scenario file is the first scenario file listed.
      QString mainScenarioFile = mFileList.front();

      if (!SetWorkingDirectoryToProject(mainScenarioFile))
      {
         QMessageBox::warning(nullptr,
                              "Change Directory Error",
                              QString("Changing the current working directory to ") + mCurrentProjectPath +
                                 " failed.\nScenario may not be execute correctly.");
      }

      files = mFileList;

      SetMostRecentScenario(mFileList);
   }

   if (wkfEnv.GetMainWindow())
   {
      wkfEnv.GetMainWindow()->setWindowTitle(QString("[ %1 ]").arg(GetScenarioDisplayName(files, true)));
   }

   if (!wizEnv.LoadScenario(mFileList))
   {
      QMessageBox::warning(nullptr, "Invalid Scenario", "There was an error loading the scenario.");
   }

   return true;
}

void wizard::RunManager::Shutdown()
{
   if (mInstancePtr)
   {
      delete mInstancePtr;
   }
}

QString wizard::RunManager::GetScenarioDisplayName(const QStringList& aFileList, bool aDisplayFullPath)
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

QString wizard::RunManager::GetApplicationPath() const noexcept
{
   return mApplicationPath;
}

bool wizard::RunManager::SetWorkingDirectoryToApplication() noexcept
{
   // prevent unnecessary setting of working directory
   if (!IsWorkingDirectoryApplication())
   {
      mCurrentProjectPath.clear();
      return QDir::setCurrent(mApplicationPath);
   }
   return true;
}

bool wizard::RunManager::IsWorkingDirectoryApplication() const noexcept
{
   return QDir::currentPath() == mApplicationPath;
}

QString wizard::RunManager::GetCurrentProjectPath() const noexcept
{
   return mCurrentProjectPath;
}

bool wizard::RunManager::SetWorkingDirectoryToProject(const QString& aMainScenarioFile) noexcept
{
   // prevent unnecessary setting of working directory
   auto filePath = QFileInfo(aMainScenarioFile).dir().absolutePath();
   if (!aMainScenarioFile.isEmpty() && mCurrentProjectPath != filePath)
   {
      mCurrentProjectPath = filePath;
      return QDir::setCurrent(mCurrentProjectPath);
   }
   return true;
}

bool wizard::RunManager::SetWorkingDirectoryToProject(const std::string& aMainScenarioFile) noexcept
{
   return SetWorkingDirectoryToProject(QString::fromStdString(aMainScenarioFile));
}

bool wizard::RunManager::SetWorkingDirectoryToProject(const UtPath& aMainScenarioFile) noexcept
{
   return SetWorkingDirectoryToProject(QString::fromStdString(aMainScenarioFile.GetNormalizedPath()));
}

bool wizard::RunManager::IsWorkingDirectoryProject() const noexcept
{
   return QDir::currentPath() == mCurrentProjectPath;
}

void wizard::RunManager::ReadSettings()
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
}

void wizard::RunManager::WriteHistory()
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

void wizard::RunManager::SetMostRecentScenario(const QStringList& aFileList)
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
   }
}

QStringList wizard::RunManager::StandardizeFilePaths(const QStringList& aFileList, bool* aFilesExist)
{
   // Standardize all file paths through QFileInfo for exact matching
   QStringList files;
   if (aFilesExist)
   {
      *aFilesExist = true;
   }
   foreach (const QString& filepath, aFileList)
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
