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

#include "WsfTextPreview.hpp"

#include <QTextBlock>

#include "EditorManager.hpp"
#include "Project.hpp"
#include "WsfExe.hpp"
#include "WsfExeManager.hpp"
#include "WsfStyler.hpp"

wizard::WsfTextPreview::WsfTextPreview(QWidget* aParent)
   : QPlainTextEdit(aParent)
{
   setWordWrapMode(QTextOption::NoWrap);
   setLineWrapMode(QPlainTextEdit::NoWrap);
   mHighlightLine            = ut::npos;
   mParseCompleteCallbackPtr = nullptr;
   mProjectPtr               = nullptr;
   mStyleComplete            = false;
}

wizard::WsfTextPreview::~WsfTextPreview()
{
   if (mParseCompleteCallbackPtr)
   {
      delete mParseCompleteCallbackPtr;
   }
}

void wizard::WsfTextPreview::SetScenario(Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr)
   {
      return;
   }
   mProjectPtr    = aProjectPtr;
   mStyleComplete = false;
   WsfStyler::ApplyEditorStyleSettings(this, 10);
}

void wizard::WsfTextPreview::SetText(Project* aProjectPtr, UtTextDocumentRange aRange)
{
   SetScenario(aProjectPtr);
   bool rangeChanged = !(aRange.GetPosition() == mRange.GetPosition());
   mRange            = aRange;
   if (rangeChanged)
   {
      document()->clear();
   }
   mStyleComplete = false;
   if (!mProjectPtr)
   {
      return;
   }
   if (mProjectPtr->GetParseResults())
   {
      SetTextResolve();
   }
   else
   {
      if (mParseCompleteCallbackPtr)
      {
         delete mParseCompleteCallbackPtr;
         mParseCompleteCallbackPtr = nullptr;
      }
      mParseCompleteCallbackPtr = new UtCallbackN<void()>(std::bind(&WsfTextPreview::SetTextResolve, this));
      mProjectPtr->ExecuteWhenParseComplete(mParseCompleteCallbackPtr);
   }
}

void wizard::WsfTextPreview::SetTextResolve()
{
   if (!mProjectPtr)
   {
      return;
   }
   std::string text;
   mRange.Text(text);
   document()->setPlainText(text.c_str());
   QTextCursor cur = textCursor();
   cur.movePosition(QTextCursor::Start);
   setTextCursor(cur);
   UpdateStyle();
}

void wizard::WsfTextPreview::UpdateStyle()
{
   if (mProjectPtr && mProjectPtr->GetParseResults())
   {
      WsfStyler* stylerPtr = wizExeMgr.GetStyler();
      if (stylerPtr)
      {
         mFormatChanges.resize(mRange.Length());

         WsfStyler::StyleRequest request;
         request.mShowNotifications = false;
         request.mShowScriptErrors  = false;
         request.mStyleIfHidden     = true; // make sure it styles the text, even if this is hidden for now.
         request.mPositionOffset    = mRange.GetBegin();
         request.mControl           = this;
         request.mStylerInterface   = this;
         request.mFirstPosition     = 0; // mRange.mBegin;
         request.mLastPosition      = mRange.GetEnd();
         request.mResultsPtr        = mProjectPtr->GetParseResults();
         request.mSourcePtr         = static_cast<TextSource*>(mRange.mSource);
         stylerPtr->UpdateStyle(request);
         EditorManager::UpdateStyleFromArray(mFormatChanges,
                                             document()->firstBlock(),
                                             document()->lastBlock().blockNumber(),
                                             0,
                                             mRange.Length());
         ApplyHighlight();
         mStyleComplete = true;
      }
      return;
   }
}

void wizard::WsfTextPreview::HighlightRange(UtTextRange& aRange)
{
   mHighlightRange = aRange;
   if (mStyleComplete)
   {
      ApplyHighlight();
   }
}

void wizard::WsfTextPreview::ApplyHighlight()
{
   if (mHighlightRange.Length() > 0)
   {
      QList<QTextEdit::ExtraSelection> extra;
      QTextEdit::ExtraSelection        sel;
      sel.cursor = QTextCursor(document());
      sel.cursor.setPosition(ut::cast_to_int(mHighlightRange.GetBegin()) - ut::cast_to_int(mRange.GetBegin()));
      sel.cursor.setPosition(sel.cursor.position() + ut::cast_to_int(mHighlightRange.Length()), QTextCursor::KeepAnchor);
      sel.format.setBackground(QColor(0xff, 0xf7, 0));
      extra.push_back(sel);
      setExtraSelections(extra);
   }
}

void wizard::WsfTextPreview::HighlightLine(size_t aLine)
{
   mHighlightLine = aLine;
   if (mStyleComplete)
   {
      ApplyHighlight();
   }
}

QSize wizard::WsfTextPreview::sizeHint() const
{
   return QSize(400, 300);
}

void wizard::WsfTextPreview::SetTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt)
{
   size_t end = std::min(aStart + aCount, mRange.Length());
   for (size_t i = std::max(size_t{0}, aStart); i < end; ++i)
   {
      mFormatChanges[i] = aFmt;
   }
}

void wizard::WsfTextPreview::MergeTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt)
{
   size_t end = std::min(aStart + aCount, mRange.Length());
   for (size_t i = std::max(size_t{0}, aStart); i < end; ++i)
   {
      mFormatChanges[i].merge(aFmt);
   }
}
