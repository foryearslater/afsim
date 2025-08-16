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

#ifndef WSFDEBUGVALUEMODEL_HPP
#define WSFDEBUGVALUEMODEL_HPP

#include <QAbstractItemModel>
#include <QPointer>
#include <QTimer>

#include "ScriptDebugger.hpp"
#include "UtCallbackHolder.hpp"
#include "UtQtAbstractTreeModel.hpp"
#include "UtReferenceTracked.hpp"
#include "WsfScriptDebugger.hpp"

class WsfXIO_Connection;

namespace SimulationManager
{
class WsfDebugValueQuery
{
public:
   explicit WsfDebugValueQuery();

   int  GetRequestId();
   void NewRequestId();

   void* mContext;
   int   mStackFrame;
   // Variable name to query or expression
   std::string      mVariableName;
   std::vector<int> mEntryAddress;

private:
   // Each request is given a unique ID
   int mRequestId;
};

class WsfDebugValueModelVariableValue
{
public:
   explicit WsfDebugValueModelVariableValue();

   QVariant ToVariant();

   int     mBreakSequenceNumber; // when this data was refreshed
   int     mLastRequestSequenceNumber;
   int     mExpressionEvaluationId;
   QString mName;
   QString mValue;
   QString mType;
   int     mChildEntryCount;
   bool    mLookupFailed;
};

class WsfDebugValueModelNode;

class WsfDebugValueModelQueryData;
class WsfDebugValueModelData : public QObject, public UtReferenceTracked
{
   Q_OBJECT

public:
   explicit WsfDebugValueModelData(wizard::ScriptDebugger* aDebuggerPtr);
   ~WsfDebugValueModelData() override { UtReferenceTracked::FreeReference(); }
   void requestData(WsfDebugValueModelNode* aParent, int aRow);
   void DebugAtBreak(wizard::ScriptDebugger* aDebugger, bool aIsAtBreak);
   void RequestDataTimeout();
   void DebugSessionActive(wizard::ScriptDebugger* aDebugger, bool aIsActive);
   void DebugCallStackFrameSwitch(wizard::ScriptDebugger*, int);

   void UpdateFrame(int aIndex);

   QString mRootName;

   wizard::ScriptDebugger* mDebuggerPtr;

   QTimer                                    mRequestTimer;
   WsfXIO_Connection*                        mConnectionPtr;
   std::vector<WsfDebugValueModelQueryData*> mQueries;
   UtCallbackHolder                          mCallbacks;
   bool                                      mCanEditQueries;
};


class WsfDebugValueModelQueryData : public UtReferenceTracked
{
public:
   WsfDebugValueModelQueryData(WsfDebugValueModelData* aParent, bool aIsRoot = false)
   {
      mIsRoot   = aIsRoot;
      mDataPtr  = aParent;
      mRootNode = nullptr;
      if (!aIsRoot)
      {
         mDataPtr->mQueries.push_back(this);
      }
   }
   ~WsfDebugValueModelQueryData()
   {
      UtReferenceTracked::FreeReference();
      if (!mIsRoot)
      {
         if (mDataPtr) // mDataPtr can be set to null if the root class is deleted first (likely at shutdown)
         {
            mDataPtr->mQueries.erase(std::find(mDataPtr->mQueries.begin(), mDataPtr->mQueries.end(), this));
         }
      }
      else
      {
         for (auto query : mDataPtr->mQueries)
         {
            // notify queries that are dependent on this root that it is no longer available
            query->mDataPtr = nullptr;
         }
         delete mDataPtr;
      }
   }

   void RequestedValueReceived(const std::vector<int>& aEntryAddress, const WsfDebugValueModelVariableValue* aValue);


   typedef std::vector<int>        EntryId;
   bool                            mIsRoot;
   WsfDebugValueModelData*         mDataPtr;
   WsfDebugValueQuery              mQuery;
   std::vector<EntryId>            mRequestedEntries;
   WsfDebugValueModelVariableValue mValue;
   WsfDebugValueModelNode*         mRootNode;
   // When an expression (like PLATFORM.Name()) is used, it is only evaluated once.
   // Upon evaluation, an expression ID is returned that must be used to lookup sub-entries at a later time.
   // int                                    mReceivedExpressionId;
};


class WsfDebugValueModelNode;

class WsfDebugValueModelNode : public UtQtAbstractModelItem
{
public:
   explicit WsfDebugValueModelNode(WsfDebugValueModelNode* aParentPtr)
      : UtQtAbstractModelItem()
      , mNodeType(cQUERY_CHILD)
   {
      mRootData = aParentPtr->mRootData;
   }
   explicit WsfDebugValueModelNode(WsfDebugValueModelQueryData* aDataPtr)
      : UtQtAbstractModelItem()
      , mRootData(aDataPtr)
   {
      mNodeType = mRootData->mIsRoot ? cROOT : cQUERY_ROOT;
      assert(mRootData->mRootNode == nullptr);
      mRootData->mRootNode = this;
   }
   ~WsfDebugValueModelNode() override
   {
      if (mNodeType != cQUERY_CHILD)
      {
         delete mRootData;
      }
   }

   WsfDebugValueModelNode* ParentPtr() const { return static_cast<WsfDebugValueModelNode*>(GetAbstractItemParent()); }
   WsfDebugValueModelNode* FindQueryRoot()
   {
      WsfDebugValueModelNode* p = this;
      while (p->mNodeType != cQUERY_ROOT)
      {
         p = p->ParentPtr();
      }
      return p;
   }
   WsfDebugValueModelNode* ChildPtr(int index) const
   {
      return static_cast<WsfDebugValueModelNode*>(GetAbstractItem(index));
   }
   QVariant DisplayWorking() const { return "..."; }
   QVariant data(int row, int col, int role) override;
   QVariant data(int col, int role) override;
   bool     setData(int row, int col, const QVariant& value, int role) override;

   virtual int   columnCount(int aRow, int aCol) { return 3; }
   Qt::ItemFlags flags(int col) const override { return Qt::ItemIsEnabled | Qt::ItemIsSelectable; }
   Qt::ItemFlags flags(int row, int column) const override;

   bool removeRows(int row, int count) override;
   bool ForceRemoveRows(int row, int count);

   WsfDebugValueModelNode* GetOrMakeEntry(int aIndex);

   WsfDebugValueModelNode* HasChildEntry(int aIndex) const
   {
      return dynamic_cast<WsfDebugValueModelNode*>(GetAbstractItem(aIndex));
   }

   WsfDebugValueModelVariableValue* HasChildValue(int aIndex) const
   {
      std::map<int, WsfDebugValueModelVariableValue>::const_iterator i = mChildValues.find(aIndex);
      if (i != mChildValues.end())
      {
         return const_cast<WsfDebugValueModelVariableValue*>(&i->second);
      }
      else
      {
         return nullptr;
      }
   }

   void                UpdateRowCount();
   int                 rowCount(int aRow) const override;
   int                 rowCount() const override;
   int                 columnCount() const override;
   WsfDebugValueQuery* FindRootQuery() { return &mRootData->mQuery; }

   enum NodeType
   {
      cROOT,
      cQUERY_ROOT,
      cQUERY_CHILD
   };


   void fetchMore(int aRow) override;
   bool canFetchMore(int aRow) const override;

   // WsfDebugValueModelData*                      mRootData;
   WsfDebugValueModelQueryData* mRootData;
   NodeType                     mNodeType;
   // QMap<int, WsfDebugValueModelVariableValue>   mChildValues;
   std::map<int, WsfDebugValueModelVariableValue> mChildValues;
};

class WsfDebugValueModelNodeLocals : public WsfDebugValueModelNode
{
   Q_OBJECT

public:
   explicit WsfDebugValueModelNodeLocals(WsfDebugValueModelQueryData* aDataPtr);

   QVariant data(int col, int role) override;
   QVariant data(int row, int col, int role) override;

   int rowCount() const override;

public slots:

   void LoadLocals();

private:
   void DebugCallstackUpdated(wizard::ScriptDebugger* aDebugger);
   void DebugCallStackFrameSwitch(wizard::ScriptDebugger* aDebugger, int aFrameIndex);
   void SetCurrentScript(int64_t aScriptHandle, wizard::ScriptDebugger::ScriptInfo* aInfo);
   void UpdateLocals(wizard::ScriptDebugger* aDebugger);

   UtCallbackHolder            mCallbacks;
   int64_t                     mScriptHandle;
   int                         mLocalVariableCount;
   QPointer<WsfScriptDebugger> mDebuggerPtr;
   mutable QTimer              mDelayedLoadTimer;
};

//
// class WsfDebugValueModelNodeGlobals : public WsfDebugValueModelNode
// {
//    Q_OBJECT
//    public:
//       WsfDebugValueModelNodeGlobals(WsfDebugValueModelQueryData* aDataPtr);
//
//       virtual QVariant data(int col, int role);
//       virtual QVariant data(int row, int col, int role);
//
//       virtual int  rowCount() const;
//       virtual int  columnCount() const;
//
//       virtual void fetchMore(int aRow);
//
//       virtual bool canFetchMore(int aRow) const;
//
//    protected:
//
//       //int64_t                 mScriptHandle;
//       //int                     mLocalVariableCount;
//       //WsfScriptDebugger*  mDebuggerPtr;
//       //mutable QTimer          mDelayedLoadTimer;
// };
} // namespace SimulationManager
#endif
