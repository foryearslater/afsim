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

#include "ContextMenuActions.hpp"

#include <QCursor>
#include <QMessageBox>

#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "RunManager.hpp"
#include "TextSource.hpp"
#include "WsfPM_Root.hpp"

wizard::ContextMenuActions::OpenFileInProject::OpenFileInProject(const std::string& aFileFullPath,
                                                                 QWidget*           aParentWidget,
                                                                 QObject*           aParent)
   : QAction("", aParent)
   , mFilePath(aFileFullPath)
   , mParentWidgetPtr(aParentWidget)
{
   if (Project::Instance())
   {
      setText(Project::Instance()->WorkingDirectory().GetRelativePath(aFileFullPath).GetNormalizedPath().c_str());
   }
   connect(this, &OpenFileInProject::triggered, this, &OpenFileInProject::OnTriggered);
}

void wizard::ContextMenuActions::OpenFileInProject::OnTriggered()
{
   Project* proj = Project::Instance();
   if (proj)
      proj->OpenFile(mFilePath);
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::MessageReporter::MessageReporter(const QString& aMessageTitle,
                                                             QWidget*       aParentWidget,
                                                             QObject*       aParent)
   : QObject(aParent)
   , mMessageTitle(aMessageTitle)
   , mParentWidgetPtr(aParentWidget)
{
}

void wizard::ContextMenuActions::MessageReporter::ShowWarningMessage(const QString& aMessage)
{
   QMessageBox::warning(mParentWidgetPtr, mMessageTitle, aMessage);
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::SetAsStartupFile::SetAsStartupFile(const QString& aFilePath, QObject* aParent)
   : QAction("Set as Startup File", aParent)
   , mFilePath(aFilePath)
{
   connect(this, &SetAsStartupFile::triggered, this, &SetAsStartupFile::OnTriggered);
}

void wizard::ContextMenuActions::SetAsStartupFile::OnTriggered()
{
   // set the working directory to the new start-up file's directory
   if (wizRunMgr.SetWorkingDirectoryToProject(mFilePath))
   {
      std::vector<UtPath> startupFiles;
      startupFiles.push_back(mFilePath.toStdString());
      Project::Instance()->SetStartupFiles(startupFiles);
   }
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::AddToStartupFiles::AddToStartupFiles(const QString& aFilePath, QObject* aParent)
   : QAction("Add to Startup Files", aParent)
   , mFilePath(aFilePath)
{
   connect(this, &AddToStartupFiles::triggered, this, &AddToStartupFiles::OnTriggered);
}

void wizard::ContextMenuActions::AddToStartupFiles::OnTriggered()
{
   std::vector<UtPath> startupFiles = Project::Instance()->GetStartupFiles();
   startupFiles.push_back(mFilePath.toStdString());
   Project::Instance()->SetStartupFiles(startupFiles);
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::RemoveFromStartupFiles::RemoveFromStartupFiles(const QString& aFilePath, QObject* aParent)
   : QAction("Remove from Startup Files", aParent)
   , mFilePath(aFilePath)
{
   connect(this, &RemoveFromStartupFiles::triggered, this, &RemoveFromStartupFiles::OnTriggered);
}

void wizard::ContextMenuActions::RemoveFromStartupFiles::OnTriggered()
{
   std::vector<UtPath>           startupFiles = Project::Instance()->GetStartupFiles();
   const UtPath&                 path         = mFilePath.toStdString();
   std::vector<UtPath>::iterator iter         = std::find(startupFiles.begin(), startupFiles.end(), path);
   if (iter != startupFiles.end())
   {
      startupFiles.erase(iter);
   }
   // if the first start-up file changed, set the working directory to the its directory
   if (wizRunMgr.SetWorkingDirectoryToProject(startupFiles.front()))
   {
      Project::Instance()->SetStartupFiles(startupFiles);
   }
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::ShowStartupFilesDialog::ShowStartupFilesDialog(QObject* aParent)
   : QAction("Configure Startup Files...", aParent)
{
   connect(this, &ShowStartupFilesDialog::triggered, this, &ShowStartupFilesDialog::OnTriggered);
}

void wizard::ContextMenuActions::ShowStartupFilesDialog::OnTriggered()
{
   Project::Instance()->ShowStartupFileDialog();
}

// -----------------------------------------------------------------------------

wizard::ContextMenuActions::OpenFileInEditor::OpenFileInEditor(const QString& aFilePath, QObject* aParent)
   : QAction("Open in Editor", aParent)
   , mFilePath(aFilePath)
{
   connect(this, &OpenFileInEditor::triggered, this, &OpenFileInEditor::OnTriggered);
}

void wizard::ContextMenuActions::OpenFileInEditor::OnTriggered()
{
   TextSource* sourcePtr = Project::Instance()->GetWorkspace()->FindSource(mFilePath.toStdString(), true, false);
   Project::Instance()->GotoFile(sourcePtr, true);
}
