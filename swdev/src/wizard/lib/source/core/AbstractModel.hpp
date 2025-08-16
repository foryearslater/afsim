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

#ifndef ABSTRACTMODEL_HPP
#define ABSTRACTMODEL_HPP

#include <vector>

#include <QAbstractItemModel>

#include "ViExport.hpp"

namespace wizard
{
class AbstractModelItem;

//! Implements a model of a tree where each item in the tree is derived from AbstractModelItem
class VI_EXPORT AbstractModel : public QAbstractItemModel
{
public:
   friend class AbstractModelItem;

   AbstractModel()
      : mRootPtr()
      , mChangeMode(0)
   {
   }
   void SetRoot(AbstractModelItem* aItemPtr);

   AbstractModelItem* FindItem(const QModelIndex& idx) const;

   int rowCount(const QModelIndex& parent) const override;

   int         columnCount(const QModelIndex& parent) const override;
   QModelIndex index(int row, int column, const QModelIndex& parent) const override;

   QModelIndex parent(const QModelIndex& child) const override;

   QModelIndex GetRowIndex(AbstractModelItem& aRow) const;

   void FakeInsert(AbstractModelItem& aParent);

protected:
   void BeginInsert(AbstractModelItem& aParent, int aIndex);
   void EndInsert();
   void BeginRemove(AbstractModelItem& aParent, int aIndex, int aCount = 1);
   void EndRemove();
   void BeginMove(AbstractModelItem& aSource, int aSourceIndex, int aCount, AbstractModelItem& aDestination, int aDestinationIndex);
   void EndMove();

   AbstractModelItem* mRootPtr;
   int                mChangeMode;
};

//! Objects inheriting this class may be added to a AbstractModel
class VI_EXPORT AbstractModelItem : public QObject
{
public:
   friend class AbstractModel;

   AbstractModelItem();
   ~AbstractModelItem() override;

   AbstractModelItem* GetAbstractItemParent() const { return mItemParentPtr; }

   int GetItemIndex() { return mItemIndex; }
   // Returns the model that contains this item.  May be null
   AbstractModel*     GetModel() const { return mModelPtr; }
   AbstractModelItem* GetAbstractItem(int aIdx) const { return mItemChildren[aIdx]; }
   int                GetAbstractItemCount() const { return (int)mItemChildren.size(); }

   // Warning: These mutators should be used carefully.  Normally these are called by the

   // derived class.
   void               AddAbstractItem(AbstractModelItem* aItemPtr, int aIndex = -1);
   void               RemoveAbstractItem(AbstractModelItem* aItemPtr);
   AbstractModelItem* RemoveAbstractItem(int aIndex);
   void               RemoveAllAbstractItems(bool aDelete = false);

   void MoveAbstractItem(AbstractModelItem* aItemPtr, int aIndex = -1);

   void AbstractItemChanged();

   void RemoveItemFromParent();

protected:
   // By default, when an abstract item is deleted, it deletes all children.
   // Override this method to change auto-delete behavior
   virtual void RequestItemDelete();
   virtual void PreRemoveAbstractItem(int aIndex) {}
   virtual void AbstractItemParentChanged() {}
   virtual void AddedToModel() {}
   void         SetAbstractItemParent(AbstractModelItem* aParent)
   {
      if (mItemParentPtr != aParent)
      {
         mItemParentPtr = aParent;
         AbstractItemParentChanged();
      }
   }

   bool BeginSetModel(AbstractModel* aModelPtr);

   void FinalizeSetModel();
   bool IsInModel() const { return mModelPtr != nullptr; }

private:
   void ReIndexChildrenP(int aFirstIndex);

   AbstractModelItem*              mItemParentPtr;
   AbstractModel*                  mModelPtr;
   int                             mItemIndex;
   std::vector<AbstractModelItem*> mItemChildren;
};
} // namespace wizard
#endif
