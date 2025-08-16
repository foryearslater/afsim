//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
#ifndef ORBITALDATAPLUGIN_HPP
#define ORBITALDATAPLUGIN_HPP

#include "OrbitalDataContainer.hpp"
#include "OrbitalDataSimInterface.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class UnitTreeWidgetItem;
class PlatformDataTreeItem;
} // namespace wkf

namespace WkOrbitalData
{

class Plugin : public warlock::PluginT<SimInterface>
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

   QList<QTreeWidgetItem*> GetPlatformData(const std::string& aPlatformName) override;

   std::unique_ptr<warlock::PlotUpdater> GetPlotUpdater(const QString& aPlatformName,
                                                        const QString& aY,
                                                        const unsigned aContext = 0) const override;

   std::unique_ptr<wkf::Updater> GetUpdater(const QString& aPlatformName,
                                            const QString& aDatum,
                                            const unsigned aContext = 0) const override;

private:
   void GuiUpdate() override;

   enum OrbitalDataType
   {
      cECCENTRICITY,
      cSEMI_MAJOR_AXIS,
      cINCLINATION,
      cRAAN,
      cARG_OF_PERIAPSIS,
      cTRUE_ANOMALY
   };
   void CreateItem(OrbitalDataType aType);

   std::map<OrbitalDataType, QString>                  mDataToStringMap;
   std::map<OrbitalDataType, wkf::UnitTreeWidgetItem*> mUnitDataWidgets;
   QList<QTreeWidgetItem*>                             mTopLevelWidgets;
   QTreeWidgetItem*                                    mOrbitalDataItemPtr;
   DataContainer                                       mDataContainer;
};

} // namespace WkOrbitalData
#endif
