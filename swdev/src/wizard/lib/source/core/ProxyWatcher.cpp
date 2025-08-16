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

#include "ProxyWatcher.hpp"

#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyMerge.hpp"
#include "Signals.hpp"
#include "UtMemory.hpp"
#include "UtQt.hpp"

UtCallbackListN<void(WsfPProxy*)>                                 wizard::ProxyWatcher::ProxyAvailable;
UtCallbackListN<void(const WsfPProxyPath&, const WsfPProxyPath&)> wizard::ProxyWatcher::NodeRenamed;

namespace
{
std::unique_ptr<wizard::ProxyWatcher> gProxyWatcherPtr = nullptr;
}

wizard::ProxyWatchNode& wizard::ProxyWatcher::Watch(const WsfPProxyPath& aPath)
{
   return mRoot.GetChild(aPath);
}

wizard::ProxyWatcher::ProxyWatcher()
   : mProxyAvailable(nullptr)
{
   QObject::connect(ProjectWorkspace::Instance(),
                    &ProjectWorkspace::ProxyUpdated,
                    std::bind(&ProxyWatcher::HandleMerge, this, std::placeholders::_1));
   mCallbacks += wizSignals->ProxyInvalidate.Connect(&ProxyWatcher::HandleProxyInvalidate, this);
}

void wizard::ProxyWatcher::Create()
{
   gProxyWatcherPtr = ut::make_unique<ProxyWatcher>();
}

void wizard::ProxyWatcher::Destroy()
{
   gProxyWatcherPtr.reset(nullptr);
}

wizard::ProxyWatcher* wizard::ProxyWatcher::Instance()
{
   return gProxyWatcherPtr.get();
}

namespace wizard
{
class ProxyWatcher_Merger : public ProxyMergeVisitor
{
public:
   ProxyWatcher* mWatcherPtr;
   WsfPProxy*    mProxyPtr;
   ProxyWatcher_Merger(WsfPProxy* aProxyPtr, ProxyWatcher* aWatcherPtr)
   {
      mProxyPtr   = aProxyPtr;
      mWatcherPtr = aWatcherPtr;
   }

   // Accept() is called at every proxy value which has changed, providing the old and new values
   // return 'true' to continue accepting children of the proxy value
   // return 'false' to indicate no children need to be processed
   bool Accept(CompareResult aCompareResult, const WsfPProxyPath& aPath, WsfPProxyValue aOldValue, WsfPProxyValue aNewValue) override
   {
      if (aCompareResult == cNO_CHANGE)
         return false; // shouldn't get here....
      ProxyWatchNode* nodePtr = mWatcherPtr->mRoot.FindLeaf(aPath);
      if (!nodePtr)
         return false;
      switch (aCompareResult)
      {
      case cNEW:
      {
         ProxyChange change(ProxyChange::cADDED, WsfPProxyNode(mProxyPtr, aPath, aNewValue));
         nodePtr->EmitChangeReverse(change);
         return false;
      }
      break;
      case cCHANGED:
      {
         ProxyChange change(ProxyChange::cUPDATED, WsfPProxyNode(mProxyPtr, aPath, aNewValue));
         switch (aNewValue.GetType()->mTypeKind)
         {
         case WsfProxy::cSTRUCT:
            // if struct type has changed, just yield one 'change' of the struct itself
            if (aOldValue.GetType()->mTypeName == aNewValue.GetType()->mTypeName)
            {
               // type hasn't changed, recurse
               return true;
            }
            else
            {
               nodePtr->EmitChangeReverse(change);
               return false;
            }
         case WsfProxy::cLIST:
         {
            nodePtr->EmitChangeReverse(change);
         }
            return false;
         case WsfProxy::cOBJECT_MAP:
            // nodePtr->emitAnyUpdateImmediate(change);
            //  keep descending
            return true;
         default:
            // basic value
            {
               nodePtr->EmitChangeReverse(change);
               // if (nodePtr->mParent) nodePtr->mParent->emitDirectUpdateImmediate(change);
            }
            return false;
         }
      }
      break;
      case cDELETED:
      {
         ProxyChange change(ProxyChange::cREMOVED, WsfPProxyNode(mProxyPtr, aPath, aNewValue));
         nodePtr->EmitChangeReverse(change);
         return false;
      }
      default:
         return false;
      }
   }
};
} // namespace wizard

void wizard::ProxyWatcher::HandleMerge(ProxyMerge* aMerge)
{
   Project*   projectPtr = ProjectWorkspace::Instance()->GetProject();
   WsfPProxy* proxyPtr   = nullptr;
   if (projectPtr)
   {
      proxyPtr = projectPtr->GetParseResults()->Proxy();
   }

   if (proxyPtr)
   {
      if (aMerge)
      {
         ProxyWatcher_Merger visitor(aMerge->GetNewProxy(), this);
         aMerge->Visit(&visitor);
      }
      else
      {
         // broadcast to all
         Project* projectPtr = ProjectWorkspace::Instance()->GetProject();
         if (!projectPtr)
            return;
         WsfPProxy* proxyPtr = projectPtr->GetParseResults()->Proxy();
         if (proxyPtr)
         {
            ProxyChange change(ProxyChange::cUPDATED, WsfPProxyNode::FromProxy(proxyPtr));
            mRoot.EmitChange(change);
         }
      }
   }
   if (mProxyAvailable != proxyPtr)
   {
      mProxyAvailable = proxyPtr;
      ProxyAvailable(proxyPtr);
   }
}

wizard::ProxyWatchNode* wizard::ProxyWatcher::Find(const WsfPProxyPath& aPath, int aMaxDepth)
{
   return mRoot.FindChild(aPath, aMaxDepth);
}

void wizard::ProxyWatcher::HandleProxyMoved(const WsfPProxyNode& aOldNode, const WsfPProxyNode& aNewNode)
{
   ProxyChange change(ProxyChange::cRENAMED, aNewNode, aOldNode.GetName());
   mRoot.EmitChange(change);
   NodeRenamed(aOldNode.GetPath(), aNewNode.GetPath());
}

void wizard::ProxyWatcher::HandleProxyRemoved(const WsfPProxyNode& aNode)
{
   ProxyChange change(ProxyChange::cREMOVED, aNode);
   mRoot.EmitChange(change);
}

void wizard::ProxyWatcher::HandleProxyAdded(const WsfPProxyNode& aNode)
{
   ProxyChange change(ProxyChange::cADDED, aNode);
   mRoot.EmitChange(change);
}

void wizard::ProxyWatcher::HandleProxyModified(const WsfPProxyNode& aNode)
{
   ProxyChange change(ProxyChange::cUPDATED, aNode);
   mRoot.EmitChange(change);
}

void wizard::ProxyWatcher::HandleProxyInvalidate()
{
   if (mProxyAvailable)
   {
      mProxyAvailable = nullptr;
      ProxyAvailable(mProxyAvailable);
   }
}

WsfPProxy* wizard::ProxyWatcher::GetActiveProxy()
{
   return gProxyWatcherPtr->mProxyAvailable;
}

void wizard::ProxyWatcher::EmitOptionChange(const WsfPProxyNode& aNode)
{
   ProxyChange change(ProxyChange::cOPTIONS_CHANGED, aNode);
   mRoot.EmitChange(change);
}

void wizard::ProxyWatcher::EmitOptionChange(const WsfPProxyPath& aPath)
{
   EmitOptionChange(WsfPProxyNode(mProxyAvailable, aPath));
}

wizard::ProxyWatchNode& wizard::WatchProxy(const WsfPProxyPath& aPath)
{
   assert(gProxyWatcherPtr != nullptr);
   return gProxyWatcherPtr->Watch(aPath);
}
