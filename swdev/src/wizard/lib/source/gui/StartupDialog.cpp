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
#include "StartupDialog.hpp"

#include "ActionManager.hpp"
#include "ProjectWorkspace.hpp"
#include "RunManager.hpp"

wizard::StartupDialog::StartupDialog(QStringList&    aInputFiles,
                                     QWidget*        aParent,
                                     Qt::WindowFlags aFlags /*= Qt::WindowFlags()*/)
   : wkf::StartupDialog<QStringList>(aInputFiles, aParent, aFlags)
{
   mDocLinks    = {{"wizard.html", "Main Page"},
                {"wizard_users_guide.html", "User's Guide"},
                {"wizard_reference_guide.html", "Reference Guide"},
                {"wizard_change_log.html", "Change Log"},
                {"main_page.html", "AFSIM Main Page"}};
   auto* newPtr = mUi.buttonBox->addButton("New...", QDialogButtonBox::DestructiveRole);
   // According to Qt docs, clicking a button defined with a "destructive" role should close the dialog, but it does
   // not. This is reported in https://bugreports.qt.io/browse/QTBUG-67168. Hence the need to call QDialog::done.
   connect(newPtr,
           &QPushButton::clicked,
           this,
           [this]()
           {
              done(0);
              ProjectWorkspace::Instance()->GetActionManager()->NewProject();
           });
}

QStringList wizard::StartupDialog::Browse()
{
   // If we have a previous scenario, open the Browse Scenario dialog to the same directory that scenario is in.
   QString directory = wizRunMgr.GetMostRecentDirectory();

   // This does not use wkf::getOpenFileNames due to wkf::Environment not being set up yet,
   // instead the Browse button is disabled directly to prevent file browsing
   return QFileDialog::getOpenFileNames(this,
                                        "Select Scenario File(s)",
                                        directory,
                                        "AFSIM Input(*.afproj *.txt);; AFSIM Project(*.afproj);; All Files (*.*)");
}

void wizard::StartupDialog::RemoveRecent(const QStringList& aRecent)
{
   wizRunMgr.RemoveRecentScenario(aRecent);
}

void wizard::StartupDialog::ClearRecents()
{
   wizRunMgr.ClearRecentScenarios();
}

const QList<QStringList>& wizard::StartupDialog::GetRecents() const
{
   return wizRunMgr.GetRecentScenarios();
}

QString wizard::StartupDialog::GetRecentLabelDisplayName(const QStringList& aFileList, bool aDisplayFullPath) const
{
   return wizRunMgr.GetScenarioDisplayName(aFileList, aDisplayFullPath);
}

void wizard::StartupDialog::BuildRecentContextMenu(QMenu& aMenu, const QStringList& aLink)
{
   connect(aMenu.addAction("Open File"),
           &QAction::triggered,
           [aLink]()
           {
              for (const auto& file : aLink)
              {
                 QDesktopServices::openUrl(QUrl::fromLocalFile(file));
              }
           });
}

void wizard::StartupDialog::PopulateLabels()
{
   mUi.recentsGroupBox->setTitle("Recent Projects");
   mUi.newGroupBox->setTitle("Open Project");
   mUi.filesLineEdit->setPlaceholderText("Drag and drop or enter scenario file(s)...");
   mUi.appDesc->setText("<p style=\"font - size:10pt; \">Wizard is the AFSIM scenario editor.  It can be used to view "
                        "scenarios, launch a virtual or constructive simulation, and launch post-process viewers.</p>");
   mUi.appIcon->setPixmap(QPixmap(QString::fromUtf8(":/icons/wizard_logo_icon.png")));
   mUi.buttonBox->button(QDialogButtonBox::Ok)->setText("Open");
}

bool wizard::StartupDialog::ValidateSelection()
{
   // Check to see if all files were found
   for (const auto& file : mSelection)
   {
      QFileInfo info(file);
      if (!info.exists())
      {
         QMessageBox::warning(this, "File Not Found", "File: " + file + " was not found.");
         return false;
      }
   }
   return true;
}

QStringList wizard::StartupDialog::ParseDroppedUrls(const QList<QUrl>& urls) const
{
   QStringList filePaths;
   foreach (QUrl url, urls)
   {
      if (url.isLocalFile())
      {
         filePaths.push_back(url.toLocalFile());
      }
   }

   return filePaths;
}
