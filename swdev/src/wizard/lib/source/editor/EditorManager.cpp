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

#include "EditorManager.hpp"

#include <QApplication>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QTabBar>
#include <QToolBar>

#include "EditModeBanner.hpp"
#include "Editor.hpp"
#include "EditorDock.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "UiResources.hpp"
#include "Undo.hpp"
#include "UtMemory.hpp"
#include "Util.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"

wizard::EditorManager::EditorManager()
   : mCurrent(nullptr)
   , mIsReadOnly(false)
   , mUndoAction(false)
   , mUndoCapture(false)
   , mPreviousUndoEditor(nullptr)
{
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ActiveProjectChanged, this, &EditorManager::ActiveProjectChanged);
   connect(wizSignals, &Signals::EditModeSwitch, this, &EditorManager::EditModeSwitch);

   mEditModeBanner = new EditModeBanner(true, Undo::Instance());
   wkfEnv.GetMainWindow()->AddCustomBanner(mEditModeBanner);
}

wizard::EditorManager::~EditorManager()
{
   delete mEditModeBanner;
   ClearEditors();
}

wizard::Editor* wizard::EditorManager::GetCurrentEditor()
{
   return mCurrent;
}

void wizard::EditorManager::VisibilityChanged(Editor* aEditor, bool aVisible)
{
   if (aVisible)
   {
      SetCurrent(aEditor);
      emit wizSignals->ActionsStateChange();
   }
   else
   {
      if (mCurrent == aEditor)
      {
         mCurrent = nullptr;
         emit wizSignals->ActionsStateChange();
      }
   }
}

void wizard::EditorManager::EditorClosed(Editor* aEditor)
{
   if (aEditor == mCurrent)
   {
      mCurrent = nullptr;
      emit wizSignals->ActionsStateChange();
   }
   std::vector<std::vector<Editor*>>::iterator act;
   for (act = mUndoStack.begin(); act != mUndoStack.end();)
   {
      std::vector<Editor*>::iterator change;
      for (change = act->begin(); change != act->end();)
      {
         if (*change == aEditor)
         {
            change = act->erase(change);
         }
         else
         {
            ++change;
         }
      }
      if (act->empty())
      {
         act = mUndoStack.erase(act);
      }
      else
      {
         ++act;
      }
   }
   mPreviousUndoEditor = nullptr;

   for (act = mRedoStack.begin(); act != mRedoStack.end();)
   {
      std::vector<Editor*>::iterator change;
      for (change = act->begin(); change != act->end();)
      {
         if (*change == aEditor)
         {
            change = act->erase(change);
         }
         else
         {
            ++change;
         }
      }
      if (act->empty())
      {
         act = mRedoStack.erase(act);
      }
      else
      {
         ++act;
      }
   }
}

void wizard::EditorManager::AddTab(Editor* aEditor) {}

void wizard::EditorManager::SetCurrent(Editor* aEditor)
{
   if (mCurrent != aEditor)
   {
      mCurrent = aEditor;
      emit wizSignals->ActiveEditorChanged(mCurrent);
   }
   emit        wizSignals->ActionsStateChange();
   EditorDock* ed = mCurrent->GetDock();
   if (ed)
   {
      ed->raise();
   }
   // ed->raise() will trigger VisibilityChanged(), which can clear mCurrent make it invalid
   if (mCurrent)
   {
      mCurrent->setFocus();
   }
}

void wizard::EditorManager::TabFocusIn(Editor* aEditor) {}

void wizard::EditorManager::RecordCurrentLocation() {}

void wizard::EditorManager::ProjectClosing(Project* aProject) {}

void wizard::EditorManager::SetFindRequest(const FindTextRequest& aRequest)
{
   mFindRequest = aRequest;
}

void wizard::EditorManager::SetReplaceRequest(const ReplaceTextRequest& aRequest)
{
   mReplaceRequest = aRequest;
}

wizard::FindTextRequest& wizard::EditorManager::GetActiveFindRequest()
{
   return mFindRequest;
}

wizard::ReplaceTextRequest& wizard::EditorManager::GetActiveReplaceRequest()
{
   return mReplaceRequest;
}

void wizard::EditorManager::TabFocusOut(Editor* aEditor) {}

void wizard::EditorManager::RequestClose(Editor* aEditor)
{
   if (aEditor != nullptr)
   {
      aEditor->GetDock()->close();
   }
}

bool wizard::EditorManager::IsReadOnly()
{
   return false;
}

void wizard::EditorManager::RecordHistoryLocation(const UtTextDocumentLocation& aLoc, bool aForceNewEntry) {}

void wizard::EditorManager::UpdateStyleFromArray(const std::vector<QTextCharFormat>& aFormatChanges,
                                                 const QTextBlock&                   aFirstBlk,
                                                 int                                 aLastBlockNumber,
                                                 size_t                              aStartPos,
                                                 size_t                              aEndPos)
{
}

void wizard::EditorManager::RegisterEditor(Editor* aEditorPtr)
{
   mEditorList[aEditorPtr->windowTitle()] = aEditorPtr;
   aEditorPtr->SetGlobalReadOnly(mIsReadOnly);
}

void wizard::EditorManager::DeregisterEditor(Editor* aEditorPtr)
{
   mEditorList.erase(aEditorPtr->windowTitle());
}

void wizard::EditorManager::ActiveProjectChanged()
{
   ClearEditors();
   emit wizSignals->ActionsStateChange();
}

void wizard::EditorManager::CloseAll()
{
   for (auto&& it : mEditorList)
   {
      it.second->GetDock()->close();
   }
}

QStringList wizard::EditorManager::GetOpenEditorNames() const
{
   QStringList editorNames;
   for (const auto& it : mEditorList)
   {
      editorNames.push_back(it.first);
   }
   return editorNames;
}

void wizard::EditorManager::CloseAllButStartup()
{
   for (auto&& it : mEditorList)
   {
      TextSource* source = it.second->GetSource();
      if (!ProjectWorkspace::Instance()->GetProject()->IncludesStartupFile(source))
      {
         it.second->GetDock()->close();
      }
   }
}

void wizard::EditorManager::CloseAllButIncluded()
{
   for (auto&& it : mEditorList)
   {
      TextSource* source = it.second->GetSource();
      if (!ProjectWorkspace::Instance()->GetProject()->IncludesFile(source))
      {
         it.second->GetDock()->close();
      }
   }
}

void wizard::EditorManager::CloseAllLeft(Editor* aEditor)
{
   int      index  = 0;
   QTabBar* tabBar = FindTabBarWithEditor(aEditor, index);
   if (tabBar)
   {
      for (int i = index - 1; i >= 0; i--)
      {
         QVariant var = tabBar->tabData(i);

         QList<EditorDock*> editors =
            wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
         for (const auto& editor : editors)
         {
            if (var == editor->Key())
            {
               editor->close();
            }
         }
      }
   }
}

void wizard::EditorManager::CloseAllRight(Editor* aEditor)
{
   int      index  = 0;
   QTabBar* tabBar = FindTabBarWithEditor(aEditor, index);
   if (tabBar)
   {
      for (int i = tabBar->count() - 1; i > index; i--)
      {
         QVariant var = tabBar->tabData(i);

         QList<EditorDock*> editors =
            wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
         for (const auto& editor : editors)
         {
            if (var == editor->Key())
            {
               editor->close();
            }
         }
      }
   }
}

void wizard::EditorManager::CloseInactive()
{
   for (auto&& it : mEditorList)
   {
      if (it.second->visibleRegion().isEmpty())
      {
         it.second->GetDock()->close();
      }
   }
}

void wizard::EditorManager::TabBack()
{
   QTabBar* tabBar = FindActiveTabBar();
   if (tabBar)
   {
      int index = tabBar->currentIndex();
      // Calculate the next index
      (index == 0 ? (index = tabBar->count() - 1) : index--);

      SetActiveEditorTab(tabBar->tabData(index));
   }
}

void wizard::EditorManager::TabForward()
{
   QTabBar* tabBar = FindActiveTabBar();
   if (tabBar)
   {
      int index = tabBar->currentIndex();
      // Calculate the next index
      (index == (tabBar->count() - 1) ? index = 0 : index++);

      SetActiveEditorTab(tabBar->tabData(index));
   }
}

QTabBar* wizard::EditorManager::FindActiveTabBar()
{
   QList<EditorDock*> editors =
      wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
   QList<QTabBar*> tabBars =
      wkfEnv.GetMainWindow()->centralWidget()->findChildren<QTabBar*>(QString(), Qt::FindDirectChildrenOnly);
   for (auto& tabBar : tabBars)
   {
      QVariant var = tabBar->tabData(tabBar->currentIndex());
      if (var.isValid())
      {
         for (const auto& editor : editors)
         {
            if (var == editor->Key())
            {
               if (editor->GetEditor()->hasFocus())
               {
                  return tabBar;
               }
            }
         }
      }
   }
   return nullptr;
}

QTabBar* wizard::EditorManager::FindTabBarWithEditor(Editor* aEditor, int& aIndex)
{
   // Get the dock widget associated with the current editor.
   EditorDock* editorDock = GetDockFromEditor(aEditor);
   // If we found the dock widget
   if (editorDock)
   {
      // Find the tabBar that contains the Editor Dock Widget
      QList<QTabBar*> tabBars =
         wkfEnv.GetMainWindow()->centralWidget()->findChildren<QTabBar*>(QString(), Qt::FindDirectChildrenOnly);
      for (auto& tabBar : tabBars)
      {
         // Check each tab in this tabBar and see if it is the EditorDock we are looking for
         for (int i = 0; i < tabBar->count(); i++)
         {
            QVariant var = tabBar->tabData(i);
            if (var.isValid())
            {
               if (var == editorDock->Key())
               {
                  aIndex = i;
                  return tabBar;
               }
            }
         }
      }
   }
   return nullptr;
}

wizard::EditorDock* wizard::EditorManager::GetDockFromEditor(Editor* aEditor)
{
   QList<EditorDock*> editorDocks =
      wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
   for (auto dock : editorDocks)
   {
      if (aEditor == dock->GetEditor())
      {
         return dock;
      }
   }
   return nullptr;
}

bool wizard::EditorManager::SetActiveEditorTab(QVariant aEditorPtr)
{
   if (aEditorPtr.isValid())
   {
      QList<EditorDock*> editors =
         wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
      for (const auto& editor : editors)
      {
         if (aEditorPtr == editor->Key())
         {
            editor->raise();
            editor->GetEditor()->setFocus();
            mCurrent = editor->GetEditor();
            return true;
         }
      }
   }
   return false;
}

bool wizard::EditorManager::CheckEditorsInChangeList(const std::vector<Editor*>& aChangeList) const
{
   auto prefObj = wkfEnv.GetPreferenceObject<EditorPrefObject>();
   if (prefObj && prefObj->UndoPopupsEnabled())
   {
      // Identify if multiple editors are about to be modified
      bool multipleEditors = false;
      if (!aChangeList.empty())
      {
         Editor* editorPtr = aChangeList.at(0);
         for (auto&& ed : aChangeList)
         {
            if (editorPtr != ed)
            {
               multipleEditors = true;
               break;
            }
         }
      }

      // Prompt user about multiple editors being affected
      if (multipleEditors)
      {
         auto response = QMessageBox::question(
            nullptr,
            "Multiple Editors",
            "You about to undo a change that affects multiple files. Are you sure, you want to undo this change?",
            QMessageBox::Yes | QMessageBox::No);
         return (response != QMessageBox::No);
      }

      // Check to see if it is a different Editor to be modified.
      if (!aChangeList.empty() && mPreviousUndoEditor != nullptr && mPreviousUndoEditor != aChangeList.at(0))
      {
         auto response = QMessageBox::question(
            nullptr,
            "Different Editors",
            "You are about to undo a change in a different file. Are you sure, you want to undo this change?",
            QMessageBox::Yes | QMessageBox::No);
         if (response == QMessageBox::No)
         {
            return false;
         }
      }
   }
   return true;
}

void wizard::EditorManager::ClearEditors()
{
   std::vector<Editor*> leditorList;
   for (auto&& it : mEditorList)
   {
      leditorList.push_back(it.second);
   }
   for (auto&& it : leditorList)
   {
      delete it;
   }
   mCurrent = nullptr;
}

void wizard::EditorManager::EditModeSwitch(Vi::EditMode aEditMode)
{
   if (aEditMode != Vi::cEDIT_MODE_TEXT)
   {
      mIsReadOnly = true;
   }
   else
   {
      mIsReadOnly = false;
   }
   for (auto&& it : mEditorList)
   {
      it.second->SetGlobalReadOnly(mIsReadOnly);
   }
}

void wizard::EditorManager::CloseAllBut(Editor* aEditorPtr)
{
   std::vector<Editor*> leditorList;
   for (auto&& it : mEditorList)
   {
      if (it.first != aEditorPtr->windowTitle())
      {
         leditorList.push_back(it.second);
      }
   }
   for (auto&& it : leditorList)
   {
      it->GetDock()->close();
   }
   mCurrent = aEditorPtr;
}

bool wizard::EditorManager::AcceptsClose(TextSource* aSourcePtr)
{
   Editor*                              editor = nullptr;
   std::map<QString, Editor*>::iterator it     = mEditorList.find(aSourcePtr->GetFilePath().GetSystemPath().c_str());
   if (it != mEditorList.end())
   {
      editor = it->second;
   }
   if (editor)
   {
      return editor->AcceptsClose();
   }
   return true;
}

void wizard::EditorManager::CloseEditor(TextSource* aSourcePtr)
{
   Editor*                              editor = nullptr;
   std::map<QString, Editor*>::iterator it     = mEditorList.find(aSourcePtr->GetFilePath().GetSystemPath().c_str());
   if (it != mEditorList.end())
   {
      editor = it->second;
   }
   if (editor)
   {
      editor->close();
   }
}

void wizard::EditorManager::BeginUndoCapture() noexcept
{
   mUndoCapture = true;
   mUndoStack.emplace_back(std::vector<Editor*>());
   mPreviousUndoEditor = nullptr;
}

bool wizard::EditorManager::IsCapturingUndo() const noexcept
{
   return mUndoCapture;
}

void wizard::EditorManager::EndUndoCapture() noexcept
{
   if (mUndoStack.back().empty())
   {
      mUndoStack.pop_back();
      mPreviousUndoEditor = nullptr;
   }
   if (IsCapturingUndo())
   {
      mUndoCapture = false;
   }
}

void wizard::EditorManager::ModificationOccured(Editor* aEditorPtr)
{
   if (IsCapturingUndo())
   {
      mRedoStack.clear();
      mUndoStack.back().push_back(aEditorPtr);
      // record the change
   }
   else if (!mUndoAction)
   {
      mRedoStack.clear();
      BeginUndoCapture();
      mUndoStack.back().push_back(aEditorPtr);
      EndUndoCapture();
   }
}

void wizard::EditorManager::Undo()
{
   if (!IsCapturingUndo())
   {
      mUndoAction = true;
      if (!mUndoStack.empty())
      {
         std::vector<Editor*> changeList = mUndoStack.back();

         bool continueUndoAction = CheckEditorsInChangeList(changeList);
         if (continueUndoAction)
         {
            mUndoStack.pop_back();
            Editor* lastEd = nullptr;
            for (auto&& ed : changeList)
            {
               ed->undo();
               lastEd = ed;
            }
            if (lastEd)
            {
               mPreviousUndoEditor = lastEd;
               SetCurrent(lastEd);
            }
            mRedoStack.emplace_back(changeList);
         }
      }
      mUndoAction = false;
   }
}

void wizard::EditorManager::Redo()
{
   if (!IsCapturingUndo())
   {
      mUndoAction = true;
      if (!mRedoStack.empty())
      {
         std::vector<Editor*> changeList = mRedoStack.back();
         mRedoStack.pop_back();
         Editor* lastEd = nullptr;
         for (auto&& ed : changeList)
         {
            ed->redo();
            lastEd = ed;
         }
         if (lastEd)
         {
            SetCurrent(lastEd);
         }
         mPreviousUndoEditor = nullptr;
         mUndoStack.emplace_back(changeList);
      }
      mUndoAction = false;
   }
}

void wizard::EditorManager::ClearUndoStacks()
{
   mPreviousUndoEditor = nullptr;
   mUndoStack.clear();
   mRedoStack.clear();
}

bool wizard::EditorManager::eventFilter(QObject* aObjPtr, QEvent* aEventPtr)
{
   if (aEventPtr->type() == QEvent::MouseButtonPress)
   {
      QTabBar* tabbar = dynamic_cast<QTabBar*>(aObjPtr);
      if (tabbar)
      {
         QMouseEvent* mouseEventPtr = static_cast<QMouseEvent*>(aEventPtr);
         int          tabIndex      = tabbar->tabAt(mouseEventPtr->pos());
         QVariant     tabData       = tabbar->tabData(tabIndex);
         if (tabData.isValid())
         {
            QList<EditorDock*> editors =
               wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
            for (const auto& iter : editors)
            {
               if (tabData == iter->Key())
               {
                  if (mouseEventPtr->button() == Qt::MiddleButton)
                  {
                     iter->GetEditor()->RequestCloseAction();
                     return true;
                  }
                  else if (mouseEventPtr->button() == Qt::RightButton)
                  {
                     QMenu menu(iter);
                     iter->GetEditor()->BuildTitleBarMenu(menu);
                     menu.exec(tabbar->mapToGlobal(mouseEventPtr->pos()));
                     return true;
                  }
               }
            }
         }
      }
   }

   return QObject::eventFilter(aObjPtr, aEventPtr);
}

void wizard::EditorManager::AdjustFontSize(int aValue)
{
   auto prefObj = wkfEnv.GetPreferenceObject<EditorPrefObject>();
   if (prefObj)
   {
      unsigned int fs = prefObj->FontSize();
      fs += aValue;
      fs = std::min(fs, UiResources::cMAX_FONT_SIZE);
      fs = std::max(fs, UiResources::cMIN_FONT_SIZE);
      prefObj->SetFontSize(fs);
   }
}
