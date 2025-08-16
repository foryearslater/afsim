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

#include "View.hpp"

#include <QContextMenuEvent>
#include <QMimeData>

#include "DockWidget.hpp"
#include "Item.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"

// -----------------------------------------------------------------------------
ProjectBrowser::View::View(QWidget* aParent)
   : QTreeView(aParent)
   , mExpanding(false)
   , mParentPtr(nullptr)
{
}

void ProjectBrowser::View::MoveCursor(int aDirection)
{
   QModelIndex idx   = currentIndex();
   bool        moved = false;
   if (aDirection == -2)
   {
      moved      = true;
      idx        = moveCursor(MoveHome, Qt::NoModifier);
      aDirection = 1;
   }

   while (true)
   {
      if (moved && (!idx.isValid() || mParentPtr->IsFilteredItem(idx)))
      {
         setCurrentIndex(idx);
         break;
      }
      QModelIndex newIdx;
      switch (aDirection)
      {
      case -1:
         newIdx = moveCursor(MoveUp, Qt::NoModifier);
         break;
      case 1:
         newIdx = moveCursor(MoveDown, Qt::NoModifier);
         break;
      }
      if (newIdx == idx)
      {
         break;
      }
      moved = true;
      idx   = newIdx;
      setCurrentIndex(idx);
   }
}

void ProjectBrowser::View::contextMenuEvent(QContextMenuEvent* event)
{
   mParentPtr->ContextEvent(currentIndex(), event->globalPos());
}

void ProjectBrowser::View::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
   mParentPtr->CurrentChanged(current);
   QTreeView::currentChanged(current, previous);
}

void ProjectBrowser::View::rowsInserted(const QModelIndex& parent, int start, int end)
{
   QTreeView::rowsInserted(parent, start, end);
}

// -----------------------------------------------------------------------------

ProjectBrowser::Model::Model(QObject* aParent)
{
   connect(wizSignals, &wizard::Signals::ProjectDirectoriesChanged, this, &Model::DirectorySetChanged);
   connect(wizSignals, &wizard::Signals::ProjectStartupFilesChanged, this, &Model::ProjectStartupFilesChanged);
   connect(wizSignals, &wizard::Signals::SourceModifiedStateChange, this, &Model::SourceModifiedStateChange);

   setParent(aParent);
   SetRoot(new HiddenRootItem());

   mActualRoot = new RootItem();
   mRootPtr->AddAbstractItem(mActualRoot);
}

ProjectBrowser::Model::~Model()
{
   wizard::AbstractModelItem* root = mRootPtr;
   SetRoot(nullptr);
   delete root;
}

Qt::ItemFlags ProjectBrowser::Model::flags(const QModelIndex& index) const
{
   unsigned int flagBits = wizard::AbstractModel::flags(index);
   flagBits |= Qt::ItemIsDragEnabled;
   return (Qt::ItemFlags)flagBits;
}

QVariant ProjectBrowser::Model::data(const QModelIndex& index, int role) const
{
   wizard::AbstractModelItem* ami     = FindItem(index);
   Item*                      itemPtr = dynamic_cast<Item*>(ami);
   if (!itemPtr)
   {
      return QVariant();
   }

   switch (role)
   {
   case Qt::FontRole:
      return itemPtr->GetFontRole();
   case Qt::DisplayRole:
      return itemPtr->GetDisplayRole();
   case Qt::DecorationRole:
      return itemPtr->GetDecorationRole();
   case Qt::ToolTipRole:
      return itemPtr->GetToolTip();
   default:
      return QVariant();
   }
   return QVariant();
}

void ProjectBrowser::Model::DirectorySetChanged(const std::vector<UtPath>& aDirectories)
{
   QStringList paths;
   for (const auto& directory : aDirectories)
   {
      QDir dir(QString::fromStdString(directory.GetSystemPath()));
      paths.push_back(dir.canonicalPath());
   }
   root()->SetDirectories(paths);
}

void ProjectBrowser::Model::ProjectStartupFilesChanged(const std::vector<UtPath>& aMainFiles)
{
   QStringList paths;
   for (const auto& mainFile : aMainFiles)
   {
      QFileInfo info(QString::fromStdString(mainFile.GetSystemPath()));
      paths.push_back(info.absoluteFilePath());
   }
   root()->SetStartupFiles(paths);
}

QMimeData* ProjectBrowser::Model::mimeData(const QModelIndexList& indexes) const
{
   if (indexes.size() == 1)
   {
      Item* itemPtr = dynamic_cast<Item*>(this->FindItem(indexes[0]));
      if (itemPtr)
      {
         QList<QUrl> urls;
         QFileInfo*  fileInfoPtr = itemPtr->GetFileInfo();
         if (fileInfoPtr)
         {
            urls << QUrl::fromLocalFile(fileInfoPtr->absoluteFilePath());
            auto* mimePtr = new QMimeData();
            mimePtr->setUrls(urls);
            return mimePtr;
         }
      }
   }
   return nullptr;
}

void ProjectBrowser::Model::SourceModifiedStateChange(wizard::TextSource* aSourcePtr)
{
   QString path    = QDir::fromNativeSeparators(QString::fromStdString(aSourcePtr->GetSystemPath()));
   Item*   itemPtr = root()->FindFromAbsolutePath(path);
   if (itemPtr)
   {
      itemPtr->AbstractItemChanged();
   }
}

ProjectBrowser::RootItem* ProjectBrowser::Model::root()
{
   return mActualRoot;
}
