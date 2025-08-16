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

#ifndef PROXYMERGE_HPP
#define PROXYMERGE_HPP

#include <map>

#include "UtMemoryPool.hpp"
#include "UtSHA.hpp"
#include "ViExport.hpp"
#include "WsfPProxyPath.hpp"
#include "WsfPProxyValue.hpp"

class WsfPProxy;

namespace wizard
{
class VI_EXPORT ProxyHashNode
{
public:
   UtSHA_Digest mDigest;
   ~ProxyHashNode();
   ProxyHashNode* FindChild(const WsfPProxyKey& e);
   // Initialize() Must be called prior to calling FindChild()
   void           Initialize();
   ProxyHashNode* Add(const WsfPProxyKey& aAddr);
   struct Entry
   {
      bool operator<(const Entry& e) const { return mAddr < e.mAddr; }

      WsfPProxyKey   mAddr;
      ProxyHashNode* mNode;
   };

   const std::vector<Entry>& Children() { return mEntries; }

private:
   // Define a std::map with a memory pool allocator.  This provides a sorted container with contiguous memory.
   // typedef UtMemoryPoolAllocator<ProxyHashNode> NodeAllocator;
   // typedef std::map<WsfPProxyPathEntry, ProxyHashNode, std::less<WsfPProxyPathEntry>, NodeAllocator> NodeMap;
   // typedef std::unordered_map<WsfPProxyPathEntry, ProxyHashNode*, WsfPProxyPathEntryHash>  NodeMap;
   // NodeMap                           mChildren;
   std::vector<Entry> mEntries;
};

class ProxyHash
{
public:
   explicit ProxyHash(WsfPProxyValue aRootValue);

   ProxyHashNode* Find(const WsfPProxyPath& aPath);
   ProxyHashNode& Root() { return mRoot; }
   WsfPProxyValue RootValue() const { return mRootValue; }

protected:
   WsfPProxyHash  RecurseHash(ProxyHashNode*      aParentPtr,
                              const WsfPProxyKey& aValueAddr,
                              WsfPProxyValue      aRootValue,
                              bool                aIsParentStruct);
   ProxyHashNode  mRoot;
   WsfPProxyValue mRootValue;
   UT_MEMORY_DEBUG_MARKER(cMDB_ProxyHash);
};

class ProxyMergeVisitor
{
public:
   virtual ~ProxyMergeVisitor() = default;
   enum CompareResult
   {
      cNEW,     // aNewValue did not exist before, aOldValue is null
      cCHANGED, // The value has changed.  aNewValue contains the new value.
      cDELETED, // The value has been deleted, aNewValue is null
      cNO_CHANGE
   };

   // Accept() is called at every proxy value which has changed, providing the old and new values
   // return 'true' to continue accepting children of the proxy value
   // return 'false' to indicate no children need to be processed
   virtual bool Accept(CompareResult        aCompareResult,
                       const WsfPProxyPath& aPath,
                       WsfPProxyValue       aOldValue,
                       WsfPProxyValue       aNewValue) = 0;
};

class ProxyMerge
{
public:
   ProxyMerge(ProxyHash* aOldHash, ProxyHash* aNewHash, WsfPProxy* aOldProxy, WsfPProxy* aNewProxy);

   void Visit(ProxyMergeVisitor* aVisitor);

   WsfPProxyValue GetOldRoot() const { return mOldHash ? mOldHash->RootValue() : WsfPProxyValue(); }
   WsfPProxyValue GetNewRoot() const { return mNewHash ? mNewHash->RootValue() : WsfPProxyValue(); }
   WsfPProxy*     GetOldProxy() const { return mOldProxy; }
   WsfPProxy*     GetNewProxy() const { return mNewProxy; }

private:
   void VisitR(ProxyMergeVisitor* aVisitor,
               ProxyHashNode*     aNewHash,
               ProxyHashNode*     aOldHash,
               WsfPProxyValue     aNewValue,
               WsfPProxyValue     aOldValue,
               WsfPProxyPath      aPath);
   void VisitBasicChildren(ProxyMergeVisitor* aVisitorPtr,
                           WsfPProxyValue     aNewValue,
                           WsfPProxyValue     aOldValue,
                           WsfPProxyPath&     aPath);

   ProxyHash* mOldHash;
   ProxyHash* mNewHash;
   WsfPProxy* mOldProxy;
   WsfPProxy* mNewProxy;
};
} // namespace wizard
#endif
