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
#include "AbstractModel.hpp"

#include <assert.h>

void wizard::AbstractModel::SetRoot(AbstractModelItem* aItemPtr)
{
   if (mRootPtr)
   {
      mRootPtr->mModelPtr = nullptr;
      mRootPtr            = nullptr;
   }
   if (aItemPtr)
   {
      mRootPtr             = aItemPtr;
      mRootPtr->mModelPtr  = this;
      mRootPtr->mItemIndex = 0;
   }
}

void wizard::AbstractModel::BeginInsert(AbstractModelItem& aParent, int aIndex)
{
   assert(mChangeMode == 0);
   mChangeMode = 1;
   beginInsertRows(GetRowIndex(aParent), aIndex, aIndex);
}

void wizard::AbstractModel::EndInsert()
{
   assert(mChangeMode == 1);
   mChangeMode = 0;
   endInsertRows();
}

void wizard::AbstractModel::BeginRemove(AbstractModelItem& aParent, int aIndex, int aCount)
{
   assert(mChangeMode == 0);
   mChangeMode = 2;
   beginRemoveRows(GetRowIndex(aParent), aIndex, aIndex + aCount - 1);
}

void wizard::AbstractModel::BeginMove(AbstractModelItem& aSource,
                                      int                aSourceIndex,
                                      int                aCount,
                                      AbstractModelItem& aDestination,
                                      int                aDestinationIndex)
{
   assert(mChangeMode == 0);
   mChangeMode = 3;
   // rowsAboutToBeMoved(GetRowIndex(aSource), aSourceIndex, aSourceIndex + aCount - 1, GetRowIndex(aDestination), aDestinationIndex);
   beginMoveRows(GetRowIndex(aSource), aSourceIndex, aSourceIndex + aCount - 1, GetRowIndex(aDestination), aDestinationIndex);
}

wizard::AbstractModelItem* wizard::AbstractModel::FindItem(const QModelIndex& idx) const
{
   if (idx.isValid())
   {
      AbstractModelItem* p = static_cast<AbstractModelItem*>(idx.internalPointer());
      assert(p && "Invalid index");
      if (p)
      {
         return ((size_t)idx.row() < p->mItemChildren.size()) ? p->GetAbstractItem(idx.row()) : nullptr;
      }
   }
   else
   {
      return mRootPtr;
   }

   return nullptr;
}

int wizard::AbstractModel::rowCount(const QModelIndex& parent) const
{
   AbstractModelItem* itemPtr = FindItem(parent);
   if (itemPtr)
   {
      return (int)itemPtr->mItemChildren.size();
   }
   return 0;
}

int wizard::AbstractModel::columnCount(const QModelIndex& parent) const
{
   AbstractModelItem* itemPtr = FindItem(parent);
   if (itemPtr && !itemPtr->mItemChildren.empty())
   {
      return 1;
   }
   return 0;
}

QModelIndex wizard::AbstractModel::index(int row, int column, const QModelIndex& parent) const
{
   AbstractModelItem* itemPtr = FindItem(parent);
   if (column != 0 || !itemPtr)
      return QModelIndex();
   return createIndex(row, column, itemPtr);
}

QModelIndex wizard::AbstractModel::parent(const QModelIndex& child) const
{
   if (!child.isValid())
      return QModelIndex();
   AbstractModelItem* parentPtr = static_cast<AbstractModelItem*>(child.internalPointer());
   if (parentPtr != nullptr)
   {
      if (parentPtr->GetAbstractItemParent())
      {
         return createIndex(parentPtr->mItemIndex, 0, parentPtr->GetAbstractItemParent());
      }
   }
   return QModelIndex();
}

QModelIndex wizard::AbstractModel::GetRowIndex(AbstractModelItem& aRow) const
{
   assert(aRow.mItemIndex >= 0);
   if (&aRow == mRootPtr)
   {
      return QModelIndex();
   }
   else
   {
      return createIndex(aRow.mItemIndex, 0, aRow.GetAbstractItemParent());
   }
}

void wizard::AbstractModel::FakeInsert(AbstractModelItem& aParent)
{
   BeginInsert(aParent, 0);
   EndInsert();
}

void wizard::AbstractModel::EndRemove()
{
   assert(mChangeMode == 2);
   mChangeMode = 0;
   endRemoveRows();
}

void wizard::AbstractModel::EndMove()
{
   assert(mChangeMode == 3);
   mChangeMode = 0;
   endMoveRows();
}

wizard::AbstractModelItem::AbstractModelItem()
   : mItemParentPtr()
   , mModelPtr()
   , mItemIndex(-1)
{
}

wizard::AbstractModelItem::~AbstractModelItem()
{
   assert(!IsInModel());
   RemoveAllAbstractItems(true);
}

//! Add a child item before aIndex.  If aIndex == -1, insert at end.
void wizard::AbstractModelItem::AddAbstractItem(AbstractModelItem* aItemPtr, int aIndex)
{
   if (aIndex == -1)
   {
      aIndex = (int)mItemChildren.size();
   }
   assert(aIndex >= 0 && aIndex <= (int)mItemChildren.size());

   // assert(mModelPtr);

   if (IsInModel())
      mModelPtr->BeginInsert(*this, aIndex);
   aItemPtr->SetAbstractItemParent(this);
   aItemPtr->mItemIndex = aIndex;
   // aItemPtr->SetModel(mModelPtr);
   bool addedToModel = aItemPtr->BeginSetModel(mModelPtr);
   mItemChildren.insert(mItemChildren.begin() + aIndex, aItemPtr);
   ReIndexChildrenP(aIndex + 1);
   if (IsInModel())
   {
      mModelPtr->EndInsert();
   }
   if (addedToModel)
   {
      aItemPtr->FinalizeSetModel();
   }
}

wizard::AbstractModelItem* wizard::AbstractModelItem::RemoveAbstractItem(int aIndex)
{
   PreRemoveAbstractItem(aIndex);

   int                idx     = aIndex;
   AbstractModelItem* itemPtr = mItemChildren[idx];
   if (IsInModel())
   {
      mModelPtr->BeginRemove(*this, idx);
   }

   mItemChildren.erase(mItemChildren.begin() + idx);
   ReIndexChildrenP(idx);
   itemPtr->SetAbstractItemParent(nullptr);
   itemPtr->mItemIndex = -1;
   itemPtr->mModelPtr  = nullptr;

   if (IsInModel())
   {
      mModelPtr->EndRemove();
   }

   return itemPtr;
}

void wizard::AbstractModelItem::RemoveAbstractItem(AbstractModelItem* aItemPtr)
{
   assert(aItemPtr->mItemIndex != -1);
   assert(mItemChildren[aItemPtr->mItemIndex] == aItemPtr);
   RemoveAbstractItem(aItemPtr->mItemIndex);
}

// Move aItemPtr from its current location to a child of this at aIndex
void wizard::AbstractModelItem::MoveAbstractItem(AbstractModelItem* aItemPtr, int aIndex /*=-1*/)
{
   // Only do special move logic if we are in an item model
   if (mModelPtr == nullptr)
   {
      if (aItemPtr->GetAbstractItemParent())
      {
         aItemPtr->GetAbstractItemParent()->RemoveAbstractItem(aItemPtr);
      }
      AddAbstractItem(aItemPtr, aIndex);
      return;
   }

   assert(mModelPtr);

   if (aIndex == -1)
   {
      aIndex = (int)mItemChildren.size();
   }
   assert(aIndex >= 0 && aIndex <= (int)mItemChildren.size());

   int                oldIndex     = aItemPtr->GetItemIndex();
   AbstractModelItem* oldParentPtr = aItemPtr->GetAbstractItemParent();
   if ((oldParentPtr == this) && (aIndex == (int)mItemChildren.size()))
   {
      // adjust the index if the parent is the same and we are moving to the end of the list
      aIndex--;
   }

   // if we are not moving anywhere, abort
   if (oldParentPtr == this && aIndex == oldIndex)
   {
      return;
   }

   PreRemoveAbstractItem(oldIndex);

   QModelIndex sourceIndex;
   if (IsInModel())
   {
      mModelPtr->BeginMove(*oldParentPtr, oldIndex, 1, *this, aIndex);
      sourceIndex = mModelPtr->GetRowIndex(*aItemPtr);
   }

   oldParentPtr->mItemChildren.erase(oldParentPtr->mItemChildren.begin() + oldIndex);
   oldParentPtr->ReIndexChildrenP(oldIndex);
   aItemPtr->SetAbstractItemParent(this);
   aItemPtr->mItemIndex = aIndex;
   // aItemPtr->SetModel(mModelPtr);
   bool addedToModel = aItemPtr->BeginSetModel(mModelPtr);
   mItemChildren.insert(mItemChildren.begin() + aIndex, aItemPtr);
   ReIndexChildrenP(aIndex + 1);

   if (IsInModel())
   {
      mModelPtr->EndMove();
   }
   if (addedToModel)
   {
      aItemPtr->FinalizeSetModel();
   }
}

void wizard::AbstractModelItem::RemoveAllAbstractItems(bool aDelete)
{
   if (mItemChildren.empty())
      return;
   if (IsInModel())
      mModelPtr->BeginRemove(*this, 0, (int)mItemChildren.size());
   for (size_t i = 0; i < mItemChildren.size(); ++i)
   {
      mItemChildren[i]->SetAbstractItemParent(nullptr);
      mItemChildren[i]->mItemIndex = -1;
      mItemChildren[i]->mModelPtr  = nullptr;
      if (aDelete)
      {
         // Allow overriding delete
         mItemChildren[i]->RequestItemDelete();
      }
   }
   mItemChildren.clear();
   if (IsInModel())
      mModelPtr->EndRemove();
}

void wizard::AbstractModelItem::ReIndexChildrenP(int aFirstIndex)
{
   for (int i = aFirstIndex; i < (int)mItemChildren.size(); ++i)
   {
      mItemChildren[i]->mItemIndex = i;
   }
}

bool wizard::AbstractModelItem::BeginSetModel(AbstractModel* aModelPtr)
{
   if (mModelPtr != aModelPtr)
   {
      mModelPtr = aModelPtr;
      if (!mModelPtr)
      {
         return false;
      }
      return true;
   }
   return false;
}

void wizard::AbstractModelItem::FinalizeSetModel()
{
   assert(mModelPtr != nullptr && mItemIndex != -1);
   if (!mItemChildren.empty())
   {
      mModelPtr->beginInsertRows(mModelPtr->GetRowIndex(*this), 0, (int)mItemChildren.size() - 1);
      for (auto&& it : mItemChildren)
      {
         bool reallyAdded = it->BeginSetModel(mModelPtr);
         assert(reallyAdded);
         (void)reallyAdded;
      }
      mModelPtr->endInsertRows();
      for (auto&& it : mItemChildren)
      {
         it->FinalizeSetModel();
      }
   }
   AddedToModel();
}

//! Informs Qt that the item's data has changed
void wizard::AbstractModelItem::AbstractItemChanged()
{
   if (mModelPtr)
   {
      QModelIndex idx = mModelPtr->GetRowIndex(*this);
      mModelPtr->dataChanged(idx, idx);
   }
}

void wizard::AbstractModelItem::RequestItemDelete()
{
   delete this;
}

void wizard::AbstractModelItem::RemoveItemFromParent()
{
   if (mItemParentPtr)
   {
      mItemParentPtr->RemoveAbstractItem(this);
   }
}
