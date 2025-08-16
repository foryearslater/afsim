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
#include "WsfPM_Platform.hpp"
#include "WsfPProxyNode.hpp"

namespace CommandChain
{
class Model;

class Item : public wizard::AbstractModelItem
{
public:
   static const std::string cUNASSIGNED_ROOT;

   Item()
      : wizard::AbstractModelItem()
      , mCommandChain("")
      , mIsPlatform(false)
      , mIsUnassignedRoot(false)
   {
   }
   ~Item() override = default;

   Model*      ChainModel();
   bool        IsUnassignedRoot() const { return mIsUnassignedRoot; }
   bool        Loops() const;
   std::string Side() const;

   WsfPProxyNode  GetProxyChain();
   WsfPM_Platform GetPlatformProxy() const { return mPlatform; }

   bool SortsBefore(Item* aOtherPtr);

   bool  CanPrune();
   void  BeforeRemove();
   Item* Parent() { return static_cast<Item*>(GetAbstractItemParent()); }
   Item* FindChainRoot()
   {
      if (mIsPlatform || mIsUnassignedRoot)
      {
         Item* p = Parent();
         if (p)
         {
            return p->FindChainRoot();
         }
      }
      else
      {
         return this;
      }
      return nullptr;
   }
   virtual QVariant GetDecorationRole();

   WsfPM_Platform mPlatform;
   std::string    mName;
   std::string    mCommandChain;
   bool           mIsPlatform; // if true, this is a platform item, not a command chain root item
   bool           mIsUnassignedRoot;
};
} // namespace CommandChain

#endif
