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

#include "RenameFileDialog.hpp"

#include <cstdio>
#include <sstream>

#include <QMessageBox>
#include <QStandardItemModel>

#include "EditorManager.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "RunManager.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"

wizard::RenameFileDialog::RenameFileDialog(QWidget* aParentPtr)
   : QDialog(aParentPtr)
{
   mUi.setupUi(this);
   mModelPtr = new QStandardItemModel(this);
   mUi.includeList->setModel(mModelPtr);

   QObject::connect(mUi.okBn, &QPushButton::clicked, this, &RenameFileDialog::OkClick);
   QObject::connect(mUi.cancelBn, &QPushButton::clicked, this, &RenameFileDialog::CancelClick);
}

void wizard::RenameFileDialog::FileRename(Editor* aEditorPtr)
{
   UtPath      filePath = aEditorPtr->GetSource()->GetFilePath();
   std::string systemPath{filePath.GetSystemPath()};
   TextSource* sourcePtr = aEditorPtr->GetProject()->GetWorkspace()->FindSource(systemPath, false, false);
   if (sourcePtr)
   {
      mEditorPtr = aEditorPtr;
      mUi.currentNameLabel->setText(QString::fromStdString(systemPath));
      mUi.newNameInput->setText(QString::fromStdString(filePath.GetFileName()));
      mSourcePtr = sourcePtr;
      UpdateList();
      show();
   }
}

void wizard::RenameFileDialog::UpdateList()
{
   mColumnSettings.Save(*mUi.includeList);
   mModelPtr->clear();

   Project* projectPtr = mSourcePtr->GetProject();
   projectPtr->WaitForParsing();

   std::vector<UtTextDocumentRange> includes;
   ParseResults*                    resultsPtr = projectPtr->GetParseResults();
   if (resultsPtr)
   {
      resultsPtr->FindIncludes(mSourcePtr, includes);
      for (auto&& range : includes)
      {
         size_t      line = range.mSource->PositionToLineNumber(range.GetBegin());
         std::string path = projectPtr->ProjectDirectory().GetRelativePath(range.mSource->GetFilePath()).GetSystemPath();
         QString     txt  = QString::fromStdString(path) + " : " + QString::number(line);

         QList<QStandardItem*> row;
         row.push_back(new QStandardItem(txt));
         row.push_back(new QStandardItem(QString::fromStdString(range.mSource->GetLineString(line)).trimmed()));
         mModelPtr->appendRow(row);
      }

      QStringList header;
      header.push_back("Location");
      header.push_back("Text");
      mModelPtr->setHorizontalHeaderLabels(header);
      mColumnSettings.Load(*mUi.includeList);
   }
}

void wizard::RenameFileDialog::OkClick()
{
   Project*      projectPtr = mSourcePtr->GetProject();
   ParseResults* resultsPtr = projectPtr->GetParseResults();

   QString newname      = mUi.newNameInput->text();
   UtPath  path         = mSourcePtr->GetSource()->GetFilePath();
   UtPath  originalPath = path;
   path.Up();
   path += newname.toStdString();

   QString                          currentName = mSourcePtr->GetFileName().c_str();
   std::vector<UtTextDocumentRange> includes;
   if (!resultsPtr || currentName == newname)
   {
      hide();
      return;
   }

   resultsPtr->FindIncludes(mSourcePtr, includes);
   for (auto&& range : includes)
   {
      QString text{QString::fromStdString(range.Text())};
      if (text.indexOf(currentName) != -1)
      {
         text.replace(currentName, newname);
         TextSource* txtSourcePtr = projectPtr->GetSourceCache().FindSource(range.mSource);
         if (txtSourcePtr)
         {
            txtSourcePtr->Replace(range, text.toStdString());
         }
      }
   }

   if (std::rename(originalPath.GetSystemPath().c_str(), path.GetSystemPath().c_str()) != 0)
   {
      std::stringstream message;
      message << "File Rename Error - " << std::strerror(errno);
      QMessageBox::warning(this, std::strerror(errno), QString::fromStdString(message.str()));
   }
   else
   {
      std::vector<UtPath> startupFiles = Project::Instance()->GetStartupFiles();
      std::string         newPath      = path.GetNormalizedPath();
      for (size_t i = 0; i < startupFiles.size(); i++)
      {
         if (startupFiles[i] == originalPath)
         {
            startupFiles[i] = newPath;
            break;
         }
      }
      // if the first start-up file changed, set the working directory to the its directory
      if (wizRunMgr.SetWorkingDirectoryToProject(startupFiles.front()))
      {
         Project::Instance()->SetStartupFiles(startupFiles);
         wizard::Project::Instance()->GotoFile(newPath);
         mEditorPtr->close();
         delete mEditorPtr;
      }
   }

   mModelPtr->clear();
   hide();
   projectPtr->GetWorkspace()->ScheduleCheckingFilesForModification();
}

void wizard::RenameFileDialog::CancelClick()
{
   hide();
}
