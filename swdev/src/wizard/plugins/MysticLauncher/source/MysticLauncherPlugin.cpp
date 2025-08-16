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

#include "MysticLauncherPlugin.hpp"

#include <QMenu>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "Util.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(MysticLauncher::Plugin, "Mystic Launcher", "Launches Mystic", "wizard")

MysticLauncher::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   wizard::FileTypeCache::RegisterDetector(&mDetector);
   wizard::FileTypeCache::SetFileTypeIcon("wsf_pipe", "mystic");

   connect(wizSignals, &wizard::Signals::ActionsForFileRequested, this, &Plugin::ActionsForFileRequested);
}

MysticLauncher::Plugin::~Plugin()
{
   wizard::FileTypeCache::UnregisterDetector(&mDetector);
}

void MysticLauncher::Plugin::ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
{
   if ((wizard::FileTypeCache::DetermineFileType(aFileInfo)) == "wsf_pipe")
   {
      mFileInfo = aFileInfo;
      connect(aMenu->addAction(QIcon::fromTheme("mystic"), "Open with Mystic"), &QAction::triggered, this, &Plugin::Startup);
   }
}

void MysticLauncher::Plugin::ProcessErrorCB(QProcess::ProcessError aError)
{
   QProcess* process = dynamic_cast<QProcess*>(QObject::sender());

   char cdata[2048] = "\0";
   if (process)
   {
      process->read(cdata, 2048);
   }
   QString title = "Mystic Execution Failed!";
   QString error = "Mystic ended execution prematurely - ";
   QString data  = cdata;
   switch (aError)
   {
   case QProcess::FailedToStart:
      error += "Failed to start\n\n";
      break;
   case QProcess::Crashed:
      error += "Crashed\n\n";
      break;
   case QProcess::Timedout:
      error += "Timed-out\n\n";
      break;
   case QProcess::ReadError:
      error += "Read-Error\n\n";
      break;
   case QProcess::WriteError:
      error += "Write-Error\n\n";
      break;
   case QProcess::UnknownError:
      error += "Unknown Error\n\n";
      break;
   }
   error += data;
   QDialog dialog(wkfEnv.GetMainWindow());
   dialog.setWindowTitle(title);
   QVBoxLayout* vbox = new QVBoxLayout(&dialog);
   dialog.setLayout(vbox);
   QTextBrowser* tb = new QTextBrowser(&dialog);
   tb->setReadOnly(true);
   tb->setText(error);
   vbox->addWidget(tb);
   QPushButton* ok = new QPushButton("OK", &dialog);
   connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);
   vbox->addWidget(ok);
   dialog.exec();
}

void MysticLauncher::Plugin::Startup()
{
   UtPath epath = UtPath::GetExePath();
   epath.Up();
   QString workingDir = epath.GetSystemPath().c_str();

#ifdef _WIN32
   QString exePath = workingDir + "/mystic.exe";
#else
   QString exePath = workingDir + "/mystic";
#endif
   QStringList argList;
   argList.push_back(mFileInfo.absoluteFilePath());
   if (wizard::Util::IsExecutable(UtPath(exePath.toStdString())))
   {
      QProcess* process = new QProcess(nullptr);
      connect(process, &QProcess::errorOccurred, this, &Plugin::ProcessErrorCB);
      process->setWorkingDirectory(workingDir);
      process->start(exePath, argList);
   }
   else
   {
      QMessageBox::information(nullptr, "Can't execute", "Cannot execute " + exePath + "; invalid file type.");
   }
}

std::string MysticLauncher::FileDetector::DetectFileType(const QFileInfo& aFileInfo, const std::string& aFileHeader)
{
   if (aFileHeader.size() > cHEADER.size() && aFileHeader.compare(0, cHEADER.size(), cHEADER) == 0)
   {
      return "wsf_pipe";
   }
   else
   {
      return std::string();
   }
}
