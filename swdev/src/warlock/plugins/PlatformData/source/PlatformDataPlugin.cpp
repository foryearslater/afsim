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
#include "PlatformDataPlugin.hpp"

#include <sstream>

#include <QAction>
#include <QMenu>

#include "DisIO.hpp"
#include "PlatformDataPlotUpdaters.hpp"
#include "PlatformDataUpdaters.hpp"
#include "WkfDefines.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlatformDataTreeItem.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitTypes.hpp"
#include "WkfVtkEnvironment.hpp"
#include "data_ring/WkfAttachmentDataRing.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkPlatformData::Plugin,
                          "Platform Data",
                          "The Platform Data plugin is responsible for reading data from a platform and displaying it "
                          "within the Platform Details dialog.",
                          "warlock")

WkPlatformData::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   mDataToStringMap[eLATITUDE]      = "Latitude";
   mDataToStringMap[eLONGITUDE]     = "Longitude";
   mDataToStringMap[eALTITUDE]      = "Altitude";
   mDataToStringMap[eSPEED]         = "Speed";
   mDataToStringMap[eMACH]          = "Mach";
   mDataToStringMap[eTYPE]          = "Type";
   mDataToStringMap[eHEADING]       = "Heading";
   mDataToStringMap[ePITCH]         = "Pitch";
   mDataToStringMap[eROLL]          = "Roll";
   mDataToStringMap[eSIDE]          = "Side";
   mDataToStringMap[eINDEX]         = "Index";
   mDataToStringMap[eSPATIALDOMAIN] = "Spatial Domain";
   mDataToStringMap[eDAMAGEFACTOR]  = "Damage Factor";
   mDataToStringMap[eFUELCAPACITY]  = "Fuel Capacity";
   mDataToStringMap[eFUELCURRENT]   = "Fuel";
   mDataToStringMap[eAUXDATA]       = "Aux Data";
   mDataToStringMap[eLOCALITY]      = "Locality";
   mDataToStringMap[eDIS_ID]        = "DIS Id";
   mDataToStringMap[eDIS_TYPE]      = "DIS Type";
   mDataToStringMap[eMARKING]       = "Marking";

   auto* platformStateParent = new wkf::PlatformDataTreeItem();
   platformStateParent->setText(0, "Platform State");
   platformStateParent->setData(0, wkf::DISPLAY_PRECEDENCE, 0);
   mTopLevelWidgets.push_back(platformStateParent);

   CreateItem(platformStateParent, eLATITUDE);
   CreateItem(platformStateParent, eLONGITUDE);
   CreateItem(platformStateParent, eALTITUDE);
   CreateItem(platformStateParent, eHEADING);
   CreateItem(platformStateParent, ePITCH);
   CreateItem(platformStateParent, eROLL);
   CreateItem(platformStateParent, eSPEED);
   CreateItem(platformStateParent, eMACH);
   CreateItem(platformStateParent, eTYPE);
   CreateItem(platformStateParent, eSIDE);
   CreateItem(platformStateParent, eSPATIALDOMAIN);
   CreateItem(platformStateParent, eDAMAGEFACTOR);
   CreateItem(platformStateParent, eFUELCAPACITY);
   CreateItem(platformStateParent, eFUELCURRENT);
   CreateItem(platformStateParent, eINDEX);
   CreateItem(platformStateParent, eAUXDATA, true, false);

   auto* networkParent = new wkf::PlatformDataTreeItem();
   networkParent->setText(0, "Network");
   mTopLevelWidgets.push_back(networkParent);

   CreateItem(networkParent, eLOCALITY);
   CreateItem(networkParent, eDIS_ID);
   CreateItem(networkParent, eDIS_TYPE);
   CreateItem(networkParent, eMARKING);

   // Register platform updaters with Environment
   wkfEnv.RegisterUpdater<AltitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<HeadingUpdater>(*this);
   wkfEnv.RegisterUpdater<LatitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<LongitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<PitchUpdater>(*this);
   wkfEnv.RegisterUpdater<RollUpdater>(*this);
   wkfEnv.RegisterUpdater<SpeedUpdater>(*this);
   wkfEnv.RegisterUpdater<MachUpdater>(*this);
   wkfEnv.RegisterUpdater<DamageFactorUpdater>(*this);
   wkfEnv.RegisterUpdater<FuelUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformTypeUpdater>(*this);
   wkfEnv.RegisterUpdater<PositionUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformSideUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformDomainUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformIndexUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformLocalityUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformDIS_IdUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformDIS_TypeUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformDIS_MarkingUpdater>(*this);

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &Plugin::PlatformOfInterestChanged);

   mDataRingOption = RegisterOption(nullptr, "Data Rings")->type();
}

WkPlatformData::Plugin::~Plugin()
{
   for (auto& i : mTopLevelWidgets)
   {
      delete i;
   }
}

void WkPlatformData::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   std::vector<wkf::AttachmentDataRing*> rings = aEntityPtr->FindAttachmentsOfType<wkf::AttachmentDataRing>();
   if (rings.size() > 0)
   {
      QMenu* removeRingMenu = aMenu->addMenu("Remove Data Rings");
      for (const auto& ring : rings)
      {
         wkf::Updater* updater = ring->GetUpdater();
         if (updater != nullptr)
         {
            QAction* ringAction =
               new QAction(QIcon::fromTheme("data_ring"), "Remove " + updater->GetUpdaterName() + " Ring", removeRingMenu);

            unsigned int ringId    = ring->GetUniqueId();
            int          ringLevel = ring->GetLevel();
            connect(ringAction,
                    &QAction::triggered,
                    [aEntityPtr, ringId, ringLevel]()
                    {
                       aEntityPtr->RemoveAttachment(ringId);

                       for (const auto& ring : aEntityPtr->FindAttachmentsOfType<wkf::AttachmentDataRing>())
                       {
                          if (ring->GetLevel() > ringLevel)
                          {
                             ring->SetLevel(ring->GetLevel() - 1);
                          }
                       }
                    });
            removeRingMenu->addAction(ringAction);
         }
      }

      QAction* removeAllAction = new QAction(QIcon::fromTheme("data_ring"), "Remove all Rings", removeRingMenu);

      connect(removeAllAction,
              &QAction::triggered,
              [aEntityPtr, rings]()
              {
                 for (auto& ring : rings)
                 {
                    aEntityPtr->RemoveAttachment(ring->GetUniqueId());
                 }
              });
      removeRingMenu->addAction(removeAllAction);
   }
}

QList<QTreeWidgetItem*> WkPlatformData::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   mInterfacePtr->SetPlatformOfInterest(aPlatformName);

   while (mStringDataWidgets[eAUXDATA]->childCount() > 0)
   {
      mStringDataWidgets[eAUXDATA]->removeChild(mStringDataWidgets[eAUXDATA]->child(0));
   }
   mAuxDataWidgets.clear();

   return mTopLevelWidgets;
}

std::unique_ptr<warlock::PlotUpdater> WkPlatformData::Plugin::GetPlotUpdater(const QString& aPlatformName,
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
      if (aY == mDataToStringMap.at(eLATITUDE))
      {
         return ut::make_unique<LatitudePlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eLONGITUDE))
      {
         return ut::make_unique<LongitudePlotUpdater>(aPlatformName.toStdString(), 1);
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
      if (aY == mDataToStringMap.at(eSPEED))
      {
         return ut::make_unique<SpeedPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eMACH))
      {
         return ut::make_unique<MachPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eDAMAGEFACTOR))
      {
         return ut::make_unique<DamageFactorPlotUpdater>(aPlatformName.toStdString(), 1);
      }
      if (aY == mDataToStringMap.at(eFUELCURRENT))
      {
         return ut::make_unique<FuelPlotUpdater>(aPlatformName.toStdString(), 1);
      }
   }
   else if (aContext == 1)
   {
      return ut::make_unique<AuxDataPlotUpdater>(aPlatformName.toStdString(), aY.toStdString(), 1);
   }
   return nullptr;
}

std::unique_ptr<wkf::Updater> WkPlatformData::Plugin::GetUpdater(const QString&     aPlatformName,
                                                                 const QString&     aDatum,
                                                                 const unsigned int aContext) const
{
   if (aContext == 0)
   {
      if ((aDatum == "Latitude") || (aDatum == "Longitude"))
      {
         return wkfEnv.GetUpdater("Position", aPlatformName.toStdString());
      }
      return wkfEnv.GetUpdater(aDatum, aPlatformName.toStdString());
   }
   else if (aContext == eAUXDATA)
   {
      return wkf::make_updater<PlatformAuxDataUpdater>(aPlatformName.toStdString(), aDatum.toStdString());
   }

   return nullptr;
}

void WkPlatformData::Plugin::ResetOptionStates()
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario != nullptr)
   {
      auto platformMap = scenario->GetIndexPlatformMap();
      for (auto& platformPair : platformMap)
      {
         auto rings = platformPair.second->FindAttachmentsOfType<wkf::AttachmentDataRing>();
         for (auto& ring : rings)
         {
            ring->SetStateVisibility(true);
         }
      }
   }
}

void WkPlatformData::Plugin::SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatform)
{
   auto rings = aPlatform->FindAttachmentsOfType<wkf::AttachmentDataRing>();
   for (auto ring : rings)
   {
      ring->SetStateVisibility(aState);
   }
}

Qt::CheckState WkPlatformData::Plugin::GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatform) const
{
   Qt::CheckState state = Qt::Checked;
   auto           rings = aPlatform->FindAttachmentsOfType<wkf::AttachmentDataRing>();
   for (auto ring : rings)
   {
      if (ring->GetStateVisibility() == false)
      {
         state = Qt::Unchecked;
         break;
      }
   }
   return state;
}

void WkPlatformData::Plugin::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform == nullptr)
   {
      // Clear the platform of interest from the Sim Interface
      mInterfacePtr->SetPlatformOfInterest("");
   }
}

void WkPlatformData::Plugin::GuiUpdate()
{
   PlatformData data;
   mInterfacePtr->ProcessEvents(data);

   if (!data.name.empty())
   {
      mUnitDataWidgets[eLATITUDE]->SetValue(data.latitude);
      mUnitDataWidgets[eLONGITUDE]->SetValue(data.longitude);
      mUnitDataWidgets[eALTITUDE]->SetValue(data.altitude_m);
      mUnitDataWidgets[eSPEED]->SetValue(data.speed_m);
      mUnitDataWidgets[eMACH]->SetValue(data.mach);
      mStringDataWidgets[eTYPE]->setText(1, QString::fromStdString(data.platformType));
      mUnitDataWidgets[eHEADING]->SetValue(UtMath::NormalizeAngle0_TwoPi(data.heading_rad));
      mUnitDataWidgets[ePITCH]->SetValue(data.pitch_rad);
      mUnitDataWidgets[eROLL]->SetValue(data.roll_rad);
      mStringDataWidgets[eSIDE]->setText(1, QString::fromStdString(data.side));
      mUnitDataWidgets[eINDEX]->SetValue(data.index);
      mStringDataWidgets[eSPATIALDOMAIN]->setText(1, QString::fromStdString(data.spatialDomain));
      mUnitDataWidgets[eDAMAGEFACTOR]->SetValue(data.damageFactor);
      mUnitDataWidgets[eFUELCAPACITY]->SetValue(data.fuelCapacity);
      mUnitDataWidgets[eFUELCURRENT]->SetValue(data.fuelCurrent);

      // Configure each DataWidget to display only if its appropriate for the platform of interest
      CheckDataCompatability(data);


      for (auto& iter : data.auxData)
      {
         auto& item = mAuxDataWidgets[iter.first];
         if (item == nullptr)
         {
            item = new wkf::PlatformDataTreeItem(mStringDataWidgets[eAUXDATA]);
            item->SetUnhideable();
            item->setText(0, QString::fromStdString(iter.first));
            item->SetPlottable();
         }
         item->setText(1, QString::fromStdString(iter.second));
         item->setData(0, wkf::UPDATER_ROLE, true);
         item->setData(0, wkf::UPDATER_CONTEXT, eAUXDATA);
      }

      mStringDataWidgets[eLOCALITY]->setText(1, QString::fromStdString(data.locality));
      {
         std::ostringstream oss;
         oss << data.disId;
         mStringDataWidgets[eDIS_ID]->setText(1, oss.str().c_str());
      }
      {
         std::ostringstream oss;
         oss << data.disType;
         mStringDataWidgets[eDIS_TYPE]->setText(1, oss.str().c_str());
      }
      mStringDataWidgets[eMARKING]->setText(1, data.marking.c_str());
   }
}

void WkPlatformData::Plugin::CheckDataCompatability(PlatformData& aData)
{
   // Restrict mach to display on air and space domain platforms
   mUnitDataWidgets[eMACH]->SetHidden((aData.spatialDomain != "air" && aData.spatialDomain != "space"),
                                      wkf::PlatformDataTreeItem::PROGRAMMATIC);

   // Restrict fuel capacity to display only on platforms that have fuel capacity
   mUnitDataWidgets[eFUELCAPACITY]->SetHidden((!aData.fuelCapacityValid), wkf::PlatformDataTreeItem::PROGRAMMATIC);
}

void WkPlatformData::Plugin::CreateItem(QTreeWidgetItem* parent,
                                        ePlatformData    aType,
                                        bool             aDefaultVisibility,
                                        bool             aAssociateWithUpdater)
{
   wkf::PlatformDataTreeItem* item = nullptr;
   switch (aType)
   {
   case eLATITUDE:
      mUnitDataWidgets[aType] = new wkf::LatitudeTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   case eLONGITUDE:
      mUnitDataWidgets[aType] = new wkf::LongitudeTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   // WkAngleItems:
   case eHEADING:
   case ePITCH:
   case eROLL:
      mUnitDataWidgets[aType] = new wkf::AngleTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   // WkAltitudeItems:
   case eALTITUDE:
      mUnitDataWidgets[aType] = new wkf::AltitudeTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   // WkSpeedItems:
   case eSPEED:
      mUnitDataWidgets[aType] = new wkf::SpeedTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   // WkUnitlessItems:
   case eINDEX:
   case eDAMAGEFACTOR:
      mUnitDataWidgets[aType] = new wkf::UnitlessTreeWidgetItem(parent, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   case eMACH:
      mUnitDataWidgets[aType] = new wkf::UnitlessTreeWidgetItem(parent, mDataToStringMap[aType]);
      mUnitDataWidgets[aType]->SetPrecision(2, 'f');
      item = mUnitDataWidgets[aType];
      break;
   case eFUELCAPACITY:
   case eFUELCURRENT:
      mUnitDataWidgets[aType] = new wkf::MassTreeWidgetItem(parent, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   // QString Data (QTreeWidgetItems)
   case eAUXDATA:
   case eSIDE:
   case eSPATIALDOMAIN:
   case eTYPE:
   case eLOCALITY:
   case eDIS_ID:
   case eDIS_TYPE:
   case eMARKING:
      mStringDataWidgets[aType] = new wkf::PlatformDataTreeItem(parent);
      mStringDataWidgets[aType]->setText(0, mDataToStringMap[aType]);
      item = mStringDataWidgets[aType];
      break;
   default:
      // error
      break;
   }

   if (item != nullptr)
   {
      item->setData(0, wkf::UPDATER_ROLE, aAssociateWithUpdater);
      item->SetHidden(!aDefaultVisibility);
      parent->addChild(item);
   }
}

void WkPlatformData::Plugin::SetItemVisibility(ePlatformData aType, bool aVisible)
{
   if (mUnitDataWidgets.count(aType) > 0)
   {
      mUnitDataWidgets[aType]->SetHidden(!aVisible);
   }
   if (mStringDataWidgets.count(aType) > 0)
   {
      mStringDataWidgets[aType]->SetHidden(!aVisible);
   }
}
