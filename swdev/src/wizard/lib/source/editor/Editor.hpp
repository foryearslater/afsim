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

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>

#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>

#include "Signals.hpp"

class QTextCursor;
class QTextDocument;
class UtTextDocumentRange;
class UtTextRange;

#include "ViExport.hpp"

namespace wizard
{
class Editor;
class EditorDock;
class EditorManager;
class FindTextRequest;
class Project;
class ScriptDebugger;
class TextSource;

class EditorMargin : public QWidget
{
public:
   explicit EditorMargin(Editor* aP);

   void paintEvent(QPaintEvent*) override;

   QSize   sizeHint() const override { return QSize(30, 0); }
   void    mouseMoveEvent(QMouseEvent* e) override;
   void    mousePressEvent(QMouseEvent* e) override;
   void    leaveEvent(QEvent*) override;
   void    contextMenuEvent(QContextMenuEvent*) override;
   Editor* mParent;
};

class VI_EXPORT Editor : public QPlainTextEdit
{
public:
   friend class EditorMargin;

   Editor(QWidget* aParent, Project* aProject, TextSource* aTextSource, EditorManager* aManagerPtr);
   ~Editor() override;

   void Initialize();

   void         PrintFile();
   void         DeleteLine();
   virtual void Paste();
   void         FindNextAction();
   void         FindPreviousAction();
   void         GoToLineAction();
   virtual void LoadSettings();

   bool        ReplaceFoundText();
   void        Replace(const UtTextRange& aRange, const std::string aText);
   std::string GetSelectedText();
   bool        SearchNext(const FindTextRequest& aFindRequest, QTextCursor* aCursorPtr = nullptr);
   bool        SearchNextNoWrap(const FindTextRequest& aFindRequest,
                                QTextCursor*           aCursorPtr  = nullptr,
                                int                    aLowerBound = -1,
                                int                    aUpperBound = -1);
   bool        SearchPrevious(const FindTextRequest& aFindRequest);
   bool        GetSelectionRange(UtTextDocumentRange& aRange);
   void        SetSelection(const UtTextRange& aRange);

   void        GetCurrentPosition(size_t& aOffset, size_t& aLine, size_t& aColumn);
   std::string GetLineText(const size_t& aLine);
   bool        TextIsSelected();

   void        GetLineColumn(const size_t& aOffset, size_t& aLine, size_t& aColumn);
   QPoint      GetPoint(const size_t aOffset);
   size_t      GetOffset(const size_t aLine, const size_t aColumn);
   int         GetLineHeight(const int aLine);
   TextSource* GetSource() { return mSourcePtr; }
   Project*    GetProject() { return mProjectPtr; }
   size_t      GetPosition(size_t aLine, size_t aColumn);
   size_t      GetPosition(const QPoint& aPoint);

   std::string RangeText(size_t aStart, size_t aEnd);
   void        RequestCloseAction();

   virtual void BeginNonUserAction() { ++mNonUserAction; }
   virtual void EndNonUserAction() { --mNonUserAction; }
   virtual bool ExecutingNonUserAction() { return 0 != mNonUserAction; }
   virtual void ReloadComplete() {}

   void RecordCurrentLocation(bool aForceNewEntry = true);
   void GoToLine(size_t aLine);
   void GoToRange(const UtTextRange& aRange);

   void CopyLine();
   void CutLine();

   QString            ToPlainText();
   static std::string ToAscii(const QString& aText);

   virtual void UpdateStyle() {}
   static void  UpdateStyleFromArray(const QVector<QTextCharFormat>& aFormatChanges,
                                     const QTextBlock&               firstBlk,
                                     int                             aLastBlockNumber,
                                     int                             startPos,
                                     int                             endPos);

   void SetGlobalReadOnly(bool aIsGlobalReadOnly);

   void        Unindent();
   void        Indent();
   void        SetManager(EditorManager* aManagerPtr);
   virtual int MarginWidth();
   int         LineNumberWidth();

   void BuildTitleBarMenu(QMenu& aMenu);

   bool AcceptsClose();

   QString GetShortName(bool aTrim = true) const;

   // This will split a string into a string list that
   // 1. records white space
   // 2. separates comment blocks
   // 3. has identifiable executable tokens by using Empty
   static QStringList Split(const QString& aStr);

   // Looks at a token string list created by Split and starting from aStart (inclusive) finds the next
   // useful data token, and returns its index.
   static int FindNextValidToken(const QStringList& aList, int aStart);

   void mousePressEvent(QMouseEvent* aEvent) override;

   //! Get the EditorDock containing this Editor
   //! @return the containing EditorDock
   const EditorDock* GetDock() const noexcept;
   //! Get the EditorDock containing this Editor
   //! @return the containing EditorDock
   EditorDock* GetDock() noexcept;

public slots:
   void DebugRunStateChange(wizard::ScriptDebugger* aDebuggerPtr, bool aIsRunning);
   void TextModified();

protected:
   void           keyPressEvent(QKeyEvent* e) override;
   EditorManager* Manager() { return mManagerPtr; }
   bool           FoldingEnabled() { return mFoldingEnabled; }
   void           RedrawMargin();
   QTimer&        DeferUpdateTimer() { return mDeferredUpdateSelectionsTimer; }
   EditorMargin*  Margin() { return mMarginWidget; }
   virtual void   PaintMargin(QPaintEvent* event);
   virtual void   OnCursorPositionChanged();
   void           GetCursorPoint(int& aX, int& aY, int& aLineHeight);
   virtual void   OnUpdateRequest(const QRect&, int);
   virtual void   DocContentsChange(int aFrom, int aCharsRemoves, int aCharsAdded);
   void           AddCurrentLineSelection(QList<QTextEdit::ExtraSelection>& aSelections);
   bool           event(QEvent* e) override;
   void           UpdateMarginGeometry();

   static const int cSYMBOL_MARGIN_WIDTH = 16;
   void             focusInEvent(QFocusEvent* e) override;

private:
   virtual void UpdateAutoComplete() {}


   void EnsureCurrentLineVisible();

   void UpdateMarginViewport();
   void OnUndoAvailable(bool aAvailable);
   void OnRedoAvailable(bool aAvailable);
   void OnModificationChanged(bool aIsModified);
   void OnTextChanged();

   int  SpaceFromIndent(int aIndentSize) const;
   int  LineSpaceFromIndent(int aIndentSize, size_t aLine) const;
   bool SameWhitespaceOffset(size_t aFirstLine, size_t aLastLine, int aIndentSize) const;

   virtual void UpdateExtraSelections();
   void         wheelEvent(QWheelEvent* e) override;
   void         resizeEvent(QResizeEvent* e) override;
   void         showEvent(QShowEvent* e) override;
   void         closeEvent(QCloseEvent* e) override;
   virtual void marginMouseMoveEvent(QMouseEvent*) {}
   virtual void marginLeaveEvent(QEvent*) {}
   virtual void marginMousePressEvent(QMouseEvent* e) {}
   virtual void marginContextMenuEvent(QContextMenuEvent*) {}

   Project*    mProjectPtr;
   TextSource* mSourcePtr;

   EditorManager* mManagerPtr;
   bool           mFoldingEnabled;
   bool           mUndoEnabled;
   bool           mRedoEnabled;
   bool           mGlobalReadOnly;
   bool           mSendModified;
   bool           mInitalized;
   int            mNonUserAction;
   EditorMargin*  mMarginWidget;

   QTimer mDeferredUpdateSelectionsTimer;

   UtCallbackHolder mCallbacks;
};
} // namespace wizard

#endif // EDITOR_HPP
