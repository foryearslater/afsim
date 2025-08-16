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

#include "FindInFilesDialog.hpp"

#include <QKeyEvent>
#include <QLineEdit>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "FindResultsControl.hpp"

wizard::FindInFilesDialog::FindInFilesDialog(QMainWindow*        aMainWindowPtr,
                                             EditorManager*      aEditorPtr,
                                             FindResultsControl* aResultsControlPtr)
   : QDialog(aMainWindowPtr, /*Qt::Tool | */ Qt::WindowStaysOnTopHint)
   , mEditorPtr(aEditorPtr)
   , mResultsControlPtr(aResultsControlPtr)
{
   mUi.setupUi(this);
   this->setFocusProxy(aMainWindowPtr);
   // disable auto-completion
   mUi.findInput->setCompleter(0);

   QObject::connect(mUi.findAllBn, &QPushButton::clicked, this, &FindInFilesDialog::FindAllClick);
   QObject::connect(mUi.findNextBn, &QPushButton::clicked, this, &FindInFilesDialog::FindNextClick);
   QObject::connect(mUi.findPreviousBn, &QPushButton::clicked, this, &FindInFilesDialog::FindPreviousClick);
   QObject::connect(mUi.lookInDocumentOption, &QRadioButton::toggled, this, &FindInFilesDialog::LookInChanged);
   QObject::connect(mUi.lookInProjectOption, &QRadioButton::toggled, this, &FindInFilesDialog::LookInChanged);
   QObject::connect(mUi.lookInDirectories, &QRadioButton::toggled, this, &FindInFilesDialog::LookInChanged);
}

wizard::FindTextRequest wizard::FindInFilesDialog::GetRequest()
{
   FindTextRequest request;
   request.mCaseSensitive = mUi.matchCaseOption->isChecked();
   QString txt            = mUi.findInput->currentText();
   request.mText          = txt.toStdString();
   if (mUi.lookInDocumentOption->isChecked())
   {
      request.mSearchLocation = FindTextRequest::cACTIVE_FILE;
   }
   else if (mUi.lookInDirectories->isChecked())
   {
      request.mSearchLocation = FindTextRequest::cDIRECTORIES;
   }
   return request;
}
void wizard::FindInFilesDialog::FindAllClick()
{
   FindTextRequest request = GetRequest();
   if (request.mText.empty())
   {
      return;
   }
   // If item is not already in the list then add it
   if (mUi.findInput->findText(QString::fromStdString(request.mText)) == -1)
   {
      mUi.findInput->addItem(QString::fromStdString(request.mText));
   }
   mResultsControlPtr->NewFind(request);
   hide();
   mResultsControlPtr->setFocus();
}

void wizard::FindInFilesDialog::FindNextClick()
{
   FindTextRequest request = GetRequest();
   if (request.mText.empty())
   {
      return;
   }
   // If item is not already in the list then add it
   if (mUi.findInput->findText(QString::fromStdString(request.mText)) == -1)
   {
      mUi.findInput->addItem(QString::fromStdString(request.mText));
   }
   mEditorPtr->SetFindRequest(request);
   Editor* current = mEditorPtr->GetCurrentEditor();
   if (current)
   {
      current->FindNextAction();
   }
}

void wizard::FindInFilesDialog::FindPreviousClick()
{
   FindTextRequest request = GetRequest();
   if (request.mText.empty())
   {
      return;
   }
   // If item is not already in the list then add it
   if (mUi.findInput->findText(QString::fromStdString(request.mText)) == -1)
   {
      mUi.findInput->addItem(QString::fromStdString(request.mText));
   }
   mEditorPtr->SetFindRequest(request);
   Editor* current = mEditorPtr->GetCurrentEditor();
   if (current)
   {
      current->FindPreviousAction();
   }
}

void wizard::FindInFilesDialog::showEvent(QShowEvent* aEventPtr)
{
   mUi.findInput->setFocus();
   mUi.findInput->lineEdit()->setSelection(0, 0xffffff);
   QWidget::showEvent(aEventPtr);
}

void wizard::FindInFilesDialog::SetSearchText(const std::string& aText)
{
   mUi.findInput->setEditText(QString::fromStdString(aText));
}

void wizard::FindInFilesDialog::LookInChanged(bool)
{
   if (mUi.lookInProjectOption->isChecked())
      SetSearchLocation(FindTextRequest::cPROJECT);
   if (mUi.lookInDocumentOption->isChecked())
      SetSearchLocation(FindTextRequest::cACTIVE_FILE);
   if (mUi.lookInDirectories->isChecked())
      SetSearchLocation(FindTextRequest::cDIRECTORIES);
}

void wizard::FindInFilesDialog::DefaultAction()
{
   if (mUi.findAllBn->isDefault())
   {
      FindAllClick();
   }
   else
   {
      FindNextClick();
   }
}

void wizard::FindInFilesDialog::SetSearchLocation(FindTextRequest::SearchLocation aLocation)
{
   bool lookingInSingleFile = aLocation == FindTextRequest::cACTIVE_FILE;
   mUi.lookInProjectOption->setChecked(aLocation == FindTextRequest::cPROJECT);
   mUi.lookInDocumentOption->setChecked(aLocation == FindTextRequest::cACTIVE_FILE);
   mUi.lookInDirectories->setChecked(aLocation == FindTextRequest::cDIRECTORIES);
   mUi.findNextBn->setEnabled(lookingInSingleFile);
   mUi.findPreviousBn->setEnabled(lookingInSingleFile);
   mUi.findAllBn->setDefault(!lookingInSingleFile);
   mUi.findAllBn->setAutoDefault(!lookingInSingleFile);
   mUi.findNextBn->setDefault(lookingInSingleFile);
   mUi.findNextBn->setAutoDefault(lookingInSingleFile);
}

void wizard::FindInFilesDialog::keyPressEvent(QKeyEvent* aEventPtr)
{
   QDialog::keyPressEvent(aEventPtr);
}
