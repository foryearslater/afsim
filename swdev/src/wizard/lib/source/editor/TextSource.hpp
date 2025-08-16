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

#ifndef TEXTSOURCE_HPP
#define TEXTSOURCE_HPP

#include <algorithm>
#include <deque>
#include <string>
#include <vector>

#include <QVector>

#include "FileSignature.hpp"
#include "TextSourceChange.hpp"
#include "UtCallback.hpp"
#include "UtPath.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"

class QDataStream;
class QWidget;
class UtTextRange;
class WsfParseNode;
class QTextCursor;
class QTextDocument;

namespace wizard
{
class Editor;
class Project;
class ProjectWorkspace;
class TextSourceView;

//! TextSource is a file referenced by the project.  The file may or may not be loaded into memory.
//! Because of multi-threaded parsing, this class maintains a separate copy of the text when loaded in the text editor.
//! Changes made to the document are queued and only applied when the parser has stopped.
//! While TextSource inherits from UtTextDocument, using mutators from UtTextDocument directly is not advised.
class VI_EXPORT TextSource : public UtTextDocument
{
public:
   //! Callback invoked when the given TextSource has characters removed
   //! @tparam void(TextSource*, size_t, int) is the function signature.  The first parameter is the given text source
   //! that changed.  The second parameter is the position where the text changed.  The third parameter is the amount of
   //! characters removed from the text.
   static UtCallbackListN<void(TextSource*, size_t, int)> TextSourceCharDeleted;

   enum Flags
   {
      cINCLUDED = 1, // The file has been included by a scenario and must stay in memory
      cCRITICAL = 2, // The file is critical in some manner and must be kept in memory

      cCRITICAL_MASK = cCRITICAL | cINCLUDED
   };
   friend class wizard::Project;
   friend class wizard::ProjectWorkspace;

   TextSource(ProjectWorkspace* aProjectManagerPtr, const UtPath& aSourcePath);
   virtual ~TextSource();

   void AddedToWorkspace();

   //! Load the file into memory if it has not been loaded.
   bool Load();

   UtTextDocument*                     GetSource();
   bool                                IsDeleted() const { return mDeleted; }
   void                                SetDeleted(bool aIsDeleted);
   bool                                CheckForFileModifications(bool aAlwaysFullyReload);
   void                                HandleInsert(int aPosition, int aLength, const char* aText);
   void                                HandleDelete(int aPosition, int aLength);
   TextSourceView*                     NewView(Project* aProjectPtr);
   TextSourceView*                     GetView(Project* aParentPtr);
   const std::vector<TextSourceView*>& GetViews() const { return mViews; }
   void                                EditorClosed(Editor* aEditorPtr);
   void*                               GetQtDocument() const { return mDocumentPtr; }
   QTextDocument* GetDocumentAsQTextDocument() const { return static_cast<QTextDocument*>(mDocumentPtr); }
   // void UpdateStyling();
   void        ProjectParseUpdated();
   bool        Save();
   void        SaveWithNotifications();
   void        SaveAs();
   std::string GetFileName();
   std::string GetSystemPath() const { return mFilePath.GetSystemPath(); }

   void Replace(const UtTextRange& aReplaceRange, const std::string& aText);
   void Append(const std::string& aText);

   const std::vector<TextSourceChange>& GetPendingChanges() const { return mPendingChanges; }

   //! Executes when file's modification state has changed
   UtCallbackListN<void(TextSource&)> FileModifiedChange;

   bool IsModified() const { return mModified; }
   void SavePointReached();
   bool ReadSource(bool aAlwaysFullyReload = false);

   ProjectWorkspace* GetWorkspace() const { return mProjectWorkspacePtr; }
   Project*          GetProject();
   void              WaitForSourceModifications();
   int               GetFlags() const { return mFlags; }
   void              SetFlags(int aFlags) { mFlags = aFlags; }
   bool              IsCritical() const { return (cCRITICAL_MASK & mFlags) != 0; }
   bool              IsLoaded() const { return mLoaded; }
   void              Unload();

   void ApplyModifications(UtTextRange& aRange, long long aStartSequenceNumber) const;

   size_t ApplyModifications(size_t aPos, long long aStartSequenceNumber) const;

   void ApplyPendingModifications(UtTextRange& aRange, UtTextDocument& aDocument) const;

   bool HasAppliedChanges(long long aSequenceNumber) const;
   bool HasPendingChanges() const { return !mPendingChanges.empty(); }

   // For each change to a text document, it is given a sequence number.  These changes are recorded
   // and stored until no longer needed.  The Obsolete sequence number is the last sequence number that
   // needs to be kept.
   static long long GetNextSequenceNumber() { return sNextSequenceNumber; }
   static void      SetObsoleteSequenceNumber(long long aNumber) { sObsoleteSequenceNumber = aNumber; }

   FileSignature FileSignatureAtLoad() const { return mFileSignature; }

   void ReloadViews();

   static void DiffDocuments(const char* aOld, const char* aNew, QVector<TextSourceChange>& aChanges);
   void        FindIncludeLocations();
   void        BrowseContainingFolder();
   void        CopyPathToClipboard();

private:
   void SaveAndStore();

   void ApplyChangeToQt(const TextSourceChange& aChange, Editor& aEditor);

   void ApplyChangeToQt(const TextSourceChange& aChange, QTextCursor& aCursor);
   int  GetNextAppliedChange(long long aSequenceNumber) const;
   void InsertText(size_t aPosition, size_t aLength, const char* aText);
   void DeleteText(size_t aPosition, size_t aLength);

   void ReloadEditor(Editor& aEditor, bool aInitialLoad, bool aForceUnmodified = false, bool aForceReload = false);

   void SetModified(bool aModified, bool aTellQt = true)
   {
      if (mModified != aModified)
      {
         SetModifiedUpdate(aModified, aTellQt);
      }
   }
   void SetModifiedUpdate(bool aModified, bool aTellQt);

   void NotifyProjectOfChange();
   void HandleChangeContents(int aPos, int aRemovedChars, int aAddChars);

   void SyncChangesToSource();

   int mFlags;

   ProjectWorkspace* mProjectWorkspacePtr;

   //! Has the source been read at least once
   bool mLoaded;
   //! Has this file been deleted
   bool mDeleted;
   //! Has this been modified after save
   bool mModified;

   //! Changes made while parsing, not yet applied to this document
   std::vector<TextSourceChange> mPendingChanges;
   //! Changes that have been applied to this document.  These are kept to help completion code
   //! which needs to run before the new parse is completed.
   std::deque<TextSourceChange> mAppliedChanges;
   //! Number of bytes used by mAppliedChanges.
   int           mAppliedChangesBytes;
   FileSignature mFileSignature;
   //! List of all views of this document
   std::vector<TextSourceView*> mViews;

   //! Opaque pointer to the Qt document.  This is only used to sync
   //! more than one editor to the same document.
   void* mDocumentPtr;

   static long long sNextSequenceNumber;
   static long long sObsoleteSequenceNumber;
};
} // namespace wizard

#endif
