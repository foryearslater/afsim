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

#include <QTimer>

#include "AbstractModel.hpp"
#include "ProxyChange.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPM_Root.hpp"

namespace PlatformBrowser
{

class Item;

//! A platform model.
class Model : public wizard::AbstractModel
{
   friend class Item;

public:
   Model();
   ~Model() override;
   QVariant      data(const QModelIndex& index, int role) const override;
   bool          setData(const QModelIndex& index, const QVariant& value, int role) override { return false; }
   Qt::ItemFlags flags(const QModelIndex& index) const override;
   QMimeData*    mimeData(const QModelIndexList& indexes) const override;

   Item* GetRootItem() { return mRootItem; }

   // void UpdateProxy(WsfPProxyNode aRoot);
   WsfPProxyNode& ProxyRoot() { return mProxyRoot; }

private:
   void UpdateTimeout();
   void PlatformListChange(const wizard::ProxyChange& aChange);
   void AnyPlatformChange(const wizard::ProxyChange& aChange);
   void ProxyAvailable(WsfPProxy* aProxyPtr);
   void AddPlatform(const WsfPM_Platform& aPlatform);
   void RemovePlatform(const std::string& aName);
   void RenamePlatform(const WsfPProxyNode& aPlatform, const std::string& aOldName);

   Item*      mRootItem;
   WsfPM_Root mProxyRoot;
   bool       mIsFrozen;

   UtCallbackHolder      mCallbacks;
   UtCallbackHolder      mPlatformListWatch;
   QTimer                mUpdateTimer;
   std::set<std::string> mChangedPlatforms;
};
} // namespace PlatformBrowser


#endif
