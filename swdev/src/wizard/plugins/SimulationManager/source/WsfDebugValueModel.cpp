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

#include "WsfDebugValueModel.hpp"

#include <QColor>

#include "Signals.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"
#include "xio/WsfXIO_Request.hpp"

namespace SimulationManager
{
class VariableValueRequest : public WsfXIO_Request
{
public:
   UtWeakReference<WsfDebugValueModelQueryData> mQueryPtr;
   VariableValueRequest(WsfXIO_Connection* aConnectionPtr, WsfDebugValueModelQueryData* aModelPtr)
      : WsfXIO_Request(aConnectionPtr)
      , mBreakSequenceNumber(-1)
   {
      mQueryPtr = aModelPtr;
   }

   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (!mQueryPtr.IsValid())
      {
         return;
      }
      if (WsfXIO_RequestScriptDataResponsePkt::cPACKET_ID == aPkt.ID())
      {
         WsfXIO_RequestScriptDataResponsePkt& pkt = (WsfXIO_RequestScriptDataResponsePkt&)aPkt;
         assert(pkt.mVariableValues.size() == mRequest.mVariableQueries.size());
         for (size_t i = 0; i < mRequest.mVariableQueries.size(); ++i)
         {
            WsfXIO_RequestScriptDataPkt::VariableQuery&               query        = mRequest.mVariableQueries[i];
            WsfXIO_RequestScriptDataResponsePkt::VariableQueryResult* queryResults = pkt.mVariableValues[i];
            for (size_t j = 0; j < query.mEntryAddresses.size(); ++j)
            {
               WsfDebugValueModelVariableValue                     valdata;
               WsfXIO_RequestScriptDataResponsePkt::VariableValue& val = queryResults->mValues[j];
               if (!val.IsNull())
               {
                  valdata.mType                      = QString::fromStdString(val.mTypeName);
                  valdata.mName                      = QString::fromStdString(val.mEntryName);
                  valdata.mValue                     = QString::fromStdString(val.mValueText);
                  valdata.mBreakSequenceNumber       = mBreakSequenceNumber;
                  valdata.mLastRequestSequenceNumber = mBreakSequenceNumber;
                  valdata.mChildEntryCount           = val.mEntryCount;
                  // valdata.mExpressionEvaluationId = val->mExpressionReferenceNumber;
                  mQueryPtr.Get()->RequestedValueReceived(query.mEntryAddresses[j], &valdata);
               }
               else
               {
                  mQueryPtr.Get()->RequestedValueReceived(query.mEntryAddresses[j], nullptr);
               }
            }
         }
      }
      // mModelPtr->update(); //dataChanged(QModelIndex(), QModelIndex());
   }
   void Initialized() override { SendRequest(mRequest); }

   WsfXIO_RequestScriptDataPkt mRequest;
   int                         mBreakSequenceNumber;
};

// class VariableValueRequest
//{
//    public:
//       UtWeakReference<ViWsfDebugValueModelQueryData> mQueryPtr;
//       VariableValueRequest(WsfXIO_Connection*               aConnectionPtr,
//                            ViWsfDebugValueModelQueryData* aModelPtr) {}
// };
} // namespace SimulationManager

SimulationManager::WsfDebugValueQuery::WsfDebugValueQuery()
   : mContext(nullptr)
   , mStackFrame(-1)
   , mRequestId(-1)
{
}

int SimulationManager::WsfDebugValueQuery::GetRequestId()
{
   if (mRequestId >= 0)
   {
      return mRequestId;
   }
   NewRequestId();
   return mRequestId;
}

void SimulationManager::WsfDebugValueQuery::NewRequestId()
{
   static int sNextRequestId = 1;
   mRequestId                = sNextRequestId;
   sNextRequestId++;
}


void SimulationManager::WsfDebugValueModelQueryData::RequestedValueReceived(const std::vector<int>& aEntryAddress,
                                                                            const WsfDebugValueModelVariableValue* aValue)
{
   WsfDebugValueModelNode* p          = static_cast<WsfDebugValueModelNode*>(mRootNode->GetAbstractItemParent());
   size_t                  i          = mQuery.mEntryAddress.size();
   int                     entryIndex = mRootNode->GetItemIndex(); // mQuery.mEntryAddress[0];
   while (i < aEntryAddress.size())
   {
      int next   = aEntryAddress[i++];
      p          = p->GetOrMakeEntry(entryIndex);
      entryIndex = next;
   }
   if (aValue)
   {
      // mReceivedExpressionId = aValue->mExpressionEvaluationId;
      p->mChildValues[entryIndex] = *aValue;
   }
   else
   {
      WsfDebugValueModelVariableValue& val                = p->mChildValues[entryIndex];
      int                              requestSequenceNum = val.mLastRequestSequenceNumber;
      val                                                 = WsfDebugValueModelVariableValue();
      val.mLastRequestSequenceNumber                      = requestSequenceNum;
      val.mLookupFailed                                   = true;
   }

   WsfDebugValueModelNode* entryPtr = p->HasChildEntry(entryIndex);
   if (entryPtr)
   {
      entryPtr->mChildValues.clear();
      entryPtr->UpdateRowCount();
   }
   p->AbstractItemChanged(entryIndex, 0, entryIndex, 2);
   // p->dataChanged(entryIndex, entryIndex, 0, 1);
}


void SimulationManager::WsfDebugValueModelData::requestData(WsfDebugValueModelNode* aParent, int aRow)
{
   WsfDebugValueModelQueryData* queryPtr = nullptr;
   if (aParent->mRootData->mIsRoot)
   {
      if (aRow < (int)aParent->mRootData->mDataPtr->mQueries.size())
      {
         queryPtr = aParent->mRootData->mDataPtr->mQueries[aRow];
      }
      else
      {
         return;
      }
   }
   else
   {
      queryPtr = aParent->mRootData;
   }
   auto entryId = queryPtr->mQuery.mEntryAddress;
   if (aParent)
   {
      WsfDebugValueModelVariableValue& val = aParent->mChildValues[aRow];
      if (val.mLastRequestSequenceNumber < mDebuggerPtr->BreakSequenceNumber())
      {
         val.mLastRequestSequenceNumber = mDebuggerPtr->BreakSequenceNumber();
         if (aParent->mNodeType != WsfDebugValueModelNode::cROOT)
         {
            entryId.push_back(aRow);
            WsfDebugValueModelNode* p = aParent;
            while (p->ParentPtr() && p->ParentPtr()->mNodeType != WsfDebugValueModelNode::cROOT)
            {
               entryId.push_back(p->GetItemIndex());
               p = p->ParentPtr();
            }
            // Make sure root has been updated
            // requestData(p->parentPtr(), p->GetItemIndex());

            //             WsfDebugValueModelVariableValue* rootValue =
            //             p->parentPtr()->hasChildValue(p->GetItemIndex()); if (rootValue)
            //             {
            //                rootValue->mExpressionEvaluationId
            //             }
            std::reverse(entryId.begin(), entryId.end());
         }
         else
         {
         }
         queryPtr->mRequestedEntries.push_back(entryId);

         mRequestTimer.start(100);
      }
   }
}

SimulationManager::WsfDebugValueModelData::WsfDebugValueModelData(wizard::ScriptDebugger* aDebuggerPtr)
   : mDebuggerPtr(aDebuggerPtr)
{
   mCanEditQueries = true;
   mConnectionPtr  = mDebuggerPtr->Connection();
   mCallbacks += wizSignals->DebugAtBreak.Connect(&WsfDebugValueModelData::DebugAtBreak, this);
   mCallbacks += wizSignals->DebugSessionActive.Connect(&WsfDebugValueModelData::DebugSessionActive, this);
   mCallbacks += wizSignals->DebugCallStackFrameSwitch.Connect(&WsfDebugValueModelData::DebugCallStackFrameSwitch, this);
   connect(&mRequestTimer, &QTimer::timeout, this, &WsfDebugValueModelData::RequestDataTimeout);
}

void SimulationManager::WsfDebugValueModelData::DebugSessionActive(wizard::ScriptDebugger* aDebugger, bool aIsActive)
{
   if (!aIsActive)
   {
      mConnectionPtr = nullptr;
   }
   else
   {
      mConnectionPtr = aDebugger->Connection();
   }
}
QVariant SimulationManager::WsfDebugValueModelNode::data(int col, int role)
{
   if (role == Qt::DisplayRole || role == Qt::EditRole)
   {
      if (col == 0)
      {
         return mRootData->mDataPtr->mRootName;
      }
      return QString();
   }
   return QVariant();
}

int SimulationManager::WsfDebugValueModelNode::columnCount() const
{
   return 3;
}

QVariant SimulationManager::WsfDebugValueModelNode::data(int row, int col, int role)
{
   if (role == Qt::DisplayRole || role == Qt::EditRole)
   {
      if (col == 0 && mNodeType == cROOT)
      {
         if (row < (int)mRootData->mDataPtr->mQueries.size())
         {
            QString queryVar = QString::fromStdString(mRootData->mDataPtr->mQueries[row]->mQuery.mVariableName);
            if (queryVar == "_debug_enumerate_GLOBALS")
            {
               return QString("Simulation");
            }
            return queryVar;
         }
         return QString();
      }
      else if (col == 0 || col == 1 || col == 2)
      {
         WsfDebugValueModelVariableValue* valPtr = nullptr;
         if (mChildValues.find(row) != mChildValues.end())
         {
            valPtr = &mChildValues[row];
         }

         if (!valPtr || valPtr->mBreakSequenceNumber < mRootData->mDataPtr->mDebuggerPtr->BreakSequenceNumber())
         {
            if (col == 1)
            {
               mRootData->mDataPtr->requestData(this, row);
            }
         }
         if (col == 0)
         {
            if (valPtr && !valPtr->mName.isEmpty())
            {
               return valPtr->mName;
            }
            return QString::number(row);
         }
         else if (col == 1)
         {
            if (valPtr)
            {
               return valPtr->ToVariant();
            }
            return DisplayWorking();
         }
         else if (col == 2)
         {
            if (valPtr)
            {
               return valPtr->mType;
            }
            return DisplayWorking();
         }
      }
   }
   if (role == Qt::TextColorRole && col == 1)
   {
      WsfDebugValueModelVariableValue* valPtr = nullptr;
      if (mChildValues.find(row) != mChildValues.end())
      {
         valPtr = &mChildValues[row];
      }
      if (valPtr && valPtr->mLookupFailed)
      {
         return QColor(Qt::red);
      }
      return QVariant();
   }
   return QVariant();
}
bool SimulationManager::WsfDebugValueModelNode::setData(int row, int col, const QVariant& value, int role)
{
   if (mNodeType == cROOT && role == Qt::EditRole)
   {
      WsfDebugValueModelNode* queryRoot = ChildPtr(row);
      if (queryRoot)
      {
         queryRoot->mRootData->mQuery.mVariableName = value.toString().toStdString();
         queryRoot->mRootData->mQuery.NewRequestId();
         mChildValues.erase(row);
         queryRoot->AbstractItemChanged();
         return true;
      }
   }
   return false;
}

Qt::ItemFlags SimulationManager::WsfDebugValueModelNode::flags(int row, int column) const
{
   Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

   if (column == 0)
   {
      if (mNodeType == cROOT && mRootData->mDataPtr->mCanEditQueries)
      {
         flags |= Qt::ItemIsEditable;
      }
   }
   if (column == 1 || column == 2)
   {
      const WsfDebugValueModelVariableValue* valPtr = nullptr;
      if (mRootData->mDataPtr->mConnectionPtr)
      {
         std::map<int, WsfDebugValueModelVariableValue>::const_iterator iter = mChildValues.find(row);
         if (iter != mChildValues.end())
         {
            valPtr = &iter->second;
         }
      }
      if (!valPtr || valPtr->mBreakSequenceNumber < mRootData->mDataPtr->mDebuggerPtr->BreakSequenceNumber())
      {
         flags = flags & ~(Qt::ItemIsEnabled);
      }
   }
   return flags;
}

void SimulationManager::WsfDebugValueModelData::RequestDataTimeout()
{
   if (mConnectionPtr)
   {
      for (size_t j = 0; j < mQueries.size(); ++j)
      {
         WsfDebugValueModelQueryData* dataPtr = mQueries[j];
         if (dataPtr->mRequestedEntries.empty())
         {
            continue;
         }
         VariableValueRequest*        request = new VariableValueRequest(mConnectionPtr, dataPtr);
         WsfXIO_RequestScriptDataPkt& pkt     = request->mRequest;
         request->mBreakSequenceNumber        = mDebuggerPtr->BreakSequenceNumber();
         pkt.mInformationType                 = WsfXIO_RequestScriptDataPkt::cVARIABLE_QUERY;

         pkt.mVariableQueries.push_back(WsfXIO_RequestScriptDataPkt::VariableQuery());
         WsfXIO_RequestScriptDataPkt::VariableQuery& query = pkt.mVariableQueries.back();
         query.mExpressionReferenceNumber                  = dataPtr->mQuery.GetRequestId();
         query.mVariableName                               = dataPtr->mQuery.mVariableName;
         query.mStackFrame                                 = dataPtr->mQuery.mStackFrame;
         query.mContext                                    = dataPtr->mQuery.mContext;
         for (size_t i = 0; i < dataPtr->mRequestedEntries.size(); ++i)
         {
            auto& id = dataPtr->mRequestedEntries[i];
            query.mEntryAddresses.push_back(id);
         }
         mConnectionPtr->GetInterface().GetRequestManager().AddRequest(request);

         dataPtr->mRequestedEntries.clear();
      }
   }
   mRequestTimer.stop();
}

void SimulationManager::WsfDebugValueModelData::DebugAtBreak(wizard::ScriptDebugger* aDebugger, bool aIsAtBreak)
{
   if (aIsAtBreak)
   {
      UpdateFrame(-1);
      for (size_t i = 0; i < mQueries.size(); ++i)
      {
         mQueries[i]->mRootNode->AbstractItemChanged(0, 2);
      }
      // mRootNode->GetModel()->reset();
      // mRootNode->AbstractItemChanged(0, 0, mRootNode->rowCount() - 1, 3);
      // update();
   }
}

// The selected stack frame has changed
void SimulationManager::WsfDebugValueModelData::UpdateFrame(int aIndex)
{
   if (!mCanEditQueries)
   {
      return;
   }
   WsfDebugValueModelNode* rootNodePtr = nullptr;
   // Update the stack frame requested, clear out old data, and inform Qt about the item's data changing
   for (size_t i = 0; i < mQueries.size(); ++i)
   {
      if (mQueries[i]->mQuery.mStackFrame != aIndex)
      {
         mQueries[i]->mQuery.mStackFrame = aIndex;
         rootNodePtr                     = mQueries[i]->mRootNode->ParentPtr();
         rootNodePtr->mChildValues.erase((int)i);
      }
   }
   if (rootNodePtr)
   {
      rootNodePtr->AbstractItemChanged();
   }
}

void SimulationManager::WsfDebugValueModelData::DebugCallStackFrameSwitch(wizard::ScriptDebugger*, int aFrameIndex)
{
   if (!mCanEditQueries)
   {
      return;
   }
   UpdateFrame(aFrameIndex);
}

SimulationManager::WsfDebugValueModelVariableValue::WsfDebugValueModelVariableValue()
   : mBreakSequenceNumber(-1)
   , mLastRequestSequenceNumber(-1)
   , mExpressionEvaluationId(-1)
   , mValue("...")
   , mChildEntryCount(0)
   , mLookupFailed(false)
{
}

QVariant SimulationManager::WsfDebugValueModelVariableValue::ToVariant()
{
   if (mLookupFailed)
   {
      return QString("---");
   }
   if (mType == "MAP_ITERATOR")
   {
      return "<Map Entry>";
   }

   return mValue;
}

void SimulationManager::WsfDebugValueModelNode::fetchMore(int aRow)
{
   WsfDebugValueModelVariableValue* valPtr = HasChildValue(aRow);
   if (valPtr)
   {
      int                     childEntryCount = valPtr->mChildEntryCount;
      WsfDebugValueModelNode* cPtr            = GetOrMakeEntry(aRow);
      cPtr->SetNonItemChildCount(childEntryCount - cPtr->GetAbstractItemCount());
   }
}

bool SimulationManager::WsfDebugValueModelNode::canFetchMore(int aRow) const
{
   WsfDebugValueModelVariableValue* valPtr = HasChildValue(aRow);
   if (valPtr && !HasChildEntry(aRow))
   {
      return valPtr->mChildEntryCount > 0;
   }
   return false;
}

int SimulationManager::WsfDebugValueModelNode::rowCount(int aRow) const
{
   std::map<int, WsfDebugValueModelVariableValue>::const_iterator i = mChildValues.find(aRow);
   if (i != mChildValues.end())
   {
      return i->second.mChildEntryCount;
   }
   return UtQtAbstractModelItem::rowCount(aRow);
}

int SimulationManager::WsfDebugValueModelNode::rowCount() const
{
   return UtQtAbstractModelItem::rowCount();
   // if (mNodeType == cROOT)
   //{
   // return 1;
   // }
   //  shouldn't get here
}

SimulationManager::WsfDebugValueModelNode* SimulationManager::WsfDebugValueModelNode::GetOrMakeEntry(int aIndex)
{
   if (aIndex < (int)GetAbstractItemCount())
   {
      WsfDebugValueModelNode* c = static_cast<WsfDebugValueModelNode*>(GetAbstractItem(aIndex));
      if (c)
      {
         return c;
      }
   }

   while (aIndex >= GetAbstractItemCount())
   {
      WsfDebugValueModelNode* child = new WsfDebugValueModelNode(this);
      AddAbstractItem(child);
   }
   return dynamic_cast<WsfDebugValueModelNode*>(GetAbstractItem(aIndex));
}

void SimulationManager::WsfDebugValueModelNode::UpdateRowCount()
{
   int desiredRows = GetAbstractItemParent()->rowCount(GetItemIndex());
   int childrenNow = GetAbstractItemCount() + GetNonItemChildCount();
   if (childrenNow < desiredRows)
   {
      SetNonItemChildCount(desiredRows - GetAbstractItemCount());
   }
   else if (desiredRows < childrenNow)
   {
      if (desiredRows >= GetAbstractItemCount())
      {
         SetNonItemChildCount(desiredRows - GetAbstractItemCount());
      }
      else
      {
         SetNonItemChildCount(0);
         while (GetAbstractItemCount() > desiredRows)
         {
            delete RemoveAbstractItem(GetAbstractItemCount() - 1);
         }
      }
   }
}
bool SimulationManager::WsfDebugValueModelNode::ForceRemoveRows(int row, int count)
{
   if (mNodeType == cROOT)
   {
      if (GetAbstractItem(row) != nullptr)
      {
         delete RemoveAbstractItem(row);
         // This is the only case of erasing a value from the middle of a list
         // we need to move values already received up one
         //       WIZARD_TODO: Refactor this into a simple vector?
         std::map<int, WsfDebugValueModelVariableValue>::iterator iter = mChildValues.lower_bound(row);
         while (iter != mChildValues.end())
         {
            std::map<int, WsfDebugValueModelVariableValue>::iterator nextIter = iter;
            ++nextIter;
            mChildValues[iter->first - 1] = iter->second;
            mChildValues.erase(iter->first);
            iter = nextIter;
         }
      }
   }
   return false;
}
bool SimulationManager::WsfDebugValueModelNode::removeRows(int row, int count)
{
   if (mRootData->mDataPtr->mCanEditQueries)
   {
      return ForceRemoveRows(row, count);
   }
   return false;
}

void SimulationManager::WsfDebugValueModelNodeLocals::UpdateLocals(wizard::ScriptDebugger* aDebugger)
{
   mDebuggerPtr      = dynamic_cast<WsfScriptDebugger*>(aDebugger);
   const auto* frame = aDebugger->CurrentCallFrame();
   if (frame)
   {
      wizard::ScriptDebugger::ScriptInfo* scriptInfo = aDebugger->FindScriptInfo(frame->mScriptHandle);
      if (scriptInfo)
      {
         SetCurrentScript(frame->mScriptHandle, scriptInfo);
      }
   }
}

void SimulationManager::WsfDebugValueModelNodeLocals::SetCurrentScript(int64_t aScriptHandle,
                                                                       wizard::ScriptDebugger::ScriptInfo* aInfo)
{
   if (mScriptHandle == aScriptHandle)
   {
      return;
   }
   RemoveAllAbstractItems(true);
   mLocalVariableCount = (int)aInfo->mLocalVariableNames.size();
   mScriptHandle       = aScriptHandle;
   SetNonItemChildCount(mLocalVariableCount);

   AbstractItemChanged();
}

void SimulationManager::WsfDebugValueModelNodeLocals::DebugCallstackUpdated(wizard::ScriptDebugger* aDebugger)
{
   UpdateLocals(aDebugger);
}

void SimulationManager::WsfDebugValueModelNodeLocals::DebugCallStackFrameSwitch(wizard::ScriptDebugger* aDebugger,
                                                                                int                     aFrameIndex)
{
   UpdateLocals(aDebugger);
}

void SimulationManager::WsfDebugValueModelNodeLocals::LoadLocals()
{
   if (GetAbstractItemCount() != 0)
   {
      return;
   }
   if (mDebuggerPtr)
   {
      auto* infoPtr = mDebuggerPtr->FindScriptInfo(mScriptHandle);
      if (infoPtr)
      {
         for (size_t i = 0; i < infoPtr->mLocalVariableNames.size(); ++i)
         {
            WsfDebugValueModelQueryData* queryData = new WsfDebugValueModelQueryData(mRootData->mDataPtr);
            WsfDebugValueModelNode*      queryNode = new WsfDebugValueModelNode(queryData);
            queryData->mQuery.mContext             = nullptr;
            queryData->mQuery.mStackFrame          = mDebuggerPtr->CurrentCallFrameIndex();
            queryData->mQuery.mVariableName        = infoPtr->mLocalVariableNames[i];
            AddAbstractItem(queryNode);
         }
      }
   }
   mDelayedLoadTimer.stop();
}

int SimulationManager::WsfDebugValueModelNodeLocals::rowCount() const
{
   mDelayedLoadTimer.start(50);
   return mLocalVariableCount;
}

QVariant SimulationManager::WsfDebugValueModelNodeLocals::data(int row, int col, int role)
{
   // Determine if we have a request for children not yet created
   // If so, create those children later
   if (row < mLocalVariableCount)
   {
      UtQtAbstractModelItem* itemPtr = GetAbstractItem(row);
      if (!itemPtr)
      {
         mDelayedLoadTimer.start(50);
      }
   }
   return WsfDebugValueModelNode::data(row, col, role);
}

QVariant SimulationManager::WsfDebugValueModelNodeLocals::data(int col, int role)
{
   if (col == 0 && role == Qt::DisplayRole)
   {
      return "Locals";
   }
   return WsfDebugValueModelNode::data(col, role);
}

SimulationManager::WsfDebugValueModelNodeLocals::WsfDebugValueModelNodeLocals(WsfDebugValueModelQueryData* aDataPtr)
   : WsfDebugValueModelNode(aDataPtr)
   , mLocalVariableCount(0)
   , mDebuggerPtr(nullptr)
{
   aDataPtr->mDataPtr->mCanEditQueries = false;
   mScriptHandle                       = 0;
   connect(&mDelayedLoadTimer, &QTimer::timeout, this, &WsfDebugValueModelNodeLocals::LoadLocals);
   mCallbacks +=
      wizSignals->DebugCallStackFrameSwitch.Connect(&WsfDebugValueModelNodeLocals::DebugCallStackFrameSwitch, this);
   mCallbacks += wizSignals->DebugCallstackUpdated.Connect(&WsfDebugValueModelNodeLocals::DebugCallstackUpdated, this);
}


//
// SimulationManager::WsfDebugValueModelNodeGlobals::WsfDebugValueModelNodeGlobals(WsfDebugValueModelQueryData* aDataPtr)
//  : WsfDebugValueModelNode(aDataPtr)
// {
//
// }
//
// QVariant SimulationManager::WsfDebugValueModelNodeGlobals::data(int col, int role)
// {
//    if (col == 0 && role == Qt::DisplayRole)
//    {
//       return "Globals";
//    }
//    return WsfDebugValueModelNode::data(col, role);
// }
//
// QVariant SimulationManager::WsfDebugValueModelNodeGlobals::data(int row, int col, int role)
// {
//    // Skip this node, just show single child
//    if (GetAbstractItemCount() > 0)
//    {
//       GetAbstractItem(0)->data(row, col, role);
//    }
//    // Determine if we have a request for children not yet created
//    // If so, create those children later
//    if (row < GetAbstractItemCount())
//    {
//       return GetAbstractItem(row)->data(col, role);
//    }
// //    if (row < mLocalVariableCount)
// //    {
// //       UtQtAbstractModelItem* itemPtr = GetAbstractItem(row);
// //       if (! itemPtr)
// //       {
// //          mDelayedLoadTimer.start(50);
// //       }
// //    }
//    return WsfDebugValueModelNode::data(row, col, role);
// }
//
// int SimulationManager::WsfDebugValueModelNodeGlobals::rowCount() const
// {
//    if (GetAbstractItemCount() > 0)
//    {
//       return GetAbstractItem(0)->rowCount();
//    }
//    return 1;
// }
//
// int SimulationManager::WsfDebugValueModelNodeGlobals::columnCount() const
// {
//    if (GetAbstractItemCount() > 0)
//    {
//       return GetAbstractItem(0)->columnCount();
//    }
//    return 3;
// }
//
// void SimulationManager::WsfDebugValueModelNodeGlobals::fetchMore(int aRow)
// {
//
// }
