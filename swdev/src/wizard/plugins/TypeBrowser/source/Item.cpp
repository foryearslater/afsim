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

#include "Item.hpp"

TypeBrowser::Item::~Item() {}

bool TypeBrowser::Item::SortsBefore(Item* aOtherPtr)
{
   if (mTypeKind < aOtherPtr->mTypeKind)
   {
      return true;
   }
   else if ((mTypeKind == aOtherPtr->mTypeKind) && (mName < aOtherPtr->mName))
   {
      return true;
   }
   return false;
}

void TypeBrowser::Item::SetBase(Item* aBaseItemPtr)
{
   Item* basePtr = dynamic_cast<Item*>(GetAbstractItemParent());
   if (basePtr != aBaseItemPtr)
   {
      if (basePtr)
      {
         aBaseItemPtr->MoveAbstractItem(this);
      }
      else
      {
         aBaseItemPtr->AddAbstractItem(this);
      }
   }
}

// It isn't safe to store the proxy value, as it gets deleted when a new proxy is generated
WsfPProxyValue TypeBrowser::Item::GetValue()
{
   return mTypeNode.GetValue();
}
