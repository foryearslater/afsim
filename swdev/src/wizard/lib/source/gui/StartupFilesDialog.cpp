// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "StartupFilesDialog.hpp"

#include <QMessageBox>

#include "RunManager.hpp"

wizard::StartupFilesDialog::StartupFilesDialog(QWidget*                   aParent,
                                               const std::vector<UtPath>& aStartupFiles,
                                               const UtPath&              aProjectDir)
   : QDialog(aParent)
   , mUI()
   , mStartupFiles(aStartupFiles)
   , mProjectDir(aProjectDir)
{
   mUI.setupUi(this);
   setWindowModality(Qt::ApplicationModal);
   show();

   connect(mUI.buttons, &QDialogButtonBox::accepted, [this] { accept(); });
   connect(mUI.buttons, &QDialogButtonBox::rejected, [this] { this->close(); });
}

void wizard::StartupFilesDialog::accept()
{
   std::vector<UtPath> startupFiles;
   QStringList         pathList = mUI.startupFileText->toPlainText().split('\n');
   for (int i = 0; i < pathList.size(); ++i)
   {
      QString path = pathList[i].trimmed();
      if (path.isEmpty())
      {
         continue;
      }
      UtPath utPath = mProjectDir + path.toStdString();
      if (utPath.Stat() == UtPath::cFILE)
      {
         startupFiles.push_back(utPath);
      }
      else
      {
         QMessageBox::critical(this,
                               "File does not exists",
                               QString::fromStdString("File does not exist: " + utPath.GetSystemPath()),
                               QMessageBox::Ok);
         return;
      }
   }
   // if the first start-up file changed, set the working directory to its directory
   if (wizRunMgr.SetWorkingDirectoryToProject(startupFiles.front()))
   {
      mStartupFiles = startupFiles;
   }
   QDialog::accept();
}

void wizard::StartupFilesDialog::showEvent(QShowEvent*)
{
   QStringList files;
   for (const auto& startupFile : mStartupFiles)
   {
      QString path = QString::fromStdString(mProjectDir.GetRelativePath(startupFile).GetNormalizedPath());
      files.push_back(path);
   }
   mUI.startupFileText->setText(files.join("\n"));
}

const std::vector<UtPath>& wizard::StartupFilesDialog::startupFiles() const
{
   return mStartupFiles;
}
