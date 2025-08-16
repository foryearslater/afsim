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

#include "OrbitalDataPlugin.hpp"

#include "OrbitalDataPlotUpdaters.hpp"
#include "OrbitalDataUpdaters.hpp"
#include "WkfUnitTypes.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkOrbitalData::Plugin,
                          "Orbital Element Data",
                          "Adds orbital elements for platforms with a space mover to the Platform Data widget.",
                          "warlock",
                          true)

WkOrbitalData::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   mDataToStringMap[cECCENTRICITY]     = "Eccentricity";
   mDataToStringMap[cSEMI_MAJOR_AXIS]  = "Semi-major Axis";
   mDataToStringMap[cINCLINATION]      = "Inclination";
   mDataToStringMap[cRAAN]             = "RAAN";
   mDataToStringMap[cARG_OF_PERIAPSIS] = "Argument of Periapsis";
   mDataToStringMap[cTRUE_ANOMALY]     = "True Anomaly";

   mOrbitalDataItemPtr = new wkf::PlatformDataTreeItem();
   mOrbitalDataItemPtr->setText(0, "Orbital Elements");
   mTopLevelWidgets.push_back(mOrbitalDataItemPtr);

   CreateItem(cECCENTRICITY);
   CreateItem(cSEMI_MAJOR_AXIS);
   CreateItem(cINCLINATION);
   CreateItem(cRAAN);
   CreateItem(cARG_OF_PERIAPSIS);
   CreateItem(cTRUE_ANOMALY);
}

WkOrbitalData::Plugin::~Plugin()
{
   for (auto& i : mTopLevelWidgets)
   {
      delete i;
   }
}

QList<QTreeWidgetItem*> WkOrbitalData::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   if (mDataContainer.IsOrbitalPlatform(aPlatformName))
   {
      mDataContainer.SetPlatformOfInterest(aPlatformName);
      mInterfacePtr->SetPlatformOfInterest(aPlatformName);
      return mTopLevelWidgets;
   }
   else
   {
      mDataContainer.SetPlatformOfInterest("");
      mInterfacePtr->SetPlatformOfInterest("");
      return QList<QTreeWidgetItem*>();
   }
}


std::unique_ptr<warlock::PlotUpdater> WkOrbitalData::Plugin::GetPlotUpdater(const QString& aPlatformName,
                                                                            const QString& aY,
                                                                            const unsigned aContext) const
{
   // Assume the X value is simulation time
   if (aContext == 0)
   {
      if (aY == mDataToStringMap.at(cECCENTRICITY))
      {
         return ut::make_unique<EccentricityPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(cSEMI_MAJOR_AXIS))
      {
         return ut::make_unique<SemiMajorAxisPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(cINCLINATION))
      {
         return ut::make_unique<InclinationPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(cRAAN))
      {
         return ut::make_unique<RAAN_PlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(cARG_OF_PERIAPSIS))
      {
         return ut::make_unique<ArgOfPeriapsisPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(cTRUE_ANOMALY))
      {
         return ut::make_unique<TrueAnomalyPlotUpdater>(aPlatformName.toStdString(), 1);
      }
   }
   return nullptr;
}

std::unique_ptr<wkf::Updater> WkOrbitalData::Plugin::GetUpdater(const QString& aPlatformName,
                                                                const QString& aDatum,
                                                                const unsigned aContext) const
{
   if (aDatum == mDataToStringMap.at(cECCENTRICITY))
   {
      return wkf::make_updater<EccentricityUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(cSEMI_MAJOR_AXIS))
   {
      return wkf::make_updater<SemiMajorAxisUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(cINCLINATION))
   {
      return wkf::make_updater<InclinationUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(cRAAN))
   {
      return wkf::make_updater<RAAN_Updater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(cARG_OF_PERIAPSIS))
   {
      return wkf::make_updater<ArgOfPeriapsisUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(cTRUE_ANOMALY))
   {
      return wkf::make_updater<TrueAnomalyUpdater>(aPlatformName.toStdString());
   }
   return nullptr;
}

void WkOrbitalData::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDataContainer);

   auto& data = mDataContainer.GetPlatformData();
   if (data.Valid())
   {
      mUnitDataWidgets[cECCENTRICITY]->SetValue(data.mEccentricity);
      mUnitDataWidgets[cSEMI_MAJOR_AXIS]->SetValue(data.mSemiMajorAxis);
      mUnitDataWidgets[cINCLINATION]->SetValue(data.mInclination);
      mUnitDataWidgets[cRAAN]->SetValue(data.mRAAN);
      mUnitDataWidgets[cARG_OF_PERIAPSIS]->SetValue(data.mArgOfPeriapsis);
      mUnitDataWidgets[cTRUE_ANOMALY]->SetValue(data.mTrueAnomaly);
   }
}

void WkOrbitalData::Plugin::CreateItem(OrbitalDataType aType)
{
   switch (aType)
   {
   case cECCENTRICITY:
      mUnitDataWidgets[aType] = new wkf::UnitlessTreeWidgetItem(nullptr, mDataToStringMap[aType]);
      break;
   case cSEMI_MAJOR_AXIS:
      mUnitDataWidgets[aType] = new wkf::LengthTreeWidgetItem(nullptr, mDataToStringMap[aType]);
      break;
   case cINCLINATION:
   case cRAAN:
   case cARG_OF_PERIAPSIS:
   case cTRUE_ANOMALY:
      mUnitDataWidgets[aType] = new wkf::AngleTreeWidgetItem(nullptr, mDataToStringMap[aType]);
      break;
   default:
      break;
   }

   mUnitDataWidgets[aType]->setData(0, wkf::UPDATER_ROLE, true);
   mOrbitalDataItemPtr->addChild(mUnitDataWidgets[aType]);
}
