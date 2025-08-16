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
#ifndef PROXYCHANGE_HPP
#define PROXYCHANGE_HPP

#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"

namespace wizard
{
//! Records a change to proxy data
class VI_EXPORT ProxyChange
{
public:
   enum Reason
   {
      cADDED,           // changed() is the item added.
      cREMOVED,         // changed() is the removed item.  the node is not valid, but the path is.
      cRENAMED,         // changed() is the item renamed at its new location.
      cUPDATED,         // changed() is the item updated.  May have changed any children too.
      cLIST_INSERTION,  // changed() is the new item.  other items were shifted right
      cLIST_DELETION,   // changed() is where the item used to be.
      cOPTIONS_CHANGED, // changed() is the proxy path whose options changed
      cUNKNOWN          //(invalid)
   };

   ProxyChange(Reason aReason, const WsfPProxyNode& aNode);
   ProxyChange(Reason aReason, const WsfPProxyNode& aNode, const std::string& aOldName);
   ~ProxyChange() = default;

   //! The path changed
   const WsfPProxyPath& path() const { return mChanged.GetPath(); }
   //! The node changed
   const WsfPProxyNode& changed() const { return mChanged; }

   // Valid for cRENAMED only;  the previous name and path of the object
   const std::string& oldName() const { return mOldName; }
   WsfPProxyPath      oldPath() const { return mChanged.GetParent().GetPath() + mOldName; }

   //! What kind of change
   Reason reason() const { return mReason; }

private:
   Reason        mReason;
   WsfPProxyNode mChanged;
   std::string   mOldName;
};
} // namespace wizard
#endif
