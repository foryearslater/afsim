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

#ifndef CREATENEWFILE_HPP
#define CREATENEWFILE_HPP

#include <QDialog>
#include <QMainWindow>

#include "ui_CreateNewFile.h"

class UtPath;

namespace wizard
{
class Project;

class CreateNewFile : public QDialog
{
public:
   CreateNewFile(QMainWindow* aMainWindowPtr, const std::string& aApplicationName);
   ~CreateNewFile() override = default;
   void Open(Project* aProjectPtr, const UtPath& aInitialDirectory);

   void     OkClick();
   void     CancelClick();
   void     BrowseClick();
   Project* mProjectPtr;

private:
   Ui::CreateNewFile mUI;
   std::string       mApplicationName;
};
} // namespace wizard

#endif
