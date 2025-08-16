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
#include "ApplicationLauncherToolbar.hpp"

#include <fstream>

#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QProcess>

#include "UtPath.hpp"
#include "UtStringUtil.hpp"
#include "WkRunManager.hpp"
#include "WkfAction.hpp"
#include "WsfStandardApplication.hpp"

WkApplicationLauncher::Toolbar::Toolbar(SimInterface* aSimInterface)
   : QToolBar("Launcher")
   , mSimInterface(aSimInterface)
{
   setIconSize(QSize(16, 16));
   setObjectName("ApplicationLauncherToolbar");

   QIcon wizardIcon = QIcon::fromTheme("wizard");
   mWizardActionPtr = new wkf::Action(wizardIcon, "Wizard", this);
   connect(mWizardActionPtr, &QAction::triggered, this, &Toolbar::HandleWizardAction);

   QIcon mysticIcon = QIcon::fromTheme("mystic");
   mMysticActionPtr = new wkf::Action(mysticIcon, "Mystic", this);
   connect(mMysticActionPtr, &QAction::triggered, this, &Toolbar::HandleMysticAction);
}

void WkApplicationLauncher::Toolbar::HandleWizardAction()
{
   // In Wizard, the first scenario file will be displayed, while the others go under "external items"
   // Command line arguments will always come before the first scenario file
   // the input files in RunManager are limited to scenarios (no config files, etc.)
   QStringList                     argList;
   const std::vector<std::string>& inputFiles = warlock::RunManager::GetInstance().GetApplicationOptions().mInputFiles;
   for (const auto& it : inputFiles)
   {
      argList.push_back(QDir::fromNativeSeparators(QString::fromStdString(it)));
   }
#ifdef _WIN32
   StartApplication("/wizard.exe", argList);
#else
   StartApplication("/wizard", argList);
#endif
}

void WkApplicationLauncher::Toolbar::HandleMysticAction()
{
   if (mSimInterface->GetOutputFile().empty())
   {
      QMessageBox::warning(
         this,
         "Application Launcher",
         "Unable to open record file.\n\nThe currently loaded scenario does not specify an AFSIM event recording file "
         "(.aer). To add a record file to the scenario, add the event_pipe block.");
   }
   else
   {
      QStringList argList;
      QString     outputFilePath =
         QDir::fromNativeSeparators(QString::fromStdString(UtPath::WorkingDirectory().GetSystemPath()) +
                                    QDir::separator() + QString::fromStdString(mSimInterface->GetOutputFile()));
      argList.push_back(outputFilePath);
#ifdef _WIN32
      StartApplication("/mystic.exe", argList);
#else
      StartApplication("/mystic", argList);
#endif
   }
}

void WkApplicationLauncher::Toolbar::StartApplication(const QString& aExecutableName, const QStringList& aArgumentList)
{
   QString executableDirectory = QDir::fromNativeSeparators(QCoreApplication::applicationDirPath());
   QString executablePath      = executableDirectory + aExecutableName;
   if (IsExecutable(executablePath.toStdString()))
   {
      QProcess* process = new QProcess(nullptr);
      process->setWorkingDirectory(executableDirectory);
      process->start(executablePath, aArgumentList);
   }
   else
   {
      QString errorMessage =
         "Could not find \"" + aExecutableName + "\" in the executable directory (\"" + executableDirectory + "\").";
      QMessageBox::warning(this, "Application Launcher", errorMessage);
   }
}

bool WkApplicationLauncher::Toolbar::IsExecutable(const UtPath& aExePath)
{
   if (aExePath.Stat() == UtPath::cFILE)
   {
#ifdef _WIN32
      // File extension can be tested on windows only
      std::string extension = aExePath.GetFileExtension();
      UtStringUtil::ToLower(extension);
      if (extension != ".exe")
      {
         return false;
      }
#endif
      std::string   sysPath = aExePath.GetSystemPath();
      std::ifstream infile(sysPath.c_str(), std::ios::binary);
      char          hdr[5] = {};
      infile.read(hdr, 4);
      infile.close();
      bool exeHeader(false);
#ifdef _WIN32
      exeHeader = (hdr[0] == 'M' && hdr[1] == 'Z');
#else
      // Executable and link format
      exeHeader = (hdr[0] == 127 && hdr[1] == 'E' && hdr[2] == 'L' && hdr[3] == 'F');
#endif
      return exeHeader;
   }
   return false;
}
