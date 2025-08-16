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

#include "Model.hpp"

#include "ComponentPropertyManager.hpp"
#include "DragAndDrop.hpp"
#include "ProxyWatcher.hpp"
#include "WkfEnvironment.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WsfPProxyStructType.hpp"

CommandChain::Model::Model()
   : mRootItem(ut::make_unique<Item>())
{
   mRootItem->mName = "scenario";
   mCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Model::ProxyAvailable, this);
   SetRoot(mRootItem.get());
   ProxyAvailable(wizard::ProxyWatcher::GetActiveProxy());
}

CommandChain::Model::~Model()
{
   ClearScenario();

   SetRoot(nullptr);
}

void CommandChain::Model::ProxyAvailable(WsfPProxy* aProxy)
{
   if (aProxy)
   {
      WsfPProxyStructType* platformStructPtr =
         static_cast<WsfPProxyStructType*>(aProxy->GetRegistry()->GetType("Platform"));
      if (platformStructPtr != nullptr)
      {
         mCommandChainPropertyIndex = platformStructPtr->GetMemberIndex("commandChains");

         mProxyCallbacks +=
            wizard::WatchProxy(WsfPM_Root(aProxy).platforms().GetPath()).ChildUpdate.Connect(&Model::PlatformListChange, this);
         mProxyCallbacks +=
            wizard::WatchProxy(WsfPM_Root(aProxy).platforms().GetPath()).AnyUpdate.Connect(&Model::AnyPlatformChange, this);

         mProxyRoot                  = aProxy;
         WsfPM_PlatformMap platforms = mProxyRoot.platforms();
         for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
         {
            WsfPM_Platform plat = platforms.AtT(i);
            PlatformAdded(plat);
         }
         AddChain("default"); // make sure we always show the 'default' command chain
      }
      else
      {
         mCommandChainPropertyIndex = ut::npos;
      }
   }
   else
   {
      mRootItem->RemoveAllAbstractItems(true);
      mCommandChainMap.clear();
      mPlatformMap.clear();
      mProxyRoot = WsfPM_Root();
      mProxyCallbacks.Clear();
   }
}

QVariant CommandChain::Model::data(const QModelIndex& index, int role) const
{
   Item* itemPtr = dynamic_cast<Item*>(FindItem(index));
   if (!itemPtr)
      return QVariant();

   switch (role)
   {
   case Qt::DisplayRole:
   {
      QString displayName = QString::fromStdString(itemPtr->mName);
      if (itemPtr->IsUnassignedRoot())
      {
         displayName += " (" + QString::number(itemPtr->GetAbstractItemCount()) + ")";
      }
      return displayName;
   }
   break;
   case Qt::DecorationRole:
      return itemPtr->GetDecorationRole();
   case Qt::ToolTipRole:
   {
      if (itemPtr->Loops())
      {
         return QString("Loop detected");
      }
   }
      return QVariant();
      break;
   case Qt::BackgroundColorRole:
   {
      if (itemPtr->mPlatform.IsValid())
      {
         UtColor color(
            wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(itemPtr->mPlatform.Side()));

         return QColor(color[0] * 255, color[1] * 255, color[2] * 255, 85);
      }
   }
      return QVariant();
      break;
   case cDEFAULT_EXPANDED:
   {
      // Do we want this node expanded?
      return !itemPtr->IsUnassignedRoot();
   }
   case cSIDE_NAME:
      if (itemPtr->mPlatform.IsValid())
      {
         return QString::fromStdString(itemPtr->mPlatform.Side());
      }
      else
      {
         return QString();
      }
   default:
      return QVariant();
      break;
   }
}

void CommandChain::Model::DeleteItem(Item* aItemPtr)
{
   if (aItemPtr->mIsPlatform)
   {
      if (mPlatformMap.find(aItemPtr->mCommandChain) != mPlatformMap.end())
      {
         NameToPlatform& ntp = mPlatformMap[aItemPtr->mCommandChain];
         if (ntp.find(aItemPtr->mName) != ntp.end())
         {
            ntp.erase(aItemPtr->mName);
         }
      }
   }
   else if (mCommandChainMap.find(aItemPtr->mName) != mCommandChainMap.end() && mCommandChainMap[aItemPtr->mName] == aItemPtr)
   {
      mCommandChainMap.erase(aItemPtr->mName);
   }
}

CommandChain::Item* CommandChain::Model::UsePlatformItem(const std::string& aPlatformName,
                                                         const std::string& aChainName,
                                                         Item*              aChainRoot)
{
   Item* nodePtr = FindPlatformModelItem(aChainName, aPlatformName);
   if (nodePtr)
      return nodePtr;
   nodePtr                                 = new Item;
   nodePtr->mIsPlatform                    = true;
   nodePtr->mName                          = aPlatformName;
   nodePtr->mCommandChain                  = aChainName;
   nodePtr->mPlatform                      = mProxyRoot.platforms().FindT(aPlatformName);
   mPlatformMap[aChainName][aPlatformName] = nodePtr;

   // if the platform doesn't exist or if currently unassigned, move it to top-level.
   //    if (!nodePtr->mPlatform.IsValid() || nodePtr->Parent()->IsUnassignedRoot())
   //    {
   //
   //    }
   MoveNode(nodePtr, static_cast<Item*>(aChainRoot->GetAbstractItem(0)));
   // aChainRoot);
   //    else
   //    {
   //       MoveNode(nodePtr, (ViCommandChainModelItem*)aChainRoot->GetAbstractItem(0));
   //    }
   return nodePtr;
}

bool CommandChain::Model::MoveNode(Item* aChild, Item* aParent)
{
   Item* oldParent = static_cast<Item*>(aChild->GetAbstractItemParent());
   if ((aParent == oldParent) || (aParent == nullptr))
   {
      return true;
   }

   bool  isLoop   = false;
   Item* ancestor = aParent;
   while (ancestor != nullptr)
   {
      if (ancestor == aChild)
      {
         // detected loop
         isLoop = true;
      }
      ancestor = static_cast<Item*>(ancestor->GetAbstractItemParent());
   }
   if (isLoop)
      return false;

   int index = -1;
   int count = aParent->GetAbstractItemCount();
   int i     = 0;
   for (; i < count; ++i)
   {
      Item* otherItem = dynamic_cast<Item*>(aParent->GetAbstractItem(i));
      if (aChild->SortsBefore(otherItem))
      {
         index = i;
         break;
      }
   }

   if (aChild->GetAbstractItemParent() == nullptr)
   {
      aParent->AddAbstractItem(aChild, index);
   }
   else
   {
      aParent->MoveAbstractItem(aChild, index);
   }
   if (oldParent && oldParent->mIsPlatform && !oldParent->IsUnassignedRoot())
   {
      TryPrune(oldParent);
   }
   return true;
}

CommandChain::Item* CommandChain::Model::FindPlatformModelItem(const std::string& aCommandChainName,
                                                               const std::string& aPlatformName)
{
   Item* result = nullptr;

   NameToPlatform&          platMap = mPlatformMap[aCommandChainName];
   NameToPlatform::iterator iter    = platMap.find(aPlatformName);
   if (iter != platMap.end())
   {
      result = iter->second;
   }

   return result;
}

void CommandChain::Model::PlatformListChange(const wizard::ProxyChange& aChange)
{
   WsfPM_Platform changed(aChange.changed());
   switch (aChange.reason())
   {
   case wizard::ProxyChange::cADDED:
      PlatformAdded(aChange.changed());
      break;
   case wizard::ProxyChange::cREMOVED:
      PlatformRemoved(aChange.path().Back().GetMapKey());
      break;
   case wizard::ProxyChange::cRENAMED:
      PlatformRenamed(aChange.changed(), aChange.oldName());
      break;
   default:
      break;
   }
}

void CommandChain::Model::AnyPlatformChange(const wizard::ProxyChange& aChange)
{
   const WsfPProxyPath& path = aChange.path();
   if (path.size() != 4)
      return;
   // looking for path of 4 elements like this:
   // platform.<PlatformName>.commandChains.<ChainName>
   if (path[2].GetIndex() != mCommandChainPropertyIndex)
      return;
   WsfPM_Platform platform(aChange.changed());
   platform.TrimPath(2);

   switch (aChange.reason())
   {
   case wizard::ProxyChange::cADDED: // a command chain is being added, platformAdded() will take care of this
      PlatformAdded(platform);
   case wizard::ProxyChange::cUPDATED: // a commander has been changed
      PlatformAdded(platform);
      break;
   case wizard::ProxyChange::cREMOVED: // command chain membership removed
      TryPrune(FindPlatformModelItem(aChange.path().Back().GetMapKey(), platform.GetName()));
      TryPrune(FindCommandChainRoot(aChange.path().Back().GetMapKey()));
      UpdateMembership(platform);
      break;
   default:
      break;
   }
}

void CommandChain::Model::ClearScenario()
{
   mRootItem->RemoveAllAbstractItems(true);
   mPlatformMap.clear();
   mCommandChainMap.clear();
}

CommandChain::Item* CommandChain::Model::FindCommandChainRoot(const std::string& aCommandChainName)
{
   CommandChainMap::iterator iter = mCommandChainMap.find(aCommandChainName);
   if (iter != mCommandChainMap.end())
   {
      return iter->second;
   }
   return nullptr;
}

CommandChain::Item* CommandChain::Model::RealizeCommandChainRoot(const std::string& aCommandChainName)
{
   CommandChainMap::iterator iter = mCommandChainMap.find(aCommandChainName);
   if (iter != mCommandChainMap.end())
   {
      return iter->second;
   }
   Item* newChain          = new Item;
   newChain->mName         = aCommandChainName;
   newChain->mCommandChain = aCommandChainName;
   newChain->mIsPlatform   = false;
   MoveNode(newChain, mRootItem.get());
   mCommandChainMap[aCommandChainName] = newChain;

   Item* unassignedRoot              = new Item;
   unassignedRoot->mIsPlatform       = true;
   unassignedRoot->mName             = Item::cUNASSIGNED_ROOT;
   unassignedRoot->mCommandChain     = aCommandChainName;
   unassignedRoot->mIsUnassignedRoot = true;
   MoveNode(unassignedRoot, newChain);

   // if a command chain is added, make sure all platforms are added to it
   WsfPM_PlatformMap platforms = mProxyRoot.platforms();
   for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
   {
      UpdateMembership(platforms.AtT(i), aCommandChainName, newChain);
   }

   CommandChainListChange();

   return newChain;
}


QMimeData* CommandChain::Model::mimeData(const QModelIndexList& indexes) const
{
   wizard::DragAndDrop::PropertyList propList;
   for (auto&& idx : indexes)
   {
      Item* itemPtr = dynamic_cast<Item*>(FindItem(idx));
      if (itemPtr && itemPtr->mIsPlatform)
      {
         auto platformProxy = itemPtr->GetPlatformProxy();
         if (platformProxy.IsValid())
         {
            wizard::DragAndDrop::PropertyMap propMap;
            propMap["proxy-node"]    = wizard::DragAndDrop::ToString(platformProxy);
            propMap["command-chain"] = QString::fromStdString(itemPtr->mCommandChain);
            propList << propMap;
         }
      }
   }
   if (propList.empty())
      return nullptr;
   return wizard::DragAndDrop::CreatePropertySetMime(propList);
}

Qt::ItemFlags CommandChain::Model::flags(const QModelIndex& index) const
{
   Qt::ItemFlags defaultFlags = wizard::AbstractModel::flags(index);
   Item*         itemPtr      = dynamic_cast<Item*>(FindItem(index));
   if (itemPtr)
   {
      if (itemPtr->mIsPlatform && !itemPtr->IsUnassignedRoot())
      {
         defaultFlags |= Qt::ItemIsDragEnabled;
      }
      defaultFlags |= Qt::ItemIsDropEnabled;
   }
   return defaultFlags;
}

void CommandChain::Model::PlatformAdded(const WsfPM_Platform& aPlatform)
{
   // if (mPlatformMap.find(aPlatform.GetName()) != mPlatformMap.end()) return;  // error check
   std::vector<std::string> chains = aPlatform.CommandChainNames();
   for (auto&& chain : chains)
   {
      AddChain(chain);
   }
   UpdateMembership(aPlatform);
}

void CommandChain::Model::PlatformRemoved(const std::string& aName)
{
   for (CommandChainMap::iterator i = mCommandChainMap.begin(); i != mCommandChainMap.end();)
   {
      // ViCommandChainModelItem* chainRoot = i->second;
      Item* itemPtr = FindPlatformModelItem(i->first, aName);
      ++i; // iterator may go invalid
      if (itemPtr)
      {
         TryPrune(itemPtr);
      }
   }
}
void CommandChain::Model::PlatformRenamed(const WsfPM_Platform& aPlatform, const std::string& aOldName)
{
   for (CommandChainMap::iterator i = mCommandChainMap.begin(); i != mCommandChainMap.end();)
   {
      std::string chainName = i->first;
      // ViCommandChainModelItem* chainRoot = i->second;
      Item* existingItemPtr = FindPlatformModelItem(chainName, aOldName);
      if (!existingItemPtr)
      {
         ++i;
         continue; // should not happen...
      }
      existingItemPtr->mPlatform = aPlatform;
      existingItemPtr->mName     = aPlatform.GetName();

      Item* itemPtr = FindPlatformModelItem(i->first, aPlatform.GetName());

      ++i; // iterator may go invalid
      if (itemPtr)
      {
         TryPrune(itemPtr);
      }

      mPlatformMap[chainName][existingItemPtr->mName] = existingItemPtr;
      mPlatformMap[chainName].erase(aOldName);
      existingItemPtr->AbstractItemChanged();
   }
}

void CommandChain::Model::TryPrune(Item* aItemPtr)
{
   if (!aItemPtr)
      return;
   if (aItemPtr->GetAbstractItemParent() == nullptr)
      return;
   if (aItemPtr->IsUnassignedRoot())
   {
      TryPrune(static_cast<Item*>(aItemPtr->GetAbstractItemParent()));
      return;
   }
   if (aItemPtr->GetAbstractItemParent() == mRootItem.get()) // if item is a command chain
   {
      if (aItemPtr->GetAbstractItemCount() == 1) // if everything is in the 'unassigned' group
      {
         if (aItemPtr->mName != "default") // don't remove the 'default' command chain
         {
            aItemPtr->BeforeRemove();

            mRootItem->RemoveAbstractItem(aItemPtr);
            mCommandChainMap.erase(aItemPtr->mName);
            mPlatformMap.erase(aItemPtr->mName);
            DeleteItem(aItemPtr);
            CommandChainListChange();
         }
      }
      return;
   }
   // if the platform is no longer a top-level commander, move to unassigned
   if (aItemPtr->mPlatform.IsValid())
   {
      if (aItemPtr->GetAbstractItemCount() == 0 && aItemPtr->mPlatform.CommanderName(aItemPtr->mCommandChain).empty())
      {
         Item* chainRoot = aItemPtr->FindChainRoot();
         if (chainRoot)
         {
            // if cmdr is not set and no subordinates, put it in 'unassigned'
            MoveNode(aItemPtr, static_cast<Item*>(chainRoot->GetAbstractItem(0)));
         }
      }
      return;
   }

   if (aItemPtr->CanPrune())
   {
      Item* parentPtr = static_cast<Item*>(aItemPtr->GetAbstractItemParent());
      parentPtr->RemoveAbstractItem(aItemPtr);
      DeleteItem(aItemPtr);
      TryPrune(parentPtr);
   }
}

void CommandChain::Model::AddChain(const std::string& aChainName)
{
   if (mCommandChainMap.find(aChainName) != mCommandChainMap.end())
      return;
   RealizeCommandChainRoot(aChainName);
}

void CommandChain::Model::UpdateMembership(const WsfPM_Platform& aPlatform)
{
   CommandChainMap existingChains = mCommandChainMap;
   for (auto&& it : existingChains)
   {
      UpdateMembership(aPlatform, it.first, it.second);
   }
}

void CommandChain::Model::UpdateMembership(const WsfPM_Platform& aPlatform, const std::string& aChainName, Item* aChainRoot)
{
   std::string platformName = aPlatform.GetName();
   Item*       platformItem = UsePlatformItem(platformName, aChainName, aChainRoot);
   std::string cmdrName     = aPlatform.CommanderName(aChainName);

   if (cmdrName == WsfPM_Platform::CommanderSelfValue() || cmdrName == platformName ||
       (cmdrName.empty() && platformItem->GetAbstractItemCount() > 0))
   {
      // if designated as commander self, or no commander designation but has children
      MoveNode(platformItem, aChainRoot);
   }
   else if (cmdrName.empty() && platformItem->GetAbstractItemCount() == 0)
   {
      // if cmdr is not set and no subordinates, put it in 'unassigned'
      MoveNode(platformItem, static_cast<Item*>(aChainRoot->GetAbstractItem(0)));
   }
   else
   {
      // Else, has commander.
      Item* cmdrItem = static_cast<Item*>(UsePlatformItem(cmdrName, aChainName, aChainRoot));
      // add to cmdr
      if (!MoveNode(platformItem, cmdrItem))
      {
         // loop detected, insert at root
         MoveNode(platformItem, aChainRoot);
      }
      else // added successfully, make sure cmdr isn't in unassigned list
      {
         if (cmdrItem->Parent()->IsUnassignedRoot())
         {
            MoveNode(cmdrItem, cmdrItem->FindChainRoot());
         }
      }
   }
}
