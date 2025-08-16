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

#include "ActionManager.hpp"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include "BackupPrefObject.hpp"
#include "BrowserWidget.hpp"
#include "CommandDocumentation.hpp"
#include "ContextDocumentationControl.hpp"
#include "ContextMenuActions.hpp"
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Environment.hpp"
#include "FindInFilesDialog.hpp"
#include "FindResultsControl.hpp"
#include "ProjectWorkspace.hpp"
#include "RenameFileDialog.hpp"
#include "ReplaceInFilesDialog.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "Undo.hpp"
#include "Util.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"
#include "WsfEditor.hpp"

wizard::ActionManager::ActionManager()
   : mFindResultsControlPtr(
        new FindResultsControl(wkfEnv.GetMainWindow(), ProjectWorkspace::Instance()->GetEditorManager()))
   , mFindInFilesDialogPtr(new FindInFilesDialog(wkfEnv.GetMainWindow(),
                                                 ProjectWorkspace::Instance()->GetEditorManager(),
                                                 mFindResultsControlPtr))
   , mRenameFileDialogPtr(new RenameFileDialog(wkfEnv.GetMainWindow()))
   , mReplaceInFilesDialogPtr(
        new ReplaceInFilesDialog(wkfEnv.GetMainWindow(), ProjectWorkspace::Instance()->GetEditorManager()))
   , mContextDocumentationControlPtr(new ContextDocumentationControl())
{
   mFindResultsControlPtr->hide();
   wkfEnv.GetMainWindow()->addDockWidget(Qt::BottomDockWidgetArea, mFindResultsControlPtr);

   mContextDocumentationControlPtr->hide();
   wkfEnv.GetMainWindow()->addDockWidget(Qt::BottomDockWidgetArea, mContextDocumentationControlPtr);

   mCallbacks += wizSignals->OpenCommandDocumentation.Connect(&ActionManager::OpenCommandDocumentation, this);
   connect(wizSignals, &Signals::ActionsStateChange, this, &ActionManager::UpdateActions);

   BackupPrefObject* bpo = wkfEnv.GetPreferenceObject<BackupPrefObject>();
   connect(bpo, &BackupPrefObject::Changed, this, &ActionManager::UpdateActions);

   BuildUpFileMenu();
   BuildUpViewMenu();
   BuildUpEditMenu();
   BuildUpProjectMenu();
   BuildUpHelpMenu();

   UpdateActions();
}

wizard::ActionManager::~ActionManager()
{
   delete mFindResultsControlPtr;
   delete mContextDocumentationControlPtr;
}

void wizard::ActionManager::BuildUpEditMenu()
{
   QMenu* editMenu = wkfEnv.GetMainWindow()->FindOrCreateMenu("Edit", "View");

   mGoToDefinition = new wkf::Action(QIcon::fromTheme("find_definition"),
                                     QString("Go To Definition"),
                                     wkfEnv.GetMainWindow(),
                                     QKeySequence("Alt+G"));
   QObject::connect(mGoToDefinition, &QAction::triggered, [this]() { GoToDefinition(); });
   mFindReferences = new wkf::Action(QIcon::fromTheme("find_references"),
                                     QString("Find References"),
                                     wkfEnv.GetMainWindow(),
                                     QKeySequence("Ctrl+Alt+G"));
   QObject::connect(mFindReferences, &QAction::triggered, [this]() { FindReferences(); });

   mUndo = new wkf::Action(QIcon::fromTheme("undo"), "Undo", qApp, QKeySequence("Ctrl+Z"), Qt::ApplicationShortcut);
   QObject::connect(mUndo, &QAction::triggered, [this]() { Undo(); });
   mRedo = new wkf::Action(QIcon::fromTheme("redo"), "Redo", qApp, QKeySequence("Ctrl+Shift+Z"), Qt::ApplicationShortcut);
   QObject::connect(mRedo, &QAction::triggered, [this]() { Redo(); });

   mCut = new wkf::Action(QIcon::fromTheme("scissors"), QString("Cut"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+X"));
   QObject::connect(mCut, &QAction::triggered, [this]() { Cut(); });
   mCopy = new wkf::Action(QIcon::fromTheme("copy"), QString("Copy"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+C"));
   QObject::connect(mCopy, &QAction::triggered, [this]() { Copy(); });
   mPaste = new wkf::Action(QIcon::fromTheme("paste"), QString("Paste"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+V"));
   QObject::connect(mPaste, &QAction::triggered, [this]() { Paste(); });
   mSelectAll = new wkf::Action(QString("Select All"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+A"));
   QObject::connect(mSelectAll, &QAction::triggered, [this]() { SelectAll(); });

   mFind =
      new wkf::Action(QIcon::fromTheme("find_text"), QString("Find..."), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+F"));
   QObject::connect(mFind, &QAction::triggered, [this]() { Find(); });
   mFindNext = new wkf::Action(QString("Find Next"), wkfEnv.GetMainWindow(), QKeySequence("F3"));
   QObject::connect(mFindNext, &QAction::triggered, [this]() { FindNext(); });
   mFindPrevious = new wkf::Action(QString("Find Previous"), wkfEnv.GetMainWindow(), QKeySequence("Shift+F3"));
   QObject::connect(mFindPrevious, &QAction::triggered, [this]() { FindPrevious(); });
   mFindInFiles = new wkf::Action(QIcon::fromTheme("find_in_files"),
                                  QString("Find In Files..."),
                                  wkfEnv.GetMainWindow(),
                                  QKeySequence("Ctrl+Shift+F"));
   QObject::connect(mFindInFiles, &QAction::triggered, [this]() { FindInFiles(); });
   mReplace = new wkf::Action(QString("Replace..."), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+H"));
   QObject::connect(mReplace, &QAction::triggered, [this]() { Replace(); });

   mGoToLine = new wkf::Action(QString("Go To line..."), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+G"));
   QObject::connect(mGoToLine, &QAction::triggered, [this]() { GoToLine(); });

   mAutoComplete =
      new wkf::Action(QIcon::fromTheme("autocomplete"), "Auto-Complete", wkfEnv.GetMainWindow(), QKeySequence("Ctrl+Space"));
   QObject::connect(mAutoComplete, &QAction::triggered, [this]() { Autocomplete(); });
   mCommentSelection =
      new wkf::Action(QString("Comment Selection"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+K,Ctrl+C"));
   QObject::connect(mCommentSelection, &QAction::triggered, [this]() { CommentSelection(); });
   mUncommentSelection =
      new wkf::Action(QString("Uncomment Selection"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+K,Ctrl+U"));
   QObject::connect(mUncommentSelection, &QAction::triggered, [this]() { UncommentSelection(); });

   mToggleComment = new wkf::Action(QString("Toggle Selection Comment"), wkfEnv.GetMainWindow(), QKeySequence("#"));
   QObject::connect(mToggleComment, &QAction::triggered, [this]() { CommentToggle(); });

   mFormatSelection = new wkf::Action(QIcon::fromTheme("format_text"),
                                      "Format Selection",
                                      wkfEnv.GetMainWindow(),
                                      QKeySequence("Ctrl+K,Ctrl+F"));
   QObject::connect(mFormatSelection, &QAction::triggered, [this]() { FormatSelection(); });

   mTabForward = new wkf::Action(QIcon::fromTheme("right"),
                                 QString("Move Forward Between Tabs"),
                                 wkfEnv.GetMainWindow(),
                                 QKeySequence("Ctrl+Tab"));
   QObject::connect(mTabForward, &QAction::triggered, [this]() { MoveTabForward(); });
   mTabBackward = new wkf::Action(QIcon::fromTheme("left"),
                                  QString("Move Backward Between Tabs"),
                                  wkfEnv.GetMainWindow(),
                                  QKeySequence("Ctrl+Shift+Tab"));
   QObject::connect(mTabBackward, &QAction::triggered, [this]() { MoveTabBackward(); });
   mCloseCurrent = new wkf::Action(QString("Close Current Tab"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+W"));
   QObject::connect(mCloseCurrent, &QAction::triggered, [this]() { CloseCurrentTab(); });

   mDelCurrentLine = new wkf::Action(QString("Delete Current Line"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+Shift+K"));
   QObject::connect(mDelCurrentLine, &QAction::triggered, [this]() { DeleteCurrentLine(); });

   mWsfRequiredActions += mGoToDefinition;
   mWsfRequiredActions += mFindReferences;
   //   mFocusRequiredActions += mUndo;
   //   mFocusRequiredActions += mRedo;
   mFocusRequiredActions += mCut;
   mFocusRequiredActions += mCopy;
   mFocusRequiredActions += mPaste;
   mFileRequiredActions += mSelectAll;
   mFileRequiredActions += mFind;
   mFileRequiredActions += mFindNext;
   mFileRequiredActions += mFindPrevious;
   mFileRequiredActions += mFindInFiles;
   mFileRequiredActions += mReplace;
   mFileRequiredActions += mGoToLine;
   mWsfRequiredActions += mAutoComplete;
   mWsfRequiredActions += mCommentSelection;
   mWsfRequiredActions += mUncommentSelection;
   mWsfRequiredActions += mToggleComment;
   mWsfRequiredActions += mFormatSelection;

   mWsfRequiredActions += mTabForward;
   mWsfRequiredActions += mTabBackward;
   mWsfRequiredActions += mCloseCurrent;
   mWsfRequiredActions += mDelCurrentLine;

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QToolBar*        editToolbar   = new QToolBar("Edit", mainWindowPtr);
   editToolbar->setIconSize(QSize(16, 16));
   editToolbar->setObjectName("Edit");
   editToolbar->addAction(mCut);
   editToolbar->addAction(mCopy);
   editToolbar->addAction(mPaste);
   editToolbar->addAction(mFind);
   editToolbar->addAction(mFindInFiles);
   editToolbar->addAction(mTabBackward);
   editToolbar->addAction(mTabForward);
   mainWindowPtr->AddToolBar(editToolbar);

   AddActions(editMenu, {mGoToDefinition, mFindReferences}, cBACK, true);
   AddActions(editMenu, {mUndo, mRedo}, cBACK, true);
   AddActions(editMenu, {mCut, mCopy, mPaste, mSelectAll}, cBACK, true);
   AddActions(editMenu, {mFind, mFindNext, mFindPrevious, mFindInFiles, mReplace}, cBACK, true);
   AddActions(editMenu, {mGoToLine, mTabBackward, mTabForward}, cBACK, true);
   AddActions(editMenu, {mAutoComplete, mCommentSelection, mUncommentSelection, mToggleComment, mFormatSelection}, cBACK, true);

   QList<wkf::Action*> list = {mGoToDefinition,
                               mFindReferences,
                               mUndo,
                               mRedo,
                               mCut,
                               mCopy,
                               mPaste,
                               mSelectAll,
                               mFind,
                               mFindNext,
                               mFindPrevious,
                               mFindInFiles,
                               mReplace,
                               mGoToLine,
                               mAutoComplete,
                               mCommentSelection,
                               mUncommentSelection,
                               mToggleComment,
                               mFormatSelection,
                               mTabForward,
                               mTabBackward,
                               mCloseCurrent,
                               mDelCurrentLine};
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(list, "Application");
}

void wizard::ActionManager::BuildUpFileMenu()
{
   QMenu*          fileMenu   = wkfEnv.GetMainWindow()->FindOrCreateMenu("File");
   QList<QAction*> actions    = fileMenu->actions();
   wkf::Action*    newProject = new wkf::Action(QString("New Project..."), wkfEnv.GetMainWindow());
   QObject::connect(newProject, &QAction::triggered, [this]() { NewProject(); });
   mNewFile = new wkf::Action(QString("New File..."), wkfEnv.GetMainWindow());
   QObject::connect(mNewFile, &QAction::triggered, [this]() { NewFile(); });
   wkf::Action* openFile = new wkf::Action(QString("Open File..."), wkfEnv.GetMainWindow());
   QObject::connect(openFile, &QAction::triggered, [this]() { OpenFile(); });
   mCloseProject = new wkf::Action(QString("Close Project"), wkfEnv.GetMainWindow());
   QObject::connect(mCloseProject, &QAction::triggered, [this]() { CloseProject(); });
   mSaveProject = new wkf::Action(QString("Save Project"), wkfEnv.GetMainWindow());
   QObject::connect(mSaveProject, &QAction::triggered, [this]() { SaveProject(); });
   mExportProject = new wkf::Action(QString("Export Project..."), wkfEnv.GetMainWindow());
   QObject::connect(mExportProject, &QAction::triggered, [this]() { ExportProject(); });

   mFileSave =
      new wkf::Action(QIcon::fromTheme("disk"), QString("Save File"), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+S"));
   QObject::connect(mFileSave, &QAction::triggered, [this]() { SaveCurrent(); });
   mFileSaveAs = new wkf::Action(QString("Save File As..."), wkfEnv.GetMainWindow());
   QObject::connect(mFileSaveAs, &QAction::triggered, [this]() { SaveCurrentAs(); });
   mFileSaveAll = new wkf::Action(QIcon::fromTheme("disks"), "Save All", wkfEnv.GetMainWindow(), QKeySequence());
   QObject::connect(mFileSaveAll, &QAction::triggered, [this]() { SaveAll(); });
   mFilePrint = new wkf::Action(QString("&Print File..."), wkfEnv.GetMainWindow(), QKeySequence("Ctrl+P"));
   QObject::connect(mFilePrint, &QAction::triggered, [this]() { PrintCurrentFile(); });
   if (actions.empty())
   {
      fileMenu->addAction(newProject);
      fileMenu->addSeparator();
      fileMenu->addAction(mCloseProject);
      fileMenu->addAction(mSaveProject);
      fileMenu->addAction(mExportProject);
      fileMenu->addSeparator();
      fileMenu->addAction(mNewFile);
      fileMenu->addAction(openFile);
      fileMenu->addSeparator();
      fileMenu->addAction(mFileSave);
      fileMenu->addAction(mFileSaveAs);
      fileMenu->addAction(mFileSaveAll);
      fileMenu->addSeparator();
      fileMenu->addAction(mFilePrint);
      fileMenu->addSeparator();
   }
   else
   {
      fileMenu->insertAction(actions.front(), newProject);
      fileMenu->insertSeparator(actions.front());
      fileMenu->insertAction(actions.front(), mNewFile);
      fileMenu->insertAction(actions.front(), openFile);
      fileMenu->insertSeparator(actions.front());
      fileMenu->insertAction(actions.front(), mCloseProject);
      fileMenu->insertAction(actions.front(), mSaveProject);
      fileMenu->insertAction(actions.front(), mExportProject);
      fileMenu->insertSeparator(actions.front());
      fileMenu->insertAction(actions.front(), mFileSave);
      fileMenu->insertAction(actions.front(), mFileSaveAs);
      fileMenu->insertAction(actions.front(), mFileSaveAll);
      fileMenu->insertSeparator(actions.front());
      fileMenu->insertAction(actions.front(), mFilePrint);
      fileMenu->insertSeparator(actions.front());
   }

   mFileRequiredActions += mFileSaveAs;
   mFileRequiredActions += mFilePrint;
   mProjectRequiredActions += mFileSaveAll;
   mProjectRequiredActions += mNewFile;
   mProjectRequiredActions += openFile;
   mProjectRequiredActions += mCloseProject;
   mProjectRequiredActions += mSaveProject;
   mProjectRequiredActions += mExportProject;

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QToolBar*        fileToolbar   = new QToolBar("File", mainWindowPtr);
   fileToolbar->setObjectName("File");
   fileToolbar->setIconSize(QSize(16, 16));
   fileToolbar->addAction(mFileSave);
   fileToolbar->addAction(mFileSaveAll);
   mainWindowPtr->AddToolBar(fileToolbar);

   QList<wkf::Action*> list = {newProject,
                               mNewFile,
                               openFile,
                               mCloseProject,
                               mSaveProject,
                               mExportProject,
                               mFileSave,
                               mFileSaveAs,
                               mFileSaveAll,
                               mFilePrint};
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(list, "Application");
}

void wizard::ActionManager::BuildUpViewMenu()
{
   QMenu* sub = wkfEnv.GetMainWindow()->FindOrCreateViewSubMenu("Editors");

   mCloseAll = new wkf::Action(QString("Close All"), sub);
   QObject::connect(mCloseAll, &QAction::triggered, [this]() { CloseAll(); });
   mCloseNonStartup = new wkf::Action(QString("Close All But Startup"), sub);
   QObject::connect(mCloseNonStartup, &QAction::triggered, [this]() { CloseAllButStartup(); });
   mCloseNonIncluded = new wkf::Action(QString("Close All But Included"), sub);
   QObject::connect(mCloseNonIncluded, &QAction::triggered, [this]() { CloseAllButIncluded(); });
   mCloseInactive = new wkf::Action(QString("Close Inactive"), sub);
   QObject::connect(mCloseInactive, &QAction::triggered, [this]() { CloseInactive(); });

   AddActions(sub, {mCloseAll, mCloseNonStartup, mCloseNonIncluded, mCloseInactive}, cFRONT, true);

   QList<wkf::Action*> list = {mCloseAll, mCloseNonStartup, mCloseNonIncluded, mCloseInactive};
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(list, "Application");
}

void wizard::ActionManager::BuildUpProjectMenu()
{
   QMenu* viewMenu = wkfEnv.GetMainWindow()->FindOrCreateMenu("Project");
   mAddDirectory   = new wkf::Action(QString("Add Directory to Project..."), wkfEnv.GetMainWindow());
   QObject::connect(mAddDirectory, &QAction::triggered, [this]() { AddDirectory(); });
   mChangeHistory = new wkf::Action(QIcon::fromTheme("archive"), "Change History...", wkfEnv.GetMainWindow());
   QObject::connect(mChangeHistory, &QAction::triggered, [this]() { ChangeHistory(); });
   mReparse = new wkf::Action(QIcon::fromTheme("repeat"), "Reparse", wkfEnv.GetMainWindow());
   QObject::connect(mReparse, &QAction::triggered, [this]() { Reparse(); });
   mProjectSettings = new wkf::Action(QString("Settings..."), wkfEnv.GetMainWindow());
   QObject::connect(mProjectSettings, &QAction::triggered, [this]() { ProjectSettings(); });

   AddActions(viewMenu, {mAddDirectory, mChangeHistory, mReparse, mProjectSettings}, cFRONT, true);
   mProjectRequiredActions += mAddDirectory;
   mProjectRequiredActions += mChangeHistory;
   mProjectRequiredActions += mReparse;
   mProjectRequiredActions += mProjectSettings;

   QList<wkf::Action*> list = {mAddDirectory, mChangeHistory, mReparse, mProjectSettings};
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(list, "Application");
}

void wizard::ActionManager::BuildUpHelpMenu()
{
   QMenu* helpMenu = wkfEnv.GetMainWindow()->FindOrCreateMenu("Help");

   mHelpCommandDocumentation =
      new wkf::Action(QString("Command Documentation"), wkfEnv.GetMainWindow(), QKeySequence("F1"));
   QObject::connect(mHelpCommandDocumentation, &QAction::triggered, [this]() { ShowCommandDocumentation(); });
   AddActions(helpMenu, {mHelpCommandDocumentation}, cBACK, false);

   mFileRequiredActions += mHelpCommandDocumentation;

   QList<wkf::Action*> list = {mHelpCommandDocumentation};
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(list, "Application");
}

void wizard::ActionManager::AppendEditActions(QMenu* aMenuPtr)
{
   AddActions(aMenuPtr, {mGoToDefinition, mFindReferences}, cBACK, true);
   AddActions(aMenuPtr, {mUndo, mRedo}, cBACK, true);
   AddActions(aMenuPtr, {mCut, mCopy, mPaste, mSelectAll}, cBACK, true);
   AddActions(aMenuPtr, {mFind, mFindNext, mFindPrevious, mFindInFiles, mReplace}, cBACK, true);
   AddActions(aMenuPtr, {mGoToLine}, cBACK, true);
   AddActions(aMenuPtr, {mAutoComplete, mCommentSelection, mUncommentSelection, mToggleComment, mFormatSelection}, cBACK, true);
}

void wizard::ActionManager::BuildUpTitleBarMenu(QMenu& aMenu, Editor* aEditorPtr)
{
   if (aEditorPtr->document()->isModified())
   {
      QAction* fileSave = new QAction(QIcon::fromTheme("disk"), "Save File", &aMenu);
      connect(fileSave, &QAction::triggered, [aEditorPtr]() { aEditorPtr->GetSource()->Save(); });
      aMenu.addAction(fileSave);
   }
   QAction* fileSaveAs = new QAction(QString("Save File As..."), &aMenu);
   connect(fileSaveAs, &QAction::triggered, [aEditorPtr]() { aEditorPtr->GetSource()->SaveAs(); });
   aMenu.addAction(fileSaveAs);
   aMenu.addSeparator();
   QAction* filePrint = new QAction(QString("Print..."), &aMenu);
   connect(filePrint, &QAction::triggered, [aEditorPtr]() { aEditorPtr->PrintFile(); });
   aMenu.addAction(filePrint);
   aMenu.addSeparator();
   QAction* findInclude = new QAction(QString("Find Include Locations..."), &aMenu);
   connect(findInclude, &QAction::triggered, [aEditorPtr]() { aEditorPtr->GetSource()->FindIncludeLocations(); });
   aMenu.addAction(findInclude);
   QAction* browseFolder = new QAction(QString("Browse Containing Folder..."), &aMenu);
   connect(browseFolder, &QAction::triggered, [aEditorPtr]() { aEditorPtr->GetSource()->BrowseContainingFolder(); });
   aMenu.addAction(browseFolder);
   QAction* renameFile = new QAction(QString("Rename File..."), &aMenu);
   connect(renameFile, &QAction::triggered, [this, aEditorPtr]() { Rename(aEditorPtr); });
   aMenu.addAction(renameFile);
   QAction* copyPath = new QAction(QString("Copy Full Path to Clipboard"), &aMenu);
   connect(copyPath, &QAction::triggered, [aEditorPtr]() { aEditorPtr->GetSource()->CopyPathToClipboard(); });
   aMenu.addAction(copyPath);
   aMenu.addSeparator();
   QAction* closeEditor = new QAction(QString("Close Editor"), &aMenu);
   connect(closeEditor, &QAction::triggered, [aEditorPtr]() { aEditorPtr->close(); });
   aMenu.addAction(closeEditor);
   QAction* closeAllEditors = aMenu.addAction("Close All Editors");
   connect(closeAllEditors, &QAction::triggered, this, &ActionManager::CloseAll);
   QAction* closeAllBut = new QAction(QString("Close All Other Editors"), &aMenu);
   connect(closeAllBut, &QAction::triggered, [this, aEditorPtr]() { CloseAllBut(aEditorPtr); });
   aMenu.addAction(closeAllBut);
   QAction* closeAllLeft = new QAction(QString("Close All Editors to the Left"), &aMenu);
   connect(closeAllLeft, &QAction::triggered, [this, aEditorPtr]() { CloseAllTabsLeft(aEditorPtr); });
   aMenu.addAction(closeAllLeft);
   QAction* closeAllRight = new QAction(QString("Close All Editors to the Right"), &aMenu);
   connect(closeAllRight, &QAction::triggered, [this, aEditorPtr]() { CloseAllTabsRight(aEditorPtr); });
   aMenu.addAction(closeAllRight);
   aMenu.addSeparator();
   QAction* openProjectStartup = new QAction(QString("Open Project Startup File(s)"), &aMenu);
   connect(openProjectStartup, &QAction::triggered, this, &ActionManager::OpenProjectStartup);
   aMenu.addAction(openProjectStartup);
   QAction* openFile = new QAction(QString("Open File"), &aMenu);
   connect(openFile, &QAction::triggered, this, &ActionManager::OpenFile);
   aMenu.addAction(openFile);
   QAction* createFile = new QAction(QString("Create New File"), &aMenu);
   connect(createFile, &QAction::triggered, this, &ActionManager::NewFile);
   aMenu.addAction(createFile);
}

void wizard::ActionManager::BuildUpProjectContextMenu(QMenu& aMenu)
{
   aMenu.addAction(mCloseProject);
   aMenu.addAction(mSaveProject);
   aMenu.addAction(mExportProject);
   aMenu.addSeparator();
   aMenu.addAction(mFileSaveAll);
   aMenu.addSeparator();
   aMenu.addAction(mAddDirectory);
   aMenu.addAction(mChangeHistory);
   aMenu.addAction(mReparse);
   aMenu.addAction(mProjectSettings);
}

void wizard::ActionManager::BuildUpTextSourceContextMenu(QMenu& aMenu, TextSource* aSource, int aMainFile)
{
   if (aSource->IsModified())
   {
      QAction* fileSave = new QAction(QIcon::fromTheme("disk"), "Save File", &aMenu);
      connect(fileSave, &QAction::triggered, [aSource]() { aSource->Save(); });
      aMenu.addAction(fileSave);
   }
   QAction* fileSaveAs = new QAction(QString("Save File As..."), &aMenu);
   connect(fileSaveAs, &QAction::triggered, [aSource]() { aSource->SaveAs(); });
   aMenu.addAction(fileSaveAs);
   aMenu.addSeparator();
   QAction* copyPath = new QAction(QString("Copy Full Path to Clipboard"), &aMenu);
   connect(copyPath, &QAction::triggered, [aSource]() { aSource->CopyPathToClipboard(); });
   aMenu.addAction(copyPath);
   QAction* findInclude = new QAction(QString("Find Include Locations..."), &aMenu);
   connect(findInclude, &QAction::triggered, [aSource]() { aSource->FindIncludeLocations(); });
   aMenu.addAction(findInclude);
   QAction* renameFile = new QAction(QString("Rename File..."), &aMenu);
   connect(renameFile, &QAction::triggered, [this, aSource]() { Rename(aSource); });
   aMenu.addAction(renameFile);
   QAction* browseFolder = new QAction(QString("Browse Containing Folder..."), &aMenu);
   connect(browseFolder, &QAction::triggered, [aSource]() { aSource->BrowseContainingFolder(); });
   aMenu.addAction(browseFolder);

   const QString& itemPathStr = aSource->GetFilePath().GetSystemPath().c_str();

   if (!aMainFile)
   {
      if (!Util::IsNonTextFile(itemPathStr))
      {
         aMenu.addSeparator();
         aMenu.addAction(new ContextMenuActions::SetAsStartupFile(itemPathStr, &aMenu));
         aMenu.addAction(new ContextMenuActions::AddToStartupFiles(itemPathStr, &aMenu));
      }
   }
   else
   {
      aMenu.addSeparator();
      aMenu.addAction(new ContextMenuActions::RemoveFromStartupFiles(itemPathStr, &aMenu));
      aMenu.addAction(new ContextMenuActions::ShowStartupFilesDialog(&aMenu));
   }

   aMenu.addSeparator();
   emit wizSignals->ActionsForFileRequested(QFileInfo(itemPathStr), &aMenu);
}

void wizard::ActionManager::AddActions(QMenu* aMenu, QList<QAction*> aActions, MenuPlace aPlace, bool aPostSeparator)
{
   if (aPlace == cBACK)
   {
      for (auto&& it : aActions)
      {
         aMenu->addAction(it);
      }
      if (aPostSeparator)
      {
         aMenu->addSeparator();
      }
   }
   else if (aPlace == cFRONT)
   {
      QList<QAction*> actions = aMenu->actions();
      if (actions.isEmpty())
      {
         for (auto&& it : aActions)
         {
            aMenu->addAction(it);
         }
         if (aPostSeparator)
         {
            aMenu->addSeparator();
         }
      }
      else
      {
         for (auto&& it : aActions)
         {
            aMenu->insertAction(actions.front(), it);
         }
         if (aPostSeparator)
         {
            aMenu->insertSeparator(actions.front());
         }
      }
   }
}

void wizard::ActionManager::UpdateActions()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   bool    enabled = (current != nullptr);

   if (current)
   {
      mFileSave->setEnabled(current->GetSource()->IsModified());
   }
   else
   {
      mFileSave->setEnabled(false);
   }
   if (dynamic_cast<WsfEditor*>(current))
   {
      for (auto&& it : mWsfRequiredActions)
      {
         it->setEnabled(enabled);
      }
   }

   UpdateFocusActions();

   for (auto&& it : mFileRequiredActions)
   {
      it->setEnabled(enabled);
   }
   bool proj = ((ProjectWorkspace::Instance() != nullptr) && (ProjectWorkspace::Instance()->GetProject() != nullptr));
   for (auto&& it : mProjectRequiredActions)
   {
      it->setEnabled(proj);
   }
   bool prefObjectEnabled = wkfEnv.GetPreferenceObject<wizard::BackupPrefObject>()->Enabled();
   if (!prefObjectEnabled)
   {
      mChangeHistory->setDisabled(true);
   }
}

void wizard::ActionManager::UpdateFocusActions()
{
   Editor* current  = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   bool    hasFocus = (current && current->hasFocus());
   for (auto& action : mFocusRequiredActions)
   {
      action->setEnabled(hasFocus);
   }
}

void wizard::ActionManager::AddDirectory()
{
   if (wizard::Project::Instance())
   {
      wizard::Project::Instance()->AddProjectDirectoryBrowse();
   }
}

void wizard::ActionManager::Autocomplete()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->BeginAutoComplete();
   }
}

void wizard::ActionManager::ChangeHistory()
{
   if (wizard::Project::Instance())
   {
      wizard::Project::Instance()->ShowRevisionManager();
   }
}

void wizard::ActionManager::CloseProject()
{
   if (ProjectWorkspace::Instance()->GetProject() && ProjectWorkspace::Instance()->RequestClose())
   {
      mCloseProject->setEnabled(false);
      mSaveProject->setEnabled(false);
      mExportProject->setEnabled(false);
      mNewFile->setEnabled(false);
   }
}

void wizard::ActionManager::CommentSelection()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->CommentBlock();
   }
}

void wizard::ActionManager::CommentToggle()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->ToggleSelectionComment();
   }
}

void wizard::ActionManager::Copy()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      if (current->hasFocus())
      {
         current->CopyLine();
      }
      else
      {
         wizEnv.CopyFocusedData();
      }
   }
}

void wizard::ActionManager::CloseAll()
{
   ProjectWorkspace::Instance()->GetEditorManager()->CloseAll();
}

void wizard::ActionManager::CloseAllBut(Editor* aEditor)
{
   ProjectWorkspace::Instance()->GetEditorManager()->CloseAllBut(aEditor);
}

void wizard::ActionManager::CloseAllButStartup()
{
   ProjectWorkspace::Instance()->GetEditorManager()->CloseAllButStartup();
}

void wizard::ActionManager::CloseAllButIncluded()
{
   ProjectWorkspace::Instance()->GetEditorManager()->CloseAllButIncluded();
}

void wizard::ActionManager::CloseAllTabsLeft(Editor* aEditor)
{
   EditorManager* editorManager = ProjectWorkspace::Instance()->GetEditorManager();
   if (editorManager && aEditor)
   {
      editorManager->CloseAllLeft(aEditor);
   }
}

void wizard::ActionManager::CloseAllTabsRight(Editor* aEditor)
{
   EditorManager* editorManager = ProjectWorkspace::Instance()->GetEditorManager();
   if (editorManager && aEditor)
   {
      editorManager->CloseAllRight(aEditor);
   }
}

void wizard::ActionManager::CloseCurrentTab()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current && current->hasFocus())
   {
      current->close();
   }
}

void wizard::ActionManager::CloseInactive()
{
   ProjectWorkspace::Instance()->GetEditorManager()->CloseInactive();
}

void wizard::ActionManager::Cut()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current && current->hasFocus())
   {
      current->CutLine();
   }
}

void wizard::ActionManager::DeleteCurrentLine()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current && current->hasFocus())
   {
      current->DeleteLine();
   }
}

void wizard::ActionManager::ExportProject()
{
   if (ProjectWorkspace::Instance() && ProjectWorkspace::Instance()->GetProject())
   {
      ProjectWorkspace::Instance()->GetProject()->ExportProjectAction();
   }
}

void wizard::ActionManager::Find()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      if (current->hasFocus())
      {
         mFindInFilesDialogPtr->SetSearchText(current->GetSelectedText());
      }
      mFindInFilesDialogPtr->SetSearchLocation(FindTextRequest::cACTIVE_FILE);
      mFindInFilesDialogPtr->show();
      mFindInFilesDialogPtr->activateWindow();
   }
}

void wizard::ActionManager::FindInFiles()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if ((current) && current->hasFocus())
   {
      mFindInFilesDialogPtr->SetSearchText(current->GetSelectedText());
   }
   mFindInFilesDialogPtr->SetSearchLocation(FindTextRequest::cPROJECT);
   mFindInFilesDialogPtr->show();
   mFindInFilesDialogPtr->activateWindow();
}

void wizard::ActionManager::FindNext()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->FindNextAction();
   }
}

void wizard::ActionManager::FindPrevious()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->FindPreviousAction();
   }
}

void wizard::ActionManager::FindReferences()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->FindReferencesAction();
   }
}

void wizard::ActionManager::FormatSelection()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->FormatSelection();
   }
}

void wizard::ActionManager::GoToDefinition()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->GoToDefinitionAction();
   }
}

void wizard::ActionManager::GoToLine()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->GoToLineAction();
   }
}

void wizard::ActionManager::MoveTabBackward()
{
   EditorManager* editorManager = ProjectWorkspace::Instance()->GetEditorManager();
   Editor*        current       = editorManager->GetCurrentEditor();
   if (editorManager && current && current->hasFocus())
   {
      editorManager->TabBack();
   }
}

void wizard::ActionManager::MoveTabForward()
{
   EditorManager* editorManager = ProjectWorkspace::Instance()->GetEditorManager();
   Editor*        current       = editorManager->GetCurrentEditor();
   if (editorManager && current && current->hasFocus())
   {
      editorManager->TabForward();
   }
}

void wizard::ActionManager::NewProject()
{
   if (ProjectWorkspace::Instance() && ProjectWorkspace::Instance()->RequestClose())
   {
      ProjectWorkspace::Instance()->NewProject();
   }
}

void wizard::ActionManager::NewFile()
{
   if (wizard::Project::Instance())
   {
      UtPath openDir = wizard::Project::Instance()->ProjectDirectory();
      ProjectWorkspace::Instance()->CreateA_NewFile(wizard::Project::Instance(), openDir);
   }
}

void wizard::ActionManager::OpenCommandDocumentation(TextSource* aSourcePtr, UtTextDocumentRange aRange)
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current && current->GetSource()->GetSource() == aRange.mSource)
   {
      mContextDocumentationControlPtr->show();
      mContextDocumentationControlPtr->raise();
      mContextDocumentationControlPtr->mBrowserPtr->SetAddress(CommandDocumentation::GetDocUrl(aRange));
   }
}

void wizard::ActionManager::OpenFile()
{
   if (ProjectWorkspace::Instance() && ProjectWorkspace::Instance()->GetProject())
   {
      ProjectWorkspace::Instance()->GetProject()->OpenAction();
   }
   else
   {
      Environment::GetInstance().FileOpen();
   }
}

void wizard::ActionManager::OpenProjectStartup()
{
   if (ProjectWorkspace::Instance() && ProjectWorkspace::Instance()->GetProject())
   {
      std::vector<UtPath> startupFiles = ProjectWorkspace::Instance()->GetProject()->GetStartupFiles();
      for (const auto& startupFile : startupFiles)
      {
         ProjectWorkspace::Instance()->GetProject()->OpenFile(
            QFileInfo(QString::fromStdString(startupFile.GetFileName())).absoluteFilePath().toStdString());
      }
   }
}

void wizard::ActionManager::Paste()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->Paste();
   }
}

void wizard::ActionManager::PrintCurrentFile()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->PrintFile();
   }
}

void wizard::ActionManager::ProjectSettings()
{
   if (wizard::Project::Instance())
   {
      wizard::Project::Instance()->ShowSettings();
   }
}

void wizard::ActionManager::Redo()
{
   if (wizSignals->GetProxyEditMode() == Vi::cEDIT_MODE_TEXT)
   {
      wizEnv.Redo();
   }
   else if (wizSignals->GetProxyEditMode() == Vi::cEDIT_MODE_VISUAL)
   {
      Undo::Instance()->UndoStack()->redo();
   }
}

void wizard::ActionManager::Rename(Editor* aEditorPtr)
{
   if (aEditorPtr && aEditorPtr->AcceptsClose())
   {
      mRenameFileDialogPtr->FileRename(aEditorPtr);
   }
}

void wizard::ActionManager::Rename(TextSource* aSourcePtr)
{
   // Call rename on the respective Editor if found
   const auto& map = ProjectWorkspace::Instance()->GetEditorManager()->GetEditorMap();
   auto        it  = map.find(QString::fromStdString(aSourcePtr->GetFilePath().GetSystemPath()));
   if (it != map.end())
   {
      Rename(it->second);
   }
}

void wizard::ActionManager::Reparse()
{
   if (wizard::Project::Instance())
   {
      ProjectWorkspace::Instance()->ReparseAll();
   }
}

void wizard::ActionManager::Replace()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      if (current->hasFocus())
      {
         mReplaceInFilesDialogPtr->SetSearchText(current->GetSelectedText());
      }
      mReplaceInFilesDialogPtr->SetSearchLocation(FindTextRequest::cACTIVE_FILE);
      mReplaceInFilesDialogPtr->show();
      mReplaceInFilesDialogPtr->activateWindow();
   }
}

void wizard::ActionManager::SaveProject()
{
   if (ProjectWorkspace::Instance()->GetProject())
   {
      ProjectWorkspace::Instance()->GetProject()->SaveProject();
   }
}

void wizard::ActionManager::SaveCurrent()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->GetSource()->Save();
   }
}

void wizard::ActionManager::SaveAll()
{
   ProjectWorkspace::Instance()->SaveAll();
}

void wizard::ActionManager::SaveCurrentAs()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      current->GetSource()->SaveAs();
   }
}

void wizard::ActionManager::SelectAll()
{
   QTextEdit* editor = dynamic_cast<QTextEdit*>(QApplication::focusWidget());
   if (editor)
   {
      editor->selectAll();
   }
   else
   {
      QPlainTextEdit* editw = dynamic_cast<QPlainTextEdit*>(QApplication::focusWidget());
      if (editw)
      {
         editw->selectAll();
      }
   }
}


void wizard::ActionManager::ShowCommandDocumentation()
{
   Editor* current = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (current)
   {
      UtTextDocumentRange range;
      current->GetSelectionRange(range);
      mContextDocumentationControlPtr->show();
      mContextDocumentationControlPtr->raise();
      mContextDocumentationControlPtr->mBrowserPtr->SetAddress(CommandDocumentation::GetDocUrl(range));
   }
}

void wizard::ActionManager::UncommentSelection()
{
   WsfEditor* editor = dynamic_cast<WsfEditor*>(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
   if (editor)
   {
      editor->UncommentBlock();
   }
}

void wizard::ActionManager::Undo()
{
   if (wizSignals->GetProxyEditMode() == Vi::cEDIT_MODE_TEXT)
   {
      wizEnv.Undo();
   }
   else if (wizSignals->GetProxyEditMode() == Vi::cEDIT_MODE_VISUAL)
   {
      Undo::Instance()->UndoStack()->undo();
   }
}
