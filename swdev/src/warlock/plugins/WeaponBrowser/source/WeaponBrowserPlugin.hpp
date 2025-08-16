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

#ifndef WkPluginAdvancedWeaponBrowser_HPP
#define WkPluginAdvancedWeaponBrowser_HPP

#include "WeaponBrowserDataContainer.hpp"
#include "WeaponBrowserDockWidget.hpp"
#include "WeaponBrowserSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfUnitTypes.hpp"

namespace WkWeaponBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<QTreeWidgetItem*> GetPlatformData(const std::string& aPlatformName) override;

private:
   struct WeaponItem
   {
      wkf::PlatformDataTreeItem*   mParentPtr;
      wkf::UnitlessTreeWidgetItem* mQuantityPtr;
      wkf::PlatformDataTreeItem*   mTypePtr;
   };
   std::map<std::string, WeaponItem> mWeaponToItemMap;

   void                    GuiUpdate() override;
   QList<QTreeWidgetItem*> mTopLevelWidgets;
   DataContainer           mWeaponData;
   QPointer<DockWidget>    mDockWidget;
};
} // namespace WkWeaponBrowser

#endif
