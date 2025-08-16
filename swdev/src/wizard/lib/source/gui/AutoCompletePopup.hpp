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

#ifndef AUTOCOMPLETEPOPUP_HPP
#define AUTOCOMPLETEPOPUP_HPP

#include <string>
#include <vector>

#include <QListWidget>
#include <QTimer>
#include <QWidget>

#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "UtTextDocument.hpp"
#include "WsfScriptLookup.hpp"
class WsfParseNode;
class WsfParseCommandReader;
class WsfParseAlternate;
class WsfParseRule;
class UsSymbol;
class QListWidget;
class QKeyEvent;
class UtTextDocumentRange;
class QLabel;

namespace wizard
{
class BrowserWidget;
class TextSource;
class WsfEditor;
class WsfScriptLookup;
class WsfTextPreview;

//! Provides auto-completion for a WsfEditor
class AutoCompletePopup : public QWidget
{
   Q_OBJECT

public:
   class SuggestionList : public QListWidget
   {
   public:
      explicit SuggestionList(AutoCompletePopup* aParentPtr)
         : QListWidget(aParentPtr)
         , mParentPtr(aParentPtr)
      {
         QObject::connect(this, &SuggestionList::itemDoubleClicked, this, &SuggestionList::OnItemDoubleClick);
      }
      void               keyPressEvent(QKeyEvent* event) override;
      bool               InterceptKeyDown(QKeyEvent* aEventPtr);
      void               SetCurrentRow(int aRow, int aSearchDiration);
      void               OnItemDoubleClick(QListWidgetItem* aItem);
      AutoCompletePopup* mParentPtr;
   };
   friend class SuggestionList;
   AutoCompletePopup(QWidget* aParentPtr, WsfEditor* aEditControlPtr);
   ~AutoCompletePopup() override;

   void CursorMove(int aX, int aY, int aLineHeight);
   void UpdateAutocomplete(size_t aCursorLine, size_t aCursorColumn);

   bool InterceptKeyDown(QKeyEvent* aEventPtr);
   bool HasFocus();
   void UpdateWindowPosition();

protected:
   void paintEvent(QPaintEvent*) override;

private:
   void DocLinkClicked(const QUrl& aUrl);
   void LinkActivated(const QString& link);

   bool            UpdateScriptSymbols();
   WsfTextPreview* GetPreview();
   BrowserWidget*  GetDocumentationView();
   void            ClearDocumentationView();
   enum ContextType
   {
      cNONE,
      cWSF,
      cWSF_SCRIPT
   };
   void        ItemClicked(QListWidgetItem* aItemPtr);
   std::string GetLineText(UtTextDocumentRange& aRange);
   bool        InsertSelection(QListWidgetItem* aItemPtr = nullptr);
   void        UpdateAutocompleteWSF();

   bool UpdateAutocompleteScript();

   void UpdateSize();
   bool FilterAutoCompleteList();

   void CurrentSuggestionChanged(QListWidgetItem* current, QListWidgetItem* previous);
   void SelectedItem(QListWidgetItem* aItemPtr);
   bool ScriptMode() { return mContextType == cWSF_SCRIPT; }

   bool CharCompare(char a, char b) const;
   int  GetSymbolMatchScore(const std::string& aInput, const std::string& aSymbol) const;

   ContextType         mContextType;
   bool                mNeedsUpdated;
   size_t              mCursorLine;
   size_t              mCursorColumn;
   UtTextDocumentRange mReplaceRange;
   WsfEditor*          mEditControlPtr;
   UtCallbackHolder    mCallbacks;
   SuggestionList*     mSuggestionListPtr;

   QWidget*        mDetailPane;
   QLabel*         mDetailHeader;
   BrowserWidget*  mDetailDocumentation;
   WsfTextPreview* mDetailPreview;
   QLabel*         mDetailFooter;

   size_t mInsertPosition;
   size_t mReplaceCharacters;
   bool   mSuggestionIsLastToken;
   bool   mSuggestionIsPartialToken;

   int   mCursorX;
   int   mCursorY;
   int   mLineHeight;
   QSize mCurrentSize;

   size_t                   mPreviousCommandPosition;
   bool                     mNeedsNewSuggestionList;
   std::vector<std::string> mCurrentSuggestions;
   UtCallbackN<void()>*     mUpdateCallbackPtr;

   WsfScriptLookup*           mScriptLookupPtr;
   WsfScriptLookup::SymbolMap mScriptSymbols;
   bool                       mShowScriptTypes;
};
} // namespace wizard
#endif
