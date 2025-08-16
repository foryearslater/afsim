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

#ifndef PROXYWATCHER_HPP
#define PROXYWATCHER_HPP

#include "ProxyChange.hpp"
#include "ProxyWatchNode.hpp"
#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "WsfPProxyPath.hpp"

class WsfPProxyNode;

namespace wizard
{
class ProxyMerge;
class ProxyWatcher_Merger;

// usage:
//  watchProxy(myPath).DirectUpdate.Connect(this, &MyClass::MyFunction);
class VI_EXPORT ProxyWatcher
{
public:
   friend class ProxyWatcher_Merger;

   // Called when the proxy becomes available or unavailable
   // aProxyPtr is null if not available
   //          ProxyAvailable(WsfPProxy* aProxyPtr);
   static UtCallbackListN<void(WsfPProxy*)> ProxyAvailable;
   static WsfPProxy*                        GetActiveProxy();

   // Called when a proxy node is renamed.
   //          HandleNodeRenamed(const WsfPProxyPath& aOldPath,
   //                            const WsfPProxyNode& aRenamedNode);
   static UtCallbackListN<void(const WsfPProxyPath&, const WsfPProxyPath&)> NodeRenamed;

   ProxyWatcher();
   ~ProxyWatcher() = default;

   static void          Create();
   static void          Destroy();
   static ProxyWatcher* Instance();

   void EmitOptionChange(const WsfPProxyNode& aNode);
   void EmitOptionChange(const WsfPProxyPath& aPath);

   ProxyWatchNode& Watch(const WsfPProxyPath& aPath);

   void HandleProxyModified(const WsfPProxyNode& aNode);
   void HandleProxyAdded(const WsfPProxyNode& aNode);
   void HandleProxyRemoved(const WsfPProxyNode& aNode);
   void HandleProxyMoved(const WsfPProxyNode& aOldNode, const WsfPProxyNode& aNewNode);

private:
   void            HandleMerge(ProxyMerge* aMerge);
   void            HandleProxyInvalidate();
   void            NotifyUp(const ProxyChange& aChange);
   void            NotifyDown(const ProxyChange& aChange);
   ProxyWatchNode* Find(const WsfPProxyPath& aPath, int aMaxDepth = 10000);

   UtCallbackHolder mCallbacks;
   ProxyWatchNode   mRoot;
   WsfPProxy*       mProxyAvailable;
};

VI_EXPORT ProxyWatchNode& WatchProxy(const WsfPProxyPath& aPath);
} // namespace wizard
#endif
