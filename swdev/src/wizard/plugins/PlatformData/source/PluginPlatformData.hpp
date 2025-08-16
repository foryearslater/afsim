// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLUGINPLATFORMDATA_HPP
#define PLUGINPLATFORMDATA_HPP

#include "Plugin.hpp"

namespace wkf
{
class ComboBoxTreeWidgetItem;
class PlatformDataTreeItem;
class PlatformGrouping;
class UnitTreeWidgetItem;
} // namespace wkf

namespace PlatformData
{
class Interface;

class Plugin : public wizard::Plugin
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;
   QList<QTreeWidgetItem*> GetPlatformData(const std::string& aPlatformName) override;
   void                    PlatformUpdated(const QString& aPlatformName) override;
   void                    UpdateGui();


private:
   Interface* mInterfacePtr;

   enum ePlatformData
   {
      eTYPE,
      eSIDE,
      eICON,
      eLATITUDE,
      eLONGITUDE,
      eALTITUDE_AGL,
      eALTITUDE_MSL,
      eYAW,
      ePITCH,
      eROLL
      // eSPEED,
   };
   void CreateItem(QTreeWidgetItem* parent, ePlatformData aType);

   std::map<ePlatformData, QString>                      mDataToStringMap;
   QList<QTreeWidgetItem*>                               mTopLevelWidgets;
   std::map<ePlatformData, wkf::PlatformDataTreeItem*>   mStringDataWidgets;
   std::map<ePlatformData, wkf::UnitTreeWidgetItem*>     mUnitDataWidgets;
   std::map<ePlatformData, wkf::ComboBoxTreeWidgetItem*> mComboBoxWidgets;
   wkf::PlatformDataTreeItem*                            mStateItemPtr;
   std::set<std::string>                                 mColors;
};
} // namespace PlatformData
#endif
