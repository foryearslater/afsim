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
#include "DockWidget.hpp"

#include <QDesktopServices>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include "ActionManager.hpp"
#include "Item.hpp"
#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfParseSourceInclude.hpp"

class ProjectBrowser::ProxyModel : public QSortFilterProxyModel
{
public:
   explicit ProxyModel(Model* aModelPtr)
      : mModelPtr(aModelPtr)
   {
      setSourceModel(aModelPtr);
   }

   void Update()
   {
      mAcceptedItems.clear();
      Walk(mModelPtr->root());
      invalidate();
   }

   bool filterAcceptsColumn(int, const QModelIndex&) const override { return true; }

   bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
   {
      if (sourceModel() != mModelPtr)
      {
         return false;
      }
      QModelIndex idx     = mModelPtr->index(source_row, 0, source_parent);
      Item*       itemPtr = dynamic_cast<Item*>(mModelPtr->FindItem(idx));
      if (!itemPtr)
      {
         return true;
      }
      return (mAcceptedItems.find(itemPtr) != mAcceptedItems.end());
   }

   Model*  mModelPtr;
   QRegExp mRegEx;

private:
   int Walk(Item* aItemPtr)
   {
      if (!aItemPtr)
      {
         return 0;
      }
      int acceptedCount = 0;
      if (aItemPtr->GetAbstractItemCount() > 0)
      {
         int count = aItemPtr->GetAbstractItemCount();
         for (int i = 0; i < count; ++i)
         {
            Item* childPtr = dynamic_cast<Item*>(aItemPtr->GetAbstractItem(i));
            acceptedCount += Walk(childPtr);
         }
      }
      else
      {
         // file
         if (mRegEx.indexIn(aItemPtr->GetName()) != -1)
         {
            acceptedCount = 1;
         }
         else
         {
            acceptedCount = 0;
         }
      }
      if (acceptedCount > 0)
      {
         mAcceptedItems.insert(aItemPtr);
      }
      return acceptedCount;
   }

   std::set<Item*> mAcceptedItems;
};

namespace
{
class SearchInputEventFilter : public QObject
{
public:
   explicit SearchInputEventFilter(ProjectBrowser::DockWidget* aParentPtr)
      : QObject(aParentPtr)
      , mParentPtr(aParentPtr)
   {
   }

   bool eventFilter(QObject* obj, QEvent* event) override
   {
      if (event->type() == QEvent::KeyPress)
      {
         QKeyEvent* evt = (QKeyEvent*)event;
         switch (evt->key())
         {
         case Qt::Key_Down:
            mParentPtr->NextSearchResult(1);
            evt->accept();
            return true;
            break;
         case Qt::Key_Up:
            mParentPtr->NextSearchResult(-1);
            evt->accept();
            return true;
            break;
         case Qt::Key_Enter:
         case Qt::Key_Return:
            mParentPtr->SelectSearchResult();
            evt->accept();
            return true;
            break;
         case Qt::Key_Escape:
            mParentPtr->ClearSearch();
            evt->accept();
            return true;
            break;
         }
      }
      return QObject::eventFilter(obj, event);
   }

   ProjectBrowser::DockWidget* mParentPtr;
};
} // namespace


ProjectBrowser::DockWidget::DockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mProjectPtr(nullptr)
   , mFilterProxyPtr(nullptr)
{
   mModelPtr = new Model(this);
   setObjectName("ProjectBrowser");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());

   mUi.fileView->mParentPtr         = this;
   mUi.fileViewFiltered->mParentPtr = this;
   // Left elide is best for long path names.  Files would be best with
   // right elide, but I don't know of a way to change on a per-item basis.
   mUi.fileView->setTextElideMode(Qt::ElideLeft);
   mUi.fileViewFiltered->setTextElideMode(Qt::ElideLeft);
   // Make view more compact
   mUi.fileView->setIndentation(10);
   mUi.fileViewFiltered->setIndentation(10);
   setWindowTitle(widget()->windowTitle());

   mUi.filterText->setAutoCompletion(false);
   mUi.filterText->installEventFilter(new SearchInputEventFilter(this));

   mUi.fileView->setSelectionBehavior(QAbstractItemView::SelectRows);
   mUi.fileView->setHeaderHidden(true);
   mUi.fileView->setDragEnabled(true);
   mUi.fileViewFiltered->setSelectionBehavior(QAbstractItemView::SelectRows);
   mUi.fileViewFiltered->setHeaderHidden(true);
   mUi.fileViewFiltered->setDragEnabled(true);

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ActiveProjectChanged, this, &DockWidget::SetProject);

   connect(mUi.fileView, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.fileViewFiltered, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.filterText, &QComboBox::editTextChanged, this, &DockWidget::FilterChanged);
   connect(mUi.clearFilterBn, &QToolButton::clicked, this, &DockWidget::ClearSearch);
   connect(wizSignals, &wizard::Signals::FileCheckRequested, this, &DockWidget::FileCheck);
   connect(wizSignals, &wizard::Signals::ProjectDirectoriesChanged, this, &DockWidget::DirectorySetChanged);

   mUi.stackedWidget->setCurrentIndex(0);
   mUi.fileView->setModel(mModelPtr);

   mUtCallbacks += wizSignals->CheckFilesForModification.Connect(&DockWidget::CheckForModifications, this);

   SetProject(nullptr);
}

void ProjectBrowser::DockWidget::NextSearchResult(int aDirection)
{
   mUi.fileViewFiltered->MoveCursor(aDirection);
}

bool ProjectBrowser::DockWidget::SelectSearchResult()
{
   bool        opened = false;
   QModelIndex idx    = mUi.fileView->currentIndex();
   if (mFilterProxyPtr && idx.isValid())
   {
      idx           = mFilterProxyPtr->mapToSource(idx);
      Item* itemPtr = dynamic_cast<Item*>(mModelPtr->FindItem(idx));
      if (OpenFile(itemPtr))
      {
         opened = true;
         mUi.filterText->addItem(mUi.filterText->currentText());
         mUi.filterText->clearEditText();
      }
   }
   return opened;
}

bool ProjectBrowser::DockWidget::IsFilteredItem(QModelIndex aIndex)
{
   if (!mFilterProxyPtr)
   {
      return false;
   }
   QModelIndex srcIndex = mFilterProxyPtr->mapToSource(mUi.fileViewFiltered->currentIndex());
   Item*       itemPtr  = dynamic_cast<Item*>(mModelPtr->FindItem(srcIndex));
   if (itemPtr != nullptr)
   {
      if (!itemPtr->IsDir())
      {
         return true;
      }
   }
   return false;
}

void ProjectBrowser::DockWidget::ContextEvent(const QModelIndex& aIndex, QPoint aPos)
{
   Item* itemPtr = FindItem(aIndex);
   if (itemPtr)
   {
      if (itemPtr->IsDir())
      {
         QMenu    menu;
         QAction* collapsePtr = menu.addAction("Collapse All");
         QAction* expandPtr   = menu.addAction("Expand All");
         menu.addSeparator();

         QAction* browsePtr         = menu.addAction("Browse...");
         QAction* newFileActionPtr  = menu.addAction("New File...");
         QAction* removeFromProject = nullptr;

         // Don't allow removal of project directory
         Item* itemParentPtr = dynamic_cast<Item*>(itemPtr->GetAbstractItemParent());
         if (!itemParentPtr->IsDir() && (itemPtr->GetItemIndex() != 0))
         {
            menu.addSeparator();
            removeFromProject = menu.addAction("Remove Directory from Project");
         }

         QAction* actionPtr = menu.exec(aPos);
         if (actionPtr)
         {
            if (actionPtr == collapsePtr)
            {
               CollapseChildren(aIndex);
            }
            else if (actionPtr == expandPtr)
            {
               ExpandChildren(aIndex);
            }
            else if (actionPtr == browsePtr)
            {
               QString path = itemPtr->GetFileInfo()->path();
               QDesktopServices::openUrl(QUrl("file:///" + path));
            }
            else if (actionPtr == newFileActionPtr)
            {
               QString path = itemPtr->GetFileInfo()->absoluteFilePath();
               wizard::ProjectWorkspace::Instance()->CreateA_NewFile(wizard::Project::Instance(),
                                                                     UtPath(path.toStdString()));
            }
            else if (actionPtr == removeFromProject)
            {
               mProjectPtr->RemoveProjectDirectory(itemPtr->GetFileInfo()->absoluteFilePath().toStdString());
            }
         }
      }
      else if (itemPtr->IsFile())
      {
         FileItem*           fileItemPtr = dynamic_cast<FileItem*>(itemPtr);
         const QString&      path        = itemPtr->GetFileInfo()->absoluteFilePath();
         wizard::TextSource* sourcePtr   = mProjectPtr->GetWorkspace()->FindSource(path.toStdString(), true, false);
         QMenu               menu;
         if (sourcePtr)
         {
            mProjectPtr->GetWorkspace()->GetActionManager()->BuildUpTextSourceContextMenu(menu,
                                                                                          sourcePtr,
                                                                                          fileItemPtr->mMainFileIndex);
         }

         menu.exec(aPos);
      }
      else if (itemPtr->IsRoot() && mProjectPtr)
      {
         QMenu menu;
         mProjectPtr->BuildUpContextMenu(menu);
         menu.exec(aPos);
      }
      else if (itemPtr->IsExternal() || itemPtr->IsExternalDir())
      {
         QMenu    menu;
         QAction* collapsePtr = menu.addAction("Collapse All");
         QAction* expandPtr   = menu.addAction("Expand All");
         menu.addSeparator();

         if (itemPtr->IsExternal())
         {
            // Scan include paths. If any paths are not yet added to project, offer to add them
            wizard::ParseResultsTiny* results = mProjectPtr->TinyParseResults();
            if (results)
            {
               bool addSep = true;
               for (size_t i = 0; i < results->mIncludePaths.size(); ++i)
               {
                  UtPath path = mProjectPtr->WorkingDirectory() + results->mIncludePaths[i].Get();
                  if (!mProjectPtr->IsDirectoryInProject(path))
                  {
                     if (addSep)
                     {
                        menu.addSeparator();
                        addSep = false;
                     }
                     QString actionStr =
                        QString("Add Directory \"%1\" to Project").arg(QString::fromStdString(path.GetSystemPath()));
                     auto lambdaVar = mProjectPtr;
                     QObject::connect(menu.addAction(actionStr),
                                      &QAction::triggered,
                                      [path, lambdaVar]() { lambdaVar->AddProjectDirectory(path); });
                  }
               }
            }
         }

         QAction* actionPtr = menu.exec(aPos);
         if (actionPtr)
         {
            if (actionPtr == collapsePtr)
            {
               CollapseChildren(aIndex);
            }
            else if (actionPtr == expandPtr)
            {
               ExpandChildren(aIndex);
            }
         }
      }
   }
}

ProjectBrowser::Item* ProjectBrowser::DockWidget::FindItem(const QModelIndex& aIndex)
{
   QModelIndex idx = aIndex;
   if (mFilterProxyPtr && aIndex.model() == mFilterProxyPtr)
   {
      idx = mFilterProxyPtr->mapToSource(aIndex);
   }
   return dynamic_cast<Item*>(mModelPtr->FindItem(idx));
}

void ProjectBrowser::DockWidget::parseUpdate(bool aFullUpdate)
{
   // Get the include paths
   QMap<QString, QString> newPathMap;
   for (size_t i = 0; i < mProjectPtr->GetParseResults()->mIncludePaths.size(); ++i)
   {
      auto   incPathStr = mProjectPtr->GetParseResults()->mIncludePaths[i].Get();
      UtPath fullPath   = mProjectPtr->WorkingDirectory() + incPathStr;
      if (mProjectPtr && !mProjectPtr->IsDirectoryInProject(fullPath))
      {
         UtPath  incPath     = incPathStr;
         QString incPathQStr = QString::fromStdString(incPath.GetSystemPath());
         incPathQStr         = QDir::fromNativeSeparators(incPathQStr);

         QString fullPathQStr = QString::fromStdString(fullPath.GetSystemPath());
         fullPathQStr         = QDir::fromNativeSeparators(fullPathQStr);
         newPathMap.insert(incPathQStr, fullPathQStr);
      }
   }
   QString workingDirectory = QString::fromStdString(mProjectPtr->WorkingDirectory().GetSystemPath());
   workingDirectory         = QDir::fromNativeSeparators(workingDirectory);
   mModelPtr->root()->UpdateIncludePaths(workingDirectory, newPathMap);

   // Update the included files
   QSet<QString> newFileSet;
   auto          includes = mProjectPtr->GetParseResults()->mFirstIncludes;
   for (auto&& it : includes)
   {
      QString inc = QString::fromStdString(it.first->GetFilePath().GetSystemPath());
      inc         = QDir::fromNativeSeparators(inc);
      newFileSet.insert(inc);
      // mModelPtr->root()->includedFileInserted(inc);
   }

   mModelPtr->root()->UpdateIncludedFiles(newFileSet);
}

void ProjectBrowser::DockWidget::CheckForModifications()
{
   mModelPtr->root()->Rescan();
}

void ProjectBrowser::DockWidget::ClearSearch()
{
   mUi.filterText->clearEditText();
}

void ProjectBrowser::DockWidget::FileCheck()
{
   mModelPtr->root()->Rescan();
}

void ProjectBrowser::DockWidget::DirectorySetChanged(const std::vector<UtPath>& aDirectories)
{
   mUi.fileView->expandToDepth(2);
}

void ProjectBrowser::DockWidget::ItemActivated(const QModelIndex& aIndex)
{
   if (!mModelPtr)
   {
      return;
   }
   Item* itemPtr = FindItem(aIndex);
   if (itemPtr && !itemPtr->IsDir())
   {
      OpenFile(itemPtr);
   }
}

void ProjectBrowser::DockWidget::FilterChanged(const QString& aFilterText)
{
   if (aFilterText.isEmpty())
   {
      mUi.stackedWidget->setCurrentIndex(0);
      mUi.fileViewFiltered->setModel(nullptr);
      delete mFilterProxyPtr;
      mFilterProxyPtr = nullptr;
      return;
   }
   if (!mFilterProxyPtr)
   {
      mFilterProxyPtr = new ProxyModel(mModelPtr);
      mUi.fileViewFiltered->setModel(mFilterProxyPtr);
   }

   mFilterProxyPtr->mRegEx = wizard::Util::SimpleRegex(aFilterText);
   mFilterProxyPtr->Update();
   mUi.fileViewFiltered->expandAll();
   mUi.stackedWidget->setCurrentIndex(1);
}

void ProjectBrowser::DockWidget::SetProject(wizard::Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
   ClearSearch();
   if (mProjectPtr)
   {
      mUtCallbacks += mProjectPtr->ParseUpdatedDeferred.Connect(&DockWidget::parseUpdate, this);
   }

   mModelPtr->root()->Clear();
   mUi.fileView->setRowHidden(0, QModelIndex(), mProjectPtr == nullptr);
}

bool ProjectBrowser::DockWidget::OpenFile(Item* aItemPtr)
{
   if (aItemPtr && aItemPtr->IsFile())
   {
      mProjectPtr->OpenFile(aItemPtr->GetFileInfo()->absoluteFilePath().toStdString());
   }
   return false;
}

void ProjectBrowser::DockWidget::CurrentChanged(const QModelIndex& aIndex)
{
   //    // Current item has changed, update the action context to point to the item's context
   //
   //    // Remove existing links
   //    for (std::set<ViActionContext*>::iterator i = mTemporaryContexts.begin(); i != mTemporaryContexts.end(); ++i)
   //    {
   //       RemoveSubContext(*i);
   //       RemoveSuperContext(*i);
   //    }
   //    mTemporaryContexts.clear();
   //
   //    ViScenarioBrowserItem* itemPtr = FindItem(aIndex);
   //    if (itemPtr && !itemPtr->IsDir())
   //    {
   //       UtPath path(itemPtr->GetInfo().absoluteFilePath().toStdString());
   //       wizard::TextSource* sourcePtr = mProjectPtr->FindSource(path.GetSystemPath(), false);
   //       if (sourcePtr)
   //       {
   //          AddSuperContext(sourcePtr);
   //          AddSubContext(sourcePtr);
   //          mTemporaryContexts.insert(sourcePtr);
   //       }
   //    }
}

void ProjectBrowser::DockWidget::CollapseChildren(const QModelIndex& aIndex)
{
   if (!aIndex.isValid())
   {
      return;
   }

   int childCount = aIndex.model()->rowCount(aIndex);
   for (int i = 0; i < childCount; ++i)
   {
      const QModelIndex& child = aIndex.child(i, 0);
      // Recursively call the function for each child node.
      CollapseChildren(child);
   }

   mUi.fileView->collapse(aIndex);
}

void ProjectBrowser::DockWidget::ExpandChildren(const QModelIndex& aIndex)
{
   if (!aIndex.isValid())
   {
      return;
   }

   int childCount = aIndex.model()->rowCount(aIndex);
   for (int i = 0; i < childCount; ++i)
   {
      const QModelIndex& child = aIndex.child(i, 0);
      // Recursively call the function for each child node.
      ExpandChildren(child);
   }

   mUi.fileView->expand(aIndex);
}
