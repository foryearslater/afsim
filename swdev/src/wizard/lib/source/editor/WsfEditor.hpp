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

#ifndef WSFEDITOR_HPP
#define WSFEDITOR_HPP

#include "Editor.hpp"
#include "TextStyler.hpp"
#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"

class UtTextDocument;

namespace wizard
{
class AutoCompletePopup;
class EditorManager;
class Project;
class ProxyMerge;
class SyntaxTips;
class TextSource;

class VI_EXPORT WsfEditor : public Editor, public TextStyler
{
   friend class ActionManager;

public:
   WsfEditor(QWidget* aParent, Project* aProject, TextSource* aTextSource, EditorManager* aManagerPtr);
   ~WsfEditor() override = default;

   void Initialize();
   void Initialize2();

   void  TipsMoved(QRect aRect);
   int   GetContextMenuCharacterPosition();
   void  HideAutocomplete();
   QRect GetTipsGeometry();
   void  UpdateStyle() override;

protected:
   void GoToDefinitionAction() { return GoToDefinition(false); }
   void FindReferencesAction() { return GoToDefinition(true); }
   void BeginAutoComplete();
   bool CommentBlock();
   bool UncommentBlock();
   void FormatSelection();

private:
   struct foldStates
   {
      int  first;
      int  second;
      bool mIsExpanded;

      foldStates(std::pair<int, int> aPair)
      {
         first       = aPair.first;
         second      = aPair.second;
         mIsExpanded = true;
      }
   };
   void GeneralPreferencesChanged(int aValue);
   void EditorPreferencesChanged();

   bool        event(QEvent* event) override;
   void        mousePressEvent(QMouseEvent* e) override;
   void        insertFromMimeData(const QMimeData* aSource) override;
   void        UnhideLines(int aFirstLine, int aLastLine);
   void        focusInEvent(QFocusEvent* e) override;
   void        focusOutEvent(QFocusEvent* e) override;
   void        keyPressEvent(QKeyEvent* e) override;
   void        contextMenuEvent(QContextMenuEvent* e) override;
   void        GoToInclude();
   void        UpdateAutoComplete() override;
   void        OpenHelp() {}
   void        GoToDefinition(bool aAllRefs);
   void        marginMouseMoveEvent(QMouseEvent*) override;
   void        marginLeaveEvent(QEvent* e) override;
   void        marginMousePressEvent(QMouseEvent* e) override;
   void        marginContextMenuEvent(QContextMenuEvent*) override;
   bool        HoverTimeout();
   void        CommandDocumentationAction();
   void        CreateFileFromInclude();
   void        LoadSettings() override;
   void        FocusCheck();
   void        ToggleSelectionComment();
   void        OnProxyUpdate(ProxyMerge* aMerge);
   void        NotificationAction();
   void        Escape();
   bool        isValidSymbolChar(char c) const;
   std::string GetSymbolToken(const char* buf, int len, int pos) const;
   int         IsBrace(char c) const;
   char        OtherBrace(char c) const;
   int         findMatch(int startIndex, int aDirection, const QString& aText, char aBeginChar, char aEndChar) const;
   void        UpdateBraceMatch();
   void        UpdateBreakpointMargin();
   void        CurrentDebugLineChanged();
   void        ReloadComplete() override;
   void        ParseResultsInvalidate(Project* aProjectPtr) {}
   void        UpdateExtraSelections() override;
   int         MarginWidth() override;
   void        PaintMargin(QPaintEvent* aEvent) override;
   void        OnCursorPositionChanged() override;
   void        AddMarker(int aLineNumber, const QImage& aImage);
   void        OnUpdateRequest(const QRect& rect, int w) override;
   void        StyleLines(int aFirstLine, int aLastLine);
   void        SetTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt) override;
   void        MergeTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt) override;
   void        StyleVisibleLines();
   void        DocContentsChange(int from, int charsRemoves, int charsAdded) override;
   void        Paste() override;
   void        dragEnterEvent(QDragEnterEvent* e) override;
   void        dragLeaveEvent(QDragLeaveEvent* e) override;
   void        dragMoveEvent(QDragMoveEvent* e) override;
   void        dropEvent(QDropEvent* e) override;
   void        hideEvent(QHideEvent* e) override;
   bool        PasteMime(const QMimeData* aMimeData);

   AutoCompletePopup* mAutocompletePopupPtr;
   SyntaxTips*        mSyntaxTipsPtr;
   QTimer             mFocusCheckTimer;
   int                mContextMenuCharacterPosition;
   std::string        mDragIncludeText;
   int                mDragIncludeLine;
   bool               mFirstStyleComplete;
   bool               mUpdatingBreakpoints;
   // Background of scripts is colored differently:
   QList<QTextEdit::ExtraSelection> mScriptBackgroundSelections;
   QList<QTextEdit::ExtraSelection> mBraceHighlightSelections;

   // The fold levels.  Each fold region is stored as (first, last) line
   std::vector<foldStates> mFoldLevels;

   // True if the mouse cursor is over the fold margin
   bool mHoveringFolds;

   std::map<int, QImage> mMarkers;

   // The first and last lines which are fully styled
   int mFullStyleFirstLine;
   int mFullStyleLastLine;

   // Members used to implement TextStyler:
   // {
   QVector<QTextCharFormat> mFormatChanges;
   // The first position and length being styled right now
   size_t mCurrentStylePos;
   size_t mCurrentStyleLength;
   // Make sure we don't reenter StyleLines()
   bool             mStylingNow;
   std::vector<int> mModifiedFolds;


   UtTextDocument*  mClickedIncludePtr;
   std::string      mCreateFileFromIncludeName;
   UtCallbackHolder mUtCallbacks;

   static const int cFOLD_MARGIN_WIDTH = 10;
};
} // namespace wizard

#endif // WSFEDITOR_HPP
