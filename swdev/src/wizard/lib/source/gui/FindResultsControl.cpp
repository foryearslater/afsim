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

#include "FindResultsControl.hpp"

#include <cctype>

#include <QDir>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "TextLocationItemModel.hpp"
#include "TextSourceCache.hpp"
#include "UtQt.hpp"
#include "UtQtTextDelegate.hpp"
#include "UtStringUtil.hpp"

wizard::FindResultsControl::FindResultsControl(QMainWindow* aParentPtr, EditorManager* aEditorPtr)
   : QDockWidget(nullptr, Qt::Tool)
   , mEditorPtr(aEditorPtr)
   , mProjectPtr()
   , mWaitBox(nullptr)
{
   setObjectName("FindResultsControl");
   mModelPtr = nullptr;
   setWidget(new QWidget(this));
   mUi.setupUi(widget());
   setWindowTitle(widget()->windowTitle());
   mUi.resultList->setModel(mModelPtr);
   mUi.resultList->setItemDelegateForColumn(1, new UtQtTextDelegate(mUi.resultList));
   setFocusProxy(mUi.resultList);

   connect(mUi.resultList, &QTreeView::activated, this, &FindResultsControl::ItemActivated);
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ProjectOpened, this, &FindResultsControl::ProjectOpened);
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ProjectClosed, this, &FindResultsControl::ProjectClosed);
}

void wizard::FindResultsControl::NewFind(FindTextRequest aRequest)
{
   delete mModelPtr;
   mModelPtr = nullptr;
   mResults.clear();
   std::string lcSearch = aRequest.mText;
   UtStringUtil::ToLower(lcSearch);
   QString heading("Results of searching ");

   // Searching all included or open files
   if (aRequest.mSearchLocation == FindTextRequest::cPROJECT)
   {
      heading.append("project");
      const TextSourceCache::SourceMap& sources = ProjectWorkspace::Instance()->GetSourceCache()->GetSources();
      for (auto&& source : sources)
      {
         TextSource* sourcePtr = source.second;
         SearchFile(aRequest, sourcePtr, lcSearch);
      }
   }
   // Searching current document
   else if (aRequest.mSearchLocation == FindTextRequest::cACTIVE_FILE)
   {
      heading.append("document");
      Editor* editControlPtr = mEditorPtr->GetCurrentEditor();
      if (editControlPtr)
      {
         SearchFile(aRequest, editControlPtr->GetSource(), lcSearch);
      }
   }
   else // Searching all files in directory
   {
      heading.append("directory");
      mWaitBox = new QLabel(this, Qt::Window | Qt::WindowStaysOnTopHint);
      mWaitBox->setCursor(Qt::WaitCursor);
      mWaitBox->setWindowModality(Qt::ApplicationModal);
      mWaitBox->resize(QSize(400, 150));
      QPoint waitBoxPos = QCursor::pos();
      waitBoxPos.rx() -= 200;
      waitBoxPos.ry() -= 75;
      mWaitBox->move(waitBoxPos);
      mWaitBox->show();
      std::vector<UtPath> directories = mProjectPtr->GetProjectDirectories();
      for (auto&& dir : directories)
      {
         SearchDirectory(QString::fromStdString(dir.GetSystemPath()), aRequest, lcSearch);
      }
      delete mWaitBox;
   }
   // model is deleted when new find is envoked; see top of this function
   mModelPtr            = new TextLocationItemModel(mResults);
   mModelPtr->mShowText = true;
   mUi.resultList->setModel(mModelPtr);

   // Complete the heading for the find control
   heading.append(" for \"");
   heading.append(QString::fromStdString(aRequest.mText));
   heading.append("\": ");
   heading.append(QString::number(mResults.size()));
   mUi.resultLabel->setText(heading);

   // Resize the first column based on the content width
   mUi.resultList->resizeColumnToContents(0);

   show();
   UtQtRaiseWidget(this);
}

int wizard::FindResultsControl::SearchFile(const FindTextRequest& aRequest,
                                           TextSource*            aSourcePtr,
                                           const std::string&     aLowerCaseString)
{
   int             entriesAdded = 0;
   UtTextDocument& doc          = *aSourcePtr->GetSource();
   char*           textPtr      = doc.GetPointer();
   size_t          size         = doc.GetText().Size();
   char*           start        = textPtr;
   char*           end          = start + size;
   while (start != end)
   {
      char* newStart;
      if (aRequest.mCaseSensitive)
      {
         newStart = std::search(start, end, aRequest.mText.begin(), aRequest.mText.end());
      }
      else
      {
         newStart = std::search(start,
                                end,
                                aLowerCaseString.begin(),
                                aLowerCaseString.end(),
                                [](char a, char b) { return std::tolower(a) == b; });
      }
      if (newStart != end)
      {
         int offset = newStart - textPtr;
         AddResult(aSourcePtr, offset, offset + (int)aRequest.mText.size() - 1);
         // jump to end of line
         while (newStart != end && *newStart != '\n')
         {
            ++newStart;
         }
         ++entriesAdded;
      }
      start = newStart;
   }
   return entriesAdded;
}

void wizard::FindResultsControl::AddResult(TextSource* aSourcePtr, int aCharOffset, int aCharOffsetEnd)
{
   // int lineNumber = aSourcePtr->GetSource()->PositionToLineNumber(aCharOffset);
   UtTextDocumentAutoUpdateRange range(aSourcePtr, UtTextRange(aCharOffset, aCharOffsetEnd));
   mResults.push_back(range);
}

void wizard::FindResultsControl::ItemActivated(const QModelIndex& aIndex)
{
   if (mModelPtr && aIndex.row() >= 0 && aIndex.row() < (int)mModelPtr->GetLocations().size())
   {
      UtTextDocumentRange range = mModelPtr->GetLocations()[aIndex.row()];
      ProjectWorkspace::Instance()->GotoRange(range);
   }
}

void wizard::FindResultsControl::ProjectOpened(Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
}

void wizard::FindResultsControl::ProjectClosed(Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr)
   {
      delete mModelPtr;
      mModelPtr = nullptr;
      mResults.clear();
      mProjectPtr = nullptr;
   }
}

void wizard::FindResultsControl::SearchDirectory(const QString&         aDirectory,
                                                 const FindTextRequest& aRequest,
                                                 const std::string&     aLowerCaseString)
{
   if (!mWaitBox->isVisible())
   {
      return;
   }
   mWaitBox->setText("Searching: " + aDirectory);
   QApplication::processEvents();
   QDir        dir(aDirectory);
   QStringList files = dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
   for (int i = 0; i < files.size(); ++i)
   {
      QFileInfo info(dir, files[i]);
      if (info.isDir())
      {
         SearchDirectory(info.absoluteFilePath(), aRequest, aLowerCaseString);
      }
      else
      {
         UtPath      filePath(info.absoluteFilePath().toStdString());
         std::string pathStr         = filePath.GetSystemPath();
         bool        unloadAfterScan = false;
         TextSource* src             = mProjectPtr->GetWorkspace()->FindSource(pathStr, false, true);
         if (!src)
         {
            if (!Util::IsNonTextFile(info.absoluteFilePath()))
            {
               src             = mProjectPtr->GetWorkspace()->FindSource(pathStr, true, true);
               unloadAfterScan = true;
            }
         }
         if (src)
         {
            int entriesAdded = SearchFile(aRequest, src, aLowerCaseString);
            if (unloadAfterScan && entriesAdded == 0)
            {
               mProjectPtr->GetSourceCache().DeleteSource(src);
            }
         }
      }
   }
}
