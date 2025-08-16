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

#ifndef ACTIONMANAGER_HPP
#define ACTIONMANAGER_HPP

#include <memory>

#include <QObject>

#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"

class QAction;
class QMenu;
class QString;
class UtTextDocumentRange;

namespace wkf
{
class Action;
}

namespace wizard
{
class ContextDocumentationControl;
class Editor;
class FindInFilesDialog;
class FindResultsControl;
class RenameFileDialog;
class ReplaceInFilesDialog;
class TextSource;

class VI_EXPORT ActionManager : public QObject
{
public:
   ActionManager();
   ~ActionManager() override;

   void BuildUpProjectContextMenu(QMenu& aMenu);
   void BuildUpTitleBarMenu(QMenu& aMenu, Editor* aEditorPtr);
   void AppendEditActions(QMenu* aMenuPtr);
   void BuildUpTextSourceContextMenu(QMenu& aMenu, TextSource* aSource, int aMainFile);

   void UpdateFocusActions();
   void NewProject();

private:
   void BuildUpEditMenu();
   void BuildUpFileMenu();
   void BuildUpViewMenu();
   void BuildUpProjectMenu();
   void BuildUpHelpMenu();

   enum MenuPlace
   {
      cFRONT,
      cBACK
   };
   void AddActions(QMenu* aMenu, QList<QAction*> aActions, MenuPlace aPlace, bool aPostSeparator);
   void UpdateActions();

   void AddDirectory();
   void Autocomplete();
   void ChangeHistory();
   void CloseProject();
   void CommentSelection();
   void CommentToggle();
   void Copy();
   void CloseAll();
   void CloseAllBut(Editor* aEditor);
   void CloseAllButStartup();
   void CloseAllButIncluded();
   void CloseAllTabsLeft(Editor* aEditor);
   void CloseAllTabsRight(Editor* aEditor);
   void CloseCurrentTab();
   void CloseInactive();
   void Cut();
   void DeleteCurrentLine();
   void ExportProject();
   void Find();
   void FindInFiles();
   void FindNext();
   void FindPrevious();
   void FindReferences();
   void FormatSelection();
   void GoToDefinition();
   void GoToLine();
   void MoveTabBackward();
   void MoveTabForward();
   void NewFile();
   void OpenCommandDocumentation(TextSource* aSourcePtr, UtTextDocumentRange aRange);
   void OpenFile();
   void OpenProjectStartup();
   void Paste();
   void PrintCurrentFile();
   void ProjectSettings();
   void Redo();
   void Rename(Editor* aEditorPtr);
   void Rename(TextSource* aSourcePtr);
   void Reparse();
   void Replace();
   void SaveProject();
   void SaveCurrent();
   void SaveAll();
   void SaveCurrentAs();
   void SelectAll();
   void ShowCommandDocumentation();
   void UncommentSelection();
   void Undo();

   wkf::Action* mCloseProject;
   wkf::Action* mSaveProject;
   wkf::Action* mExportProject;

   wkf::Action* mFileSave;
   wkf::Action* mFileSaveAs;
   wkf::Action* mFileSaveAll;
   wkf::Action* mFilePrint;

   wkf::Action* mGoToDefinition;
   wkf::Action* mFindReferences;
   wkf::Action* mUndo;
   wkf::Action* mRedo;
   wkf::Action* mCut;
   wkf::Action* mCopy;
   wkf::Action* mPaste;
   wkf::Action* mSelectAll;
   wkf::Action* mFind;
   wkf::Action* mFindNext;
   wkf::Action* mFindPrevious;
   wkf::Action* mFindInFiles;
   wkf::Action* mReplace;
   wkf::Action* mGoToLine;
   wkf::Action* mAutoComplete;
   wkf::Action* mCommentSelection;
   wkf::Action* mUncommentSelection;
   wkf::Action* mToggleComment;
   wkf::Action* mFormatSelection;
   wkf::Action* mTabForward;
   wkf::Action* mTabBackward;
   wkf::Action* mCloseCurrent;
   wkf::Action* mDelCurrentLine;

   wkf::Action* mCloseAll;
   wkf::Action* mCloseNonStartup;
   wkf::Action* mCloseNonIncluded;
   wkf::Action* mCloseInactive;

   wkf::Action* mAddDirectory;
   wkf::Action* mChangeHistory;
   wkf::Action* mReparse;
   wkf::Action* mProjectSettings;

   wkf::Action* mHelpCommandDocumentation;

   wkf::Action* mNewFile;

   QList<QAction*>              mProjectRequiredActions;
   QList<QAction*>              mWsfRequiredActions;
   QList<QAction*>              mFileRequiredActions;
   QList<QAction*>              mFocusRequiredActions;
   FindResultsControl*          mFindResultsControlPtr;
   FindInFilesDialog*           mFindInFilesDialogPtr;
   RenameFileDialog*            mRenameFileDialogPtr;
   ReplaceInFilesDialog*        mReplaceInFilesDialogPtr;
   ContextDocumentationControl* mContextDocumentationControlPtr;

   UtCallbackHolder mCallbacks;
};
} // namespace wizard

#endif
