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

#ifndef EDITORMANAGER_HPP
#define EDITORMANAGER_HPP

#include <memory>

#include <QObject>
#include <QTabBar>
#include <QVector>

#include "FindTextRequest.hpp"
#include "ReplaceTextRequest.hpp"
#include "Vi.hpp"
#include "ViExport.hpp"

class QKeyEvent;
class QTextBlock;
class QTextCharFormat;
class QToolBar;
class QWidget;
class UtTextDocumentLocation;

namespace wizard
{
class EditModeBanner;
class Editor;
class EditorDock;
class Project;

class VI_EXPORT EditorManager : public QObject
{
public:
   friend class Editor;
   EditorManager();
   ~EditorManager() override;

   Editor*                           GetCurrentEditor();
   void                              VisibilityChanged(Editor* aEditor, bool aVisible);
   void                              EditorClosed(Editor* aEditor);
   void                              AddTab(Editor* aEditor);
   void                              SetCurrent(Editor* aEditor);
   void                              TabFocusIn(Editor* aEditor);
   void                              RecordCurrentLocation();
   void                              ProjectClosing(Project* aProject);
   void                              SetFindRequest(const FindTextRequest& aRequest);
   void                              SetReplaceRequest(const ReplaceTextRequest& aRequest);
   FindTextRequest&                  GetActiveFindRequest();
   ReplaceTextRequest&               GetActiveReplaceRequest();
   void                              TabFocusOut(Editor* aEditor);
   void                              RequestClose(Editor* aEditor);
   bool                              IsReadOnly();
   void                              RecordHistoryLocation(const UtTextDocumentLocation& aLoc, bool aForceNewEntry);
   void                              TabBack();
   void                              TabForward();
   const std::map<QString, Editor*>& GetEditorMap() const { return mEditorList; }

   static void UpdateStyleFromArray(const std::vector<QTextCharFormat>& aFormatChanges,
                                    const QTextBlock&                   aFirstBlk,
                                    int                                 aLastBlockNumber,
                                    size_t                              aStartPos,
                                    size_t                              aEndPos);

   void        CloseAll();
   void        CloseAllButStartup();
   void        CloseAllButIncluded();
   void        CloseAllLeft(Editor* aEditor);
   void        CloseAllRight(Editor* aEditor);
   void        CloseInactive();
   void        CloseAllBut(Editor* aEditorPtr);
   QStringList GetOpenEditorNames() const;

   bool AcceptsClose(TextSource* aSourcePtr);
   void CloseEditor(TextSource* aSourcePtr);

   //! Start capturing many changes to consider as one change
   void BeginUndoCapture() noexcept;
   //! Denotes whether the EditorManager is capturing many changes to consider as one change
   //! @return the status of the undo capture
   bool IsCapturingUndo() const noexcept;
   //! Stop capturing many changes to consider as one change
   void EndUndoCapture() noexcept;
   void ModificationOccured(Editor* aEditorPtr);
   void Undo();
   void Redo();
   void ClearUndoStacks();
   bool eventFilter(QObject* aObjPtr, QEvent* aEventPtr) override;
   void AdjustFontSize(int aValue);

protected:
   void RegisterEditor(Editor* aEditorPtr);
   void DeregisterEditor(Editor* aEditorPtr);

private:
   void ActiveProjectChanged();

   void ClearEditors();
   void EditModeSwitch(Vi::EditMode aEditMode);

   QTabBar*    FindActiveTabBar();
   QTabBar*    FindTabBarWithEditor(Editor* aEditor, int& aIndex);
   EditorDock* GetDockFromEditor(Editor* aEditor);

   bool SetActiveEditorTab(QVariant aEditorPtr);

   bool CheckEditorsInChangeList(const std::vector<Editor*>& aChangeList) const;

   FindTextRequest    mFindRequest;
   ReplaceTextRequest mReplaceRequest;
   Editor*            mCurrent;

   std::map<QString, Editor*>        mEditorList;
   bool                              mIsReadOnly;
   EditModeBanner*                   mEditModeBanner;
   bool                              mUndoAction;
   bool                              mUndoCapture;
   Editor*                           mPreviousUndoEditor;
   std::vector<std::vector<Editor*>> mUndoStack;
   std::vector<std::vector<Editor*>> mRedoStack;
};
} // namespace wizard

#endif // EDITORMANAGER_HPP
