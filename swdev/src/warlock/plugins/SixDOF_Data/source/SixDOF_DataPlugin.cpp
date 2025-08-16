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

#include "SixDOF_DataPlugin.hpp"

#include <sstream>

#include <QAction>
#include <QMenu>

#include "DisIO.hpp"
#include "SixDOF_DataPlotUpdaters.hpp"
#include "SixDOF_DataUpdaters.hpp"
#include "WkfDefines.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitTypes.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkSixDOF_Data::Plugin,
                          "SixDOF Data",
                          "The SixDOF Data plugin is responsible for getting data from a SixDOF Mover and displaying "
                          "it within the Platform Details dialog.",
                          "warlock")

WkSixDOF_Data::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   mDataToStringMap[eALTITUDE]             = "Altitude";
   mDataToStringMap[eVERT_SPEED_FPM]       = "Vert Spd (FPM)";
   mDataToStringMap[eCAS]                  = "KCAS";
   mDataToStringMap[eMACH]                 = "Mach";
   mDataToStringMap[eDYNAMIC_PRESSURE_PSF] = "Q (lb/sqft)";
   mDataToStringMap[eHEADING]              = "Heading";
   mDataToStringMap[ePITCH]                = "Pitch";
   mDataToStringMap[eROLL]                 = "Roll";
   mDataToStringMap[eNX]                   = "Nx";
   mDataToStringMap[eNY]                   = "Ny";
   mDataToStringMap[eGLOAD]                = "G-Load";
   mDataToStringMap[eG_AVAIL]              = "Avail Gs";
   mDataToStringMap[eALPHA]                = "Alpha";
   mDataToStringMap[eBETA]                 = "Beta";
   mDataToStringMap[eALPHA_DOT]            = "Alpha-Dot";
   mDataToStringMap[eBETA_DOT]             = "Beta-Dot";
   mDataToStringMap[eYAW_RATE]             = "Yaw Rate";
   mDataToStringMap[ePITCH_RATE]           = "Pitch Rate";
   mDataToStringMap[eROLL_RATE]            = "Roll Rate";
   mDataToStringMap[eACTIVE_PILOT]         = "Active Pilot";
   mDataToStringMap[eLIFT]                 = "Lift";
   mDataToStringMap[eDRAG]                 = "Drag";
   mDataToStringMap[eTHRUST]               = "Thrust";
   mDataToStringMap[eFUEL_FLOW]            = "Fuel Flow (PPH)";
   mDataToStringMap[eFUEL_CAPACITY]        = "Fuel Capacity (Tot)";
   mDataToStringMap[eFUEL_REMAINING]       = "Fuel Remaining";
   mDataToStringMap[eFUEL_INTERNAL]        = "Fuel Internal";
   mDataToStringMap[eFUEL_EXTERNAL]        = "Fuel External";
   mDataToStringMap[eSTICKBACK]            = "Stick Back";
   mDataToStringMap[eSTICKRIGHT]           = "Stick Right";
   mDataToStringMap[eRUDDERIGHT]           = "Rudder Right";
   mDataToStringMap[eTHROTTLE]             = "Throttle Lever";
   mDataToStringMap[eAFTERBURNER]          = "Afterburner On";
   mDataToStringMap[eSPEEDBRAKE_CMD]       = "Speedbrake Lever";
   mDataToStringMap[eSPEEDBRAKE_ANGLE]     = "Speedbrake Angle";
   mDataToStringMap[eTOT_WEIGHT]           = "Total Weight";
   mDataToStringMap[eWOW]                  = "Weight On Wheels";

   wkf::PlatformDataTreeItem* itemParent = new wkf::PlatformDataTreeItem();
   itemParent->setText(0, "SixDOF Mover");
   mTopLevelWidgets.push_back(itemParent);

   CreateItem(itemParent, eALTITUDE);
   CreateItem(itemParent, eVERT_SPEED_FPM);
   CreateItem(itemParent, eCAS);
   CreateItem(itemParent, eMACH);
   CreateItem(itemParent, eDYNAMIC_PRESSURE_PSF);
   CreateItem(itemParent, eHEADING);
   CreateItem(itemParent, ePITCH);
   CreateItem(itemParent, eROLL);
   CreateItem(itemParent, eNX);
   CreateItem(itemParent, eNY);
   CreateItem(itemParent, eGLOAD);
   CreateItem(itemParent, eG_AVAIL);
   CreateItem(itemParent, eALPHA);
   CreateItem(itemParent, eBETA);
   CreateItem(itemParent, eALPHA_DOT);
   CreateItem(itemParent, eBETA_DOT);
   CreateItem(itemParent, eYAW_RATE);
   CreateItem(itemParent, ePITCH_RATE);
   CreateItem(itemParent, eROLL_RATE);
   CreateItem(itemParent, eACTIVE_PILOT);
   CreateItem(itemParent, eLIFT);
   CreateItem(itemParent, eDRAG);
   CreateItem(itemParent, eTHRUST);
   CreateItem(itemParent, eFUEL_FLOW);
   CreateItem(itemParent, eFUEL_CAPACITY);
   CreateItem(itemParent, eFUEL_REMAINING);
   CreateItem(itemParent, eFUEL_INTERNAL);
   CreateItem(itemParent, eFUEL_EXTERNAL);
   CreateItem(itemParent, eSTICKBACK);
   CreateItem(itemParent, eSTICKRIGHT);
   CreateItem(itemParent, eRUDDERIGHT);
   CreateItem(itemParent, eTHROTTLE);
   CreateItem(itemParent, eAFTERBURNER);
   CreateItem(itemParent, eSPEEDBRAKE_CMD);
   CreateItem(itemParent, eSPEEDBRAKE_ANGLE);
   CreateItem(itemParent, eTOT_WEIGHT);
   CreateItem(itemParent, eWOW);
}

WkSixDOF_Data::Plugin::~Plugin()
{
   for (auto& i : mTopLevelWidgets)
   {
      delete i;
   }
}

QList<QTreeWidgetItem*> WkSixDOF_Data::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   if (mDataContainer.IsValidPlatform(aPlatformName))
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

std::unique_ptr<warlock::PlotUpdater> WkSixDOF_Data::Plugin::GetPlotUpdater(const QString& aPlatformName,
                                                                            const QString& aY,
                                                                            const unsigned aContext) const
{
   // Assume the X value is simulation time
   if (aContext == 0)
   {
      if (aY == mDataToStringMap.at(eALTITUDE))
      {
         return ut::make_unique<AltitudePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eVERT_SPEED_FPM))
      {
         return ut::make_unique<VertSpeedPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eCAS))
      {
         return ut::make_unique<CasPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eMACH))
      {
         return ut::make_unique<MachPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eDYNAMIC_PRESSURE_PSF))
      {
         return ut::make_unique<DynamicPressurePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eHEADING))
      {
         return ut::make_unique<HeadingPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(ePITCH))
      {
         return ut::make_unique<PitchPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eROLL))
      {
         return ut::make_unique<RollPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eNX))
      {
         return ut::make_unique<NxPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eNY))
      {
         return ut::make_unique<NyPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eGLOAD))
      {
         return ut::make_unique<GLoadPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eG_AVAIL))
      {
         return ut::make_unique<GAvailPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eALPHA))
      {
         return ut::make_unique<AlphaPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eBETA))
      {
         return ut::make_unique<BetaPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eALPHA_DOT))
      {
         return ut::make_unique<AlphaDotPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eBETA_DOT))
      {
         return ut::make_unique<BetaDotPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eYAW_RATE))
      {
         return ut::make_unique<YawRatePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(ePITCH_RATE))
      {
         return ut::make_unique<PitchRatePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eROLL_RATE))
      {
         return ut::make_unique<RollRatePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eLIFT))
      {
         return ut::make_unique<LiftPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eDRAG))
      {
         return ut::make_unique<DragPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eTHRUST))
      {
         return ut::make_unique<ThrustPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUEL_FLOW))
      {
         return ut::make_unique<FuelFlowPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUEL_CAPACITY))
      {
         return ut::make_unique<FuelCapacityPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUEL_REMAINING))
      {
         return ut::make_unique<FuelRemainingPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUEL_INTERNAL))
      {
         return ut::make_unique<FuelInternalPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUEL_EXTERNAL))
      {
         return ut::make_unique<FuelExternalPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eSTICKBACK))
      {
         return ut::make_unique<StickBackPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eSTICKRIGHT))
      {
         return ut::make_unique<StickRightPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eRUDDERIGHT))
      {
         return ut::make_unique<RudderRightPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eTHROTTLE))
      {
         return ut::make_unique<ThrottlePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eSPEEDBRAKE_CMD))
      {
         return ut::make_unique<SpeedbrakeCommandPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eSPEEDBRAKE_ANGLE))
      {
         return ut::make_unique<SpeedbrakeAnglePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eTOT_WEIGHT))
      {
         return ut::make_unique<TotalWeightPlotUpdater>(aPlatformName.toStdString(), 1);
      }
   }
   return nullptr;
}

std::unique_ptr<wkf::Updater> WkSixDOF_Data::Plugin::GetUpdater(const QString& aPlatformName,
                                                                const QString& aDatum,
                                                                const unsigned aContext) const
{
   if (aDatum == mDataToStringMap.at(eALTITUDE))
   {
      return wkf::make_updater<AltitudeUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eVERT_SPEED_FPM))
   {
      return wkf::make_updater<VerticalSpeedUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eCAS))
   {
      return wkf::make_updater<KCAS_Updater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eMACH))
   {
      return wkf::make_updater<MachUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eDYNAMIC_PRESSURE_PSF))
   {
      return wkf::make_updater<Q_Updater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eHEADING))
   {
      return wkf::make_updater<HeadingUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(ePITCH))
   {
      return wkf::make_updater<PitchUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eROLL))
   {
      return wkf::make_updater<RollUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eNX))
   {
      return wkf::make_updater<NxUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eNY))
   {
      return wkf::make_updater<NyUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eGLOAD))
   {
      return wkf::make_updater<G_LoadUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eG_AVAIL))
   {
      return wkf::make_updater<AvailGsUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eALPHA))
   {
      return wkf::make_updater<AlphaUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eBETA))
   {
      return wkf::make_updater<BetaUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eALPHA_DOT))
   {
      return wkf::make_updater<AlphaDotUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eBETA_DOT))
   {
      return wkf::make_updater<BetaDotUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eYAW_RATE))
   {
      return wkf::make_updater<YawRateUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(ePITCH_RATE))
   {
      return wkf::make_updater<PitchRateUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eROLL_RATE))
   {
      return wkf::make_updater<RollRateUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eACTIVE_PILOT))
   {
      return wkf::make_updater<PilotModeUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eLIFT))
   {
      return wkf::make_updater<LiftUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eDRAG))
   {
      return wkf::make_updater<DragUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eTHRUST))
   {
      return wkf::make_updater<ThrustUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eFUEL_FLOW))
   {
      return wkf::make_updater<FuelFlowUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eFUEL_CAPACITY))
   {
      return wkf::make_updater<FuelCapacityUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eFUEL_REMAINING))
   {
      return wkf::make_updater<FuelRemainingUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eFUEL_INTERNAL))
   {
      return wkf::make_updater<FuelInternalUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eFUEL_EXTERNAL))
   {
      return wkf::make_updater<FuelExternalUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eSTICKBACK))
   {
      return wkf::make_updater<StickBackUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eSTICKRIGHT))
   {
      return wkf::make_updater<StickRightUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eRUDDERIGHT))
   {
      return wkf::make_updater<RudderUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eTHROTTLE))
   {
      return wkf::make_updater<ThrottleUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eAFTERBURNER))
   {
      return wkf::make_updater<AfterburnerUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eSPEEDBRAKE_CMD))
   {
      return wkf::make_updater<SpeedBrakeCommandUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eSPEEDBRAKE_ANGLE))
   {
      return wkf::make_updater<SpeedBrakeAngleUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eTOT_WEIGHT))
   {
      return wkf::make_updater<WeightUpdater>(aPlatformName.toStdString());
   }
   else if (aDatum == mDataToStringMap.at(eWOW))
   {
      return wkf::make_updater<WeightOnWheelsUpdater>(aPlatformName.toStdString());
   }

   return nullptr;
}

void WkSixDOF_Data::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDataContainer);

   auto& data = mDataContainer.GetPlatformData();
   if (data.Valid())
   {
      mUnitDataWidgets[eALTITUDE]->SetValue(data.altitude_m);
      mUnitDataWidgets[eVERT_SPEED_FPM]->SetValue(data.vertSpeed_fpm);
      mUnitDataWidgets[eCAS]->SetValue(data.KCAS);
      mUnitDataWidgets[eMACH]->SetValue(data.mach);
      mUnitDataWidgets[eDYNAMIC_PRESSURE_PSF]->SetValue(data.dynamicPressure_psf);
      mUnitDataWidgets[eHEADING]->SetValue(data.heading_rad);
      mUnitDataWidgets[ePITCH]->SetValue(data.pitch_rad);
      mUnitDataWidgets[eROLL]->SetValue(data.roll_rad);
      mUnitDataWidgets[eNX]->SetValue(data.nx);
      mUnitDataWidgets[eNY]->SetValue(data.ny);
      mUnitDataWidgets[eGLOAD]->SetValue(data.gLoad);
      mUnitDataWidgets[eG_AVAIL]->SetValue(data.gAvail);
      mUnitDataWidgets[eALPHA]->SetValue(data.alpha_rad);
      mUnitDataWidgets[eBETA]->SetValue(data.beta_rad);
      mUnitDataWidgets[eALPHA_DOT]->SetValue(data.alphaDot_rps);
      mUnitDataWidgets[eBETA_DOT]->SetValue(data.betaDot_rps);
      mUnitDataWidgets[eYAW_RATE]->SetValue(data.yawRate_rps);
      mUnitDataWidgets[ePITCH_RATE]->SetValue(data.pitchRate_rps);
      mUnitDataWidgets[eROLL_RATE]->SetValue(data.rollRate_rps);
      mStringDataWidgets[eACTIVE_PILOT]->setText(1, data.activePilot.c_str());
      mUnitDataWidgets[eLIFT]->SetValue(data.lift_nt);
      mUnitDataWidgets[eDRAG]->SetValue(data.drag_nt);
      mUnitDataWidgets[eTHRUST]->SetValue(data.thrust_nt);
      mUnitDataWidgets[eFUEL_FLOW]->SetValue(data.fuelFlow_pph);
      mUnitDataWidgets[eFUEL_CAPACITY]->SetValue(data.fuelTotalCapacity_kg);
      mUnitDataWidgets[eFUEL_REMAINING]->SetValue(data.fuelRemaining_kg);
      mUnitDataWidgets[eFUEL_INTERNAL]->SetValue(data.fuelInternal_kg);
      mUnitDataWidgets[eFUEL_EXTERNAL]->SetValue(data.fuelExternal_kg);
      mUnitDataWidgets[eSTICKBACK]->SetValue(data.stickBack);
      mUnitDataWidgets[eSTICKRIGHT]->SetValue(data.stickRight);
      mUnitDataWidgets[eRUDDERIGHT]->SetValue(data.rudderRight);
      mUnitDataWidgets[eTHROTTLE]->SetValue(data.throttle);
      mStringDataWidgets[eAFTERBURNER]->setText(1, (data.afterburner_on) ? ("on") : ("off"));
      mUnitDataWidgets[eSPEEDBRAKE_CMD]->SetValue(data.speedbrakeCommand);
      mUnitDataWidgets[eSPEEDBRAKE_ANGLE]->SetValue(data.speedbrakeAngle_rad);
      mUnitDataWidgets[eTOT_WEIGHT]->SetValue(data.totalWeight_kg);
      mStringDataWidgets[eWOW]->setText(1, (data.weightOnWheels) ? ("true") : ("false"));
   }
}

void WkSixDOF_Data::Plugin::CreateItem(QTreeWidgetItem* parent, eDataType aType, bool aDefaultVisibility)
{
   wkf::PlatformDataTreeItem* item = nullptr;
   switch (aType)
   {
   case eALTITUDE:
      mUnitDataWidgets[aType] = new wkf::AltitudeTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eHEADING:
   case ePITCH:
   case eROLL:
   case eALPHA:
   case eBETA:
   case eSPEEDBRAKE_ANGLE:
      mUnitDataWidgets[aType] = new wkf::AngleTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eALPHA_DOT:
   case eBETA_DOT:
   case eYAW_RATE:
   case ePITCH_RATE:
   case eROLL_RATE:
      mUnitDataWidgets[aType] = new wkf::AngularRateTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eVERT_SPEED_FPM:
   case eCAS:
   case eMACH:
   case eDYNAMIC_PRESSURE_PSF:
   case eNX:
   case eNY:
   case eGLOAD:
   case eG_AVAIL:
   case eFUEL_FLOW:
   case eSTICKBACK:
   case eSTICKRIGHT:
   case eRUDDERIGHT:
   case eTHROTTLE:
   case eSPEEDBRAKE_CMD:
      mUnitDataWidgets[aType] = new wkf::UnitlessTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eLIFT:
   case eDRAG:
   case eTHRUST:
      mUnitDataWidgets[aType] = new wkf::ForceTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eFUEL_CAPACITY:
   case eFUEL_REMAINING:
   case eFUEL_INTERNAL:
   case eFUEL_EXTERNAL:
   case eTOT_WEIGHT:
      mUnitDataWidgets[aType] = new wkf::MassTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;

   case eAFTERBURNER:
   case eWOW:
   case eACTIVE_PILOT:
      mStringDataWidgets[aType] = new wkf::PlatformDataTreeItem(parent);
      mStringDataWidgets[aType]->setText(0, mDataToStringMap[aType]);
      item = mStringDataWidgets[aType];
   default:
      // error
      break;
   }

   if (item != nullptr)
   {
      item->setHidden(!aDefaultVisibility);
      item->setData(0, wkf::UPDATER_ROLE, true);
      parent->addChild(item);
   }
}

void WkSixDOF_Data::Plugin::SetItemVisibility(eDataType aType, bool aVisible)
{
   if (mUnitDataWidgets.count(aType) > 0)
   {
      mUnitDataWidgets[aType]->setHidden(!aVisible);
   }
   if (mStringDataWidgets.count(aType) > 0)
   {
      mStringDataWidgets[aType]->setHidden(!aVisible);
   }
}
