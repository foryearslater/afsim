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

#include "ReplaceInFilesDialog.hpp"

#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "UtStringUtil.hpp"

wizard::ReplaceInFilesDialog::ReplaceInFilesDialog(QMainWindow* aMainWindowPtr, EditorManager* aEditorPtr)
   : QDialog(aMainWindowPtr, Qt::Tool | Qt::WindowStaysOnTopHint)
   , mEditorPtr(aEditorPtr)
{
   mUi.setupUi(this);
   this->setFocusProxy(aMainWindowPtr);
   // disable auto-completion
   mUi.findInput->setCompleter(nullptr);

   QObject::connect(mUi.replaceBn, &QPushButton::clicked, this, &ReplaceInFilesDialog::ReplaceClick);
   QObject::connect(mUi.replaceAllBn, &QPushButton::clicked, this, &ReplaceInFilesDialog::ReplaceAllClick);
   QObject::connect(mUi.findNextBn, &QPushButton::clicked, this, &ReplaceInFilesDialog::FindNextClick);
   QObject::connect(mUi.findPreviousBn, &QPushButton::clicked, this, &ReplaceInFilesDialog::FindPreviousClick);
}

wizard::ReplaceTextRequest wizard::ReplaceInFilesDialog::GetRequest()
{
   ReplaceTextRequest request;
   request.mCaseSensitive  = mUi.matchCaseOption->isChecked();
   QString txt             = mUi.findInput->currentText();
   request.mText           = txt.toStdString();
   QString rpl             = mUi.replaceInput->currentText();
   request.mReplaceText    = rpl.toStdString();
   request.mSearchLocation = ReplaceTextRequest::cACTIVE_FILE;

   return request;
}

void wizard::ReplaceInFilesDialog::ReplaceClick()
{
   if (GetRequest().mText.empty())
   {
      return;
   }
   mEditorPtr->SetReplaceRequest(GetRequest());
   Editor* editorControlPtr = mEditorPtr->GetCurrentEditor();
   if (nullptr != editorControlPtr)
   {
      editorControlPtr->ReplaceFoundText();
   }
}

void wizard::ReplaceInFilesDialog::ReplaceAllClick()
{
   ReplaceTextRequest request = GetRequest();
   if (request.mText.empty())
   {
      return;
   }
   if (request.mSearchLocation == ReplaceTextRequest::cACTIVE_FILE)
   {
      mEditorPtr->SetReplaceRequest(request);
      Editor* editorControlPtr = mEditorPtr->GetCurrentEditor();

      if (nullptr != editorControlPtr)
      {
         int         lastReplace = -1;
         QTextCursor cur(editorControlPtr->document());
         cur.beginEditBlock();
         while (editorControlPtr->SearchNext(request, &cur) && cur.position() > lastReplace)
         {
            lastReplace = cur.position();
            cur.insertText(request.mReplaceText.c_str());
         }
         //          UtTextDocumentRange r = editorControlPtr->GetSelectionRange();
         //          r.mBegin = 0;
         //          r.mEnd = 0;
         //          editorControlPtr->SetSelection(r);
         //          editorControlPtr->ReplaceFoundText();
         //          int lastBegin = editorControlPtr->GetSelectionRange().mBegin;
         //          int thisBegin;
         //          bool more = true;
         //          while (more)
         //          {
         //             more = editorControlPtr->ReplaceFoundText();
         //             thisBegin = editorControlPtr->GetSelectionRange().mBegin;
         //             if (thisBegin < lastBegin)
         //             {
         //                more = false;
         //             }
         //             else
         //             {
         //                lastBegin = thisBegin;
         //             }
         //          }
         cur.endEditBlock();
      }
      hide();
   }
}

void wizard::ReplaceInFilesDialog::FindNextClick()
{
   if (GetRequest().mText.empty())
   {
      return;
   }
   mEditorPtr->SetFindRequest(GetRequest());
   Editor* current = mEditorPtr->GetCurrentEditor();
   if (current)
   {
      current->FindNextAction();
   }
}

void wizard::ReplaceInFilesDialog::FindPreviousClick()
{
   if (GetRequest().mText.empty())
   {
      return;
   }
   mEditorPtr->SetFindRequest(GetRequest());
   Editor* current = mEditorPtr->GetCurrentEditor();
   if (current)
   {
      current->FindPreviousAction();
   }
}

void wizard::ReplaceInFilesDialog::showEvent(QShowEvent* aEventPtr)
{
   mUi.findInput->setFocus();
   mUi.findInput->lineEdit()->setSelection(0, 0xffffff);
   QWidget::showEvent(aEventPtr);
}

void wizard::ReplaceInFilesDialog::SetSearchText(const std::string& aText)
{
   mUi.findInput->setEditText(QString::fromStdString(aText));
}

void wizard::ReplaceInFilesDialog::DefaultAction()
{
   FindNextClick();
}

void wizard::ReplaceInFilesDialog::SetSearchLocation(ReplaceTextRequest::SearchLocation aLocation)
{
   mUi.findNextBn->setEnabled(true);
   mUi.findPreviousBn->setEnabled(true);
   mUi.findNextBn->setDefault(false);
   mUi.findNextBn->setAutoDefault(false);
   mUi.findNextBn->setDefault(true);
   mUi.findNextBn->setAutoDefault(true);
   mUi.replaceBn->setEnabled(true);
}
