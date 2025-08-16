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

#include "CreateNewFile.hpp"

#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "Signals.hpp"
#include "UtPath.hpp"
#include "UtQt.hpp"
#include "Util.hpp"

wizard::CreateNewFile::CreateNewFile(QMainWindow* aMainWindowPtr, const std::string& aApplicationName)
   : QDialog(aMainWindowPtr)
   , mApplicationName(aApplicationName)
{
   mUI.setupUi(this);
   mUI.directoryInput->setEditable(true);
   mUI.directoryBowseBn->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
   QObject::connect(mUI.okBn, &QPushButton::clicked, this, &CreateNewFile::OkClick);
   QObject::connect(mUI.cancelBn, &QPushButton::clicked, this, &CreateNewFile::CancelClick);
   QObject::connect(mUI.directoryBowseBn, &QToolButton::clicked, this, &CreateNewFile::BrowseClick);
}

void wizard::CreateNewFile::OkClick()
{
   UtPath path((mUI.directoryInput->currentText() + "/" + mUI.fileNameInput->text()).toStdString());
   if (mUI.fileNameInput->text().isEmpty())
   {
      QMessageBox::warning(this, "Invalid File", "Enter a file name.");
   }
   else if (path.Stat() == UtPath::cSTAT_ERROR)
   {
      UtPath dir = path;
      dir.Up();
      if (dir.Stat() == UtPath::cDIRECTORY)
      {
         // Qt doesn't have a way to check if a file name is valid.  Instead, just open the file for that determination.
         if (wizard::Util::CreateNewFile(path.GetSystemPath()))
         {
            hide();
         }
         else
         {
            QMessageBox::warning(this, "Invalid File", "Could not create file.  File name may be invalid.");
         }
      }
      else
      {
         QMessageBox::warning(this, "Invalid Directory", "Directory does not exist.  Enter a valid directory.");
      }
   }
   else
   {
      QMessageBox::warning(this, "File Exists", "File already exists.  Choose a new name.");
   }
}

void wizard::CreateNewFile::CancelClick()
{
   hide();
}

void wizard::CreateNewFile::BrowseClick()
{
   QString path = QFileDialog::getExistingDirectory(this, "Select directory", mUI.directoryInput->currentText());
   if (!path.isEmpty())
   {
      mUI.directoryInput->setEditText(path);
   }
}

void wizard::CreateNewFile::Open(Project* aProjectPtr, const UtPath& aInitialDirectory)
{
   mProjectPtr           = aProjectPtr;
   ParseResults* results = mProjectPtr->WaitForParseResults();
   if (results)
   {
      std::set<std::string> allPaths;
      for (const auto& include : results->mFirstIncludes)
      {
         UtPath path = include.first->GetFilePath();
         path.Up();
         allPaths.insert(path.GetSystemPath());
      }
      mUI.directoryInput->clear();
      for (const auto& path : allPaths)
      {
         QString dirTxt = path.c_str();
         mUI.directoryInput->addItem(dirTxt);
      }
   }
   mUI.directoryInput->setEditText(aInitialDirectory.GetSystemPath().c_str());
   mUI.fileNameInput->setText("");
   mUI.fileNameInput->setFocus();
   show();
}
