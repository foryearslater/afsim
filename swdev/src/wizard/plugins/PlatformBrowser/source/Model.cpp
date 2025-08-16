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
#include "Item.hpp"
#include "ProxyWatcher.hpp"
#include "UiResources.hpp"
#include "WkfEnvironment.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WsfPProxyBasicValue.hpp"

PlatformBrowser::Model::Model()
{
   mRootItem        = new Item();
   mRootItem->mName = "scenario";
   SetRoot(mRootItem);
   mCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Model::ProxyAvailable, this);
   ProxyAvailable(wizard::ProxyWatcher::GetActiveProxy());
   mUpdateTimer.setInterval(1000);
   QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &Model::UpdateTimeout);
   mUpdateTimer.start();
}

PlatformBrowser::Model::~Model()
{
   SetRoot(nullptr);
   delete mRootItem;
}

Qt::ItemFlags PlatformBrowser::Model::flags(const QModelIndex& index) const
{
   Qt::ItemFlags defaultFlags = wizard::AbstractModel::flags(index);
   defaultFlags |= Qt::ItemIsDragEnabled;
   return defaultFlags;
}

QMimeData* PlatformBrowser::Model::mimeData(const QModelIndexList& indexes) const
{
   wizard::DragAndDrop::PropertyList propList;
   for (const auto& index : indexes)
   {
      auto* itemPtr = dynamic_cast<Item*>(FindItem(index));
      if (itemPtr && itemPtr->GetNodeType() == Item::NodeType::cPLATFORM)
      {
         wizard::DragAndDrop::PropertyMap propMap;
         propMap["proxy-node"] = wizard::DragAndDrop::ToString(itemPtr->GetProxyNode());
         propList << propMap;
      }
   }
   if (propList.empty())
      return nullptr;
   return wizard::DragAndDrop::CreatePropertySetMime(propList);
}

QVariant PlatformBrowser::Model::data(const QModelIndex& index, int role) const
{
   auto* itemPtr = dynamic_cast<Item*>(FindItem(index));
   if (!itemPtr)
      return QVariant();

   switch (role)
   {
   case Qt::DisplayRole:
      return QString::fromStdString(itemPtr->mName);
   case Qt::DecorationRole:
   {
      WsfPProxyValue proxyValue = itemPtr->GetProxyValue();
      QString        partStr    = wizard::ComponentPropertyManager::Instance()->FindPartIcon(proxyValue);
      if (!partStr.isEmpty())
      {
         return wizard::UiResources::GetInstance().GetImageFromTheme(partStr);
      }
   }
      return QVariant();
   case Qt::ToolTipRole:
      return QVariant();
   case Qt::BackgroundColorRole:
   {
      Item* platItemPtr = itemPtr;
      while (platItemPtr && platItemPtr->GetNodeType() != Item::NodeType::cPLATFORM)
      {
         platItemPtr = dynamic_cast<Item*>(platItemPtr->GetAbstractItemParent());
      }
      if (platItemPtr)
      {
         WsfPProxyValue sideVal = platItemPtr->GetProxyValue()["side"];
         if (sideVal.IsValid())
         {
            std::string sideStr  = WsfPProxyBasicValue(sideVal).ToString();
            QString     sideQStr = QString(sideStr.c_str());

            UtColor color(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(sideStr));

            return QColor(color[0] * 255, color[1] * 255, color[2] * 255, 85);
         }
      }
   }
      return QVariant();
   default:
      return QVariant();
   }
}

void PlatformBrowser::Model::AddPlatform(const WsfPM_Platform& aPlatform)
{
   std::string name    = aPlatform.GetName();
   Item*       nodePtr = mRootItem->FindChild(name);
   if (!nodePtr)
   {
      nodePtr        = new Item(Item::NodeType::cPLATFORM, aPlatform);
      nodePtr->mName = name;
      mRootItem->AddChild(nodePtr);
   }
}

void PlatformBrowser::Model::RemovePlatform(const std::string& aName)
{
   mRootItem->removeChild(aName);
}

void PlatformBrowser::Model::ProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (!aProxyPtr)
   {
      // UpdateProxy(WsfPProxyValue());
      mRootItem->RemoveAllAbstractItems(true);
      mRootItem->clearAll();
      mChangedPlatforms.clear();
      mPlatformListWatch.Clear();
      mProxyRoot = WsfPM_Root();
   }
   else
   {
      mProxyRoot = aProxyPtr;
      if (mPlatformListWatch.Empty())
      {
         mPlatformListWatch += wizard::WatchProxy(WsfPM_Root(aProxyPtr).platforms().GetPath())
                                  .ChildUpdate.Connect(&Model::PlatformListChange, this);
         mPlatformListWatch +=
            wizard::WatchProxy(WsfPM_Root(aProxyPtr).platforms().GetPath()).AnyUpdate.Connect(&Model::AnyPlatformChange, this);

         WsfPM_Root        root(aProxyPtr);
         WsfPM_PlatformMap platforms = root.platforms();
         if (platforms.GetObjectMap())
         {
            for (auto i = platforms.begin(); i != platforms.end(); ++i)
            {
               AddPlatform(platforms + i->first);
            }
         }
      }
   }
}

void PlatformBrowser::Model::AnyPlatformChange(const wizard::ProxyChange& aChange)
{
   if (aChange.path().size() > 1)
   {
      const std::string& platformName = aChange.path()[1].GetMapKey();
      mChangedPlatforms.insert(platformName);
   }
}

void PlatformBrowser::Model::PlatformListChange(const wizard::ProxyChange& aChange)
{
   if (aChange.path().size() == 2)
   {
      switch (aChange.reason())
      {
      case wizard::ProxyChange::cADDED:
         AddPlatform(aChange.changed());
         break;
      case wizard::ProxyChange::cREMOVED:
         RemovePlatform(aChange.changed().GetName());
         break;
      case wizard::ProxyChange::cRENAMED:
         RenamePlatform(aChange.changed(), aChange.oldName());
         break;
      case wizard::ProxyChange::cUPDATED:
         // not needed?
         // renamePlatform(aChange.changed(), aChange.oldName());
         break;
      default:
         break;
      }
   }
}

void PlatformBrowser::Model::RenamePlatform(const WsfPProxyNode& aPlatform, const std::string& aOldName)
{
   RemovePlatform(aOldName);
   AddPlatform(aPlatform);
}

void PlatformBrowser::Model::UpdateTimeout()
{
   if (mChangedPlatforms.empty())
      return;
   std::set<std::string> updatedPlatforms;
   std::swap(updatedPlatforms, mChangedPlatforms);
   for (const auto& updatedPlatform : updatedPlatforms)
   {
      WsfPM_Platform platform = mProxyRoot.platforms().FindT(updatedPlatform);
      if (platform)
      {
         Item* platformItem = mRootItem->FindChild(updatedPlatform);
         if (platformItem)
         {
            platformItem->SetProxyValue(platform);
         }
      }
      else
      {
         RemovePlatform(updatedPlatform);
      }
   }
}
