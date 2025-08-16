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

#include "ChangeHistory.hpp"

#include <time.h>

#include <QMessageBox>
#include <QPushButton>

#include "BackupPrefObject.hpp"
#include "EditorPrefObject.hpp"
#include "ExportFiles.hpp"
#include "RevisionStore.hpp"
#include "UtAlgorithm.hpp"
#include "UtMemory.hpp"
#include "UtQt.hpp"
#include "WkfEnvironment.hpp"

wizard::ChangeHistory::ChangeHistory(RevisionStore& aRevisionStore, const UtPath& aProjectDir, QWidget* aParent)
   : QDialog(aParent)
   , mRevisionStore(aRevisionStore)
   , mProjectDir(aProjectDir)
{
   mUi.setupUi(this);
   setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
   setWindowModality(Qt::ApplicationModal);
   setAttribute(Qt::WA_DeleteOnClose);

   SetDiffTabWidth();
   PopulateRevisions();
   UpdateWidgets();

   connect(mUi.closeButton, &QPushButton::clicked, this, &ChangeHistory::deleteLater);
   connect(mUi.restoreButton, &QPushButton::clicked, this, &ChangeHistory::RestoreSelectedRevision);
   connect(mUi.exportButton, &QPushButton::clicked, this, &ChangeHistory::ExportSelectedRevision);
   connect(mUi.removeAllButton, &QPushButton::clicked, this, &ChangeHistory::DeleteAllRevisions);
   connect(mUi.revisionList, &QListWidget::itemSelectionChanged, this, &ChangeHistory::UpdateWidgets);
   connect(mUi.diffFilesList, &QListWidget::itemClicked, this, &ChangeHistory::NavigateDiff);
   connect(mUi.diffFilesList, &QListWidget::itemSelectionChanged, this, &ChangeHistory::NavigateDiff);
   connect(mUi.contextSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ChangeHistory::UpdateDiff);
}

//! Prompts the user and attempts to restore the currently selected revision to
//! the user's project fileset.
void wizard::ChangeHistory::RestoreSelectedRevision()
{
   if (RestoreConfirmation())
   {
      QStringList badFiles;
      if (mRevisionStore.Restore(SelectedRevNo(), badFiles, this))
      {
         deleteLater();
      }
      else
      {
         UtQtShowSimpleMsg("Failed to restore",
                           QString("Failed to restore the following files:\n%1").arg(badFiles.join("\n")),
                           "Please ensure you have appropriate read/write file permissions and try again.",
                           this);
      }
   }
}

//! Prompts the user and attempts to export the currently selected revision
//! to the user-selected location on the file system.
void wizard::ChangeHistory::ExportSelectedRevision()
{
   ExportFiles exportFiles(mProjectDir, this);
   exportFiles.ExportRevision(mRevisionStore, SelectedRevNo());
}

//! Prompts the user and deletes all existing revisions from the database.
void wizard::ChangeHistory::DeleteAllRevisions()
{
   if (DeleteAllConfirmation())
   {
      // Temporarily open the database to delete all revisions if the database
      // exists on disk but is currently closed.
      const bool dbExists = mRevisionStore.Exists();
      const bool dbIsOpen = mRevisionStore.IsOpen();

      if (dbExists && !dbIsOpen)
      {
         mRevisionStore.Open();
      }

      if (mRevisionStore.DeleteAllRevisions())
      {
         PopulateRevisions();
         UpdateWidgets();
      }
      else
      {
         UtQtShowSimpleMsg("Failed to delete", "Failed to delete backup data.", "", this);
      }

      if (dbExists && !dbIsOpen)
      {
         mRevisionStore.Close();
      }
   }
}

//! Updates the diff file list and the diff itself.
void wizard::ChangeHistory::UpdateDiff()
{
   mUi.diffFilesList->clear();
   mUi.diffText->clear();

   if (IsRevSelected() && mRevisionStore.IsOpen())
   {
      const int      revNo                   = SelectedRevNo();
      const QString& workingDirChangesHtml   = GetWorkingDirChangesHtml(revNo);
      const QString& startupFilesChangesHtml = GetStartupFilesChangesHtml(revNo);
      const QString& fileDiffsHtml           = GetFileDiffsHtml(revNo);

      QString html;
      html += workingDirChangesHtml;
      html += startupFilesChangesHtml;
      html += (!html.isEmpty() && !fileDiffsHtml.isEmpty()) ? "<hr>" : "";
      html += fileDiffsHtml;

      mUi.diffText->setHtml(html);
   }
}

//! When the user selects a file in the diff file list, this function
//! will jump the diff display window to the position in the diff where
//! the file is being displayed. This allows quick diff navigation if
//! the diff is large.
void wizard::ChangeHistory::NavigateDiff()
{
   mUi.diffText->scrollToAnchor(Anchor(mUi.diffFilesList->currentRow()));
}

//! Returns the HTML anchor name of a given list index. The anchor
//! is arbitrary (so long as it's valid) but must be consistently used.
//! The anchor is used for automatically jumping/scrolling the diff
//! display window to whatever file the user selected in the diff file
//! list. An anchor is set in the diff display window for each file in
//! the file list.
//! @param listIndex The file list index to return an anchor for.
//! @returns The HTML anchor name of the given list index.
QString wizard::ChangeHistory::Anchor(size_t listIndex)
{
   return QString("listItem") + QString::number(listIndex);
}

//! @returns The number of the revision currently selected by the user.
int wizard::ChangeHistory::SelectedRevNo() const
{
   return mUi.revisionList->currentItem()->data(Qt::UserRole).toInt();
}

//! @returns Whether or not the user currently has a revision selected.
bool wizard::ChangeHistory::IsRevSelected() const
{
   return (mUi.revisionList->currentItem() != nullptr);
}

//! Updates the state/content of all widgets.
void wizard::ChangeHistory::UpdateWidgets()
{
   const bool hasRevisions = (mUi.revisionList->count() > 0);
   bool       enabled      = wkfEnv.GetPreferenceObject<BackupPrefObject>()->Enabled();

   mUi.revisionList->setEnabled(enabled);
   mUi.diffText->setEnabled(enabled && IsRevSelected());
   mUi.diffFilesList->setEnabled(enabled && IsRevSelected());
   mUi.removeAllButton->setEnabled(hasRevisions);
   mUi.restoreButton->setEnabled(hasRevisions && IsRevSelected());
   mUi.exportButton->setEnabled(hasRevisions && IsRevSelected());
   UpdateDiff();
}

//! Prompts the user with a message box, confirming whether or not the
//! user really wants to perform a file restore.
//! @returns True if the user selected "Yes".
bool wizard::ChangeHistory::RestoreConfirmation()
{
   QMessageBox prompt(this);

   prompt.setWindowTitle("Restore files");
   prompt.setText("Are you sure you want to restore your files to the selected time?");
   prompt.setInformativeText("Your current files will be backed up before performing the restore.");
   prompt.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
   prompt.setDefaultButton(QMessageBox::No);

   return (prompt.exec() == QMessageBox::Yes);
}

//! Prompts the user with a message box, confirming whether or not the
//! usere really wants to delete all existing revisions/backups from
//! the database.
//! @returns True if the user selected "Yes".
bool wizard::ChangeHistory::DeleteAllConfirmation()
{
   QMessageBox prompt(this);

   prompt.setWindowTitle("Remove all backups");
   prompt.setText("Are you sure you want to remove all backups for this project?");
   prompt.setInformativeText("This will irreversibly delete all backup data for this project's files.");
   prompt.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
   prompt.setDefaultButton(QMessageBox::No);

   return (prompt.exec() == QMessageBox::Yes);
}

//! Sets the tab width when displaying diffs according to the editor settings.
void wizard::ChangeHistory::SetDiffTabWidth()
{
   QFont diffFont("");
   diffFont.setStyleHint(QFont::TypeWriter);

   const int tabSpaces  = wkfEnv.GetPreferenceObject<EditorPrefObject>()->TabSpace();
   const int spaceWidth = QFontMetrics(diffFont).width(' ');
   const int tabWidth   = tabSpaces * spaceWidth;

   mUi.diffText->setTabStopWidth(tabWidth);
}

//! Populates the revision list. This is the list from which the user
//! can select a revision to restore.
void wizard::ChangeHistory::PopulateRevisions()
{
   const bool dbExists = mRevisionStore.Exists();
   const bool dbIsOpen = mRevisionStore.IsOpen();

   // Temporarily open the database to populate the revision list if the
   // database exists on disk but is currently closed.
   if (dbExists && !dbIsOpen)
   {
      mRevisionStore.Open();
   }

   const RevisionList& revisions = mRevisionStore.Revisions();

   mUi.revisionList->clear();
   auto iter    = revisions.begin();
   auto iterEnd = revisions.end();
   for (; iter != iterEnd; ++iter)
   {
      const Revision&  rev      = *iter;
      QListWidgetItem* listItem = new QListWidgetItem;

      listItem->setText(GetDateTimeText(rev.time));
      listItem->setToolTip(GetAbsoluteDateTimeText(rev.time));
      listItem->setData(Qt::UserRole, rev.revNo);
      mUi.revisionList->addItem(listItem);
   }

   if (dbExists && !dbIsOpen)
   {
      mRevisionStore.Close();
   }
}

//! @returns An HTML string describing the changes to the project's working
//! directory for the given revision. If there was no change, the string will
//! be empty.
QString wizard::ChangeHistory::GetWorkingDirChangesHtml(int aRevNo)
{
   QString html;

   std::string oldWorkingDir, newWorkingDir;
   const bool  oldWorkingDirExists = (aRevNo >= 2) && mRevisionStore.DB().WorkingDirectory(aRevNo - 1, oldWorkingDir);
   const bool  newWorkingDirExists = mRevisionStore.DB().WorkingDirectory(aRevNo, newWorkingDir);
   // assert(!oldWorkingDirExists || (oldWorkingDirExists && newWorkingDirExists));

   if ((!oldWorkingDirExists && newWorkingDirExists) || (oldWorkingDirExists && !newWorkingDirExists) ||
       (oldWorkingDirExists && newWorkingDirExists && (oldWorkingDir != newWorkingDir)))
   {
      html += QString("<p>The project working directory was changed to: <b>%1</b></p>").arg(newWorkingDir.c_str());
   }

   return html;
}

//! @returns An HTML string describing the changes to the project's startup
//! files for the given revision. If there was no change, the string will
//! be empty.
QString wizard::ChangeHistory::GetStartupFilesChangesHtml(int aRevNo)
{
   QString html;

   std::vector<std::string> oldStartupFiles, newStartupFiles;
   const bool oldStartupFilesExist = (aRevNo >= 2) && mRevisionStore.DB().StartupFiles(aRevNo - 1, oldStartupFiles);
   const bool newStartupFilesExist = mRevisionStore.DB().StartupFiles(aRevNo, newStartupFiles);
   // assert(!oldStartupFilesExist || (oldStartupFilesExist && newStartupFilesExist));

   if ((!oldStartupFilesExist && newStartupFilesExist) || (oldStartupFilesExist && !newStartupFilesExist) ||
       (oldStartupFilesExist && newStartupFilesExist && (oldStartupFiles != newStartupFiles)))
   {
      std::string startupFilesStr;

      for (const auto& newStartupFile : newStartupFiles)
      {
         if (!startupFilesStr.empty())
         {
            startupFilesStr += "<br />";
         }
         startupFilesStr += newStartupFile;
      }

      html += QString("<p>The project startup files were changed to:<br />"
                      "<b>%1</b></p>")
                 .arg(QString::fromStdString(startupFilesStr));
   }

   return html;
}

QString makeDiffHtml(const std::vector<std::string>&           aOldLines,
                     const std::vector<std::string>&           aNewLines,
                     const std::vector<UtSequenceDiffSection>& aChanges,
                     size_t                                    aContextLines)
{
   QString html;

   html += "<p>";

   for (size_t i = 0; i < aChanges.size(); ++i)
   {
      const auto&                     ch    = aChanges[i];
      const std::vector<std::string>* src   = &aOldLines;
      size_t                          start = ch.mBeforeRange.first;
      size_t                          end   = ch.mBeforeRange.second;
      if (ch.mSectionType == UtSequenceDiffSection::cADDED)
      {
         src   = &aNewLines;
         start = ch.mAfterRange.first;
         end   = ch.mAfterRange.second;
      }


      std::string text;
      std::string text2;
      if (ch.mSectionType == UtSequenceDiffSection::cSAME)
      {
         size_t linesToShow = aContextLines * 2;
         if (i == 0)
         {
            linesToShow -= aContextLines;
         }
         if (i + 1 == aChanges.size())
         {
            linesToShow -= aContextLines;
         }
         if (linesToShow >= end - start)
         {
            // show all lines
            for (size_t j = start; j < end; ++j)
            {
               text += (*src)[j];
               text += '\n';
            }
         }
         else
         {
            // show first and last part
            if (i != 0)
            {
               for (size_t j = start; j < start + aContextLines; ++j)
               {
                  text += (*src)[j];
                  text += '\n';
               }
            }
            if (i + 1 != aChanges.size())
            {
               for (size_t j = end - aContextLines; j < end; ++j)
               {
                  text2 += (*src)[j];
                  text2 += '\n';
               }
            }
         }
      }
      else
      {
         for (size_t j = start; j < end; ++j)
         {
            text += (*src)[j];
            text += '\n';
         }
      }
      QString qText = QString::fromStdString(text);
      qText.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
      QString qText2 = QString::fromStdString(text2);
      qText2.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");

      switch (ch.mSectionType)
      {
      case UtSequenceDiffSection::cADDED:
         html +=
            QString("<code style=\"background:#e6ffe6; color: black; white-space: pre;\">") + qText + QString("</code>");
         break;
      case UtSequenceDiffSection::cREMOVED:
         html +=
            QString(
               "<code style=\"background:#ffe6e6; color: black; text-decoration:line-through; white-space: pre;\">") +
            qText + QString("</code>");
         break;
      case UtSequenceDiffSection::cSAME:
         html += QString("<code style=\"color:#888888; white-space: pre;\">") + qText + QString("</code>");
         if (!qText2.isEmpty())
         {
            html += QString("<code style=\"color:#888888; white-space: pre;\">") + qText2 + QString("</code>");
         }
         break;
      }
   }

   html += "</p>";

   return html;
}

//! @returns An HTML string presenting the diffs for all changed files in the
//! given revision. If there were no changed files, the string will be empty.
QString wizard::ChangeHistory::GetFileDiffsHtml(int aRevNo)
{
   QString                   html;
   const RevisionChangeList& changes = mRevisionStore.DB().Changes(aRevNo);

   for (size_t i = 0; i < changes.size(); i++)
   {
      const RevisionChange& change   = changes[i];
      const QString&        filePath = QString::fromStdString(change.filePath);
      QListWidgetItem*      listItem = new QListWidgetItem(filePath);

      listItem->setToolTip(filePath);
      mUi.diffFilesList->addItem(listItem);
      html += QString("<p id=\"%1\">").arg(Anchor(i));

      switch (change.kind)
      {
      case RevisionChange::cNEW_FILE:
         html += QString("<b>%1</b> was <span style=\"color: green\">included</span>.").arg(filePath);
         break;
      case RevisionChange::cREMOVED_FILE:
         html += QString("<b>%1</b> was <span style=\"color: red\">unincluded</span>.").arg(filePath);
         break;
      case RevisionChange::cCHANGE_DELTA:
      case RevisionChange::cCHANGE_FULL:
      {
         UtTextDocument oldDoc, newDoc;

         assert(aRevNo >= 2);
         mRevisionStore.DB().GetFileAtRevision(change.filePath, aRevNo - 1, oldDoc);
         mRevisionStore.DB().GetFileAtRevision(change.filePath, aRevNo, newDoc);

         // the docs should already have null terminators, but add one if there has been a failure
         if (oldDoc.Size())
         {
            if (oldDoc[oldDoc.Size() - 1] != 0)
            {
               oldDoc.Insert(oldDoc.Size(), '\0');
            }
         }

         if (newDoc.Size())
         {
            if (newDoc[newDoc.Size() - 1] != 0)
            {
               newDoc.Insert(newDoc.Size(), '\0');
            }
         }

         std::string oldText = oldDoc.GetPointer();
         std::string newText = newDoc.GetPointer();

         const int                linesOfContext = mUi.contextSpinBox->value();
         std::vector<std::string> oldTextLines   = UtStringUtil::Split(oldText, '\n');
         std::vector<std::string> newTextLines   = UtStringUtil::Split(newText, '\n');

         auto diffs = UtSequenceDiffLines::DiffLarge(oldTextLines, newTextLines, std::equal_to<std::string>());

         html += QString("<b>%1</b> was modified:").arg(filePath);
         html += makeDiffHtml(oldTextLines, newTextLines, diffs, linesOfContext);
         break;
      }
      case RevisionChange::cNO_CHANGE:
      default:
         // We shouldn't hit this case.
         assert(false);
         break;
      }

      html += "</p>";
   }

   return html;
}

//! @returns A string that represents the given time in an absolute way.
//! @param time The time to represent as a string.
QString wizard::ChangeHistory::GetAbsoluteDateTimeText(const QDateTime& time)
{
   return time.toString("ddd, MMM d, yyyy, h:mm:ss ap");
}

//! @returns A string represents the given time in a relative way.
//! @note If the time represents a day more than one day old, then the
//! string returned will represent the time in an absolute way.
//! @param time The time to represent as a string.
QString wizard::ChangeHistory::GetDateTimeText(const QDateTime& time)
{
   QString text;

   if (DaysAgo(time) > 1)
   {
      text = GetAbsoluteDateTimeText(time);
   }
   else if (IsYesterday(time))
   {
      text = time.toString("'Yesterday,' h:mm:ss ap");
   }
   else if (HoursAgo(time) > 0)
   {
      const int     hrs    = HoursAgo(time);
      const QString hrsStr = QString::number(hrs);

      text = hrsStr + " hour" + ((hrs > 1) ? "s" : "") + " ago";
   }
   else if (MinutesAgo(time) > 0)
   {
      const int     mins    = MinutesAgo(time);
      const QString minsStr = QString::number(mins);

      text = minsStr + " minute" + ((mins > 1) ? "s" : "") + " ago";
   }
   else
   {
      const int     secs    = SecondsAgo(time);
      const QString secsStr = QString::number(secs);

      if (secs == 0)
      {
         text = "Just now";
      }
      else
      {
         text = secsStr + " second" + ((secs > 1) ? "s" : "") + " ago";
      }
   }

   return text;
}

//! @returns Whether or not the given time represents yesterday.
//! @param time The time to check.
bool wizard::ChangeHistory::IsYesterday(const QDateTime& time) const
{
   return (time.date().dayOfWeek() == (QDateTime::currentDateTime().date().dayOfWeek() - 1));
}

//! @returns How many seconds ago the given time represents.
//! @param time The time to reference.
int wizard::ChangeHistory::SecondsAgo(const QDateTime& time) const
{
   return time.secsTo(QDateTime::currentDateTime());
}

//! @returns How many minutes ago the given time represents.
//! @param time The time to reference.
int wizard::ChangeHistory::MinutesAgo(const QDateTime& time) const
{
   return SecondsAgo(time) / 60;
}

//! @returns How many hours ago the given time represents.
//! @param time The time to reference.
int wizard::ChangeHistory::HoursAgo(const QDateTime& time) const
{
   return MinutesAgo(time) / 60;
}

//! @returns How many days ago the given time represents.
//! @param time The time to reference.
int wizard::ChangeHistory::DaysAgo(const QDateTime& time) const
{
   return time.daysTo(QDateTime::currentDateTime());
}
