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

#include <array>

#include "ComponentPropertyManager.hpp"
#include "DragAndDrop.hpp"
#include "Item.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "UiResources.hpp"
#include "WsfPM_Root.hpp"

std::array<const char*, TypeBrowser::Model::cTYPE_KIND_COUNT> cTYPE_MAPS = {
   "platformType",     "processorType",  "weaponType",       "commType",          "sensorType",
   "moverType",        "fuelType",       "weaponEffectType", "routeNetwork",      "routes",
   "aeroType",         "zone",           "antennaPattern",   "acousticSignature", "infraredSignature",
   "opticalSignature", "radarSignature", "inherentContrast", "maskingPattern",    "reflectivitySignature",
   "EW_Effect",        "EA_Technique",   "EP_Technique",     "electronicAttack",  "electronicProtect"};

std::array<const char*, TypeBrowser::Model::cTYPE_KIND_COUNT> cTYPE_KIND_NAMES = {"Platform",
                                                                                  "Processor",
                                                                                  "Weapon",
                                                                                  "Comm",
                                                                                  "Sensor",
                                                                                  "Mover",
                                                                                  "Fuel",
                                                                                  "Weapon Effects",
                                                                                  "Route Network",
                                                                                  "Route",
                                                                                  "Aero",
                                                                                  "Zone",
                                                                                  "Antenna Pattern",
                                                                                  "Acoustic Signature",
                                                                                  "Infrared Signature",
                                                                                  "Optical Signature",
                                                                                  "Radar Signature",
                                                                                  "Inherent Contrast",
                                                                                  "Masking Pattern",
                                                                                  "Reflectivity",
                                                                                  "EW Effect",
                                                                                  "EA Technique",
                                                                                  "EP Technique",
                                                                                  "Electronic Attack",
                                                                                  "Electronic Protect"};

TypeBrowser::Model::Model()
   : mProxyPtr(nullptr)
{
   mHibernate = true;

   mCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Model::ProxyAvailable, this);
   mCallbacks += wizSignals->ParseUpdatedDeferred.Connect(&Model::ParseUpdateDeferred, this);

   mShowUnusedTypes = false;
   mRootItem        = new Item();
   mRootItem->mName = "";
   SetRoot(mRootItem);
   for (int i = 0; i < cTYPE_KIND_COUNT; ++i)
   {
      Item* topItem      = new Item;
      topItem->mName     = cTYPE_KIND_NAMES[i];
      topItem->mTypeKind = i;
      mRootItem->AddAbstractItem(topItem);
      mBaseTypeItems[i] = topItem;
   }

   mDeferredUpdateOnProxyChange.setInterval(1000);
   mDeferredUpdateOnProxyChange.setSingleShot(true);
   connect(&mDeferredUpdateOnProxyChange, &QTimer::timeout, this, &Model::DeferredUpdate);
}

TypeBrowser::Model::~Model()
{
   SetRoot(nullptr);
   delete mRootItem;
}

QVariant TypeBrowser::Model::data(const QModelIndex& index, int role) const
{
   Item* itemPtr = dynamic_cast<Item*>(FindItem(index));
   if (!itemPtr)
      return QVariant();

   switch (role)
   {
   case Qt::DisplayRole:
      return itemPtr->mName;
   case Qt::FontRole:
      return QVariant();
   case Qt::DecorationRole:
   {
      WsfPProxyValue val = itemPtr->GetValue();
      if (val)
      {
         QString partStr = wizard::ComponentPropertyManager::Instance()->FindPartIcon(val);
         if (!partStr.isEmpty())
         {
            return wizard::UiResources::GetInstance().GetImageFromTheme(partStr);
         }
      }
      return ChooseIcon(itemPtr->mTypeKind);
   }
   case Qt::ToolTipRole:
      return QVariant();
   default:
      return QVariant();
   }
}

QVariant TypeBrowser::Model::ChooseIcon(int aLookup) const
{
   switch (aLookup)
   {
   case cPLATFORM:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("f-18"));
   case cCOMM:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("chat"));
   case cMOVER:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("horse"));
   case cPROCESSOR:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("gears"));
   case cSENSOR:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("radar"));
   case cWEAPON:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("bomb"));
   case cFUEL:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("gas"));
   case cRADAR_SIGNATURES:
   case cACOUSTIC_SIGNATURES:
   case cINFRARED_SIGNATURES:
   case cOPTICAL_SIGNATURES:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("radar_signature"));
   case cROUTE:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("route"));
   case cAERO:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("wing"));
   case cROUTE_NETWORK:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("route_network"));
   case cZONE:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("zone"));
   case cWEAPON_EFFECT:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("explosion"));
   default:
      return wizard::UiResources::GetInstance().GetImageFromTheme(QString("box_house"));
   }
   return QVariant();
}

void TypeBrowser::Model::ProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (mHibernate)
      aProxyPtr = nullptr;

   if (mProxyPtr != aProxyPtr)
   {
      mProxyPtr = aProxyPtr;
      mProxyWatcherCallbacks.Clear();
      if (mProxyPtr)
      {
         WsfPM_Root root(mProxyPtr);
         for (int ti = 0; ti < cTYPE_KIND_COUNT; ++ti)
         {
            const char*   typeMapName = cTYPE_MAPS[ti];
            WsfPProxyNode typeMapNode = root + typeMapName;
            if (typeMapName)
            {
               mProxyWatcherCallbacks +=
                  wizard::WatchProxy(typeMapNode.GetPath()).ChildUpdate.Connect(&Model::OnProxyTypeChange, this);
            }
         }
      }
   }
   for (std::map<std::string, Item*>::iterator i = mTypes.begin(); i != mTypes.end(); ++i)
   {
      i->second->mToPare = true;
   }

   std::map<std::string, TypeSource> typeRoots;

   if (aProxyPtr)
   {
      WsfPProxyRegistry* proxReg = aProxyPtr->GetRegistry().get();
      WsfPProxyNode      root    = WsfPProxyNode::FromProxy(aProxyPtr);
      if (proxReg)
      {
         root.SwitchToBasicTypes();
         ScanForTypes(root, typeRoots, -1, true);
      }
      root.Invalidate();
      ScanForTypes(root, typeRoots, 0, false);
   }

   for (std::map<std::string, TypeSource>::iterator i = typeRoots.begin(); i != typeRoots.end(); ++i)
   {
      if (mShowUnusedTypes || !i->second.mIsCoreType)
      {
         RealizeType(i->first, typeRoots);
      }
   }

   for (std::map<std::string, Item*>::iterator i = mTypes.begin(); i != mTypes.end();)
   {
      Item* itemPtr = i->second;
      if ((itemPtr != nullptr) && (itemPtr->mToPare))
      {
         itemPtr->RemoveAllAbstractItems(false);
         if (itemPtr->GetAbstractItemParent())
         {
            itemPtr->GetAbstractItemParent()->RemoveAbstractItem(itemPtr);
         }
         mTypes.erase(i++);
         delete itemPtr;
      }
      else
      {
         ++i;
      }
   }
}

TypeBrowser::Item* TypeBrowser::Model::RealizeType(const std::string& aName, std::map<std::string, TypeSource>& aTypeSources)
{
   std::map<std::string, TypeSource>::iterator it = aTypeSources.find(aName);
   if (it != aTypeSources.end())
   {
      TypeSource& src     = it->second;
      Item*       itemPtr = GetOrMakeType(aName);
      // Only set item data once per update -- mToPare starts 'true' each update.
      if (itemPtr->mToPare)
      {
         itemPtr->mToPare = false;
         Item* basePtr    = nullptr;
         if (src.mBasePathPtr)
         {
            basePtr = RealizeType(KindNameFromPath(src.mNode.GetRoot(), *src.mBasePathPtr), aTypeSources);
         }
         if (!basePtr)
         {
            basePtr = mBaseTypeItems[src.mTypeKind];
         }
         itemPtr->mName = src.mNode.GetName().c_str(); //.mPath.Back().mMapKey.c_str();
         itemPtr->SetBase(basePtr);
         itemPtr->mTypeKind = src.mTypeKind;
         itemPtr->mTypeNode = src.mNode;
      }
      return itemPtr;
   }
   return nullptr;
}

TypeBrowser::Item* TypeBrowser::Model::GetOrMakeType(const std::string& aTypeKind)
{
   Item*& typePtr = mTypes[aTypeKind];
   if (!typePtr)
   {
      typePtr          = new Item;
      typePtr->mToPare = true;
   }
   return typePtr;
}

void TypeBrowser::Model::ScanForTypes(const WsfPProxyNode&               aRoot,
                                      std::map<std::string, TypeSource>& aTypeRoots,
                                      int                                aScenarioIndex,
                                      bool                               aScanBasicTypes)
{
   WsfPProxyValue rootVal = aRoot.GetValue();
   for (int ti = 0; ti < cTYPE_KIND_COUNT; ++ti)
   {
      const char* typeMapName = cTYPE_MAPS[ti];
      // Don't report basic types that don't exist, like WSF_ROUTE_NETWORK
      if (aScanBasicTypes && !TypeHasBuiltins(ti))
      {
         continue;
      }

      WsfPProxyObjectMap* mapPtr = rootVal[typeMapName].GetObjectMap();
      if (mapPtr != nullptr)
      {
         WsfPProxyNode mapNode(aRoot);
         if (aScanBasicTypes)
            mapNode.SwitchToBasicTypes();
         mapNode += typeMapName;
         auto& values = mapPtr->GetValues();
         for (auto i = values.begin(); i != values.end(); ++i)
         {
            WsfPProxyStructValue type = i->second;
            std::string          kindName(typeMapName);
            kindName += '.';
            kindName += i->first;
            TypeSource& src = aTypeRoots[kindName];
            if (src.mNode.GetPath().Empty())
            {
               src.mNode = mapNode + i->first;
               // src.mRoot = aRoot;
               src.mBasePathPtr = type.GetBase();
               src.mTypeKind    = ti;
               src.mIsCoreType  = aScanBasicTypes;
            }
         }
      }
   }
}

std::string TypeBrowser::Model::KindNameFromPath(const WsfPProxyStructValue& aRoot, const WsfPProxyPath& aPath)
{
   if (aPath.size() == 2 && aPath[0].IsIndex() && aPath[1].IsString())
   {
      return aRoot.GetMemberName(aPath[0].GetIndex()) + '.' + aPath[1].GetMapKey();
   }
   return std::string();
}

void TypeBrowser::Model::SetShowUnusedTypes(bool aVal)
{
   mShowUnusedTypes = aVal;
   if (wizard::ProxyWatcher::GetActiveProxy())
   {
      ProxyAvailable(wizard::ProxyWatcher::GetActiveProxy());
   }
}

void TypeBrowser::Model::Hibernate(bool aHibernate)
{
   if (mHibernate != aHibernate)
   {
      mHibernate = aHibernate;
      if (mHibernate)
      {
         ProxyAvailable(nullptr);
      }
      else
      {
         ProxyAvailable(wizard::ProxyWatcher::GetActiveProxy());
      }
   }
}

void TypeBrowser::Model::ParseUpdateDeferred(wizard::Project* aProject, bool aFull)
{
   ProxyAvailable(wizard::ProxyWatcher::GetActiveProxy());
}

QMimeData* TypeBrowser::Model::mimeData(const QModelIndexList& indexes) const
{
   wizard::DragAndDrop::PropertyList propList;
   for (int i = 0; i < indexes.size(); ++i)
   {
      Item* itemPtr = dynamic_cast<Item*>(FindItem(indexes[i]));
      if (itemPtr && itemPtr->GetProxyNode())
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

Qt::ItemFlags TypeBrowser::Model::flags(const QModelIndex& index) const
{
   Qt::ItemFlags defaultFlags = wizard::AbstractModel::flags(index);
   Item*         itemPtr      = dynamic_cast<Item*>(FindItem(index));
   if (itemPtr && itemPtr->GetProxyNode())
   {
      defaultFlags |= Qt::ItemIsDragEnabled;
   }
   return defaultFlags;
}

// Eventually executes after proxy changes are made which may affect this model
// to trigger a full update
void TypeBrowser::Model::DeferredUpdate()
{
   WsfPProxy* proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   if (proxyPtr)
   {
      ProxyAvailable(proxyPtr);
   }
}

// Called when any proxy value for an WSF type is modified (types only, no platforms).
// starts/restarts the timer for a deferred update
void TypeBrowser::Model::OnProxyTypeChange(const wizard::ProxyChange& aChange)
{
   mDeferredUpdateOnProxyChange.start();
}
