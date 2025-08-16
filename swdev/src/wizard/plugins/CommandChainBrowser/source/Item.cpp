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

#include "ComponentPropertyManager.hpp"
#include "Model.hpp"
#include "UiResources.hpp"
#include "WsfPM_Platform.hpp"

const std::string CommandChain::Item::cUNASSIGNED_ROOT{"Unassigned Platforms"};

bool CommandChain::Item::SortsBefore(Item* aOtherPtr)
{
   if (mIsUnassignedRoot)
      return true;
   if (aOtherPtr->mIsUnassignedRoot)
      return false;
   if (mName < aOtherPtr->mName)
   {
      return true;
   }
   return false;
}

WsfPProxyNode CommandChain::Item::GetProxyChain()
{
   WsfPProxyNode chainNode;
   if (mPlatform)
   {
      WsfPM_ObjectMap commandChains{mPlatform.Commanders()};
      if (commandChains)
      {
         chainNode = commandChains.Find(mCommandChain);
      }
   }
   return chainNode;
}

bool CommandChain::Item::CanPrune()
{
   if (mPlatform.IsValid())
   {
      return GetAbstractItemCount() == 0 && mPlatform.CommanderName(mCommandChain).empty();
   }
   if (GetAbstractItemCount() == 0)
      return true;
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      Item* child = dynamic_cast<Item*>(GetAbstractItem(i));
      if (!child->CanPrune())
         return false;
   }
   return true;
}

CommandChain::Model* CommandChain::Item::ChainModel()
{
   return static_cast<Model*>(GetModel());
}

void CommandChain::Item::BeforeRemove()
{
   Model* modelPtr = ChainModel();
   if (modelPtr)
   {
      for (int i = 0; i < GetAbstractItemCount(); ++i)
      {
         (static_cast<Item*>(GetAbstractItem(i)))->BeforeRemove();
      }
      modelPtr->BeforeRemove(this);
   }
}

bool CommandChain::Item::Loops() const
{
   Item* parentPtr = static_cast<Item*>(GetAbstractItemParent());
   if (mIsPlatform && !mIsUnassignedRoot && !parentPtr->mIsPlatform && mPlatform.IsValid())
   {
      std::string cmdr       = mPlatform.CommanderName(mCommandChain);
      bool        validValue = (cmdr == WsfPM_Platform::CommanderSelfValue() || cmdr == mName || cmdr.empty());
      return !validValue;
   }
   return false;
}

std::string CommandChain::Item::Side() const
{
   return mPlatform.Side();
}

QVariant CommandChain::Item::GetDecorationRole()
{
   // check if this node is part of a loop
   if (Loops())
   {
      return wizard::UiResources::GetInstance().GetIconFromTheme(QString("infinity"));
   }

   // check if this is a command chain node
   if (!mIsPlatform)
   {
      return wizard::UiResources::GetInstance().GetIconFromTheme(QString("network"));
   }

   // Unassigned
   if (mName == Item::cUNASSIGNED_ROOT)
   {
      return wizard::UiResources::GetInstance().GetIconFromTheme(QString("delete"));
   }

   // check if the platform is a valid existing platform
   if (!mPlatform.IsValid())
   {
      return wizard::UiResources::GetInstance().GetIconFromTheme(QString("questionyellow"));
   }

   QString partStr = wizard::ComponentPropertyManager::Instance()->FindPartIcon(mPlatform.GetValue());
   if (!partStr.isEmpty())
   {
      return wizard::UiResources::GetInstance().GetImageFromTheme(partStr);
   }

   return QVariant();
}
