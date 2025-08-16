// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "FileLocatorWidget.hpp"

#include <QAbstractItemView>
#include <QFileDialog>

#include "Editor.hpp"
#include "ProjectWorkspace.hpp"
#include "TextSource.hpp"
#include "editor/Editor.hpp"

namespace wizard
{
namespace EventOutput
{

FileLocatorWidget::FileLocatorWidget(QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
{
   // set up the UI and load the file names
   mUi.setupUi(this);
   mUi.mPath->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   LoadFiles();

   // do the necessary connections
   connect(mUi.mPath, QOverload<const QString&>::of(&QComboBox::activated), this, &FileLocatorWidget::FileChanged);
   connect(mUi.mBrowse, &QPushButton::clicked, this, &FileLocatorWidget::OnBrowse);
}

void FileLocatorWidget::SwitchToFile(wizard::Editor* aCurrentFile)
{
   QSignalBlocker block{mUi.mPath};
   QString        filePathName{"Select a File"};
   if (aCurrentFile)
   {
      filePathName = QString::fromStdString(aCurrentFile->GetSource()->GetFilePath().GetNormalizedPath());
   }
   mUi.mPath->setCurrentText(filePathName);
   emit FileChanged(filePathName);
}

void FileLocatorWidget::SwitchToFile(editor::Editor* aCurrentFile)
{
   SwitchToFile(aCurrentFile ? aCurrentFile->GetWizEditor() : nullptr);
}

void FileLocatorWidget::OnBrowse()
{
   QSignalBlocker block{mUi.mPath};
   // open the dialog to browse for a file on the system
   QString filePathName{QFileDialog::getOpenFileName(this, "Select Input File", "", "AFSIM Input File (*.txt)")};
   if (!filePathName.isEmpty())
   {
      if (mUi.mPath->findText(filePathName) == -1)
      {
         mUi.mPath->addItem(filePathName);
      }
      mUi.mPath->setCurrentText(filePathName);
      emit FileChanged(filePathName);
   }
}

void FileLocatorWidget::LoadFiles()
{
   QSignalBlocker block{mUi.mPath};
   mUi.mPath->clear();
   mUi.mPath->addItem("Select a File");
   mUi.mPath->insertSeparator(1);
   for (auto file : ProjectWorkspace::Instance()->GetSourceList())
   {
      mUi.mPath->addItem(QString::fromStdString(file->GetFilePath().GetNormalizedPath()));
   }
}

} // namespace EventOutput
} // namespace wizard
