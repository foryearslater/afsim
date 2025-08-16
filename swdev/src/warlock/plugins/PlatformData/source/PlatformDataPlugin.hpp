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

#include "PlatformDataSimInterface.hpp"
#include "WkPlotUpdater.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class PlatformDataTreeItem;
class UnitTreeWidgetItem;
} // namespace wkf

namespace WkPlatformData
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   QList<QTreeWidgetItem*> GetPlatformData(const std::string& aPlatformName) override;

   std::unique_ptr<warlock::PlotUpdater> GetPlotUpdater(const QString& aPlatformName,
                                                        const QString& aY,
                                                        const unsigned aContext = 0) const override;

   std::unique_ptr<wkf::Updater> GetUpdater(const QString&     aPlatformName,
                                            const QString&     aDatum,
                                            const unsigned int aContext = 0) const override;

   void           ResetOptionStates() override;
   void           SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatform) override;
   Qt::CheckState GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatform) const override;

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatform);

   void GuiUpdate() override;
   void CheckDataCompatability(PlatformData& aData);

   enum ePlatformData
   {
      eLATITUDE,
      eLONGITUDE,
      eALTITUDE,
      eSPEED,
      eMACH,
      eTYPE,
      eHEADING,
      ePITCH,
      eROLL,
      eSIDE,
      eINDEX,
      eSPATIALDOMAIN,
      eDAMAGEFACTOR,
      eFUELCAPACITY,
      eFUELCURRENT,
      eAUXDATA,
      eLOCALITY,
      eDIS_ID,
      eDIS_TYPE,
      eMARKING
   };
   void CreateItem(QTreeWidgetItem* parent,
                   ePlatformData    aType,
                   bool             aDefaultVisibility    = true,
                   bool             aAssociateWithUpdater = true);
   void SetItemVisibility(ePlatformData aType, bool aVisible);

   std::map<ePlatformData, QString> mDataToStringMap;

   QList<QTreeWidgetItem*>                             mTopLevelWidgets;
   std::map<std::string, wkf::PlatformDataTreeItem*>   mAuxDataWidgets;
   std::map<ePlatformData, wkf::PlatformDataTreeItem*> mStringDataWidgets;
   std::map<ePlatformData, wkf::UnitTreeWidgetItem*>   mUnitDataWidgets;

   int mDataRingOption;
};
} // namespace WkPlatformData
#endif
