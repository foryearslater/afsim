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

#include "AutoCompletePopup.hpp"

#include <sstream>

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QTextBrowser>
#include <QTextDocument> // for Qt::escape()
#include <QVBoxLayout>

#include "AutoComplete.hpp"
#include "BrowserWidget.hpp"
#include "EditAssist.hpp"
#include "EditorPrefObject.hpp"
#include "InputContext.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "SmallWebView.hpp"
#include "TextSource.hpp"
#include "UtCast.hpp"
#include "UtMath.hpp"
#include "UtQt.hpp"
#include "UtStringUtil.hpp"
#include "UtTextDocument.hpp"
#include "WkfEnvironment.hpp"
#include "WsfEditor.hpp"
#include "WsfExe.hpp"
#include "WsfParseAlternate.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRecurrence.hpp"
#include "WsfParseUtil.hpp"
#include "WsfScriptLookup.hpp"
#include "WsfTextPreview.hpp"

// Popup window note:
//  There is no great portable way to make a popup window that doesn't steal focus from the main window.
//  Instead, this widget is a child of the top-level window.
wizard::AutoCompletePopup::AutoCompletePopup(QWidget*   aParentPtr,
                                             WsfEditor* aEditControlPtr)
   : QWidget(aEditControlPtr->window())
   , //, Qt::WindowFlags(/*Qt::Tool |*/ Qt::FramelessWindowHint | /*Qt::CustomizeWindowHint |*/ Qt::WindowStaysOnTopHint)),
   mContextType(cNONE)
   , mNeedsUpdated(false)
   , mEditControlPtr(aEditControlPtr)
   , mCursorX(-1)
   , mCursorY(-1)
   , mLineHeight(1)
   , mPreviousCommandPosition(ut::npos)
   , mUpdateCallbackPtr()
   , mScriptLookupPtr()
{
   setLayout(new QHBoxLayout(this));
   layout()->setContentsMargins(0, 0, 0, 0);
   layout()->setSpacing(0);
   mSuggestionListPtr = new SuggestionList(this);
   setFocusProxy(mSuggestionListPtr);
   // mSuggestionListPtr->setFocusProxy(aEditControlPtr);
   layout()->addWidget(mSuggestionListPtr);
   mSuggestionListPtr->setFocusPolicy(Qt::StrongFocus);
   mSuggestionListPtr->setMinimumHeight(1);
   // Note: Setting scroll bars to always on or off because of bug in Qt 4.7.4
   mSuggestionListPtr->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mSuggestionListPtr->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

   mDetailPane = new QWidget(this);

   layout()->addWidget(mDetailPane);
   mDetailPane->setLayout(new QVBoxLayout(mDetailPane));
   mDetailPane->layout()->setContentsMargins(0, 0, 0, 0);
   mDetailPane->layout()->setSpacing(0);
   mDetailHeader = new QLabel(mDetailPane);
   mDetailHeader->setStyleSheet("background-color:#f1f1f1; color: #000000");

   mDetailPane->layout()->addWidget(mDetailHeader);
   mDetailFooter = new QLabel(mDetailPane);
   mDetailFooter->setStyleSheet("background-color:#f1f1f1; color: #000000");
   mDetailFooter->setVisible(false);
   mDetailPane->layout()->addWidget(mDetailFooter);

   QObject::connect(mDetailFooter, &QLabel::linkActivated, this, &AutoCompletePopup::LinkActivated);

   mDetailPane->layout()->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

   mDetailPane->setFixedWidth(400);
   mDetailPane->hide();

   mDetailPreview       = nullptr;
   mDetailDocumentation = nullptr;

   resize(300, 300);

   QObject::connect(mSuggestionListPtr, &SuggestionList::itemClicked, this, &AutoCompletePopup::ItemClicked);
   QObject::connect(mSuggestionListPtr, &SuggestionList::currentItemChanged, this, &AutoCompletePopup::CurrentSuggestionChanged);
}

wizard::AutoCompletePopup::~AutoCompletePopup()
{
   delete mScriptLookupPtr;
   delete mUpdateCallbackPtr;
}

void wizard::AutoCompletePopup::CursorMove(int aX, int aY, int aLineHeight)
{
   if (aX != mCursorX || aY != mCursorY || aLineHeight != mLineHeight)
   {
      mCursorX    = aX;
      mCursorY    = aY;
      mLineHeight = aLineHeight;
      UtTextDocumentRange range;
      mEditControlPtr->GetSelectionRange(range);
      size_t selBegin = range.GetBegin();
      size_t line, column;
      mEditControlPtr->GetLineColumn(selBegin, line, column);
      UpdateAutocomplete(line, column);
      UpdateWindowPosition();
   }
}

void wizard::AutoCompletePopup::UpdateWindowPosition()
{
   //  mSuggestionListPtr->UpdateSize();

   QPoint ptInWindow(mCursorX, mCursorY + mLineHeight);
   ptInWindow.rx() = UtMath::Limit(ptInWindow.rx(), 0, mEditControlPtr->width());
   ptInWindow.ry() = UtMath::Limit(ptInWindow.ry(), 0, mEditControlPtr->height());
   QPoint pt       = mEditControlPtr->mapToGlobal(ptInWindow);
   QRect  tipsRect = mEditControlPtr->GetTipsGeometry();
   if (tipsRect.isValid())
   {
      if (pt.x() + width() > tipsRect.left())
      {
         pt.ry() = tipsRect.bottom();
      }
   }
   QRect parentRect = parentWidget()->geometry();
   parentRect.moveTopLeft(parentWidget()->mapToGlobal(parentRect.topLeft()));
   // int screenHeight = QApplication::desktop()->height();

   int verticalExtent = pt.y() + frameGeometry().height();
   if (verticalExtent > parentRect.bottom())
   {
      pt.setY(pt.y() - mLineHeight - frameGeometry().height());
   }
   move(parentWidget()->mapFromGlobal(pt));
}


//! Provides the auto-complete popup with new information.  Actual update of the suggestion list
//! may happen later.
void wizard::AutoCompletePopup::UpdateAutocomplete(size_t aCursorLine, size_t aCursorColumn)
{
   mNeedsUpdated       = true;
   size_t oldLine      = mCursorLine;
   mCursorLine         = aCursorLine;
   mCursorColumn       = aCursorColumn;
   Project* projectPtr = mEditControlPtr->GetProject();
   if (projectPtr)
   {
      // If we are already displaying help, don't keep the auto-complete open when the line changes
      if (mContextType != cNONE)
      {
         if (oldLine != mCursorLine)
         {
            mEditControlPtr->HideAutocomplete();
            return;
         }
      }
      mContextType = cNONE;
      UpdateAutocompleteScript();
      if (mContextType != cWSF_SCRIPT)
      {
         mNeedsNewSuggestionList = !FilterAutoCompleteList();
         if (!mUpdateCallbackPtr)
         {
            mUpdateCallbackPtr = new UtCallbackN<void()>(std::bind(&AutoCompletePopup::UpdateAutocompleteWSF, this));
            projectPtr->ExecuteWhenParseComplete(mUpdateCallbackPtr);
         }
      }
   }
}

void wizard::AutoCompletePopup::UpdateAutocompleteWSF()
{
   if (mUpdateCallbackPtr)
   {
      delete mUpdateCallbackPtr;
      mUpdateCallbackPtr = nullptr;
   }
   TextSource* sourcePtr = mEditControlPtr->GetSource();
   if (!sourcePtr->GetPendingChanges().empty())
   {
      return;
   }

   Project* projectPtr = mEditControlPtr->GetProject();
   if (!projectPtr)
   {
      return;
   }
   if (!mNeedsNewSuggestionList)
   {
      return;
   }
   ParseResults* results = projectPtr->GetParseResults();
   if (!results)
   {
      return;
   }
   size_t lines = sourcePtr->GetSource()->GetLineCount();

   if (mCursorLine < lines)
   {
      InputContext::Context con;
      InputContext::ContextFromCursor(mEditControlPtr, mCursorLine, mCursorColumn, con);
      // Determine if the input 'context' has changed.  If a change is detected, quit showing the autocomplete menu.
      if (mPreviousCommandPosition != con.mQueryTextRange.GetBegin())
      {
         if (mPreviousCommandPosition != ut::npos)
         {
            // A changes is allowed if it is only whitespace:
            while (mPreviousCommandPosition < con.mQueryTextRange.GetBegin())
            {
               if (!WsfParseUtil::IsWhitespace(sourcePtr->GetSource()->GetText()[mPreviousCommandPosition]))
               {
                  break;
               }
               ++mPreviousCommandPosition;
            }
            if (mPreviousCommandPosition != con.mQueryTextRange.GetBegin())
            {
               InputContext::ContextFromCursor(mEditControlPtr, mCursorLine, mCursorColumn, con);
               mEditControlPtr->HideAutocomplete();
               return;
            }
         }
      }
      mPreviousCommandPosition = con.mQueryTextRange.GetBegin();

      size_t                pos = mCursorColumn + sourcePtr->GetSource()->GetLinePosition(mCursorLine);
      AutoComplete          autoCompleter;
      AutoComplete::Results acResults;
      WsfParseNode*         tmpBlock;
      tmpBlock              = con.mBlockNodePtr;
      mReplaceRange         = con.mQueryTextRange;
      size_t      insertPos = pos - mReplaceRange.GetBegin();
      std::string queryText = GetLineText(mReplaceRange);
      if (results)
      {
         autoCompleter.GetSuggestions(*results, *sourcePtr, tmpBlock, queryText, insertPos, acResults);
      }

      mInsertPosition           = acResults.mInsertPosition + mReplaceRange.GetBegin();
      mReplaceCharacters        = acResults.mReplaceCharacters;
      mSuggestionIsLastToken    = acResults.mLastToken;
      mSuggestionIsPartialToken = acResults.mPartialToken;
      mSuggestionListPtr->clear();
      mCurrentSuggestions.clear();
      QListWidgetItem items;
      size_t          firstRealItem = 0;
      mCurrentSuggestions           = acResults.mSuggestionList;
      for (size_t i = 0; i < mCurrentSuggestions.size();)
      {
         std::string suggestion = mCurrentSuggestions[i];
         if (!suggestion.empty())
         {
            bool canSelect = suggestion[0] != '<';
            // don't select items that can't be inserted by default
            if (!canSelect && i == firstRealItem)
            {
               ++firstRealItem;
            }
            // a rare few WSF inputs DO have a leading '<', they will be escaped with \\  ...
            if (suggestion[0] == '\\' && suggestion[1] == '<')
            {
               suggestion = suggestion.substr(1);
            }
            QListWidgetItem* itemPtr = new QListWidgetItem(QString::fromStdString(suggestion));
            if (!canSelect)
            {
               QFont ifont = itemPtr->font();
               ifont.setItalic(true);
               itemPtr->setFont(ifont);
            }
            mSuggestionListPtr->addItem(itemPtr);
            ++i;
         }
         else
         {
            mCurrentSuggestions.erase(mCurrentSuggestions.begin() + i);
         }
      }
      mSuggestionListPtr->SetCurrentRow((int)firstRealItem, 1);
      FilterAutoCompleteList();

      // If there are no suggestions then don't show then exit auto complete
      if (mSuggestionListPtr->count() <= 1)
      {
         mEditControlPtr->HideAutocomplete();
      }
   }
   else
   {
      UpdateSize();
      UpdateWindowPosition();
   }
   mNeedsUpdated = false;
}

//! Intercepts key events that may be useful to the autocomplete popup.
//! @param aEventPtr A point to the event.
//! @return bool Returns 'true' if the key event was intercepted.
bool wizard::AutoCompletePopup::InterceptKeyDown(QKeyEvent* aEventPtr)
{
   return mSuggestionListPtr->InterceptKeyDown(aEventPtr);
}

bool wizard::AutoCompletePopup::SuggestionList::InterceptKeyDown(QKeyEvent* aEventPtr)
{
   bool triggerHide  = false;
   bool interceptKey = false;
   switch (aEventPtr->key())
   {
   case Qt::Key_Tab:
   case Qt::Key_Return:
      if (mParentPtr->InsertSelection())
      {
         return true;
      }
      return false;
      break;
   case Qt::Key_Space:
      break;
   case Qt::Key_Down:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      {
         SetCurrentRow(currentIndex().row() + 1, 1);
      }
      interceptKey = true;
      break;
   case Qt::Key_Up:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      SetCurrentRow(currentIndex().row() - 1, -1);
      interceptKey = true;
      break;
   case Qt::Key_PageDown:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      for (int i = 0; i < 10; ++i)
      {
         SetCurrentRow(currentIndex().row() + 1, 1);
      }
      interceptKey = true;
      break;
   case Qt::Key_PageUp:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      for (int i = 0; i < 10; ++i)
      {
         SetCurrentRow(currentIndex().row() - 1, -1);
      }
      interceptKey = true;
      break;
   case Qt::Key_Home:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      SetCurrentRow(0, 1);
      interceptKey = true;
      break;
   case Qt::Key_End:
      if (aEventPtr->modifiers() != 0)
      {
         return false;
      }
      SetCurrentRow(count() - 1, -1);
      interceptKey = true;
      break;
   case Qt::Key_Escape:
      triggerHide  = true;
      interceptKey = true;
      break;
   default:
      break;
   }
   // While in script-mode, hide popup when non-identifier characters are inserted
   if (!triggerHide && !interceptKey && mParentPtr->ScriptMode())
   {
      switch (aEventPtr->key())
      {
      case Qt::Key_Space:
      case Qt::Key_Exclam:
      case Qt::Key_QuoteDbl:
      case Qt::Key_NumberSign:
      case Qt::Key_Dollar:
      case Qt::Key_Percent:
      case Qt::Key_Ampersand:
      case Qt::Key_Apostrophe:
      case Qt::Key_ParenLeft:
      case Qt::Key_ParenRight:
      case Qt::Key_Asterisk:
      case Qt::Key_Plus:
      case Qt::Key_Comma:
      case Qt::Key_Minus:
      // case Qt::Key_Period:     keep up after period
      case Qt::Key_Slash:
      case Qt::Key_Colon:
      case Qt::Key_Semicolon:
      case Qt::Key_Less:
      case Qt::Key_Equal:
      case Qt::Key_Greater:
      case Qt::Key_Question:
      case Qt::Key_At:
      case Qt::Key_BracketLeft:
      case Qt::Key_Backslash:
      case Qt::Key_BracketRight:
      case Qt::Key_AsciiCircum:
      // case Qt::Key_Underscore:
      case Qt::Key_QuoteLeft:
      case Qt::Key_BraceLeft:
      case Qt::Key_Bar:
      case Qt::Key_BraceRight:
      case Qt::Key_AsciiTilde:
         triggerHide = true;
      default:
         break;
      }
   }
   if (triggerHide)
   {
      mParentPtr->mEditControlPtr->HideAutocomplete();
   }
   return interceptKey;
}

void wizard::AutoCompletePopup::SuggestionList::keyPressEvent(QKeyEvent* event)
{
   if (InterceptKeyDown(event))
   {
      event->accept();
   }
   else
   {
      QListWidget::keyPressEvent(event);
   }
}

void wizard::AutoCompletePopup::SuggestionList::SetCurrentRow(int aRow, int aSearchDiration)
{
   aRow = std::max(0, aRow);
   aRow = std::min(aRow, count() - 1);
   for (int r = aRow;; r += aSearchDiration)
   {
      QListWidgetItem* itemPtr = item(r);
      if (!itemPtr)
      {
         break;
      }
      if (!itemPtr->isHidden())
      {
         setCurrentRow(r);
         return;
      }
   }
   for (int r = aRow;; r -= aSearchDiration)
   {
      QListWidgetItem* itemPtr = item(r);
      if (!itemPtr)
      {
         break;
      }
      if (!itemPtr->isHidden())
      {
         setCurrentRow(r);
         return;
      }
   }
}

void wizard::AutoCompletePopup::SuggestionList::OnItemDoubleClick(QListWidgetItem* aItem)
{
   mParentPtr->InsertSelection(aItem);
}

bool wizard::AutoCompletePopup::InsertSelection(QListWidgetItem* aItemPtr)
{
   QListWidgetItem* itemPtr = aItemPtr;
   if (itemPtr == nullptr)
   {
      itemPtr = mSuggestionListPtr->currentItem();
   }
   if (itemPtr != nullptr && !itemPtr->isHidden())
   {
      // italic items are not meant to be inserted
      if (itemPtr->font().italic())
      {
         return false;
      }
      std::string text = itemPtr->text().toStdString();
      if (text.empty())
      {
         return false;
      }
      std::string endToken;
      // If '...' is found in the suggestion, it implies a block with a terminator token.
      // Split the text into the two pieces here
      if (text.find("...") != std::string::npos)
      {
         size_t splitPos = text.find("...");
         endToken        = text.substr(splitPos + 3);
         text            = text.substr(0, splitPos);
      }

      if (!mSuggestionIsLastToken)
      {
         text += ' ';
      }

      UtTextRange         range(mInsertPosition, mInsertPosition + mReplaceCharacters);
      UtTextDocumentRange selectedRange;
      bool                isSelected = mEditControlPtr->GetSelectionRange(selectedRange);

      // Extend deleted area to right end of selection
      if (isSelected && selectedRange.GetEnd() > range.GetEnd())
      {
         range.SetEnd(selectedRange.GetEnd());
      }
      QTextCursor cur(mEditControlPtr->textCursor());
      cur.beginEditBlock();

      cur.setPosition(ut::cast_to_int(range.GetBegin()));
      int insertCol = cur.columnNumber();
      cur.setPosition(ut::cast_to_int(range.GetEnd()), QTextCursor::KeepAnchor);
      cur.insertText(text.c_str());
      int insertEndPos = cur.position();
      // mEditControlPtr->Replace(range, text);

      // If an end token exists, insert the end token at the end of this line.
      if (!endToken.empty())
      {
         cur.movePosition(QTextCursor::EndOfLine);
         std::string newLineStr = "\n" + std::string(insertCol, ' ') + endToken;
         cur.insertText(newLineStr.c_str());
         cur.setPosition(insertEndPos);
      }
      cur.endEditBlock();
      mEditControlPtr->setTextCursor(cur);
      mSuggestionListPtr->clear();
      mCurrentSuggestions.clear();
      hide();
      if (mSuggestionIsLastToken)
      {
         mEditControlPtr->HideAutocomplete();
      }
      return true;
   }
   else
   {
      mEditControlPtr->HideAutocomplete();
   }
   return false;
}

bool wizard::AutoCompletePopup::HasFocus()
{
   return hasFocus() || mSuggestionListPtr->hasFocus();
}

void wizard::AutoCompletePopup::ItemClicked(QListWidgetItem* aItemPtr)
{
   if (aItemPtr)
   {
      // Insert text on click when NOT doing script completion
      if (mContextType != cWSF_SCRIPT)
      {
         InsertSelection();
         mEditControlPtr->activateWindow();
      }
   }
}

std::string wizard::AutoCompletePopup::GetLineText(UtTextDocumentRange& aRange)
{
   if (!aRange.mSource)
   {
      return "";
   }
   std::string text = aRange.Text();
   return text;
}


void wizard::AutoCompletePopup::paintEvent(QPaintEvent* e)
{
   // QPainter p(this);
   // p.fillRect(e->rect(), Qt::white);
   QWidget::paintEvent(e);
   UpdateSize();
}

void wizard::AutoCompletePopup::UpdateSize()
{
   int w = std::min(400, mSuggestionListPtr->sizeHintForColumn(0) + 30);
   mSuggestionListPtr->setMaximumWidth(w);

   int rows       = mSuggestionListPtr->count();
   int hiddenRows = 0;
   for (int i = 0; i < rows; ++i)
   {
      QListWidgetItem* itemPtr = mSuggestionListPtr->item(i);
      if (itemPtr->isHidden())
      {
         ++hiddenRows;
      }
   }
   rows -= hiddenRows;
   int h = 1;
   if (rows > 0)
   {
      h = std::min(300, mSuggestionListPtr->sizeHintForRow(0) * rows + 10);
   }
   if (mDetailPane->isVisible())
   {
      w += mDetailPane->width() + 5;
      h = std::max(mDetailPane->sizeHint().height(), h);
   }
   QSize newSize(w, h);
   if (newSize != mCurrentSize)
   {
      mCurrentSize = newSize;
      resize(mCurrentSize);
   }
}

// Attempt to re-use the suggestion list after the user types more characters.
// Returns true if the suggestion list was updated successfully
bool wizard::AutoCompletePopup::FilterAutoCompleteList()
{
   if (mCurrentSuggestions.empty())
   {
      return false;
   }
   size_t offset, line, column, insertLine, insertCol;
   mEditControlPtr->GetCurrentPosition(offset, line, column);
   mEditControlPtr->GetLineColumn(mInsertPosition, insertLine, insertCol);
   bool insertPointIsValid(false);
   if (insertLine == line)
   {
      if (insertCol <= column)
      {
         std::string text     = mEditControlPtr->GetLineText(line);
         std::string wordText = text.substr(insertCol, column - insertCol);

         // Calculate the length of the word to be replaced, if it exists.
         size_t wordEnd = column;
         while (wordEnd < text.length() && !WsfParseUtil::IsWhitespace(text[wordEnd]))
         {
            wordEnd++;
         }
         mReplaceCharacters = wordEnd - insertCol;

         size_t firstChar = 0;
         for (; firstChar < wordText.size() && WsfParseUtil::IsWhitespace(wordText[firstChar]); ++firstChar)
         {
         }
         wordText = wordText.substr(firstChar);
         if (std::find_if(wordText.begin(), wordText.end(), &WsfParseUtil::IsWhitespace) == wordText.end())
         {
            int newSelRow           = -1;
            int newSelRowMatchScore = 0;
            insertPointIsValid      = true;
            for (size_t i = 0; i < mCurrentSuggestions.size(); ++i)
            {
               // bool goodMatch = true;
               int                matchScore = 0;
               const std::string& suggestion = mCurrentSuggestions[i];
               if (suggestion[0] != '<')
               {
                  if (suggestion.size() < wordText.size())
                  {
                     // bad suggestion
                  }
                  else
                  {
                     if (std::equal(wordText.begin(), wordText.end(), suggestion.begin()))
                     {
                        matchScore = 3;
                     }
                     else
                     {
                        if (std::search(suggestion.begin(), suggestion.end(), wordText.begin(), wordText.end()) !=
                            suggestion.end())
                        {
                           matchScore = 2;
                        }
                        else
                        {
                           int minShownScore = 950 * (int)wordText.size(); // make sure the match is 95% correct
                           int score         = GetSymbolMatchScore(wordText, suggestion);
                           if (score >= minShownScore)
                           {
                              matchScore = 1;
                           }
                        }
                     }
                  }
               }
               QListWidgetItem* itemPtr = mSuggestionListPtr->item((int)i);
               itemPtr->setHidden(matchScore == 0);

               if (matchScore >= 3)
               {
                  itemPtr->setForeground(
                     wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_auto_complete_3"));
               }
               else if (matchScore >= 2)
               {
                  itemPtr->setForeground(
                     wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_auto_complete_2"));
               }
               else if (matchScore >= 1)
               {
                  itemPtr->setForeground(
                     wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_auto_complete_1"));
               }
               if (matchScore != 0 && (matchScore > newSelRowMatchScore))
               {
                  newSelRow           = (int)i;
                  newSelRowMatchScore = matchScore;
               }
            }
            if (newSelRow >= 0)
            {
               mSuggestionListPtr->setCurrentRow(newSelRow);
            }
         }
      }
   }

   if (!insertPointIsValid)
   {
      hide();
   }
   else
   {
      // show();
      UpdateSize();
      UpdateWindowPosition();
      show();
   }


   return insertPointIsValid;
}

bool wizard::AutoCompletePopup::UpdateScriptSymbols()
{
   Project* projectPtr = mEditControlPtr->GetProject();
   if (!projectPtr)
   {
      return false;
   }
   if (!mScriptLookupPtr)
   {
      mScriptLookupPtr = new WsfScriptLookup(projectPtr);
   }
   else
   {
      mScriptLookupPtr->SetScenario(projectPtr);
   }
   size_t                 cursorOffset = mEditControlPtr->GetOffset(mCursorLine, mCursorColumn);
   UtTextDocumentLocation currentPos(mEditControlPtr->GetSource(), cursorOffset);
   mScriptLookupPtr->SetLocation(currentPos);
   WsfScriptLookup::UpdateResult result = mScriptLookupPtr->Update();
   if (result >= WsfScriptLookup::cSUCCESS)
   {
      if (result == WsfScriptLookup::cSUCCESS)
      {
         mScriptSymbols.clear();
         mShowScriptTypes = false;
         if (!mScriptLookupPtr->AddMethodSymbols(mScriptSymbols))
         {
            mShowScriptTypes = true;
            mScriptLookupPtr->AddSymbolsInScope(mScriptSymbols);
            mScriptLookupPtr->AddSymbolsFromThis(mScriptSymbols);
         }
         mReplaceCharacters = mScriptLookupPtr->GetReplaceCharacters();
         mInsertPosition    = mScriptLookupPtr->GetInsertPosition();
      }
      return !mScriptSymbols.empty();
   }
   return false;
}

bool wizard::AutoCompletePopup::UpdateAutocompleteScript()
{
   const size_t cMAX_RESULTS = 75;
   if (!UpdateScriptSymbols())
   {
      return false;
   }
   mContextType = cWSF_SCRIPT;
   // UtTextDocumentLocation currentPos(mEditControlPtr->GetSource(), cursorOffset);
   // //mEditControlPtr->GetSelectionRange().GetPosition();

   // mInsertPosition = currentPos.mPosition;
   // mReplaceCharacters = 0;
   mSuggestionListPtr->clear();
   QListWidgetItem items;
   using ScoreName = std::pair<int, std::string>;
   std::vector<ScoreName> matchScores;
   std::string            typedChars    = mScriptLookupPtr->GetIdentifierLeftOfCursor();
   int                    minShownScore = 750 * (int)typedChars.size(); // make sure the match is 75% correct
   for (auto&& it : mScriptSymbols)
   {
      int score = GetSymbolMatchScore(typedChars, it.first);
      if (score >= minShownScore)
      {
         matchScores.emplace_back(-score, it.first);
      }
   }
   WsfScriptData* scriptDataPtr = mScriptLookupPtr->GetScriptData();
   if (mShowScriptTypes && scriptDataPtr)
   {
      UsTypeList* typesPtr = scriptDataPtr->mRoot.mConstCtx->mTypes;
      for (auto&& typePtr : typesPtr->mTypes)
      {
         if (typePtr && typePtr->mTemplateArgs.first == nullptr)
         {
            int score = GetSymbolMatchScore(typedChars, typePtr->mName);
            if (score >= minShownScore)
            {
               matchScores.emplace_back(-score, typePtr->mName);
            }
         }
      }
   }
   // Filter list if the user has entered some criteria.
   // If the input string is empty, don't filter at all
   int maxResults = cMAX_RESULTS;
   if (minShownScore > 0)
   {
      std::sort(matchScores.begin(), matchScores.end(), std::less<ScoreName>());
   }
   else
   {
      maxResults = 10000;
   }

   for (size_t i = 0; i < matchScores.size() && (int)i < maxResults; ++i)
   {
      std::string      sym     = matchScores[i].second;
      QListWidgetItem* itemPtr = new QListWidgetItem(QString::fromStdString(sym));
      mSuggestionListPtr->addItem(itemPtr);
   }
   mSuggestionListPtr->SetCurrentRow(0, 1);

   mSuggestionIsLastToken = true;

   show();
   UpdateSize();
   UpdateWindowPosition();
   return false;
}

void wizard::AutoCompletePopup::CurrentSuggestionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
   bool hasDetails = false;
   if (mContextType == cWSF_SCRIPT)
   {
      if (!current)
      {
         mDetailPane->hide();
      }
      else
      {
         UpdateScriptSymbols();
         bool        hasPreview = false;
         bool        hasDoc     = false;
         std::string symbolText = current->text().toStdString();
         auto        i          = mScriptSymbols.find(symbolText);
         if (i != mScriptSymbols.end())
         {
            UsValRef val = i->second;
            hasDetails   = true;
            mDetailHeader->setText(QString::fromStdString(mScriptLookupPtr->GetScriptValDescription(val)));
            mDetailFooter->clear();
            mDetailFooter->hide();
            WsfScriptData* scriptDataPtr = mScriptLookupPtr->GetScriptData();
            if (val.mFunctionNamePtr == nullptr)
            {
               UtTextDocumentRange range = mScriptLookupPtr->GetDefinitionLocation(val.mSymbolPtr);
               if (range.Valid())
               {
                  size_t line = range.mSource->PositionToLineNumber(range.GetBegin());
                  mDetailFooter->setText("File: " + EditAssist::BuildFileURL(range.GetPosition()));
                  mDetailFooter->show();
                  size_t              previewLineStart = line > 4 ? line - 4 : 0;
                  size_t              previewLineEnd   = std::min(range.mSource->GetLineCount() - 1, line + 50);
                  UtTextDocumentRange previewRange(range.mSource,
                                                   range.mSource->GetLinePosition(previewLineStart),
                                                   range.mSource->GetLinePosition(previewLineEnd));
                  GetPreview()->show();
                  GetPreview()->SetText(mEditControlPtr->GetProject(), previewRange);
                  GetPreview()->HighlightRange(range);
                  GetPreview()->HighlightLine(line);
                  hasPreview = true;
               }
               ClearDocumentationView();
            }
            else
            {
               std::string pageName;
               std::string sectionName;
               // std::string fnType = val.mFunctionTypePtr->mName;
               std::string           fnName = val.mFunctionNamePtr;
               GrammarDocumentation* grammarDocData =
                  mEditControlPtr->GetProject()->GetExe()->GetGrammarDocumentation().get();
               if (grammarDocData && scriptDataPtr->FindScriptMethodDocumentation(*grammarDocData,
                                                                                  val.mFunctionTypePtr,
                                                                                  val.mFunctionNamePtr,
                                                                                  pageName,
                                                                                  sectionName))
               {
                  BrowserWidget* docView = GetDocumentationView();
                  QUrl url(QString::fromStdString("ide://documentation/" + pageName + "?doc_section=" + sectionName +
                                                  "." + fnName));
                  // qDebug() << url.toString();
                  docView->SetAddress(url);
                  hasDoc = true;
                  docView->show();
               }
               else
               {
                  ClearDocumentationView();
               }
            }
         }
         else
         {
            UsTypeList* typesPtr = mScriptLookupPtr->GetScriptData()->mRoot.mConstCtx->mTypes;
            UsType*     typePtr  = typesPtr->FindType(symbolText);
            if (typePtr)
            {
               hasDetails = true;
               std::stringstream ss;
               ss << "type <b>" << typePtr->mName << "</b>";
               for (int i = (int)typePtr->mInheritedTypes.size() - 1; i >= 0; --i)
               {
                  ss << "<BR/><i>inherits</i> " << typesPtr->GetTypeName(typePtr->mInheritedTypes[i]);
               }
               mDetailHeader->setText(QString::fromStdString(ss.str()));
               QString pageName = QString::fromStdString(typePtr->mName).toLower() + ".html";
               mDetailFooter->setText(
                  EditAssist::BuildHRef("ide://documentation/docs/script/" + pageName, "[Documentation]"));
               // qDebug() << "ide://documentation/docs/script/" + pageName;
               mDetailFooter->show();
            }
            ClearDocumentationView();
         }
         if (!hasPreview && mDetailPreview)
         {
            mDetailPreview->hide();
         }
         if (!hasDoc && mDetailDocumentation)
         {
            mDetailDocumentation->hide();
         }
      }
   }
   mDetailPane->setVisible(hasDetails);
   UpdateSize();
   UpdateWindowPosition();
}

// std::string wizard::AutoCompletePopup::GetFunctionDescription(const std::string& aFunctionName,
//                                                               const UsFunction&  aFunc,
//                                                               const std::string  aClassName)
//{
//    WsfScriptData* scriptDataPtr = mScriptLookupPtr->GetScriptData();
//    UsConstCtx* constCtx = scriptDataPtr->mTypesPtr->GetConstCtx();
//
//    std::stringstream ss;
//    ss << constCtx->mTypes->GetTypeName(aFunc.mReturnType) << ' ';
//    if (! aClassName.empty())
//    {
//       ss << aClassName << '.';
//    }
//    ss << aFunctionName
//       << '(';
//    for (size_t i = 0; i < (size_t)aFunc.mArgs.size(); ++i)
//    {
//       if (i != 0)
//       {
//          ss << ", ";
//       }
//       ss << constCtx->mTypes->GetTypeName(aFunc.mArgs[i]);
//    }
//    ss << ')';
//    return ss.str();
// }
//
//  std::string wizard::AutoCompletePopup::GetScriptValDescription(UsValRef aVal)
//  {
//     WsfScriptData* scriptDataPtr = mScriptLookupPtr->GetScriptData();
//     UsConstCtx* constCtx = scriptDataPtr->mTypesPtr->GetConstCtx();
//     if (aVal.mFunctionNamePtr == 0)
//     {
//        if (aVal.mSymbolPtr->mRTypeId >= UsTypeList::cPROTOTYPE_ID_START)
//        {
//           const UsFunction* fnPtr = constCtx->mTypes->GetPrototype(aVal.mSymbolPtr->mRTypeId);
//           return GetFunctionDescription(aVal.mSymbolPtr->mName, *fnPtr);
//        }
//        UsType* typePtr = constCtx->mTypes->GetType(aVal.mSymbolPtr->mRTypeId);
//        if (typePtr)
//        {
//           return Qt::escape(QString::fromStdString(typePtr->mName)).toStdString() + " <i>" + aVal.mSymbolPtr->mName +
//           "</i>";
//        }
//     }
//     else
//     {  // this is a built-in method
//        UsType* typePtr = aVal.mFunctionTypePtr;
//        if (typePtr)
//        {
//           UsType::MethodMap::iterator iter = typePtr->mMethods.lower_bound(aVal.mFunctionNamePtr);
//           std::string methodDefinitions;
//           for (; iter != typePtr->mMethods.end(); ++iter)
//           {
//              if (iter->first == aVal.mFunctionNamePtr)
//              {
//                 const UsFunction& func = iter->second;
//                 if (!methodDefinitions.empty()) methodDefinitions += "<BR/>";
//
//                 methodDefinitions += Qt::escape(QString::fromStdString(GetFunctionDescription(aVal.mFunctionNamePtr,
//                 func, typePtr->mName))).toStdString();
//              }
//           }
//           return methodDefinitions;
//        }
//     }
//     return "";
//  }

wizard::WsfTextPreview* wizard::AutoCompletePopup::GetPreview()
{
   if (mDetailPreview)
   {
      return mDetailPreview;
   }
   mDetailPreview = new WsfTextPreview(mDetailPane);
   ((QBoxLayout*)mDetailPane->layout())->insertWidget(1, mDetailPreview);
   mDetailPreview->setMinimumWidth(400);
   mDetailPreview->setMinimumHeight(300);
   ((QVBoxLayout*)mDetailPane->layout())->setStretchFactor(mDetailPreview, 2);
   return mDetailPreview;
}

wizard::BrowserWidget* wizard::AutoCompletePopup::GetDocumentationView()
{
   if (mDetailDocumentation)
   {
      return mDetailDocumentation;
   }
   mDetailDocumentation = new SmallWebView(mDetailPane);
   ((QBoxLayout*)mDetailPane->layout())->insertWidget(1, mDetailDocumentation);
   mDetailDocumentation->setMinimumWidth(400);
   mDetailDocumentation->setMinimumHeight(300);
   mDetailDocumentation->show();
   ((QVBoxLayout*)mDetailPane->layout())->setStretchFactor(mDetailDocumentation, 2);

   //   mDetailDocumentation->GetWebView()->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
   QObject::connect(mDetailDocumentation->GetWebView(), &QTextBrowser::anchorClicked, this, &AutoCompletePopup::DocLinkClicked);
   return mDetailDocumentation;
}

void wizard::AutoCompletePopup::ClearDocumentationView()
{
   if (mDetailDocumentation)
   {
      mDetailDocumentation->SetAddress(QUrl());
   }
}

void wizard::AutoCompletePopup::DocLinkClicked(const QUrl& aUrl)
{
   mEditControlPtr->HideAutocomplete();
}

void wizard::AutoCompletePopup::LinkActivated(const QString& link)
{
   EditAssist::ProcessUrl(link);
   mEditControlPtr->HideAutocomplete();
}

void wizard::AutoCompletePopup::SelectedItem(QListWidgetItem* aItemPtr) {}

// Score autocomplete results based on how well they match the input
bool wizard::AutoCompletePopup::CharCompare(char a, char b) const
{
   return tolower(a) == tolower(b);
}

// Score results.
int wizard::AutoCompletePopup::GetSymbolMatchScore(const std::string& aInput, const std::string& aSymbol) const
{
   // give each character matched a score of 1000
   const int cCHAR_MATCH_SCORE = 1000;
   // Subtract from each match score the number of characters skipped
   int skipDegrade = 0;
   int score       = 0;
   int inputIndex  = 0;

   // If a character is given which does not match any character in the symbol,
   // we still want to calculate a score for the rest of the input.  This code
   // will skip up to 3 chars
   for (int iteration = 0; iteration < 3; ++iteration)
   {
      for (size_t i = 0; i < aSymbol.size() && inputIndex < (int)aInput.size(); ++i)
      {
         if (CharCompare(aInput[inputIndex], aSymbol[i]))
         {
            ++inputIndex;
            if (aInput[inputIndex] == aSymbol[i])
            {
               score += 1;
            } // +1 for same case
            score += cCHAR_MATCH_SCORE - skipDegrade;
         }
         else
         {
            ++skipDegrade;
         }
      }
      ++inputIndex; // skip one input char and keep searching
      skipDegrade += 100;
   }
   return score;
}
