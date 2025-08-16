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
#include "PluginPlatformData.hpp"

#include <memory>

#include <QMenu>
#include <QStandardItemModel>

#include "PlatformDataInterface.hpp"
#include "PlatformDataTreeWidgetItem.hpp"
#include "PlatformDataUpdater.hpp"
#include "UtMemory.hpp"
#include "VaModelDatabase.hpp"
#include "WkfAction.hpp"
#include "WkfComboBoxTreeWidgetItem.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   PlatformData::Plugin,
   "Platform Data",
   "Gets platform data from the simulation and prepared it for display within the platform data display",
   "wizard")

PlatformData::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mInterfacePtr(new PlatformData::Interface(this))
{
   mDataToStringMap[eSIDE]         = "Side";
   mDataToStringMap[eTYPE]         = "Type";
   mDataToStringMap[eICON]         = "Icon";
   mDataToStringMap[eLATITUDE]     = "Latitude";
   mDataToStringMap[eLONGITUDE]    = "Longitude";
   mDataToStringMap[eALTITUDE_AGL] = "Altitude AGL";
   mDataToStringMap[eALTITUDE_MSL] = "Altitude MSL";
   mDataToStringMap[eYAW]          = "Heading";
   mDataToStringMap[ePITCH]        = "Pitch";
   mDataToStringMap[eROLL]         = "Roll";
   // mDataToStringMap[eSPEED]          = "Speed";


   wkf::PlatformDataTreeItem* infoparent = new wkf::PlatformDataTreeItem();
   infoparent->setText(0, "Info");
   mTopLevelWidgets.push_back(infoparent);
   mStateItemPtr = new wkf::PlatformDataTreeItem();
   mStateItemPtr->setText(0, "Initial State");
   mTopLevelWidgets.push_back(mStateItemPtr);

   for (auto color : UtColor::GetColorMap())
   {
      mColors.insert(color.first);
   }

   CreateItem(infoparent, eSIDE);
   CreateItem(infoparent, eTYPE);
   CreateItem(infoparent, eICON);


   CreateItem(mStateItemPtr, eLATITUDE);
   CreateItem(mStateItemPtr, eLONGITUDE);
   CreateItem(mStateItemPtr, eALTITUDE_AGL);
   CreateItem(mStateItemPtr, eALTITUDE_MSL);
   CreateItem(mStateItemPtr, eYAW);
   CreateItem(mStateItemPtr, ePITCH);
   CreateItem(mStateItemPtr, eROLL);
   // CreateItem(mStateItemPtr, eSPEED);

   // Register platform updaters with Environment
   wkfEnv.RegisterUpdater<AltitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<HeadingUpdater>(*this);
   wkfEnv.RegisterUpdater<LatitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<LongitudeUpdater>(*this);
   wkfEnv.RegisterUpdater<PitchUpdater>(*this);
   wkfEnv.RegisterUpdater<RollUpdater>(*this);
   // wkfEnv.RegisterUpdater<SpeedUpdater>(*this);
   wkfEnv.RegisterUpdater<PlatformTypeUpdater>(*this);
   wkfEnv.RegisterUpdater<PositionUpdater>(*this);
}


QList<wkf::PrefWidget*> PlatformData::Plugin::GetPreferencesWidgets() const
{
   return QList<wkf::PrefWidget*>();
}

QList<QTreeWidgetItem*> PlatformData::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   mInterfacePtr->SetPlatformOfInterest(aPlatformName);
   UpdateGui();
   return mTopLevelWidgets;
}


void PlatformData::Plugin::PlatformUpdated(const QString& aPlatformName)
{
   if (mInterfacePtr->GetPlatformData().name != aPlatformName.toStdString())
   {
      return;
   }

   mInterfacePtr->SetPlatformOfInterest(aPlatformName.toStdString());
   UpdateGui();
}

void PlatformData::Plugin::UpdateGui()
{
   PlatformData::Interface::PlatformData data = mInterfacePtr->GetPlatformData();

   mComboBoxWidgets[eSIDE]->setText(1, QString::fromStdString(data.side));
   mStringDataWidgets[eTYPE]->setText(1, QString::fromStdString(data.type));
   mComboBoxWidgets[eICON]->setText(1, QString::fromStdString(data.icon));

   if (data.state_valid)
   {
      mStateItemPtr->setHidden(false);
      mUnitDataWidgets[eLATITUDE]->SetValue(data.latitude);
      mUnitDataWidgets[eLONGITUDE]->SetValue(data.longitude);
      mUnitDataWidgets[eYAW]->SetValue(data.yaw);
      mUnitDataWidgets[ePITCH]->SetValue(data.pitch);
      mUnitDataWidgets[eROLL]->SetValue(data.roll);

      // 1 = agl, 0 = msl
      if (data.altitude_reference == 1)
      {
         mUnitDataWidgets[eALTITUDE_MSL]->SetHidden(true);
         mUnitDataWidgets[eALTITUDE_AGL]->SetHidden(false);
         mUnitDataWidgets[eALTITUDE_AGL]->SetValue(data.altitude_m);
      }
      else
      {
         mUnitDataWidgets[eALTITUDE_MSL]->SetHidden(false);
         mUnitDataWidgets[eALTITUDE_AGL]->SetHidden(true);
         mUnitDataWidgets[eALTITUDE_MSL]->SetValue(data.altitude_m);
      }
   }
   else
   {
      mStateItemPtr->setHidden(true);
   }
}

void PlatformData::Plugin::CreateItem(QTreeWidgetItem* parent, ePlatformData aType)
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
   case eALTITUDE_AGL:
   case eALTITUDE_MSL:
      mUnitDataWidgets[aType] = new wizard::ValidatedAltitudeTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   case eYAW:
   case ePITCH:
   case eROLL:
      mUnitDataWidgets[aType] = new wizard::ValidatedAngleTreeWidgetItem(item, mDataToStringMap[aType]);
      item                    = mUnitDataWidgets[aType];
      break;
   case eTYPE:
      mStringDataWidgets[aType] = new wkf::PlatformDataTreeItem(parent);
      mStringDataWidgets[aType]->setText(0, mDataToStringMap[aType]);
      item = mStringDataWidgets[aType];
      break;
   case eSIDE:
   {
      auto model = ut::make_unique<QStandardItemModel>(0, 1);
      for (auto color : UtColor::GetColorMap())
      {
         model->appendRow(new QStandardItem(QString::fromStdString(color.first)));
      }
      mComboBoxWidgets[aType] = new wkf::ComboBoxTreeWidgetItem(parent, 1, model.release(), false);
      mComboBoxWidgets[aType]->setText(0, mDataToStringMap[aType]);
      item = mComboBoxWidgets[aType];
      break;
   }
   case eICON:
   {
      auto                  model    = ut::make_unique<QStandardItemModel>(0, 1);
      std::set<std::string> namelist = vaEnv.GetModelDatabase()->GetNameList();
      for (auto&& name : namelist)
      {
         const vespa::VaModelDefinition* def = vaEnv.GetModelDatabase()->GetModelDefinition(name);
         if (def)
         {
            QString cat = QString::fromStdString(def->GetCategories());
            if (!cat.isEmpty())
            {
               int catInd = -1;
               for (int i = 0; i < model->rowCount(); i++)
               {
                  if (model->item(i)->text() == cat)
                  {
                     catInd = i;
                     break;
                  }
               }
               if (catInd == -1)
               {
                  model->insertRow(0, new QStandardItem(cat));
                  model->item(0)->setSelectable(false);
                  catInd = 0;
               }
               model->item(catInd)->appendRow(new QStandardItem(QString::fromStdString(name)));
            }
         }
      }
      model->sort(0);
      mComboBoxWidgets[aType] = new wkf::ComboBoxTreeWidgetItem(parent, 1, model.release(), true);
      mComboBoxWidgets[aType]->setText(0, mDataToStringMap[aType]);
      item = mComboBoxWidgets[aType];
      break;
   }
   default:
      // error
      break;
   }

   if (item)
   {
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
      parent->addChild(item);
   }
}
