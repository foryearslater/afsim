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

#include "ProxyWatchNode.hpp"

#include "ProxyChange.hpp"

wizard::ProxyWatchNode::ProxyWatchNode()
   : mParent(nullptr)
   , mDepth(0)
{
}

void wizard::ProxyWatchNode::Merge(ProxyWatchNode& aNode)
{
   assert(mDepth == aNode.mDepth);
   ThisUpdate.Merge(aNode.ThisUpdate);
   ChildUpdate.Merge(aNode.ChildUpdate);
   AnyUpdate.Merge(aNode.AnyUpdate);
   for (ChildMap::iterator i = aNode.mChildren.begin(); i != aNode.mChildren.end(); ++i)
   {
      GetChild(i->first).Merge(i->second);
   }
}

wizard::ProxyWatchNode& wizard::ProxyWatchNode::GetChild(const WsfPProxyPath& aPath)
{
   if (mDepth < (int)aPath.size())
   {
      return GetChild(aPath[mDepth]).GetChild(aPath);
   }
   return *this;
}

wizard::ProxyWatchNode& wizard::ProxyWatchNode::GetChild(const WsfPProxyKey& aKey)
{
   wizard::ProxyWatchNode& child = mChildren[aKey];
   if (child.mDepth > mDepth)
      return child;
   child.mParent = this;
   child.mDepth  = mDepth + 1;
   return child;
}

wizard::ProxyWatchNode* wizard::ProxyWatchNode::FindChild(const WsfPProxyPath& aPath, int aMaxDepth)
{
   if (mDepth < (int)aPath.size() && mDepth < aMaxDepth)
   {
      ChildMap::iterator iter = mChildren.find(aPath[mDepth]);
      if (iter != mChildren.end())
      {
         return iter->second.FindChild(aPath, aMaxDepth);
      }
      return nullptr;
   }
   return this;
}

// find child at path or leaf node if no child exists
wizard::ProxyWatchNode* wizard::ProxyWatchNode::FindLeaf(const WsfPProxyPath& aPath)
{
   if (mDepth < (int)aPath.size())
   {
      ChildMap::iterator iter = mChildren.find(aPath[mDepth]);
      if (iter != mChildren.end())
      {
         return iter->second.FindLeaf(aPath);
      }
   }
   return this;
}

bool wizard::ProxyWatchNode::CanPrune() const
{
   return mChildren.empty() && ThisUpdate.IsEmpty() && ChildUpdate.IsEmpty() && AnyUpdate.IsEmpty();
}

void wizard::ProxyWatchNode::EmitChange(const ProxyChange& aChange)
{
   AnyUpdate(aChange);
   size_t pathSize = aChange.path().size();
   if (mDepth + 1 == (int)pathSize && aChange.reason() == ProxyChange::cRENAMED)
   {
      // for rename, move watchers to new value
      ChildMap::iterator oldIter = mChildren.find(aChange.oldName());
      if (oldIter != mChildren.end())
      {
         GetChild(aChange.path()[mDepth]).Merge(oldIter->second);
      }
   }
   if (mDepth < (int)pathSize)
   {
      const WsfPProxyKey& key  = aChange.path()[mDepth];
      ChildMap::iterator  iter = mChildren.find(key);
      if (iter != mChildren.end())
      {
         iter->second.EmitChange(aChange);
         if (iter->second.CanPrune())
         {
            mChildren.erase(iter);
         }
      }
   }
   if (mDepth + 1 == (int)pathSize)
   {
      ChildUpdate(aChange);
   }
   else if (mDepth == (int)pathSize)
   {
      ThisUpdate(aChange);
   }
}

void wizard::ProxyWatchNode::EmitChangeReverse(const ProxyChange& aChange)
{
   if (mParent)
      mParent->EmitChangeReverse(aChange);
   AnyUpdate(aChange);
   size_t pathSize = aChange.path().size();
   if (mDepth + 1 == (int)pathSize)
   {
      ChildUpdate(aChange);
   }
   else if (mDepth == (int)pathSize)
   {
      ThisUpdate(aChange);
   }
}

void wizard::ProxyWatchNode::EmitChangeImmediate(const ProxyChange& aChange)
{
   AnyUpdate(aChange);
   if (mDepth + 1 == (int)aChange.path().size())
   {
      ChildUpdate(aChange);
   }
   else if (mDepth == (int)aChange.path().size())
   {
      ThisUpdate(aChange);
   }
}

void wizard::ProxyWatchNode::EmitAnyUpdateImmediate(const ProxyChange& aChange)
{
   AnyUpdate(aChange);
}

void wizard::ProxyWatchNode::EmitChildUpdateImmediate(const ProxyChange& aChange)
{
   ChildUpdate(aChange);
}

void wizard::ProxyWatchNode::EmitThisUpdateImmediate(const ProxyChange& aChange)
{
   ThisUpdate(aChange);
}
