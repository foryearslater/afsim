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

#ifndef FINDINFILESDIALOG_HPP
#define FINDINFILESDIALOG_HPP

#include <QDialog>
#include <QMainWindow>

#include "FindTextRequest.hpp"
#include "ui_FindInFilesDialog.h"

namespace wizard
{
class EditorManager;
class FindResultsControl;

class FindInFilesDialog : public QDialog
{
public:
   FindInFilesDialog(QMainWindow* aMainWindowPtr, wizard::EditorManager* aEditorPtr, FindResultsControl* aResultsControlPtr);
   ~FindInFilesDialog() override = default;
   void SetSearchText(const std::string& aText);
   void SetSearchLocation(FindTextRequest::SearchLocation aLocation);
   // static VaCallbackList1<void, FindRequest> FindNext;
   // static VaCallbackList1<void, FindRequest> FindPrevious;

   void keyPressEvent(QKeyEvent* aEventPtr) override;

protected:
   FindTextRequest GetRequest();
   void            LookInChanged(bool);
   void            showEvent(QShowEvent* aEventPtr) override;
   void            DefaultAction();
   void            FindAllClick();
   void            FindNextClick();
   void            FindPreviousClick();

private:
   wizard::EditorManager* mEditorPtr;
   FindResultsControl*    mResultsControlPtr;
   Ui::FindInFilesDialog  mUi;
};
} // namespace wizard
#endif
