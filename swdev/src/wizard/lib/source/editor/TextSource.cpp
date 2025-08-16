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

#include "TextSource.hpp"

#include <iostream>
#include <sstream>
#include <stdio.h>

#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>
#include <QUrl>

#include "Editor.hpp"
#include "EditorDock.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "TextSourceChange.hpp"
#include "TextSourceSignals.hpp"
#include "TextSourceView.hpp"
#include "UtAlgorithm.hpp"
#include "UtCast.hpp"
#include "UtTextDocument.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfEditor.hpp"


static const int cMAXIMUM_FILE_SIZE                          = 100 * 1000000; // 100 MB
static const int cMAXIMUM_CHANGE_HISTORY_ENTRIES             = 100;
static const int cMAXIMUM_CHANGE_HISTORY_BYTES               = 1000000; // 1 MB
long long        wizard::TextSource::sNextSequenceNumber     = 0;
long long        wizard::TextSource::sObsoleteSequenceNumber = 0;

UtCallbackListN<void(wizard::TextSource*, size_t, int)> wizard::TextSource::TextSourceCharDeleted;

wizard::TextSource::TextSource(ProjectWorkspace* aWorkspacePtr, const UtPath& aSourcePath)
   : UtTextDocument()
   , mFlags(0)
   , mProjectWorkspacePtr(aWorkspacePtr)
   , mLoaded(false)
   , mDeleted(false)
   , mModified(false)
   , mAppliedChangesBytes(0)
   , mDocumentPtr()
{
   ///////WARNING WARNING////////////
   // This may be executed from another thread!
   // Don't add code here unless it is safe.
   // see AddedToProject()
   ///////WARNING WARNING////////////
   mFilePath = aSourcePath;
}

// Called once the source has been added to the project.
// This is the first opportunity to do non-thread-safe stuff
void wizard::TextSource::AddedToWorkspace() {}

bool wizard::TextSource::Load()
{
   if (!mLoaded)
   {
      ReadSource();
   }

   return mLoaded;
}

wizard::TextSource::~TextSource()
{
   emit wizSignals->SourceDeleted(this);
   // Deleting the editor widget will result in the view being removed
   std::vector<TextSourceView*> viewsCopy(mViews);
   for (auto&& it : viewsCopy)
   {
      it->mEditorPtr->deleteLater();
   }
   assert(mViews.empty());
   if (mViews.empty())
   {
      mDocumentPtr = nullptr;
      Unload();
   }
}

bool wizard::TextSource::CheckForFileModifications(bool aAlwaysFullyReload)
{
   // Return quickly if the file has not been read at least once
   if (!mLoaded)
      return false;

   // Return value defaults to false
   bool fileReloaded = false;

   // Determine the type of modification
   int fileUpdateResult = mFileSignature.Update(mFilePath);
   if (fileUpdateResult != FileSignature::cFILE_NOT_FOUND)
   {
      if (fileUpdateResult & FileSignature::cTIME_CHANGE)
      {
         bool allowLoad = false;
         if (mModified)
         {
            std::stringstream ss;
            ss << "File '"
               << GetFilePath().GetSystemPath() << "' has been modified by another application.  Do you want to ABANDON any changes and reload the file?";
            QMessageBox messageBox(QMessageBox::Warning, "File Modified Externally", ss.str().c_str());
            messageBox.addButton("Reload File", QMessageBox::AcceptRole);
            messageBox.addButton("Keep Changes", QMessageBox::RejectRole);
            if (QMessageBox::AcceptRole == messageBox.exec())
            {
               allowLoad = true;
            }
         }
         else
         {
            // Allow loading of unmodified files without asking
            allowLoad = true;
         }

         if (allowLoad && mLoaded)
         {
            fileReloaded = true;
            ReadSource(aAlwaysFullyReload);
         }
         else if (allowLoad)
         {
            mFileSignature.Update(mFilePath);
         }
      }
   }
   else
   {
      fileReloaded = true;
      if (mModified)
      {
         fileReloaded = false;

         std::stringstream ss;
         ss << "File '" << GetFilePath().GetSystemPath() << "' has been moved or deleted by another application.";
         ss << " Do you want to ABANDON any changes?";

         QMessageBox messageBox(QMessageBox::Warning, "File Removed Externally", ss.str().c_str());
         messageBox.addButton("Close File", QMessageBox::AcceptRole);
         messageBox.addButton("Keep Changes", QMessageBox::RejectRole);

         if (QMessageBox::AcceptRole == messageBox.exec())
         {
            fileReloaded = true;
         }
      }

      if (fileReloaded)
      {
         ReadSource(aAlwaysFullyReload);
         std::vector<TextSourceView*> views = mViews;
         for (auto&& it : views)
         {
            it->mEditorPtr->RequestCloseAction();
         }
      }
   }
   return fileReloaded;
}

bool wizard::TextSource::ReadSource(bool aAlwaysFullyReload)
{
   bool loaded(false);

   UtPath::StatData statData;
   GetFilePath().Stat(statData);

   if (statData.mStatType == UtPath::cDIRECTORY)
   {
      return false;
   }
   if (statData.mStatType == UtPath::cFILE)
   {
      if (statData.mFileSizeBytes < (size_t)cMAXIMUM_FILE_SIZE)
      {
         // After first read, ensure the parser isn't running.
         if (mLoaded)
         {
            GetWorkspace()->WaitForAbortParsing();
         }

         if (UtTextDocument::ReadFile(GetFilePath()))
         {
            SetDeleted(false);
            SetModified(false);
            bool requiresParse = GetProject()->SourceReloaded(this);
            if (requiresParse)
            {
               GetProject()->TriggerReparse();
            }
            loaded = true;
         }
      }
      else
      {
         const QString msg("The file %1 is too large to open. (%2 MB).");
         const QString filePath(QString::fromStdString(mFilePath.GetSystemPath()));
         const double  sizeMb = (statData.mFileSizeBytes / 1000000.0);

         QMessageBox::warning(nullptr, "File too large", msg.arg(filePath).arg(sizeMb));

         if (mLoaded)
         {
            SetModified(false);
            Unload();
         }

         loaded = false;
      }

      mFileSignature.Update(mFilePath);
   }
   else if (statData.mStatType == UtPath::cSTAT_ERROR)
   {
      Clear();
      Insert(0, "\0", 1); // null terminator
      SetDeleted(true);
      SetModified(false);
      loaded = true;
   }

   if (!mViews.empty())
   {
      ReloadEditor(*mViews[0]->mEditorPtr, false, true, aAlwaysFullyReload);
   }

   if (loaded)
   {
      mPendingChanges.clear();
      mAppliedChanges.clear();
   }

   mLoaded = loaded;

   return loaded;
}

//! Safely inserts text into the document.
//! Text changes must be reconciled by ProjectWorkspace at the appropriate time,
//! therefore, the document will not be updated immediately.
void wizard::TextSource::HandleInsert(int aPosition, int aLength, const char* aText)
{
   if (mViews.empty())
      return;
   if (mViews[0]->mEditorPtr->ExecutingNonUserAction())
      return;
   InsertText(aPosition, aLength, aText);
}

// Schedules an insertion of text to the source.
// Should NOT be called if a view is opened, except from HandleInsert()
void wizard::TextSource::InsertText(size_t aPosition, size_t aLength, const char* aText)
{
   if ((aLength != ut::npos) && (aLength != 0))
   {
      TextSourceChange change;
      change.mPos = aPosition;
      change.mText.assign(aText, aText + aLength);
      // Don't want to insert null characters
      assert(change.mText.find('\0', 0) == std::string::npos);
      change.mCharsRemoved   = 0;
      change.mSequenceNumber = sNextSequenceNumber++;
      mPendingChanges.push_back(change);
      SetModified(true);
      NotifyProjectOfChange();
   }
}

//! Safely deletes text from the document.
//! Text changes must be reconciled by ProjectWorkspace at the appropriate time,
//! therefore, the document will not be updated immediately.
void wizard::TextSource::HandleDelete(int aPosition, int aLength)
{
   if (mViews.empty())
      return;
   if (mViews[0]->mEditorPtr->ExecutingNonUserAction())
      return;
   DeleteText(aPosition, aLength);
}

// Schedules a deletion of a range of text from the text source.
// Should NOT be called if a view is opened, except from HandleDelete()
void wizard::TextSource::DeleteText(size_t aPosition, size_t aLength)
{
   if ((aLength != ut::npos) && (aLength != 0))
   {
      TextSourceChange change;
      change.mPos            = aPosition;
      change.mCharsRemoved   = aLength;
      change.mSequenceNumber = sNextSequenceNumber++;
      mPendingChanges.push_back(change);
      TextSourceCharDeleted(this, aPosition, static_cast<int>(aLength));
      SetModified(true);
      NotifyProjectOfChange();
   }
}

UtTextDocument* wizard::TextSource::GetSource()
{
   return this;
}

//! Applies stored changes to the underlying document.
//! WARNING: This should not be called except by the ProjectManager
void wizard::TextSource::SyncChangesToSource()
{
   if (mPendingChanges.empty())
   {
      return;
   }
   for (auto&& change : mPendingChanges)
   {
      change.ApplyToDocument(*this);
      mAppliedChangesBytes += (int)change.mText.size();
      mAppliedChanges.push_back(TextSourceChange());
      std::swap(mAppliedChanges.back(), change); // avoid copy of string
   }
   while (!mAppliedChanges.empty() && ((int)mAppliedChanges.size() > cMAXIMUM_CHANGE_HISTORY_ENTRIES ||
                                       mAppliedChangesBytes > cMAXIMUM_CHANGE_HISTORY_BYTES ||
                                       mAppliedChanges.front().mSequenceNumber <= sObsoleteSequenceNumber))
   {
      mAppliedChangesBytes -= (int)mAppliedChanges.front().mText.size();
      mAppliedChanges.pop_front();
   }
   mPendingChanges.clear();
}

//! Reload a scintilla editor
void wizard::TextSource::ReloadEditor(Editor& aEditor, bool aInitialLoad, bool aForceUnmodified, bool aForceReload)
{
   aEditor.BeginNonUserAction();
   if (aInitialLoad || aForceReload)
   {
      // Copy text to scintilla
      char* textPtr = GetText().GetPointer(0);
      aEditor.BeginNonUserAction();
      aEditor.document()->setPlainText(textPtr);
      QTextCursor cur = aEditor.textCursor();
      cur.setPosition(0);
      aEditor.setTextCursor(cur);
      aEditor.EndNonUserAction();
      if (aInitialLoad)
      {
         aEditor.document()->clearUndoRedoStacks();
         aEditor.document()->setModified(false);
      }
   }
   else
   {
      // Diff this text document with scintilla's copy.  Apply only the changes.
      std::string               oldText = aEditor.ToAscii(aEditor.ToPlainText());
      QVector<TextSourceChange> changes;
      DiffDocuments(oldText.c_str(), GetPointer(), changes);
      if (!changes.empty())
      {
         int         firstChangePos = -1;
         QTextCursor cur            = aEditor.textCursor();
         cur.beginEditBlock();
         for (int i = 0; i < changes.size(); ++i)
         {
            const TextSourceChange& change = changes[i];
            if (i == 0)
               firstChangePos = ut::cast_to_int(change.mPos);
            ApplyChangeToQt(changes[i], cur);
         }
         cur.endEditBlock();
         if (!aForceUnmodified)
         {
            SetModified(true, true);
         }
         // scroll to first change
         if (firstChangePos != -1)
         {
            QTextCursor cur = aEditor.textCursor();
            cur.setPosition(firstChangePos);
            aEditor.setTextCursor(cur);
            aEditor.centerCursor();
         }
      }
      if (!mModified)
      {
         // aEditor.SendScintilla(SCI_SETSAVEPOINT);
      }
   }

   aEditor.EndNonUserAction();

   aEditor.ReloadComplete();
}

wizard::TextSourceView* wizard::TextSource::NewView(Project* aProjectPtr)
{
   // Ensure the text source is loaded.
   if (!Load())
   {
      return nullptr;
   }

   // Future: Detect file type, use appropriate editor
   WsfEditor* controlPtr = new WsfEditor(wkfEnv.GetMainWindow()->centralWidget(),
                                         aProjectPtr,
                                         this,
                                         ProjectWorkspace::Instance()->GetEditorManager());
   controlPtr->Initialize();
   // While non-obvious, this EditorDock creation is required and not a memory leak.
   // The EditorDock constructor registers and transfers ownership of itself via wkfEnv.
   new EditorDock(wkfEnv.GetMainWindow()->centralWidget(), controlPtr);

   // If a document already exists, reuse it
   if (!mDocumentPtr)
   {
      mDocumentPtr = controlPtr->document();
      ReloadEditor(*controlPtr, true);
   }
   else
   {
      controlPtr->setDocument((QTextDocument*)mDocumentPtr);
   }

   // Attempt to load the user's previous line/column from the settings file:

   std::pair<size_t, size_t> line_col =
      wkfEnv.GetPreferenceObject<EditorPrefObject>()->RecallFilePosition(GetFilePath().GetSystemPath().c_str());
   size_t pos = controlPtr->GetOffset(line_col.first, line_col.second);
   controlPtr->SetSelection(UtTextRange(pos, pos - 1));
   controlPtr->centerCursor();

   TextSourceView* viewPtr = new TextSourceView(controlPtr, this);
   mViews.push_back(viewPtr);
   GetWorkspace()->ViewOpened(viewPtr);
   return viewPtr;
}

wizard::TextSourceView* wizard::TextSource::GetView(Project* aProjectPtr)
{
   if (!mViews.empty())
   {
      return mViews[0];
   }
   return NewView(aProjectPtr);
}

void wizard::TextSource::EditorClosed(Editor* aEditorPtr)
{
   for (size_t i = 0; i < mViews.size(); ++i)
   {
      if (mViews[i]->mEditorPtr == aEditorPtr)
      {
         GetWorkspace()->ViewClosed(mViews[i]);
         delete mViews[i];
         mViews.erase(mViews.begin() + i);
      }
   }
   if (mViews.empty())
   {
      mDocumentPtr = nullptr;
      Unload();
   }
}

void wizard::TextSource::NotifyProjectOfChange()
{
   mProjectWorkspacePtr->NotifyOfSourceChange(this);
}

void wizard::TextSource::ProjectParseUpdated()
{
   Project* projPtr = GetProject();
   if (mViews.empty() || !projPtr)
   {
      return;
   }
   mViews[0]->mEditorPtr->UpdateStyle();
}

std::string wizard::TextSource::GetFileName()
{
   UtPath path(mFilePath);
   return path.GetFileName();
}
void wizard::TextSource::SaveWithNotifications()
{
   if (!Save())
   {
      std::stringstream ss;
      ss << "Could not save file " + GetFilePath().GetSystemPath() + ", check that the file/directory is writable.";
      int bn = QMessageBox::warning(nullptr, "Could not save", ss.str().c_str(), "Retry", "Cancel");
      if (bn == 0)
      {
         SaveWithNotifications();
      }
   }
}
bool wizard::TextSource::Save()
{
   std::ofstream outfile(GetFilePath().GetSystemPath().c_str(), std::ios::binary);
   if (outfile)
   {
      WriteFile(outfile);
      outfile.close();
      SetModified(false);
      // Save file stat data to detect changes
      UtPath::StatData statData;
      GetFilePath().Stat(statData);
      mFileSignature.Update(mFilePath);
      return true;
   }
   return false;
}

void wizard::TextSource::SavePointReached()
{
   // qt is informing us that we have reached the previous save point,
   // mark the source as unmodified
   SetModified(false, false);
}

void wizard::TextSource::SetModifiedUpdate(bool aModified, bool aTellQt)
{
   if (!aModified && aTellQt && !mViews.empty())
   {
      // Tell qt that the file is at the save point
      mViews[0]->mEditorPtr->document()->setModified(false);
   }
   mModified = aModified;

   emit wizSignals->SourceModifiedStateChange(this);
   FileModifiedChange(*this);
}


void wizard::TextSource::Replace(const UtTextRange& aReplaceRange, const std::string& aText)
{
   if (!mViews.empty())
   {
      mViews[0]->mEditorPtr->Replace(aReplaceRange, aText);
   }
   else
   {
      DeleteText(aReplaceRange.GetBegin(), aReplaceRange.Length());
      InsertText(aReplaceRange.GetBegin(), aText.size(), aText.c_str());
   }
}

void wizard::TextSource::Append(const std::string& aText)
{
   if (!mViews.empty())
   {
      mViews[0]->mEditorPtr->Replace(UtTextRange(Size() - 1, Size() - 2), aText);
   }
   else
   {
      InsertText(Size() - 1, aText.size(), aText.c_str());
   }
}

//! Unload the file from memory if possible
void wizard::TextSource::Unload()
{
   if ((cCRITICAL_MASK & mFlags) == 0)
   {
      if (mLoaded && mViews.empty())
      {
         UtTextDocument::FreeMemory();
         mLoaded = false;
      }
   }
}

wizard::Project* wizard::TextSource::GetProject()
{
   return mProjectWorkspacePtr->GetProject();
}

void wizard::TextSource::WaitForSourceModifications()
{
   mProjectWorkspacePtr->WaitForSourceModifications(this);
}

void wizard::TextSource::SaveAs()
{
   UtPath dir = GetFilePath();
   dir.Up();

   QFileDialog dlg(nullptr, "Save File As", dir.GetSystemPath().c_str(), "*.*");
   dlg.setDefaultSuffix("txt");
   dlg.selectFile(GetFileName().c_str());
   dlg.setAcceptMode(QFileDialog::AcceptSave);
   dlg.setFileMode(QFileDialog::AnyFile);

   if (dlg.exec())
   {
      QStringList selFiles = dlg.selectedFiles();
      if (selFiles.length() == 1)
      {
         // Ensure the source is ready
         WaitForSourceModifications();
         std::string   newFilePath = selFiles[0].toStdString();
         std::ofstream outfile(newFilePath.c_str(), std::ios::binary);

         while (!outfile)
         {
            std::stringstream ss;
            ss << "Could not save file " + newFilePath + ", check that the file/directory is writable.";
            int bn = QMessageBox::warning(nullptr, "Could not save", ss.str().c_str(), "Retry", "Cancel");
            if (bn != 0)
            {
               break;
            }
         }

         if (outfile)
         {
            WriteFile(outfile);
            outfile.close();

            UtPath newFile(newFilePath);
            if (newFile.Stat() == UtPath::cFILE)
            {
               // WIZARD_TODO: Open the file for the user?
               // GetProject()->AddTransientFile(newFile);
            }
            emit TextSourceSignals::GetInstance().requestBackup();
         }
      }
   }
}

void wizard::TextSource::CopyPathToClipboard()
{
   QApplication::clipboard()->setText(QString::fromStdString(GetSystemPath()));
}

void wizard::TextSource::ApplyModifications(UtTextRange& aRange, long long aStartSequenceNumber) const
{
   int startIndex = GetNextAppliedChange(aStartSequenceNumber);
   for (; startIndex < (int)mAppliedChanges.size(); ++startIndex)
   {
      const TextSourceChange& change = mAppliedChanges[startIndex];
      change.ApplyChangeToRange(aRange);
   }
}

size_t wizard::TextSource::ApplyModifications(size_t aPos, long long aStartSequenceNumber) const
{
   int    startIndex = GetNextAppliedChange(aStartSequenceNumber);
   size_t newPos     = aPos;
   for (; startIndex < (int)mAppliedChanges.size(); ++startIndex)
   {
      const TextSourceChange& change = mAppliedChanges[startIndex];
      newPos                         = change.ApplyChangeToPos(newPos);
   }
   return newPos;
}

void wizard::TextSource::ApplyPendingModifications(UtTextRange& aRange, UtTextDocument& aDocument) const
{
   for (auto&& change : mPendingChanges)
   {
      change.ApplyToPartialDocument(aDocument, aRange);
   }
}

int wizard::TextSource::GetNextAppliedChange(long long aStartSequenceNumber) const
{
   if (mAppliedChanges.empty())
      return 0;
   int startIndex = (int)mAppliedChanges.size() - 1;
   for (; startIndex >= 0; --startIndex)
   {
      if (mAppliedChanges[startIndex].mSequenceNumber <= aStartSequenceNumber)
      {
         break;
      }
   }
   ++startIndex;
   return startIndex;
}

bool wizard::TextSource::HasAppliedChanges(long long aSequenceNumber) const
{
   return !mPendingChanges.empty() ||
          (!mAppliedChanges.empty() && mAppliedChanges.back().mSequenceNumber > aSequenceNumber);
}

// Update all views.  This must be called if the UtTextDocument is modified directly.
void wizard::TextSource::ReloadViews()
{
   // Note: all text editors share the same document at this time.
   //       therefore, we only need to update one editor.  This could change in the future,
   //       as multiple views aren't yet used.
   if (!mViews.empty())
   {
      ReloadEditor(*mViews[0]->mEditorPtr, false);
   }
}

//! Compute differences between two null-terminated text strings
void wizard::TextSource::DiffDocuments(const char* aOld, const char* aNew, QVector<TextSourceChange>& aChanges)
{
   if (aOld[0] == 0)
   {
      TextSourceChange c;
      c.mPos          = 0;
      c.mCharsRemoved = 0;
      c.mText         = aNew;
      aChanges.push_back(c);
      return;
   }


   std::string currentText(aOld);
   std::string newText(aNew);

   // Break into lines and do a line diff
   std::vector<std::string> currentLines = UtStringUtil::Split(currentText, '\n');
   std::vector<std::string> newLines     = UtStringUtil::Split(newText, '\n');

   auto changes = UtSequenceDiffLines::DiffLarge(currentLines, newLines, std::equal_to<std::string>());

   // now convert line diff into character differences
   size_t currentPos = 0;
   for (size_t i = 0; i < changes.size(); ++i)
   {
      const UtSequenceDiffSection& ch = changes[i];

      if (ch.mSectionType == UtSequenceDiffSection::cADDED)
      {
         TextSourceChange change;
         change.mPos          = currentPos;
         change.mCharsRemoved = 0;

         for (size_t l = ch.mAfterRange.first; l < ch.mAfterRange.second; ++l)
         {
            change.mText += newLines[l];
            if (l < ch.mAfterRange.second - 1)
            {
               change.mText += '\n';
            }
         }

         // Handle the last line
         if ((ch.mAfterRange.first < currentLines.size()) && (i < changes.size() - 1))
         {
            // Inserting not appending so add a carriage return
            change.mText += '\n';
         }

         currentPos += change.mText.length();
         aChanges.push_back(change);
      }
      else if (ch.mSectionType == UtSequenceDiffSection::cSAME)
      {
         for (size_t l = ch.mBeforeRange.first; l < ch.mBeforeRange.second; ++l)
         {
            currentPos += currentLines[l].size() + 1;
         }
      }
      else if (ch.mSectionType == UtSequenceDiffSection::cREMOVED)
      {
         TextSourceChange change;
         change.mCharsRemoved = 0;
         change.mPos          = currentPos;
         for (size_t l = ch.mBeforeRange.first; l < ch.mBeforeRange.second; ++l)
         {
            change.mCharsRemoved += currentLines[l].size() + 1;
         }

         // Handle the last line
         // if ((ch.mBeforeRange.first < (int)currentLines.size()) &&
         //    (change.mPos > 0))
         if (ch.mBeforeRange.first < currentLines.size())
         {
            if (change.mPos > 0)
            {
               // Removing a CR/LF
               --change.mPos;
            }
            else
            {
               --change.mCharsRemoved;
            }
         }

         aChanges.push_back(change);
      }
   }
}

void wizard::TextSource::ApplyChangeToQt(const TextSourceChange& aChange, Editor& aEditor)
{
   aEditor.Replace(UtTextRange::Offset(aChange.mPos, aChange.mCharsRemoved), aChange.mText);
}

void wizard::TextSource::ApplyChangeToQt(const TextSourceChange& aChange, QTextCursor& aCursor)
{
   if (aCursor.document()->toPlainText().size() < static_cast<int>(aChange.mPos))
   {
      // Adding at the end of the document
      aCursor.movePosition(QTextCursor::End);
      aCursor.insertText("\n");
   }
   aCursor.setPosition(ut::cast_to_int(aChange.mPos));
   aCursor.setPosition(ut::cast_to_int(aChange.mPos + aChange.mCharsRemoved), QTextCursor::KeepAnchor);
   aCursor.insertText(aChange.mText.c_str());
}

void wizard::TextSource::FindIncludeLocations()
{
   if (GetProject())
   {
      GetProject()->FindIncludes(this);
   }
}

void wizard::TextSource::SaveAndStore()
{
   SaveWithNotifications();
   emit TextSourceSignals::GetInstance().requestBackup();
}

void wizard::TextSource::BrowseContainingFolder()
{
   UtPath path = mFilePath;
   path.Up();
   QString pathStr("file:///");
   pathStr += QString::fromStdString(path.GetSystemPath());
   QDesktopServices::openUrl(QUrl(pathStr));
}

void wizard::TextSource::SetDeleted(bool aIsDeleted)
{
   mDeleted = aIsDeleted;
}
