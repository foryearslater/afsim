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

#include "SyntaxTips.hpp"

#include <iostream>

#include <QVBoxLayout>

#include "AutoComplete.hpp"
#include "EditorPrefObject.hpp"
#include "InputContext.hpp"
#include "Project.hpp"
#include "TextSource.hpp"
#include "UtCast.hpp"
#include "UtMath.hpp"
#include "WkfEnvironment.hpp"
#include "WsfEditor.hpp"
#include "WsfParseUtil.hpp"
#include "WsfScriptLookup.hpp"


// Popup window note:
//  There is no great portable way to make a popup window that doesn't steal focus from the main window.
//  Instead, this widget is a child of the text editor
wizard::SyntaxTips::SyntaxTips(WsfEditor* aEditControlPtr)
   // Note: avoid Qt::Tool, it steals focus on linux
   : QWidget(aEditControlPtr,
             /*Qt::Window | Qt::CustomizeWindowHintQt::Tool | */ Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
   , mEditControlPtr(aEditControlPtr)
   , mEditShow(0)
   , mHoverShow(false)
   , mScriptLookupPtr()
   , mNeedScriptUpdateOnCursorMove(false)
{
   setAttribute(Qt::WA_ShowWithoutActivating);

   QPalette pal = palette();
   pal.setBrush(QPalette::All,
                QPalette::Background,
                wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_syntax_tips"));
   setPalette(pal);
   setAutoFillBackground(true);

   QVBoxLayout* layoutPtr = new QVBoxLayout();
   layoutPtr->setContentsMargins(0, 0, 0, 0);
   setLayout(layoutPtr);
   mLabel = new QLabel("", this);
   layout()->addWidget(mLabel);
   UpdateScenario();
   mHoverMaintainTimer.setInterval(50);
   QObject::connect(&mHoverMaintainTimer, &QTimer::timeout, this, &SyntaxTips::HoverCheck);
   QObject::connect(&mCursorMoveUpdateTimer, &QTimer::timeout, this, &SyntaxTips::UpdateScriptTips);
}

wizard::SyntaxTips::~SyntaxTips()
{
   delete mScriptLookupPtr;
   ClearLabels();
}

void wizard::SyntaxTips::ParseUpdate(bool aParseUpdate)
{
   if (mHoverShow)
   {
      if (UpdateSuggestions(mHoverCharacterOffset))
      {
         mHoverMaintainTimer.start();
      }
      else
      {
         EndHoverShow();
      }
   }
   else if (mEditShow)
   {
      size_t offset, line, column;
      mEditControlPtr->GetCurrentPosition(offset, line, column);
      if (!UpdateSuggestions(offset))
      {
         mEditShow = 0;
      }
      else
      {
         mEditShow = 2;
      }
   }
}

void wizard::SyntaxTips::UpdateWindow(size_t aCursorOffset)
{
   QPoint pt = mEditControlPtr->GetPoint(aCursorOffset);

   if (mEditShow)
   {
      // If the point is above the current cursor position, move it down
      size_t curOffset, curLine, curColumn;
      mEditControlPtr->GetCurrentPosition(curOffset, curLine, curColumn);
      QPoint curPt = mEditControlPtr->GetPoint(curOffset);
      pt.ry()      = std::max(pt.y(), curPt.y());
   }


   // pt = mEditControlPtr->mapToGlobal(pt);
   int lineHeight = mEditControlPtr->GetLineHeight(0);
   // Magic value: move tip box down 3 pixels to prevent it from covering up underscores
   pt.ry() += lineHeight + 3;
   // Move all the way to the right
   pt.rx() = mEditControlPtr->rect().right() - width() - 30;
   move(pt);
}

void wizard::SyntaxTips::ClearLabels()
{
   mLabel->clear();
}

namespace
{
QString GetScriptTypeName(int aTypeId, wizard::WsfScriptLookup& aLookup)
{
   UsType* typePtr = aLookup.GetType(aTypeId);
   if (typePtr)
   {
      QString name = QString::fromStdString(typePtr->mName);
      name.replace("<", "&lt;");
      name.replace(">", "&gt;");
      return name;
   }
   else
   {
      return "Object";
   }
}
} // namespace

bool wizard::SyntaxTips::UpdateSuggestions(size_t aCharacterOffset)
{
   // WIZARD_TODO: Need work here:
   TextSource* sourcePtr  = mEditControlPtr->GetSource();
   Project*    projectPtr = mEditControlPtr->GetProject();
   if (!projectPtr || !projectPtr->ParseIsUpToDate())
   {
      return false;
   }
   ClearLabels();
   ParseResults* parseResults = mEditControlPtr->GetProject()->GetParseResults();
   if (!parseResults)
      return false;

   InputContext::Context con;
   UtTextDocument&       doc  = *sourcePtr->GetSource();
   size_t                line = doc.PositionToLineNumber(aCharacterOffset);
   if (static_cast<int>(doc.GetLineLength(line)) <= 0)
      return false;
   size_t inLinePos = doc.GetLinePosition(line);
   size_t column    = aCharacterOffset - inLinePos;

   QString text;
   size_t  editorCommandPos = 0;
   bool    isScript(false);

   {
      if (!mScriptLookupPtr)
      {
         mScriptLookupPtr = new WsfScriptLookup(projectPtr);
      }
      else
      {
         mScriptLookupPtr->SetScenario(projectPtr);
      }
      UtTextDocumentLocation currentPos(mEditControlPtr->GetSource(), aCharacterOffset);
      mScriptLookupPtr->SetLocation(currentPos, WsfScriptLookup::cSTOP_AT_END_OF_STATEMENT);
      WsfScriptLookup::UpdateResult result = mScriptLookupPtr->Update();
      if (result != WsfScriptLookup::cNO_SCRIPT)
      {
         isScript = true;
         if (result >= WsfScriptLookup::cSUCCESS)
         {
            int                     argNum(0);
            size_t                  parenPos, closeParenPos;
            std::vector<UsFunction> funcOptions;
            std::string             fnName;
            if (mScriptLookupPtr->GetCurrentCallInfo(argNum, parenPos, closeParenPos, fnName, funcOptions))
            {
               for (size_t i = 0; i < funcOptions.size(); ++i)
               {
                  QString           tip;
                  const UsFunction& fn = funcOptions[i];
                  tip += GetScriptTypeName(fn.mReturnType, *mScriptLookupPtr);
                  tip += " ";
                  tip += QString::fromStdString(fnName);
                  tip += "(";
                  for (int j = 0; j < fn.mArgs.size(); ++j)
                  {
                     if (j != 0)
                        tip += ", ";
                     bool useBold    = (int)j == argNum;
                     bool lastVarArg = false;
                     if ((fn.mFlags & UsFunction::cVARARGS) && (j + 1 == fn.mArgs.size()) && argNum >= j)
                     {
                        useBold    = true;
                        lastVarArg = true;
                     }
                     if (useBold)
                     {
                        tip += "<b>";
                     }
                     tip += GetScriptTypeName(fn.mArgs[j], *mScriptLookupPtr);
                     if (useBold)
                     {
                        if (lastVarArg)
                        {
                           tip += "...";
                        }
                        tip += "</b>";
                     }
                  }
                  tip += ")";
                  if (i != 0)
                  {
                     text += "<br/>";
                  }
                  text += tip;
               }
               editorCommandPos = parenPos;
               mCommandExtent.SetBegin(parenPos);
               if (closeParenPos != ut::npos)
               {
                  mCommandExtent.SetEnd(closeParenPos + 1);
               }
               else
               {
                  mCommandExtent.SetEnd(mEditControlPtr->GetPosition(line + 1, 0));
               }
               mNeedScriptUpdateOnCursorMove = true;
            }
         }
      }
   }


   if (!isScript)
   {
      InputContext::ContextFromCursor(mEditControlPtr, line, column, con);

      if (con.mQueryTextRange.Valid() && con.mQueryTextRange.Length() > 1)
      {
         // Extend the command range if it does not contain a space
         {
            size_t p;
            for (p = con.mQueryTextRange.GetBegin(); p < doc.GetText().Size(); ++p)
            {
               if (WsfParseUtil::IsWhitespace(doc.GetText()[p]))
               {
                  break;
               }
            }
            if (p > con.mQueryTextRange.GetEnd())
            {
               con.mQueryTextRange.SetEnd(p);
            }
         }
         mCommandExtent = con.mQueryTextRange;
         AutoComplete autoCompleter;
         std::string  queryText    = con.mQueryTextRange.Text();
         size_t       linePosition = sourcePtr->GetSource()->GetLinePosition(line);
         size_t       pos          = column + linePosition;

         size_t firstWordPos = 1000;
         size_t nextWordPos  = 0;
         size_t insertPos    = pos - con.mQueryTextRange.GetBegin();
         // walk through command text to find word boundaries
         // artificially move the insert position to get desired results from GetSyntax()
         for (size_t i = 0; i <= queryText.size(); ++i)
         {
            if (i == queryText.size() || WsfParseUtil::IsWhitespace(queryText[i]))
            {
               if (i < firstWordPos)
               {
                  firstWordPos = i;
               }
               if (insertPos <= i && nextWordPos == 0)
               {
                  nextWordPos = i;
               }
               // break;
            }
         }
         AutoComplete::SyntaxResults results;
         size_t                      newInsertPos = std::max(firstWordPos, nextWordPos);
         autoCompleter.GetSyntax(*parseResults, *sourcePtr, con.mBlockNodePtr, queryText, newInsertPos, results);
         bool wordIndexInRange = results.mWordIndex <= results.mConcreteWordCount || mHoverShow;
         if (wordIndexInRange && !results.mSequences.empty())
         {
            for (size_t i = 0; i < results.mSequences.size(); ++i)
            {
               if (i != 0)
               {
                  text += "<BR/>";
               }
               AutoComplete::Syntax& syntax = results.mSequences[i];
               for (size_t j = 0; j < syntax.size(); ++j)
               {
                  QString word = QString::fromStdString(syntax[j]);
                  if (word == "<ignore>")
                  {
                     break;
                  }
                  word.replace("<", "&lt;");
                  word.replace(">", "&gt;");
                  if (j == results.mWordIndex)
                  {
                     word = "<b>" + word + "</b>";
                  }
                  text += word;
                  if (j + 1 < syntax.size())
                  {
                     text += " ";
                  }
               }
            }
         }
         size_t commandStartColumn = std::max(size_t{0}, con.mQueryTextRange.GetBegin() - linePosition);
         editorCommandPos          = mEditControlPtr->GetPosition(line, commandStartColumn);
      }
   }

   if (!text.isEmpty())
   {
      mLabel->setText(text);
      layout()->invalidate();
      // Compute the position of the box using the column at which the command begins
      {
         UpdateWindow(editorCommandPos);
      }
      show();
      return true;
   }
   else
   {
      hide();
      return false;
   }
}

void wizard::SyntaxTips::paintEvent(QPaintEvent* e)
{
   QWidget::paintEvent(e);
   if (mPreviousSize != mLabel->sizeHint())
   {
      mPreviousSize = mLabel->sizeHint();
      resize(mPreviousSize);
   }
   CheckSizeChange();
}

void wizard::SyntaxTips::HoverShow(size_t aCharOffset)
{
   UpdateScenario();
   mHoverCharacterOffset = aCharOffset;
   if (!mEditShow)
   {
      mHoverShow = true;

      if (getProject() && getProject()->ParseIsUpToDate())
      {
         if (UpdateSuggestions(mHoverCharacterOffset))
         {
            mHoverMaintainTimer.start();
         }
         else
         {
            EndHoverShow();
         }
      }
   }
}

void wizard::SyntaxTips::HoverCheck()
{
   // allow cursor over the tip window
   if (rect().contains(mapFromGlobal(QCursor::pos())))
   {
      return;
   }
   // make sure cursor is over the command text range
   size_t pos = mEditControlPtr->GetPosition(mEditControlPtr->mapFromGlobal(QCursor::pos()));
   if (mCommandExtent.Contains(pos))
   {
      return;
   }
   // otherwise, we need to end the hover tip
   EndHoverShow();
}

void wizard::SyntaxTips::EndHoverShow()
{
   mHoverShow = false;
   mHoverMaintainTimer.stop();
   if (!mEditShow)
   {
      hide();
   }
}

void wizard::SyntaxTips::UserModification()
{
   UpdateScenario();
   EndHoverShow();
   if (!getProject())
      return;
   mEditShow = 1;
   // On normal user edits, this is always false, but UserModification() is called
   // to trigger the tips in other situations.
   if (getProject()->ParseIsUpToDate())
   {
      ParseUpdate(true);
   }
}

void wizard::SyntaxTips::CursorMove()
{
   if (mEditShow == 2 && isVisible())
   {
      size_t pos, line, column;
      mEditControlPtr->GetCurrentPosition(pos, line, column);
      UtTextRange range{mCommandExtent.GetBegin() - 3, mCommandExtent.GetEnd() + 3};
      if (!range.Contains(pos))
      {
         mEditShow = 0;
         hide();
      }
      else if (mNeedScriptUpdateOnCursorMove)
      {
         mCursorMoveUpdateTimer.start(500);
      }
   }
}

void wizard::SyntaxTips::Hide()
{
   mEditShow                     = 0;
   mNeedScriptUpdateOnCursorMove = false;
   EndHoverShow();
   hide();
}

void wizard::SyntaxTips::CheckSizeChange()
{
   QRect geom = QRect();
   if (isVisible())
   {
      geom = frameGeometry();
   }
   if (mPreviousRect != geom)
   {
      mPreviousRect = geom;
      mEditControlPtr->TipsMoved(mPreviousRect);
   }
}

QRect wizard::SyntaxTips::GetGeometry()
{
   if (!isVisible())
   {
      return QRect();
   }
   QRect r = mPreviousRect;
   r.moveTopLeft(parentWidget()->mapToGlobal(r.topLeft()));
   return r;
}

//! Updates the ParseUpdated callback if the scenario has changed.
void wizard::SyntaxTips::UpdateScenario()
{
   if (mEditControlPtr->GetProject())
   {
      mParseUpdateCallbackPtr = mEditControlPtr->GetProject()->ParseUpdated.Connect(&SyntaxTips::ParseUpdate, this);
   }
}

// Called on a delay after cursor is moved when we need to update script information.
void wizard::SyntaxTips::UpdateScriptTips()
{
   mCursorMoveUpdateTimer.stop();
   if (!mNeedScriptUpdateOnCursorMove)
      return;
   mNeedScriptUpdateOnCursorMove = false;
   ParseUpdate(true);
}

void wizard::SyntaxTips::resizeEvent(QResizeEvent* e)
{
   // move to the right side of the screen
   QPoint p         = pos();
   QPoint editRight = mEditControlPtr->rect().bottomRight();

   p.rx() = editRight.x() - e->size().width() - 30; // shift left for scroll bar
   if (p != pos())
   {
      move(p);
   }
}

wizard::Project* wizard::SyntaxTips::getProject()
{
   return mEditControlPtr->GetProject();
}
