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

#include "ProxyMerge.hpp"

#include <algorithm>

#include "WsfPProxyBasicValue.hpp"
#include "WsfPProxyList.hpp"
#include "WsfPProxyObjectMap.hpp"
#include "WsfPProxyStructValue.hpp"
#include "WsfPProxyType.hpp"

WsfPProxyHash wizard::ProxyHash::RecurseHash(ProxyHashNode*      aParentPtr,
                                             const WsfPProxyKey& aValueAddr,
                                             WsfPProxyValue      aRootValue,
                                             bool                aIsParentStruct)
{
   if (aRootValue.GetDataPtr() == nullptr)
   {
      return WsfPProxyHash();
   }
   switch (aRootValue.GetType()->mTypeKind)
   {
   case WsfProxy::cSTRUCT:
   {
      UtSHA          structHash;
      ProxyHashNode* valueNode;
      if (aParentPtr)
      {
         valueNode = aParentPtr->Add(aValueAddr);
      }
      else
      {
         valueNode = &mRoot;
      }
      WsfPProxyStructValue inst(aRootValue);
      if (inst.IsUnset())
      {
         return WsfPProxyHash();
      }
      else
      {
         size_t       members = inst.GetMemberCount();
         WsfPProxyKey memberPath;
         for (size_t i = 0; i < members; ++i)
         {
            memberPath.SetIndex(i);
            WsfPProxyValue member     = inst.GetAtIndex(i);
            WsfPProxyHash  hashResult = RecurseHash(valueNode, memberPath, member, true);
            hashResult.AddData(structHash);
         }
         structHash.FinalDigest(valueNode->mDigest);
         return valueNode->mDigest;
      }
   }
   break;
   case WsfProxy::cLIST:
   {
      UtSHA          listHash;
      ProxyHashNode* valueNode = aParentPtr->Add(aValueAddr);
      WsfPProxyKey   memberPath;
      WsfPProxyList* listPtr = aRootValue.GetList();
      for (size_t i = 0; i < listPtr->mValues.size(); ++i)
      {
         memberPath.SetIndex(i);
         RecurseHash(valueNode, memberPath, listPtr->Get(i), false).AddData(listHash);
      }
      listHash.FinalDigest(valueNode->mDigest);
      return valueNode->mDigest;
   }
   break;
   case WsfProxy::cOBJECT_MAP:
   {
      UtSHA               mapHash;
      ProxyHashNode*      valueNode = aParentPtr->Add(aValueAddr);
      WsfPProxyKey        memberPath;
      WsfPProxyObjectMap* mapPtr = aRootValue.GetObjectMap();
      for (auto iter = mapPtr->GetValues().begin(); iter != mapPtr->GetValues().end(); ++iter)
      {
         memberPath = iter->first;
         mapHash.AddData(iter->first.c_str(), iter->first.size());
         RecurseHash(valueNode, memberPath, iter->second, false).AddData(mapHash);
      }
      mapHash.FinalDigest(valueNode->mDigest);
      return valueNode->mDigest;
   }
   break;
   default:
      if (aIsParentStruct)
      {
         return aRootValue.Hash();
      }
      else
      {
         ProxyHashNode* valueNode = aParentPtr->Add(aValueAddr);
         UtSHA          sha;
         aRootValue.Hash().AddData(sha);
         sha.FinalDigest(valueNode->mDigest);
         return valueNode->mDigest;
      }
   }
   return WsfPProxyHash();
}


wizard::ProxyHashNode* wizard::ProxyHash::Find(const WsfPProxyPath& aPath)
{
   ProxyHashNode* root       = &mRoot;
   size_t         entryCount = aPath.size();
   for (size_t i = 0; i < entryCount && root; ++i)
   {
      const WsfPProxyKey& e = aPath[i];
      root                  = root->FindChild(e);
   }
   return root;
}

wizard::ProxyHash::ProxyHash(WsfPProxyValue aRootValue)
{
   mRootValue = aRootValue;
   WsfPProxyKey nullEntry;
   RecurseHash(nullptr, nullEntry, aRootValue, false);
}

wizard::ProxyMerge::ProxyMerge(ProxyHash* aOldHash, ProxyHash* aNewHash, WsfPProxy* aOldProxy, WsfPProxy* aNewProxy)
   : mOldHash(aOldHash)
   , mNewHash(aNewHash)
   , mOldProxy(aOldProxy)
   , mNewProxy(aNewProxy)
{
}

void wizard::ProxyMerge::VisitBasicChildren(ProxyMergeVisitor* aVisitorPtr,
                                            WsfPProxyValue     aNewValue,
                                            WsfPProxyValue     aOldValue,
                                            WsfPProxyPath&     aPath)
{
   // hashes of basic values aren't stored in the ProxyHash
   // so we compare them explicitly here
   //
   // Note, we are only handling structs right now.  I am uncertain if we need to handle maps or lists
   if (aNewValue.IsStruct())
   {
      size_t count = aNewValue.GetAttrCount();
      if (aOldValue && aOldValue.GetType()->GetTypeName() == aNewValue.GetType()->GetTypeName())
      {
         for (size_t i = 0; i < count; ++i)
         {
            WsfPProxyBasicValue newChild = aNewValue.GetAttr(i);
            if (newChild)
            {
               WsfPProxyBasicValue oldChild = aOldValue.GetAttr(i);
               if (oldChild)
               {
                  if (!newChild.IsEqual(oldChild))
                  {
                     aPath += i;
                     aVisitorPtr->Accept(ProxyMergeVisitor::cCHANGED, aPath, oldChild, newChild);
                     aPath.Pop();
                  }
               }
               else
               {
                  aPath += i;
                  aVisitorPtr->Accept(ProxyMergeVisitor::cCHANGED, aPath, oldChild, WsfPProxyValue());
                  aPath.Pop();
               }
            }
         }
      }
      else
      {
         // if the old value doesn't exist, or the type doesn't match, the user should already have been notified
      }
   }
   // not handling case where new value is gone.  User should have been notified of this anyway
}

void wizard::ProxyMerge::VisitR(ProxyMergeVisitor* aVisitor,
                                ProxyHashNode*     aNewHash,
                                ProxyHashNode*     aOldHash,
                                WsfPProxyValue     aNewValue,
                                WsfPProxyValue     aOldValue,
                                WsfPProxyPath      aPath)
{
   ProxyMergeVisitor::CompareResult result = ProxyMergeVisitor::cNO_CHANGE;
   if (aNewHash)
   {
      if (aOldHash)
      {
         if (aNewHash->mDigest != aOldHash->mDigest)
         {
            result = ProxyMergeVisitor::cCHANGED;
         }
      }
      else
      {
         result = ProxyMergeVisitor::cNEW;
      }
   }
   else if (aOldHash)
   {
      result = ProxyMergeVisitor::cDELETED;
   }
   typedef ProxyHashNode::Entry Entry;
   if (result != ProxyMergeVisitor::cNO_CHANGE)
   {
      bool recurse = aVisitor->Accept(result, aPath, aOldValue, aNewValue);
      if (recurse)
      {
         if (aNewHash && aOldHash)
         {
            VisitBasicChildren(aVisitor, aNewValue, aOldValue, aPath);
            const std::vector<Entry>&          newChildren = aNewHash->Children();
            const std::vector<Entry>&          oldChildren = aOldHash->Children();
            std::vector<Entry>::const_iterator newIter     = newChildren.begin();
            std::vector<Entry>::const_iterator oldIter     = oldChildren.begin();
            while (newIter != newChildren.end() || oldIter != oldChildren.end())
            {
               int cmpResult = 0;
               if (oldIter == oldChildren.end())
               {
                  cmpResult = -1;
               }
               else if (newIter == newChildren.end())
               {
                  cmpResult = 1;
               }
               else
               {
                  const Entry& newE = *newIter;
                  const Entry& oldE = *oldIter;
                  if (newE.mAddr < oldE.mAddr)
                  {
                     cmpResult = -1;
                  }
                  else if (oldE.mAddr < newE.mAddr)
                  {
                     cmpResult = 1;
                  }
               }


               switch (cmpResult)
               {
               case -1:
               {
                  const Entry&  newE      = *newIter;
                  WsfPProxyPath childPath = aPath;
                  childPath += newE.mAddr;
                  VisitR(aVisitor, newE.mNode, nullptr, aNewValue.GetAttr(newE.mAddr), WsfPProxyValue(), childPath);
                  ++newIter;
                  break;
               }
               case 0:
               {
                  const Entry&  newE      = *newIter;
                  const Entry&  oldE      = *oldIter;
                  WsfPProxyPath childPath = aPath;
                  childPath += newE.mAddr;
                  VisitR(aVisitor, newE.mNode, oldE.mNode, aNewValue.GetAttr(newE.mAddr), aOldValue.GetAttr(oldE.mAddr), childPath);
                  ++newIter;
                  ++oldIter;
                  break;
               }
               case 1:
               {
                  const Entry&  oldE      = *oldIter;
                  WsfPProxyPath childPath = aPath;
                  childPath += oldE.mAddr;
                  VisitR(aVisitor, nullptr, oldE.mNode, WsfPProxyValue(), aOldValue.GetAttr(oldE.mAddr), childPath);
                  ++oldIter;
                  break;
               }
               }
            }
         }
         else if (aNewHash)
         {
            const std::vector<Entry>&          newChildren = aNewHash->Children();
            std::vector<Entry>::const_iterator newIter     = newChildren.begin();
            while (newIter != newChildren.end())
            {
               const Entry&  newE      = *newIter;
               WsfPProxyPath childPath = aPath;
               childPath += newE.mAddr;
               VisitR(aVisitor, newE.mNode, nullptr, aNewValue.GetAttr(newE.mAddr), WsfPProxyValue(), childPath);
               ++newIter;
            }
         }
         else if (aOldHash)
         {
            const std::vector<Entry>&          oldChildren = aOldHash->Children();
            std::vector<Entry>::const_iterator oldIter     = oldChildren.begin();
            while (oldIter != oldChildren.end())
            {
               const Entry&  oldE      = *oldIter;
               WsfPProxyPath childPath = aPath;
               childPath += oldE.mAddr;
               VisitR(aVisitor, nullptr, oldE.mNode, WsfPProxyValue(), aOldValue.GetAttr(oldE.mAddr), childPath);
               ++oldIter;
            }
         }
      }
   }
}

void wizard::ProxyMerge::Visit(ProxyMergeVisitor* aVisitor)
{
   WsfPProxyPath mEmptyPath;

   VisitR(aVisitor, &mNewHash->Root(), &mOldHash->Root(), mNewHash->RootValue(), mOldHash->RootValue(), mEmptyPath);
}

void wizard::ProxyHashNode::Initialize()
{
   std::sort(mEntries.begin(), mEntries.end());
}

wizard::ProxyHashNode* wizard::ProxyHashNode::Add(const WsfPProxyKey& aAddr)
{
   Entry e;
   e.mAddr = aAddr;
   e.mNode = new ProxyHashNode;
   mEntries.push_back(e);
   return e.mNode;
}

wizard::ProxyHashNode::~ProxyHashNode()
{
   for (auto&& it : mEntries)
   {
      delete it.mNode;
   }
}

wizard::ProxyHashNode* wizard::ProxyHashNode::FindChild(const WsfPProxyKey& e)
{
   Entry tmp;
   tmp.mAddr                         = e;
   tmp.mNode                         = nullptr;
   std::vector<Entry>::iterator iter = std::lower_bound(mEntries.begin(), mEntries.end(), tmp);
   if (iter != mEntries.end() && iter->mAddr == e)
   {
      return iter->mNode;
   }
   return nullptr;
}
