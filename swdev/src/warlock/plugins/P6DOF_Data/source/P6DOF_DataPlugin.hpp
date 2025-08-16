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
#ifndef PLUGINP6DOF_DATA_HPP
#define PLUGINP6DOF_DATA_HPP

#include "P6DOF_DataSimInterface.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class UnitTreeWidgetItem;
class PlatformDataTreeItem;
} // namespace wkf

namespace WkP6DOF_Data
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

   enum eDataType
   {
      eALTITUDE,
      eVERT_SPEED_FPM,       // unitless FPM
      eCAS,                  // unitless KCAS
      eMACH,                 // unitless Mach
      eDYNAMIC_PRESSURE_PSF, // unitless lbs/sq-ft
      eHEADING,
      ePITCH,
      eROLL,
      eNX,      // unitless G
      eNY,      // unitless G
      eGLOAD,   // unitless G
      eG_AVAIL, // unitless G
      eALPHA,
      eBETA,
      eALPHA_DOT,
      eBETA_DOT,
      eYAW_RATE,
      ePITCH_RATE,
      eROLL_RATE,
      eACTIVE_PILOT,
      eLIFT,
      eDRAG,
      eTHRUST,
      eFUEL_FLOW, // unitless PPH
      eFUEL_CAPACITY,
      eFUEL_REMAINING,
      eFUEL_INTERNAL,
      eFUEL_EXTERNAL,
      eSTICKBACK,  // unitless normalized (+Back)
      eSTICKRIGHT, // unitless normalized (+Right)
      eRUDDERIGHT, // unitless normalized (+Right)
      eTHROTTLE,   // unitless normalized (+Forward)
      eAFTERBURNER,
      eSPEEDBRAKE_CMD, // unitless normalized (+Extended)
      eSPEEDBRAKE_ANGLE,
      eTOT_WEIGHT,
      eWOW
   };
   void CreateItem(QTreeWidgetItem* parent, eDataType aType, bool aDefaultVisibility = true);
   void SetItemVisibility(eDataType aType, bool aVisible);

   DataContainer mDataContainer;

   std::map<eDataType, QString> mDataToStringMap;

   QList<QTreeWidgetItem*>                         mTopLevelWidgets;
   std::map<eDataType, wkf::PlatformDataTreeItem*> mStringDataWidgets;
   std::map<eDataType, wkf::UnitTreeWidgetItem*>   mUnitDataWidgets;
};

} // namespace WkP6DOF_Data
#endif
