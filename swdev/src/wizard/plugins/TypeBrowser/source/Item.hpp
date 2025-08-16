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

#ifndef ITEM_HPP
#define ITEM_HPP

#include "AbstractModel.hpp"
#include "WsfPProxyNode.hpp"
#include "WsfPProxyValue.hpp"

namespace TypeBrowser
{
//! An item in the file system model.
class Item : public wizard::AbstractModelItem
{
public:
   Item()
      : wizard::AbstractModelItem()
      , mTypeKind(0)
      , mToPare(false)
   {
   }
   ~Item() override;
   void           SetBase(Item* aBaseItemPtr);
   WsfPProxyValue GetValue();
   // WsfPProxyPath GetProxyPath() { return mTypePath.mPath; }
   WsfPProxyNode GetProxyNode() { return mTypeNode; }

private:
   bool SortsBefore(Item* aOtherPtr);

public:
   QString       mName;
   WsfPProxyNode mTypeNode;
   int           mTypeKind;
   bool          mToPare;
};
} // namespace TypeBrowser
#endif
