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

#include "ProjectSettingsDlg.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>

#include "Project.hpp"
#include "WsfExeManager.hpp"

wizard::ProjectSettingsDialog::ProjectSettingsDialog(QWidget* aParentPtr)
   : QDialog(aParentPtr)
{
   mUi.setupUi(this);
   mUi.rootDirectoryBrowseBn->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
   mUi.commandLineReset->setIcon(QIcon::fromTheme("reset"));

   connect(mUi.rootDirectoryBrowseBn, &QToolButton::clicked, this, &ProjectSettingsDialog::BrowseClick);
   connect(mUi.okBn, &QPushButton::clicked, this, &ProjectSettingsDialog::OkClick);
   connect(mUi.cancelBn, &QPushButton::clicked, this, &ProjectSettingsDialog::CancelClick);
   connect(mUi.commandLineReset, &QToolButton::clicked, this, &ProjectSettingsDialog::ClearCommandLine);
}

void wizard::ProjectSettingsDialog::Open(Project* aProjectPtr)
{
   mScenarioPtr = aProjectPtr;

   connect(mUi.configureStartupFiles, &QPushButton::clicked, aProjectPtr, &Project::ShowStartupFileDialog, Qt::UniqueConnection);

   UtPath path = aProjectPtr->WorkingDirectory();

   mUi.projectDirectory->setText(aProjectPtr->ProjectDirectory().GetSystemPath().c_str());

   mUi.rootDirectoryInput->setText(path.GetSystemPath().c_str());

   mUi.commandLineInput->setText(aProjectPtr->GetCommandLineArgs());
   exec();
   //   setWindowModality(Qt::ApplicationModal);
   //   show();
}

void wizard::ProjectSettingsDialog::BrowseClick()
{
   QString path = QFileDialog::getExistingDirectory(this, "Select root directory path", mUi.rootDirectoryInput->text());
   if (!path.isEmpty())
   {
      mUi.rootDirectoryInput->setText(path);
   }
}

void wizard::ProjectSettingsDialog::OkClick()
{
   Project* scenarioPtr = mScenarioPtr.Get();
   if (scenarioPtr)
   {
      if (UtPath(mUi.projectDirectory->text().toStdString()).Stat() != UtPath::cDIRECTORY)
      {
         QMessageBox::warning(this,
                              "Invalid Root Directory",
                              "The root directory is not a valid directory path: " + mUi.projectDirectory->text());
         return;
      }
      scenarioPtr->SetProjectDirectory(mUi.projectDirectory->text().toStdString());
      scenarioPtr->SetWorkingDirectory(mUi.rootDirectoryInput->text().toStdString());
      scenarioPtr->SetCommandLineArgs(mUi.commandLineInput->text());
   }
   hide();
}

void wizard::ProjectSettingsDialog::CancelClick()
{
   hide();
}

void wizard::ProjectSettingsDialog::ClearCommandLine()
{
   mUi.commandLineInput->setText(Project::GetDefaultCommandLineArgs());
}
