// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "WkStartupDialog.hpp"

#include "WkRunManager.hpp"

warlock::StartupDialog::StartupDialog(QStringList&    aInputFiles,
                                      bool            aDisableFileBrowsing,
                                      QWidget*        aParent,
                                      Qt::WindowFlags aFlags /*= Qt::WindowFlags()*/)
   : wkf::StartupDialog<QStringList>(aInputFiles, aParent, aFlags)
{
   mUi.browsePushButton->setDisabled(aDisableFileBrowsing);

   mDocLinks = {{"warlock.html", "Main Page"},
                {"warlock_users_guide.html", "User's Guide"},
                {"warlock_reference_guide.html", "Reference Guide"},
                {"warlock_change_log.html", "Change Log"},
                {"main_page.html", "AFSIM Main Page"}};
}

QStringList warlock::StartupDialog::Browse()
{
   // If we have a previous scenario, open the Browse Scenario dialog to the same directory that scenario is in.
   QString directory = RunManager::GetInstance().GetMostRecentDirectory();

   // This does not use wkf::getOpenFileNames due to wkf::Environment not being set up yet,
   // instead the Browse button is disabled directly to prevent file browsing
   return QFileDialog::getOpenFileNames(this, "Select Scenario File(s)", directory, "AFSIM (*.txt);; All Files (*.*)");
}

void warlock::StartupDialog::RemoveRecent(const QStringList& aRecent)
{
   RunManager::GetInstance().RemoveRecentScenario(aRecent);
}

void warlock::StartupDialog::ClearRecents()
{
   RunManager::GetInstance().ClearRecentScenarios();
}

const QList<QStringList>& warlock::StartupDialog::GetRecents() const
{
   return RunManager::GetInstance().GetRecentScenarios();
}

QString warlock::StartupDialog::GetRecentLabelDisplayName(const QStringList& aFileList, bool aDisplayFullPath) const
{
   return RunManager::GetInstance().GetScenarioDisplayName(aFileList, aDisplayFullPath);
}

void warlock::StartupDialog::BuildRecentContextMenu(QMenu& aMenu, const QStringList& aLink)
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

void warlock::StartupDialog::PopulateLabels()
{
   mUi.recentsGroupBox->setTitle("Recent Scenarios");
   mUi.newGroupBox->setTitle("New Scenario");
   mUi.filesLineEdit->setPlaceholderText("Drag and drop or enter scenario file(s)...");
   mUi.appDesc->setText(
      "<p style=\"font - size:10pt; \">Warlock is an Operator-in-the-Loop (OITL) tool that is integrated with AFSIM "
      "into a single executable. Warlock provides data visualization to gain insights into AFSIM decision making and "
      "execution, as well as the ability to control AFSIM entities for both war-gaming and analysis.</p>");
   mUi.appIcon->setPixmap(QPixmap(QString::fromUtf8(":/images/Splash_AFSIM-War_BW_Sm.png")));
}

bool warlock::StartupDialog::ValidateSelection()
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

QStringList warlock::StartupDialog::ParseDroppedUrls(const QList<QUrl>& urls) const
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
