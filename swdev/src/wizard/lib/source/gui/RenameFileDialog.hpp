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

#ifndef RENAMEFILEDIALOG_HPP
#define RENAMEFILEDIALOG_HPP

#include <QDialog>

#include "Editor.hpp"
#include "UtPath.hpp"
#include "UtQtColumnSettings.hpp"
#include "ui_RenameFileDialog.h"
class QStandardItemModel;
class ViSourceItem;

namespace wizard
{
class Project;
class TextSource;

class RenameFileDialog : public QDialog
{
public:
   explicit RenameFileDialog(QWidget* aParentPtr);
   ~RenameFileDialog() override = default;

   void FileRename(Editor* aEditorPtr);

protected:
   void OkClick();
   void CancelClick();

   void UpdateList();

   QStandardItemModel*  mModelPtr;
   TextSource*          mSourcePtr;
   Ui::RenameFileDialog mUi;
   Editor*              mEditorPtr;
   UtQtColumnSettings   mColumnSettings;
};
} // namespace wizard

#endif
