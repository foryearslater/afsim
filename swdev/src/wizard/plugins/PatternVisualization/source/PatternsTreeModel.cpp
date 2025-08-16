// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PatternsTreeModel.hpp"

#include <QTreeView>

#include "Pattern.hpp"
#include "PatternData.hpp"
#include "Session.hpp"
#include "WsfStandardApplication.hpp"


namespace PatternVisualizer
{
PatternsTreeModel::PatternsTreeModel(Session* aSessionPtr, QObject* aParentPtr)
   : QAbstractItemModel(aParentPtr)
   , mSessionPtr{aSessionPtr}
   , mRootItemPtr{new TreeItem{aSessionPtr}}
{
   mSessionPtr->SetPatternTreeRoot(getRootItem());
}

PatternsTreeModel::~PatternsTreeModel() {}

void PatternsTreeModel::ClearTree()
{
   beginResetModel();
   mRootItemPtr->removeAllChildren();
   endResetModel();
}

int PatternsTreeModel::columnCount(const QModelIndex& aIndex) const
{
   TreeItem* treeItem = (aIndex.isValid() ? static_cast<TreeItem*>(aIndex.internalPointer()) : getRootItem());

   return treeItem->columnCount();
}

QModelIndex PatternsTreeModel::index(int aRow, int aColumn, const QModelIndex& aParent) const
{
   if (!hasIndex(aRow, 0, aParent))
   {
      return QModelIndex();
   }

   TreeItem* treeItemPtr = (aParent.isValid() ? static_cast<TreeItem*>(aParent.internalPointer()) : getRootItem());

   TreeItem* childItemPtr = treeItemPtr->child(aRow);
   return (childItemPtr == nullptr ? QModelIndex() : createIndex(aRow, aColumn, childItemPtr));
}

QModelIndex PatternsTreeModel::parent(const QModelIndex& aIndex) const
{
   if (!aIndex.isValid())
   {
      return QModelIndex();
   }

   TreeItem* treeItemPtr   = static_cast<TreeItem*>(aIndex.internalPointer());
   TreeItem* parentItemPtr = treeItemPtr->parent();

   return (!parentItemPtr || parentItemPtr == getRootItem() ? QModelIndex() :
                                                              createIndex(parentItemPtr->row(), 0, parentItemPtr));
}

int PatternsTreeModel::rowCount(const QModelIndex& aIndex) const
{
   if (aIndex.column() > 0)
   {
      return 0;
   }

   TreeItem* treeItemPtr = (aIndex.isValid() ? static_cast<TreeItem*>(aIndex.internalPointer()) : getRootItem());

   return treeItemPtr->childCount();
}

QVariant PatternsTreeModel::data(const QModelIndex& aIndex, int aRole) const
{
   if (!aIndex.isValid())
   {
      return QVariant();
   }

   TreeItem* treeItemPtr = static_cast<TreeItem*>(aIndex.internalPointer());

   if (aRole == Qt::CheckStateRole && aIndex.column() == 0 && treeItemPtr->isCheckable())
   {
      return static_cast<int>(treeItemPtr->isChecked() ? Qt::Checked : Qt::Unchecked);
   }
   else if (aRole == Qt::DisplayRole)
   {
      return treeItemPtr->data(aIndex.column());
   }
   else
   {
      return QVariant();
   }
}

Qt::ItemFlags PatternsTreeModel::flags(const QModelIndex& aIndex) const
{
   if (!aIndex.isValid())
   {
      return 0;
   }

   Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

   if (aIndex.column() == 0)
   {
      flags |= Qt::ItemIsUserCheckable;
   }
   return flags;
}

TreeItem* PatternsTreeModel::getItem(const QModelIndex& aIndex) const
{
   if (aIndex.isValid())
   {
      TreeItem* treeItem = static_cast<TreeItem*>(aIndex.internalPointer());
      if (treeItem != nullptr)
      {
         return treeItem;
      }
   }
   return getRootItem();
}

Pattern* PatternsTreeModel::GetPatternAtIndex(const QModelIndex& aIndex)
{
   Pattern*  retvalPtr{nullptr};
   TreeItem* itemPtr = getItem(aIndex);
   if (itemPtr)
   {
      retvalPtr = itemPtr->GetPattern();
   }
   return retvalPtr;
}

bool PatternsTreeModel::setData(const QModelIndex& aIndex, const QVariant& aValue, int aRole)
{
   TreeItem* treeItemPtr = getItem(aIndex);
   if (aRole != Qt::CheckStateRole || !treeItemPtr->isCheckable())
   {
      return false;
   }

   bool checked = (static_cast<Qt::CheckState>(aValue.toInt()) == Qt::Checked);
   treeItemPtr->SetChecked(checked);

   QVector<int> roles{aRole};
   emit         dataChanged(aIndex, aIndex, roles);

   return true;
}

//################################################################################################

TreeItem::TreeItem(Session* aSessionPtr, const QList<QVariant>& aData, TreeItem* aParentPtr, Pattern* aPatternPtr)
   : mChildItems{}
   , mItemData(aData)
   , mParentItemPtr(aParentPtr)
   , mPatternPtr(aPatternPtr)
   , mSessionPtr{aSessionPtr}
   , mCheckable(aPatternPtr != nullptr)
   , mChecked(false)
{
}

TreeItem::~TreeItem()
{
   // Delete the pattern if applicable
   if (mPatternPtr != nullptr)
   {
      if (isCheckable() && isChecked())
      {
         // Set the "checked" flag so that the pattern image will be erased if necessary
         SetChecked(false, false);
      }

      delete mPatternPtr;
      mPatternPtr = nullptr;
   }

   // Delete the child items
   while (!mChildItems.isEmpty())
   {
      TreeItem* child = mChildItems.takeLast();
      delete child;
   }
}

void TreeItem::appendChild(TreeItem* aChildPtr)
{
   mChildItems.append(aChildPtr);
}

TreeItem* TreeItem::appendChild(const QString& aName)
{
   TreeItem* childPtr = new TreeItem(mSessionPtr, QList<QVariant>() << aName.toStdString().c_str(), this);
   mChildItems.append(childPtr);
   return childPtr;
}

TreeItem* TreeItem::child(int aRow)
{
   return mChildItems.value(aRow);
}

int TreeItem::childCount() const
{
   return mChildItems.count();
}

int TreeItem::columnCount() const
{
   return mItemData.count();
}

QVariant TreeItem::data(int aColumn) const
{
   return mItemData.value(aColumn);
}

Pattern* TreeItem::GetPattern()
{
   Pattern* retvalPtr{nullptr};
   if (mPatternPtr)
   {
      retvalPtr = mPatternPtr;
   }
   else
   {
      if (mParentItemPtr)
      {
         return mParentItemPtr->GetPattern();
      }
   }
   return retvalPtr;
}

TreeItem* TreeItem::parent()
{
   return mParentItemPtr;
}

void TreeItem::removeAllChildren()
{
   while (!mChildItems.isEmpty())
   {
      TreeItem* itemPtr = mChildItems.takeLast();
      delete itemPtr;
   }
}

int TreeItem::row() const
{
   try
   {
      if (mParentItemPtr != nullptr && mParentItemPtr->mChildItems.contains(const_cast<TreeItem*>(this)))
      {
         return mParentItemPtr->mChildItems.indexOf(const_cast<TreeItem*>(this));
      }
   }
   catch (UtException&)
   {
      QString name       = mItemData[0].toString();
      QString parentName = mParentItemPtr->mItemData[0].toString();
      qDebug() << "Unable to get row for " << name << " in " << parentName;
   }
   return -1;
}

void TreeItem::SetChecked(bool aChecked, bool aUserInitiated)
{
   mChecked = aChecked;
   mSessionPtr->ShowHidePattern(mPatternPtr, aChecked, aUserInitiated);
}
} // namespace PatternVisualizer
