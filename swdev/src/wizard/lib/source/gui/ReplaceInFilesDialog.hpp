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

#ifndef REPLACEINFILESDIALOG_HPP
#define REPLACEINFILESDIALOG_HPP

#include <QDialog>
#include <QMainWindow>

#include "ReplaceTextRequest.hpp"
#include "ui_ReplaceInFilesDialog.h"

namespace wizard
{
class EditorManager;

class ReplaceInFilesDialog : public QDialog
{
public:
   ReplaceInFilesDialog(QMainWindow* aMainWindowPtr, EditorManager* aEditorPtr);
   ~ReplaceInFilesDialog() override = default;
   void SetSearchText(const std::string& aText);
   void SetSearchLocation(ReplaceTextRequest::SearchLocation aLocation);

protected:
   ReplaceTextRequest GetRequest();
   void               LookInChanged(bool);
   void               showEvent(QShowEvent* aEventPtr) override;
   void               DefaultAction();
   void               ReplaceClick();
   void               ReplaceAllClick();
   void               FindNextClick();
   void               FindPreviousClick();

   EditorManager*           mEditorPtr;
   Ui::ReplaceInFilesDialog mUi;
};
} // namespace wizard
#endif
