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

#include "Editor.hpp"

#include <assert.h>

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextBlock>

#include "ActionManager.hpp"
#include "EditorDock.hpp"
#include "EditorManager.hpp"
#include "EditorPrefObject.hpp"
#include "FindTextRequest.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ReplaceTextRequest.hpp"
#include "RevisionStore.hpp"
#include "TextSource.hpp"
#include "WkfEnvironment.hpp"
#include "WsfStyler.hpp"

wizard::Editor::Editor(QWidget* aParent, Project* aProject, TextSource* aTextSource, EditorManager* aManagerPtr)
   : QPlainTextEdit(aParent)
   , mProjectPtr(aProject)
   , mSourcePtr(aTextSource)
   , mManagerPtr(aManagerPtr)
   , mFoldingEnabled(true)
   , mUndoEnabled(false)
   , mRedoEnabled(false)
   , mGlobalReadOnly(false)
   , mSendModified(false)
   , mInitalized(false)
   , mNonUserAction(0)
   , mMarginWidget(nullptr)
   , mDeferredUpdateSelectionsTimer()
{
   mMarginWidget = new EditorMargin(this);
   mSendModified = false;

   connect(this, &QPlainTextEdit::updateRequest, this, &Editor::OnUpdateRequest);
   connect(this, &QPlainTextEdit::blockCountChanged, this, &Editor::UpdateMarginViewport);
   connect(this, &QPlainTextEdit::undoAvailable, this, &Editor::OnUndoAvailable);
   connect(this, &QPlainTextEdit::redoAvailable, this, &Editor::OnRedoAvailable);
   connect(this, &QPlainTextEdit::cursorPositionChanged, this, &Editor::OnCursorPositionChanged);
   connect(this, &QPlainTextEdit::modificationChanged, this, &Editor::OnModificationChanged);
   connect(&mDeferredUpdateSelectionsTimer, &QTimer::timeout, this, &Editor::UpdateExtraSelections);
   connect(this, &Editor::textChanged, this, &Editor::TextModified);

   mCallbacks += wizSignals->DebugSessionActive.Connect(&Editor::DebugRunStateChange, this);

   // document()->setPlainText(aSourcePtr->GetText().GetPointer());
   UpdateMarginViewport();
   connect(document(), &QTextDocument::contentsChange, this, &Editor::DocContentsChange);
   connect(document(), &QTextDocument::undoCommandAdded, this, &Editor::OnTextChanged);
   this->setLineWrapMode(QPlainTextEdit::NoWrap);
   this->setWordWrapMode(QTextOption::NoWrap);
   setWindowTitle(aTextSource->GetSystemPath().c_str());
   aManagerPtr->RegisterEditor(this);
   resize(500, 500);

   // Selected line color
   // QPalette pal = palette();
   // pal.setColor(QPalette::Highlight, QColor(58, 98, 139));
   // setPalette(pal);

   mDeferredUpdateSelectionsTimer.setSingleShot(true);
   mDeferredUpdateSelectionsTimer.setInterval(5);

   wizSignals->EditorOpened();
}

wizard::Editor::~Editor()
{
   mManagerPtr->DeregisterEditor(this);
   mManagerPtr->EditorClosed(this);
   size_t              line, col;
   UtTextDocumentRange range;
   GetSelectionRange(range);
   GetLineColumn(range.GetBegin(), line, col);
   wkfEnv.GetPreferenceObject<EditorPrefObject>()->RememberFilePosition(mSourcePtr->GetFilePath().GetSystemPath().c_str(),
                                                                        line,
                                                                        col);
   wizSignals->EditorClosed(this);
   mSourcePtr->EditorClosed(this);
}

void wizard::Editor::AddCurrentLineSelection(QList<QTextEdit::ExtraSelection>& aSelections)
{
   QTextEdit::ExtraSelection selection;
   selection.format.setBackground(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_line_color"));
   selection.format.setProperty(QTextFormat::FullWidthSelection, true);
   selection.cursor = textCursor();
   selection.cursor.clearSelection();
   aSelections.append(selection);
}

void wizard::Editor::GetCursorPoint(int& aX, int& aY, int& aLineHeight)
{
   QPoint pt   = viewport()->mapToParent(cursorRect().topLeft());
   aX          = pt.x();
   aY          = pt.y();
   aLineHeight = GetLineHeight(0);
}

int wizard::Editor::LineNumberWidth()
{
   if (wkfEnv.GetPreferenceObject<EditorPrefObject>()->ShowLineNumbers())
   {
      int digits = 1;
      int max    = qMax(1, blockCount());
      while (max >= 10)
      {
         max /= 10;
         ++digits;
      }
      int space = 3 + fontMetrics().width(QLatin1Char('9')) * qMax(3, digits);
      return space;
   }
   return 0;
}

void wizard::Editor::DebugRunStateChange(wizard::ScriptDebugger* aDebuggerPtr, bool aIsRunning)
{
   mSendModified = aIsRunning;
}

void wizard::Editor::TextModified()
{
   // This slot will be triggered once when the editor opens and is populated with file text
   // Skip this signal on the first pass
   if (mSendModified && mInitalized)
   {
      emit wizSignals->TextModified(mSourcePtr);
   }

   if (!mInitalized)
   {
      mInitalized = true;
   }
}

void wizard::Editor::BuildTitleBarMenu(QMenu& aMenu)
{
   ProjectWorkspace::Instance()->GetActionManager()->BuildUpTitleBarMenu(aMenu, this);
}

bool wizard::Editor::AcceptsClose()
{
   if (GetSource()->IsModified())
   {
      std::stringstream ss;
      ss << "Save changes to the file: '" << GetSource()->GetSystemPath() << "'?";
      int bn =
         QMessageBox::warning(this, "Save Changes", ss.str().c_str(), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
      if (bn == QMessageBox::Yes)
      {
         GetSource()->WaitForSourceModifications();
         GetSource()->SaveWithNotifications();
      }
      else if (bn == QMessageBox::No)
      {
         Project::Instance()->GetRevisionStore().Backup();
         GetSource()->ReadSource();
      }
      else if (bn == QMessageBox::Cancel)
      {
         return false;
      }
   }
   return true;
}

QString wizard::Editor::GetShortName(bool aTrim) const
{
   if (aTrim)
   {
      return QString::fromStdString(mSourcePtr->GetFileName());
   }
   else
   {
      const std::vector<UtPath> startups = Project::Instance()->GetStartupFiles();
      if (startups.empty())
      {
         return QString::fromStdString(mSourcePtr->GetFilePath().GetNormalizedPath());
      }
      else
      {
         UtPath startup = startups[0];
         startup.Up();
         return QString::fromStdString(startup.GetRelativePath(mSourcePtr->GetFilePath()).GetNormalizedPath());
      }
   }
}

void wizard::Editor::mousePressEvent(QMouseEvent* aEvent)
{
   mManagerPtr->SetCurrent(this);
   QPlainTextEdit::mousePressEvent(aEvent);
}

void wizard::Editor::PaintMargin(QPaintEvent* event)
{
   // Set the fill color and the font for the margin
   QPainter painter(mMarginWidget);
   painter.fillRect(event->rect(), wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fill"));
   painter.setFont(font());
   painter.setPen(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_pen"));

   // Adding the line numbers
   int lineNumWidth = LineNumberWidth();
   if (lineNumWidth > 0)
   {
      QTextBlock block       = firstVisibleBlock();
      int        blockNumber = block.blockNumber();
      int        top         = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
      int        bottom      = top + (int)blockBoundingRect(block).height();

      while (block.isValid() && top <= event->rect().bottom())
      {
         if (block.isVisible() && bottom >= event->rect().top())
         {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumWidth, fontMetrics().height(), Qt::AlignRight, number);
         }

         block  = block.next();
         top    = bottom;
         bottom = top + (int)blockBoundingRect(block).height();
         ++blockNumber;
      }
   }
}

void wizard::Editor::resizeEvent(QResizeEvent* e)
{
   QPlainTextEdit::resizeEvent(e);

   UpdateMarginGeometry();
}

void wizard::Editor::showEvent(QShowEvent* e)
{
   if (mSourcePtr != nullptr)
   {
      UpdateStyle();
   }
   QPlainTextEdit::showEvent(e);
}

void wizard::Editor::closeEvent(QCloseEvent* e)
{
   if (AcceptsClose())
   {
      e->accept();
      QPlainTextEdit::closeEvent(e);
      deleteLater();
   }
   else
   {
      e->ignore();
   }
}

void wizard::Editor::keyPressEvent(QKeyEvent* e)
{
   //
   // Home and End keys:
   //  good text editors will use home and end to move to the first or last non-whitespace characters in a line
   //  Qt wasn't doing this, so we override this behavior here
   //
   if (e->key() == Qt::Key_Home && (0 == (e->modifiers() & Qt::ControlModifier)))
   {
      QTextCursor cur(document());
      cur.setPosition(textCursor().position());
      cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
      QString txt        = cur.selectedText();
      int     indentSize = 0;
      for (auto&& c : txt)
      {
         if (c.isSpace())
         {
            ++indentSize;
         }
         else if (indentSize)
         {
            QTextCursor newCursor = textCursor();
            newCursor.setPosition(cur.position() + indentSize,
                                  (e->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
            setTextCursor(newCursor);
            e->accept();
            return;
         }
         else
         {
            break;
         }
      }
   }
   if (e->key() == Qt::Key_End && (0 == (e->modifiers() & Qt::ControlModifier)))
   {
      QTextCursor cur(document());
      cur.setPosition(textCursor().position());
      cur.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
      QString txt            = cur.selectedText();
      int     lastChar       = -1;
      bool    trailingSpaces = false;
      for (int i = 0; i < txt.size(); ++i)
      {
         if (!txt[i].isSpace())
         {
            lastChar       = i;
            trailingSpaces = false;
         }
         else
         {
            trailingSpaces = true;
         }
      }
      if (lastChar != -1 && trailingSpaces)
      {
         QTextCursor newCursor = textCursor();
         newCursor.setPosition(newCursor.position() + lastChar + 1,
                               (e->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
         setTextCursor(newCursor);
         e->accept();
         return;
      }
   }

   // Ignore built-in cut/copy/paste
   // so that we can override that behavior
   //   if (e->modifiers() == Qt::ControlModifier
   //       && (e->key() == Qt::Key_V || e->key() == Qt::Key_C || e->key() == Qt::Key_X))
   //   {
   //      QWidget::keyPressEvent(e);
   //      return;
   //   }
   if (e->modifiers() == Qt::ShiftModifier && (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Insert))
   {
      QWidget::keyPressEvent(e);
      return;
   }

   //    if (e->key() == Qt::Key_Delete && e->modifiers() == Qt::ShiftModifier)
   //    {
   //       QWidget::keyPressEvent(e);
   //       return;
   //    }
   QPlainTextEdit::keyPressEvent(e);
}


bool wizard::Editor::event(QEvent* e)
{
   if (e->type() == QEvent::FocusOut || e->type() == QEvent::FocusIn)
   {
      ProjectWorkspace::Instance()->GetActionManager()->UpdateFocusActions();
   }
   else if (e->type() == QEvent::ShortcutOverride)
   {
      // QPlainTextEdit scarfs up some shortcuts that we try to process
      // This bypasses that behavior  ( see qwidgettextcontrol.cpp )
      QKeyEvent* ke = static_cast<QKeyEvent*>(e);
      if (ke == QKeySequence::Copy || ke == QKeySequence::Paste || ke == QKeySequence::Cut ||
          ke == QKeySequence::Redo || ke == QKeySequence::Undo)
      {
         return QAbstractScrollArea::event(e);
      }
   }
   else if (e->type() == QEvent::ToolTip)
   {
      return true; // eat it until such a time as we want to implement our own tooltips
   }
   return QPlainTextEdit::event(e);
}

void wizard::Editor::Initialize()
{
   LoadSettings();

   connect(wkfEnv.GetPreferenceObject<EditorPrefObject>(), &EditorPrefObject::Changed, this, &Editor::LoadSettings);
}

void wizard::Editor::PrintFile()
{
   QPrinter     printer;
   QPrintDialog dialog(&printer, this);
   dialog.setWindowTitle(tr("Print Document"));

   if (dialog.exec() == QDialog::Accepted)
   {
      document()->print(&printer);
   }
}

void wizard::Editor::DeleteLine()
{
   // Delete current lines toward bottom
   QTextCursor cur        = textCursor();
   int         initialPos = cur.position();
   cur.movePosition(QTextCursor::StartOfLine);
   cur.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
   cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

   bool hasText = cur.hasSelection();
   if (!hasText)
   {
      // Delete current lines toward top
      cur.setPosition(initialPos);
      cur.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
      cur.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
      cur.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

      hasText = cur.hasSelection();
      if (!hasText)
      {
         // Delete single line left
         cur.setPosition(initialPos);
         cur.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
         cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
      }
   }
   cur.beginEditBlock();
   cur.removeSelectedText();
   cur.endEditBlock();
}

void wizard::Editor::Paste()
{
   paste();
}

void wizard::Editor::FindNextAction()
{
   SearchNext(mManagerPtr->GetActiveFindRequest());
}

void wizard::Editor::FindPreviousAction()
{
   SearchPrevious(mManagerPtr->GetActiveFindRequest());
}

void wizard::Editor::GoToLineAction()
{
   size_t            lineCount = mSourcePtr->GetSource()->GetLines().Size();
   int               curLine   = textCursor().block().firstLineNumber();
   bool              ok        = false;
   std::stringstream ss;
   ss << "Line # (1 - " << lineCount << "):";
   int inputLine =
      QInputDialog::getInt(this, "Goto Line", ss.str().c_str(), curLine, 1, ut::cast_to_int(lineCount), 1, &ok);
   if (ok)
   {
      GoToLine(inputLine - 1);
   }
}

void wizard::Editor::LoadSettings()
{
   WsfStyler::ApplyEditorStyleSettings(this);

   mFoldingEnabled = wkfEnv.GetPreferenceObject<EditorPrefObject>()->FoldText();
   setTabStopWidth(fontMetrics().width(' ') * wkfEnv.GetPreferenceObject<EditorPrefObject>()->TabSpace());
   UpdateMarginViewport();
}

bool wizard::Editor::ReplaceFoundText()
{
   if (mManagerPtr)
   {
      std::string buffer = textCursor().selectedText().toStdString();

      bool stringMatch = false;
      if (mManagerPtr->GetActiveReplaceRequest().mCaseSensitive)
      {
         stringMatch = (0 == strcmp(buffer.c_str(), mManagerPtr->GetActiveReplaceRequest().mText.c_str()));
      }
      else
      {
         std::string src = buffer;
         UtStringUtil::ToLower(src);
         std::string dst = mManagerPtr->GetActiveReplaceRequest().mText;
         UtStringUtil::ToLower(dst);
         stringMatch = (0 == strcmp(src.c_str(), dst.c_str()));
      }
      if (stringMatch)
      {
         textCursor().insertText(mManagerPtr->GetActiveReplaceRequest().mReplaceText.c_str());
         SearchNext(mManagerPtr->GetActiveReplaceRequest());
         return true;
      }
      SearchNext(mManagerPtr->GetActiveReplaceRequest());
      return false;
   }
   return false;
}

void wizard::Editor::Replace(const UtTextRange& aRange, const std::string aText)
{
   SetSelection(aRange);
   textCursor().insertText(aText.c_str());
   // Autocomplete uses this function to insert suggestions
   // update the autocomplete popup if it is still around
   UpdateAutoComplete();
}

std::string wizard::Editor::GetSelectedText()
{
   return ToAscii(textCursor().selectedText());
}

bool wizard::Editor::SearchNext(const FindTextRequest& aFindRequest, QTextCursor* aCursorPtr /*= nullptr*/)
{
   QString pattern(aFindRequest.mText.c_str());
   QRegExp exp(pattern, (aFindRequest.mCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::FixedString);
   QTextCursor* nextCur = aCursorPtr;
   QTextCursor  docTextCursor;
   if (!nextCur)
   {
      docTextCursor = textCursor();
      nextCur       = &docTextCursor;
   }

   QTextCursor foundCursor = document()->find(exp, *nextCur);
   if (foundCursor.isNull())
   {
      foundCursor = document()->find(exp);
   }
   if (!foundCursor.isNull())
   {
      int selStart = foundCursor.selectionStart();
      int selEnd   = foundCursor.selectionEnd();
      nextCur->setPosition(selStart);
      nextCur->setPosition(selEnd, QTextCursor::KeepAnchor);
   }
   if (!foundCursor.isNull() && aCursorPtr == nullptr)
   {
      RecordCurrentLocation();
      setTextCursor(*nextCur);
      EnsureCurrentLineVisible();
   }
   return !foundCursor.isNull();
}


// Search for the next search result.  Use aCursorPtr if given, otherwise
// the text document's cursor is used.
// This will not wrap around to the top of the document
// returns 'true' if the text was found and moves cursor to select text
bool wizard::Editor::SearchNextNoWrap(const FindTextRequest& aFindRequest, QTextCursor* aCursorPtr, int aLowerBound, int aUpperBound)
{
   QString pattern(aFindRequest.mText.c_str());
   QRegExp exp(pattern, (aFindRequest.mCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::FixedString);
   QTextCursor* nextCur = aCursorPtr;
   QTextCursor  docTextCursor;
   if (!nextCur)
   {
      docTextCursor = textCursor();
      nextCur       = &docTextCursor;
   }

   QTextCursor foundCursor = document()->find(exp, *nextCur);

   if (!foundCursor.isNull())
   {
      // check if this is out of the defined bounds
      QTextCursor beginLineCursor = foundCursor; // get the beginning of the line to compare with the upperbound
      beginLineCursor.movePosition(QTextCursor::StartOfBlock);
      if (aUpperBound >= 0 && (beginLineCursor.position() > aUpperBound))
      {
         // it has exceeded the bounds defined, return false
         return false;
      }

      int selStart = foundCursor.selectionStart();
      int selEnd   = foundCursor.selectionEnd();
      nextCur->setPosition(selStart);
      nextCur->setPosition(selEnd, QTextCursor::KeepAnchor);
   }
   if (!foundCursor.isNull() && aLowerBound == -1)
   {
      RecordCurrentLocation();
      setTextCursor(*nextCur);
      EnsureCurrentLineVisible();
   }
   return !foundCursor.isNull();
}


bool wizard::Editor::SearchPrevious(const FindTextRequest& aFindRequest)
{
   QString pattern(aFindRequest.mText.c_str());
   QRegExp exp(pattern, (aFindRequest.mCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::FixedString);
   QTextCursor nextCur = document()->find(exp, textCursor(), QTextDocument::FindBackward);
   const bool  found   = !nextCur.isNull();
   if (found)
   {
      RecordCurrentLocation();
      setTextCursor(nextCur);
      EnsureCurrentLineVisible();
   }
   return found;
}

bool wizard::Editor::GetSelectionRange(UtTextDocumentRange& aRange)
{
   aRange.SetBegin(textCursor().selectionStart());
   size_t end = aRange.GetBegin();
   if (textCursor().hasSelection())
   {
      end = textCursor().selectionEnd();
   }
   aRange.SetEnd(end);
   if (mSourcePtr)
   {
      aRange.mSource = mSourcePtr->GetSource();
   }
   return textCursor().hasSelection();
}

void wizard::Editor::SetSelection(const UtTextRange& aRange)
{
   QTextCursor cur(document());
   auto        sourceSize = mSourcePtr->Size();
   if (sourceSize > aRange.GetBegin())
   {
      cur.setPosition(ut::cast_to_int(aRange.GetBegin()));
      if (sourceSize > aRange.GetEnd() + 1)
      {
         cur.setPosition(ut::cast_to_int(aRange.GetEnd() + 1), QTextCursor::KeepAnchor);
      }
   }
   else
   {
      cur.setPosition(0);
   }
   setTextCursor(cur);
   EnsureCurrentLineVisible();
}

void wizard::Editor::GetCurrentPosition(size_t& aOffset, size_t& aLine, size_t& aColumn)
{
   aOffset = ut::cast_to_size_t(textCursor().position());
   aLine   = ut::cast_to_size_t(textCursor().block().firstLineNumber());
   aColumn = ut::cast_to_size_t(textCursor().columnNumber());
}

std::string wizard::Editor::GetLineText(const size_t& aLine)
{
   return ToAscii(document()->findBlockByLineNumber(ut::cast_to_int(aLine)).text());
}

void wizard::Editor::GetLineColumn(const size_t& aOffset, size_t& aLine, size_t& aColumn)
{
   QTextCursor cur(document());
   cur.setPosition(ut::cast_to_int(aOffset));
   aLine   = ut::cast_to_size_t(cur.block().firstLineNumber());
   aColumn = ut::cast_to_size_t(cur.columnNumber());
}

QPoint wizard::Editor::GetPoint(const size_t aOffset)
{
   QTextCursor cur(document());
   cur.setPosition(ut::cast_to_int(aOffset));
   return viewport()->mapToParent(cursorRect(cur).topLeft());
}

size_t wizard::Editor::GetOffset(const size_t aLine, const size_t aColumn)
{
   QTextBlock  block      = document()->findBlockByLineNumber(ut::cast_to_int(aLine));
   size_t      textLength = ut::cast_to_size_t(block.text().length());
   size_t      minColumn  = std::min(textLength, aColumn);
   QTextCursor cur(block);
   return ut::cast_to_size_t(cur.position()) + minColumn;
}

int wizard::Editor::GetLineHeight(const int aLine)
{
   return fontMetrics().lineSpacing();
}

size_t wizard::Editor::GetPosition(size_t aLine, size_t aColumn)
{
   QTextBlock blk = document()->findBlockByLineNumber(ut::cast_to_int(aLine));
   return blk.position() + aColumn;
}

std::string wizard::Editor::RangeText(size_t aStart, size_t aEnd)
{
   QTextCursor cur(document());
   cur.setPosition(ut::cast_to_int(aStart));
   cur.setPosition(ut::cast_to_int(aEnd), QTextCursor::KeepAnchor);
   return ToAscii(cur.selectedText());
}

void wizard::Editor::RequestCloseAction()
{
   if (mManagerPtr)
   {
      mManagerPtr->RequestClose(this);
   }
}

void wizard::Editor::RecordCurrentLocation(bool aForceNewEntry)
{
   if (mManagerPtr)
   {
      mManagerPtr->RecordHistoryLocation(UtTextDocumentLocation(mSourcePtr, textCursor().position()), aForceNewEntry);
   }
}

void wizard::Editor::GoToLine(size_t aLine)
{
   QTextCursor cur = QTextCursor(QTextBlock(document()->findBlockByLineNumber(ut::cast_to_int(aLine))));
   setTextCursor(cur);
   EnsureCurrentLineVisible();
   RecordCurrentLocation(true);
   if (parentWidget())
   {
      parentWidget()->raise();
   }
}

void wizard::Editor::GoToRange(const UtTextRange& aRange)
{
   QTextCursor cur(document());
   cur.setPosition(ut::cast_to_int(aRange.GetEnd()));
   setTextCursor(cur);
   EnsureCurrentLineVisible();
   cur.setPosition(ut::cast_to_int(aRange.GetBegin()));
   setTextCursor(cur);
   EnsureCurrentLineVisible();
   if (parentWidget())
   {
      parentWidget()->raise();
   }
}

QString wizard::Editor::ToPlainText()
{
   return toPlainText();
}

std::string wizard::Editor::ToAscii(const QString& aText)
{
   std::string text;
   for (int i = 0; i < aText.size(); ++i)
   {
      QChar c     = aText[i];
      char  ascii = c.toLatin1();
      if (ascii)
      {
         text += ascii;
      }
      else
      {
         switch (c.unicode())
         {
         case 0x2029: // paragraph separator
            text += '\n';
            break;
         default:
            text += ' '; // not recognized
            break;
         }
      }
   }
   return text;
}

void wizard::Editor::UpdateStyleFromArray(const QVector<QTextCharFormat>& aFormatChanges,
                                          const QTextBlock&               firstBlk,
                                          int                             aLastBlockNumber,
                                          int                             startPos,
                                          int                             endPos)
{
   QTextBlock      blk = firstBlk;
   QTextCharFormat emptyFormat;
   while (blk.isValid())
   {
      QTextLayout*                    layout = blk.layout();
      QTextLayout::FormatRange        r;
      QList<QTextLayout::FormatRange> ranges = layout->additionalFormats();
      ranges.clear();
      {
         int blockStart = blk.position() - startPos;
         int i          = blockStart;
         int endI       = std::min(blockStart + blk.length(), aFormatChanges.size());
         r.start        = -1;
         while (i < endI)
         {
            while (i < endI && aFormatChanges.at(i) == emptyFormat)
            {
               ++i;
            }

            if (i >= endI)
            {
               break;
            }

            r.start  = i - blockStart;
            r.format = aFormatChanges.at(i);

            while (i < endI && aFormatChanges.at(i) == r.format)
            {
               ++i;
            }

            r.length = i - r.start - blockStart;

            if (i >= endI)
            {
               --r.length;
            }

            ranges << r;
            r.start = -1;
         }
         layout->setAdditionalFormats(ranges);
      }
      blk = blk.next();
      if (blk.blockNumber() > aLastBlockNumber)
      {
         break;
      }
   }
}

void wizard::Editor::SetGlobalReadOnly(bool aIsGlobalReadOnly)
{
   if (mGlobalReadOnly != aIsGlobalReadOnly)
   {
      setDisabled(aIsGlobalReadOnly);
      mGlobalReadOnly = aIsGlobalReadOnly;
      setReadOnly(mGlobalReadOnly);
      setUndoRedoEnabled(!mGlobalReadOnly);
      if (!mGlobalReadOnly)
      {
         OnUndoAvailable(mUndoEnabled);
         OnRedoAvailable(mRedoEnabled);
      }
   }
}

void wizard::Editor::Unindent()
{
   QTextCursor cur(textCursor());
   int         startPos = cur.selectionStart();
   int         endPos   = cur.selectionEnd();
   cur.setPosition(endPos);
   int lastBlockNumber = cur.block().blockNumber();
   cur.setPosition(startPos);
   cur.beginEditBlock();
   QTextBlock blk        = cur.block();
   int        indentSize = wkfEnv.GetPreferenceObject<EditorPrefObject>()->TabSpace();
   if (startPos == endPos)
   {
      size_t line;
      size_t column;
      GetSource()->PositionToLineColumn(cur.position(), line, column);
      std::string lineText  = GetSource()->GetLineString(line);
      int         indentAmt = SpaceFromIndent(indentSize);
      if (indentAmt == 0)
      {
         indentAmt = indentSize;
      }
      std::string indentString(indentAmt, ' ');
      for (int x = 0; indentAmt - x - 1 >= 0; x++)
      {
         if (column != 0)
         {
            column--;
            if (lineText[ut::cast_to_int(column)] != ' ')
            {
               break;
            }
            else
            {
               cur.deletePreviousChar();
            }
         }
      }
   }
   else
   {
      size_t line = GetSource()->PositionToLineNumber(blk.position());
      while (blk.isValid() && blk.blockNumber() <= lastBlockNumber)
      {
         int indentAmt = LineSpaceFromIndent(indentSize, line);
         if (indentAmt == 0 && blk.text().startsWith(' '))
         {
            indentAmt = indentSize;
         }
         cur.setPosition(blk.position());
         cur.setPosition(blk.position() + indentAmt, QTextCursor::KeepAnchor);
         endPos -= indentAmt;
         cur.removeSelectedText();
         line++;
         blk = blk.next();
      }
   }
   cur.endEditBlock();
}

void wizard::Editor::Indent()
{
   QTextCursor cur(textCursor());
   int         startPos = cur.selectionStart();
   int         endPos   = cur.selectionEnd();
   cur.setPosition(endPos);
   int lastBlockNumber = cur.block().blockNumber();
   cur.setPosition(startPos);
   int indentSize = wkfEnv.GetPreferenceObject<EditorPrefObject>()->TabSpace();

   if (startPos == endPos)
   {
      indentSize = indentSize - SpaceFromIndent(indentSize);
      std::string indentString(indentSize, ' ');
      cur.insertText(indentString.c_str());
   }
   else
   {
      cur.beginEditBlock();
      QTextBlock blk  = cur.block();
      size_t     line = GetSource()->PositionToLineNumber(cur.position());
      while (blk.isValid() && blk.blockNumber() <= lastBlockNumber)
      {
         cur.setPosition(blk.position());
         int     indentAmt = indentSize - LineSpaceFromIndent(indentSize, line);
         QString indentStringAmt(indentAmt, ' ');
         cur.insertText(indentStringAmt);
         blk = blk.next();
         line++;
      }
      cur.endEditBlock();
   }
}

int wizard::Editor::SpaceFromIndent(int aIndentSize) const
{
   size_t      line, column;
   QTextCursor cur(textCursor());
   mSourcePtr->PositionToLineColumn(cur.position(), line, column);
   int indentSize = (ut::cast_to_int(column)) % aIndentSize;
   return indentSize;
}

int wizard::Editor::LineSpaceFromIndent(int aIndentSize, size_t aLine) const
{
   std::string       lineStr  = mSourcePtr->GetLineString(aLine);
   const std::size_t numSpace = lineStr.find_first_not_of(' ');
   if (numSpace != std::string::npos)
   {
      return numSpace % aIndentSize;
   }
   return lineStr.size();
}

void wizard::Editor::SetManager(EditorManager* aManagerPtr)
{
   // SetManager should be called once with a valid editor
   if (mManagerPtr != aManagerPtr)
   {
      assert(!mManagerPtr && aManagerPtr);
      mManagerPtr = aManagerPtr;
      SetGlobalReadOnly(mManagerPtr->IsReadOnly());
   }
}

int wizard::Editor::MarginWidth()
{
   return LineNumberWidth();
}

void wizard::Editor::focusInEvent(QFocusEvent* e)
{
   QPlainTextEdit::focusInEvent(e);
}

void wizard::Editor::EnsureCurrentLineVisible()
{
   QTextBlock blk = textCursor().block();
   if (blk.isValid() && !blk.isVisible())
   {
      blk.setVisible(true);
      int firstPos = blk.position(), lastPos = blk.position() + blk.length();
      {
         QTextBlock next = blk.next();
         while (next.isValid() && !next.isVisible())
         {
            lastPos = next.position() + next.length();
            next.setVisible(true);
            next = next.next();
         }
      }
      {
         QTextBlock prev = blk.previous();
         while (prev.isValid() && !prev.isVisible())
         {
            prev.setVisible(true);
            firstPos = prev.position();
            prev     = prev.previous();
         }
      }

      document()->markContentsDirty(firstPos, lastPos);
      RedrawMargin();
   }
}

void wizard::Editor::RedrawMargin()
{
   mMarginWidget->update();
}

void wizard::Editor::UpdateMarginGeometry()
{
   if (mMarginWidget)
   {
      QRect cr = contentsRect();
      mMarginWidget->setGeometry(QRect(cr.left(), cr.top(), MarginWidth(), cr.height()));
   }
}

void wizard::Editor::OnUpdateRequest(const QRect& rect, int dy)
{
   if (dy)
   {
      mMarginWidget->scroll(0, dy);
   }
   else
   {
      mMarginWidget->update(0, rect.y(), mMarginWidget->width(), rect.height());
   }
}

void wizard::Editor::UpdateMarginViewport()
{
   setViewportMargins(MarginWidth(), 0, 0, 0);
}

void wizard::Editor::DocContentsChange(int aFrom, int aCharsRemoves, int aCharsAdded)
{
   if (mNonUserAction > 0)
   {
      return;
   }
   if (aFrom == 0 && aCharsRemoves > 0 && aCharsAdded > 0)
   {
      // Qt has some bugs... (check out QTBUG-3495)
      // we are essentially not trusting the parameters to this method and resetting all text
      // in the document if both remove and addition at position 0 is detected
      mSourcePtr->HandleDelete(0, 0x0fffffff);
      QTextCursor cur(document());
      cur.setPosition(0);
      cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
      QString newText = cur.selectedText();
      // Convert to an ascii representation.
      // needs to have the same # of characters
      std::string text = ToAscii(newText);
      mSourcePtr->HandleInsert(aFrom, newText.size(), text.c_str());
      return;
   }
   if (aCharsRemoves)
   {
      mSourcePtr->HandleDelete(aFrom, aCharsRemoves);
   }
   if (aCharsAdded)
   {
      // charsAdded -= charsRemoves;   // this is trouble....
      QTextCursor cur(document());
      cur.setPosition(aFrom);
      cur.setPosition(aFrom + aCharsAdded, QTextCursor::KeepAnchor);
      int         charsSelected = cur.selectionEnd() - cur.selectionStart();
      QTextCursor endCur        = cur;
      endCur.movePosition(QTextCursor::End);
      if (charsSelected != aCharsAdded)
      {
         assert(false); // qt-bug
         cur.setPosition(aFrom);
         cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
         int maxChars = cur.position() - aFrom;
         if (maxChars < aCharsAdded)
         {
            aCharsAdded = maxChars;
         }
      }
      QString newText = cur.selectedText();
      // Convert to an ascii representation.
      // needs to have the same # of characters
      std::string text = ToAscii(newText);
      mSourcePtr->HandleInsert(aFrom, (int)text.size(), text.c_str());
   }
}

void wizard::Editor::OnUndoAvailable(bool aIsAvailable)
{
   mUndoEnabled = aIsAvailable;
}

void wizard::Editor::OnRedoAvailable(bool aIsAvailable)
{
   mRedoEnabled = aIsAvailable;
}

void wizard::Editor::OnCursorPositionChanged()
{
   wizSignals->EditorSelectionChanged(this);
   mDeferredUpdateSelectionsTimer.start();
   RecordCurrentLocation(false);
}

void wizard::Editor::OnModificationChanged(bool aIsModified)
{
   if (!aIsModified)
   {
      mSourcePtr->SavePointReached();
   }
}

void wizard::Editor::OnTextChanged()
{
   mManagerPtr->ModificationOccured(this);
}

void wizard::Editor::UpdateExtraSelections()
{
   QList<QTextEdit::ExtraSelection> extraSelections;
   AddCurrentLineSelection(extraSelections);
   setExtraSelections(extraSelections);
}

void wizard::Editor::wheelEvent(QWheelEvent* e)
{
   if (e->modifiers() & Qt::Modifier::CTRL)
   {
      QPoint numPixels  = e->pixelDelta();
      QPoint numDegrees = e->angleDelta() / 8;

      if (!numPixels.isNull())
      {
         mManagerPtr->AdjustFontSize(numPixels.y());
      }
      else if (!numDegrees.isNull())
      {
         QPoint numSteps = numDegrees / 15;
         mManagerPtr->AdjustFontSize(numSteps.y());
      }
      e->accept();
   }
   else
   {
      QPlainTextEdit::wheelEvent(e);
   }
}

size_t wizard::Editor::GetPosition(const QPoint& aPoint)
{
   QTextCursor cur = cursorForPosition(viewport()->mapFromParent(aPoint));

   // cursorForPosition should not return null, a null cursor's position is -1.
   // ut::cast_to_size_t will throw an out_of_range exception if cur is null.
   assert(!cur.isNull());

   return ut::cast_to_size_t(cur.position());
}

namespace
{
// Looks at a token string list created by Split and identifies if it is useful data, or whitespace / comment
bool Empty(const QString& aStr)
{
   QRegExp reg("[ \f\n\r\t\v#,(//),(/*)]");
   return (0 > aStr.indexOf(reg));
}
} // namespace

QStringList wizard::Editor::Split(const QString& aStr)
{
   QStringList retval;
   int         place    = 0;
   int         oldplace = 0;
   QRegExp     reg("[ \f\n\r\t\v#,(//),(/*)]");
   place = aStr.indexOf(reg, 0);
   while (place >= 0)
   {
      if (place > oldplace) // there was text before the demarcation, add it to a token
      {
         QString nstr = aStr.mid(oldplace, place - oldplace);
         retval += nstr;
      }
      if ((aStr.at(place) == '#') || aStr.mid(place).startsWith("//")) // there is a comment to the end of the list
      {
         oldplace = place;
         place    = aStr.indexOf("\n", oldplace);
         if (place >= 0)
         {
            retval += aStr.mid(oldplace, place - oldplace);
            oldplace = place;
            place    = aStr.indexOf(reg, oldplace);
         }
         else
         {
            break; // this tag should end the list
         }
      }
      else if (aStr.mid(place).startsWith("/*"))
      {
         oldplace = place;
         place    = aStr.indexOf("*/", oldplace);
         if (place >= 0)
         {
            retval += aStr.mid(oldplace, place - oldplace + 2);
            oldplace = place + 2;
            place    = aStr.indexOf(reg, oldplace);
         }
         else
         {
            break; // the comment leaves our area of interest and should end it
         }
      }
      else
      {
         retval += aStr.mid(place, 1);
         oldplace = place + 1;
         place    = aStr.indexOf(reg, oldplace);
      }
   }
   retval += aStr.mid(oldplace);

   std::string              sstr = aStr.toStdString();
   std::vector<std::string> s;
   for (auto&& it : retval)
   {
      s.push_back(it.toStdString());
   }
   return retval;
}

int wizard::Editor::FindNextValidToken(const QStringList& aList, int aStart)
{
   int idx = aStart;
   while (idx < aList.size())
   {
      if (Empty(aList.at(idx)))
      {
         return idx;
      }
      ++idx;
   }
   return -1;
}

bool wizard::Editor::TextIsSelected()
{
   QString selected = textCursor().selectedText();
   return !selected.toStdString().empty();
}

void wizard::Editor::CopyLine()
{
   if (TextIsSelected())
   {
      copy();
   }
   else
   {
      size_t offset, row, column;
      GetCurrentPosition(offset, row, column);
      std::string lineText = GetLineText(row);
      QApplication::clipboard()->setText(QString::fromStdString(lineText + '\n'));
   }
}

void wizard::Editor::CutLine()
{
   if (TextIsSelected())
   {
      cut();
   }
   else
   {
      size_t offset, row, column;
      GetCurrentPosition(offset, row, column);
      std::string lineText = GetLineText(row);
      QApplication::clipboard()->setText(QString::fromStdString(lineText + '\n'));
      DeleteLine();
   }
}

wizard::EditorMargin::EditorMargin(Editor* aP)
   : QWidget(aP)
{
   mParent = aP;
   setMouseTracking(true);
}

void wizard::EditorMargin::paintEvent(QPaintEvent* e)
{
   QWidget::paintEvent(e);
   mParent->PaintMargin(e);
}

void wizard::EditorMargin::mouseMoveEvent(QMouseEvent* e)
{
   mParent->marginMouseMoveEvent(e);
}

void wizard::EditorMargin::leaveEvent(QEvent* e)
{
   mParent->marginLeaveEvent(e);
}
void wizard::EditorMargin::mousePressEvent(QMouseEvent* e)
{
   mParent->marginMousePressEvent(e);
}

void wizard::EditorMargin::contextMenuEvent(QContextMenuEvent* e)
{
   mParent->marginContextMenuEvent(e);
}

const wizard::EditorDock* wizard::Editor::GetDock() const noexcept
{
   return dynamic_cast<EditorDock*>(parent()->parent());
}

wizard::EditorDock* wizard::Editor::GetDock() noexcept
{
   return dynamic_cast<EditorDock*>(parent()->parent());
}
