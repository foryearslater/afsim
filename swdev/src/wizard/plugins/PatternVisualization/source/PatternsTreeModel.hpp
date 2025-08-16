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

#ifndef PATTERNSTREEMODEL_HPP
#define PATTERNSTREEMODEL_HPP

#include <memory>

#include <QAbstractItemModel>
#include <QModelIndex>

namespace PatternVisualizer
{
class Pattern;
class PatternsTreeItem;
class Session;
class TreeItem;

class PatternsTreeModel : public QAbstractItemModel
{
public:
   explicit PatternsTreeModel(Session* aSessionPtr, QObject* aParentPtr);
   ~PatternsTreeModel();

   void          ClearTree();
   int           columnCount(const QModelIndex& aIndex) const;
   QModelIndex   index(int aRow, int aColumn, const QModelIndex& aParent = QModelIndex()) const;
   QModelIndex   parent(const QModelIndex& aIndex) const;
   int           rowCount(const QModelIndex& aIndex) const;
   QVariant      data(const QModelIndex& aIndex, int aRole) const;
   Qt::ItemFlags flags(const QModelIndex& aIndex) const;
   bool          setData(const QModelIndex& aIndex, const QVariant& aValue, int aRole);
   Pattern*      GetPatternAtIndex(const QModelIndex& aIndex);

private:
   TreeItem* getRootItem() const { return mRootItemPtr.get(); }
   TreeItem* getItem(const QModelIndex& aIndex) const;

   Session*                  mSessionPtr;
   std::unique_ptr<TreeItem> mRootItemPtr;
};

class TreeItem
{
public:
   TreeItem(Session*               aSessionPtr,
            const QList<QVariant>& aData       = QList<QVariant>() << "",
            TreeItem*              aParentPtr  = nullptr,
            Pattern*               aPatternPtr = nullptr);
   ~TreeItem();

   void      appendChild(TreeItem* aChildPtr);
   TreeItem* appendChild(const QString& aName);
   TreeItem* child(int aRow);
   int       childCount() const;
   int       columnCount() const;
   QVariant  data(int aColumn) const;
   void      removeAllChildren();
   TreeItem* parent();
   int       row() const;

   Pattern* GetPattern();
   bool     isCheckable() const { return mCheckable; }
   bool     isChecked() const { return mChecked; }
   void     SetChecked(bool aChecked, bool aUserInitiated = true);
   void     SetPattern(Pattern* aPatternPtr) { mPatternPtr = aPatternPtr; }

private:
   QList<TreeItem*> mChildItems;
   QList<QVariant>  mItemData;
   TreeItem*        mParentItemPtr;
   Pattern*         mPatternPtr;
   Session*         mSessionPtr;
   bool             mCheckable;
   bool             mChecked;
};
} // namespace PatternVisualizer

#endif // PATTERNSTREEMODEL_HPP
