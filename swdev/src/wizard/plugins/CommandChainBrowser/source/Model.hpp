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

#ifndef MODEL_HPP
#define MODEL_HPP

#include "AbstractModel.hpp"
#include "Item.hpp"
#include "ProxyChange.hpp"
#include "UtCallbackHolder.hpp"
#include "WsfPM_Root.hpp"

namespace CommandChain
{
class Item;

class Model : public wizard::AbstractModel
{
   friend class Item;

public:
   enum UserRoles
   {
      cDEFAULT_EXPANDED = Qt::UserRole,
      cSIDE_NAME
   };

   typedef std::pair<std::string, std::string> CommandChainPlatformPair; // pair<commandChainName, platformName>
   typedef std::map<std::string, Item*>        CommandChainMap;          // map<commandChainName, commandChainModelItem>
   typedef std::map<std::string, Item*>        NameToPlatform;
   typedef std::map<std::string, NameToPlatform> PlatformMap; // map< pair<commandChainName, platformName>, platformModelItem >

   Model();
   ~Model() override;
   QVariant data(const QModelIndex& index, int role) const override;
   bool     setData(const QModelIndex& index, const QVariant& value, int role) override { return false; }

   Item*           GetRootItem() { return mRootItem.get(); }
   CommandChainMap GetCommandChainMap() const { return mCommandChainMap; }

   // void UpdateProxy(WsfPM_Root aRoot);
   WsfPM_Root& ProxyRoot() { return mProxyRoot; }

   void ClearScenario();

   Qt::ItemFlags flags(const QModelIndex& index) const override;
   QMimeData*    mimeData(const QModelIndexList& indexes) const override;

   Item* FindPlatformModelItem(const std::string& aCommandChainName, const std::string& aPlatformName);

   Item* FindCommandChainRoot(const std::string& aCommandChainName);

   UtCallbackListN<void()> CommandChainListChange;

private:
   void DeleteItem(Item* aItemPtr);

   void BeforeRemove(Item* aItemPtr)
   {
      if (aItemPtr->mIsPlatform)
      {
         mPlatformMap[aItemPtr->mCommandChain].erase(aItemPtr->mName);
      }
   }
   void ProxyAvailable(WsfPProxy* aProxy);
   void PlatformListChange(const wizard::ProxyChange& aChange);
   void AnyPlatformChange(const wizard::ProxyChange& aChange);

   void TraverseMarkForParing();

   void  PlatformAdded(const WsfPM_Platform& aPlatform);
   void  PlatformRemoved(const std::string& aName);
   void  PlatformRenamed(const WsfPM_Platform& aPlatform, const std::string& aOldName);
   void  AddChain(const std::string& aChainName);
   void  TryPrune(Item* aItemPtr);
   void  UpdateMembership(const WsfPM_Platform& aPlatform);
   void  UpdateMembership(const WsfPM_Platform& aPlatform, const std::string& aChainName, Item* aChainRoot);
   Item* UsePlatformItem(const std::string& aName, const std::string& aChainName, Item* aChainRoot);

   Item* RealizeCommandChainRoot(const std::string& aCommandChainName);

   bool MoveNode(Item* aChild, Item* aParent);


   std::unique_ptr<Item> mRootItem;
   WsfPM_Root            mProxyRoot;

   CommandChainMap  mCommandChainMap; // holds all command chains
   PlatformMap      mPlatformMap;     // holds all platforms and their respective command chain
   UtCallbackHolder mCallbacks;
   UtCallbackHolder mProxyCallbacks;
   // The attribute index for the platform's command chain list is looked up one time and stored here
   size_t mCommandChainPropertyIndex;
};
} // namespace CommandChain

#endif
