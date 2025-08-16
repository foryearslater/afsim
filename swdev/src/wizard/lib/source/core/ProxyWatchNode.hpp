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

#ifndef PROXYWATCHNODE_HPP
#define PROXYWATCHNODE_HPP

#include <map>

#include "UtCallback.hpp"
#include "ViExport.hpp"
#include "WsfPProxyKey.hpp"

class WsfPProxyPath;

namespace wizard
{
class ProxyChange;

// A Trie of paths being watched
class VI_EXPORT ProxyWatchNode
{
public:
   typedef UtCallbackListN<void(const ProxyChange&)> UpdateCallbackList;
   friend class ProxyWatcher_Merger;
   friend class ProxyWatcher;

   ProxyWatchNode();
   ~ProxyWatchNode() = default;

   // Called when a value or one of its members have been directly changed
   UpdateCallbackList ChildUpdate;
   UpdateCallbackList ThisUpdate;

   // Called anytime a value owned directly or indirectly has been changed
   UpdateCallbackList AnyUpdate;

private:
   void Merge(ProxyWatchNode& aNode);
   void EmitChange(const ProxyChange&);
   void EmitChangeReverse(const ProxyChange& aChange);
   void EmitChangeImmediate(const ProxyChange& aChange);
   void EmitAnyUpdateImmediate(const ProxyChange& aChange);
   void EmitChildUpdateImmediate(const ProxyChange& aChange);
   void EmitThisUpdateImmediate(const ProxyChange& aChange);

   ProxyWatchNode& GetChild(const WsfPProxyPath& aPath);
   ProxyWatchNode& GetChild(const WsfPProxyKey& aKey);
   ProxyWatchNode* FindChild(const WsfPProxyPath& aPath, int aMaxDepth = 10000);
   ProxyWatchNode* FindLeaf(const WsfPProxyPath& aPath);

   bool CanPrune() const;

   typedef std::map<WsfPProxyKey, ProxyWatchNode> ChildMap;

   ProxyWatchNode* mParent;
   int             mDepth;
   ChildMap        mChildren;
};
} // namespace wizard
#endif // PROXYWATCHNODE_HPP
