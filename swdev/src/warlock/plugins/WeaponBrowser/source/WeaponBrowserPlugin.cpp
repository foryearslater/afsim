// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WeaponBrowserPlugin.hpp"

#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkWeaponBrowser::Plugin,
                          "Weapon Browser",
                          "The Weapon Browser plugin provides a dialog for firing weapons against tracks.",
                          "warlock")

WkWeaponBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   mDockWidget = new DockWidget(GetInterface(), mWeaponData, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::LeftDockWidgetArea, mDockWidget);
   mDockWidget->hide();

   wkf::PlatformDataTreeItem* weaponParent = new wkf::PlatformDataTreeItem;
   weaponParent->setText(0, "Weapons");
   mTopLevelWidgets.push_back(weaponParent);
}

void WkWeaponBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mWeaponData);

   // Update Platform Data display
   auto weaponInfo = mWeaponData.GetWidgetData();
   for (const auto& infoIt : weaponInfo)
   {
      if (mWeaponToItemMap.count(infoIt.mName) == 0)
      {
         WeaponItem newItem;
         newItem.mParentPtr =
            new wkf::PlatformDataTreeItem(mTopLevelWidgets.at(0), {QString::fromStdString(infoIt.mName)});
         newItem.mParentPtr->SetUnhideable();
         newItem.mQuantityPtr = new wkf::UnitlessTreeWidgetItem(newItem.mParentPtr, "Quantity");
         newItem.mQuantityPtr->SetUnhideable();
         newItem.mTypePtr =
            new wkf::PlatformDataTreeItem(newItem.mParentPtr, {"Type", QString::fromStdString(infoIt.mType)});
         newItem.mTypePtr->SetUnhideable();
         mWeaponToItemMap.emplace(infoIt.mName, newItem);
      }
      mWeaponToItemMap[infoIt.mName].mQuantityPtr->SetValue(infoIt.mQuantityRemaining);
   }
}

QList<QTreeWidgetItem*> WkWeaponBrowser::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   mWeaponData.SetPlatformOfInterest(aPlatformName);
   mInterfacePtr->SetPlatformOfInterest(aPlatformName);
   auto items = mTopLevelWidgets.at(0)->takeChildren();
   for (auto& it : items)
   {
      delete it;
   }
   mWeaponToItemMap.clear();
   return mTopLevelWidgets;
}
