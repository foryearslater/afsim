// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "HDD_Plugin.hpp"

#include <QMenu>

#include "UtMemory.hpp"
#include "UtQtMemory.hpp"
#include "WkRunManager.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPrefWidget.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "hdd/WkfHDD_PrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(HDD::Plugin,
                          "Head Down View",
                          "The Head Down View plugin provides a generic, interactive glass-cockpit head down view.",
                          "warlock",
                          false) // Plugin not loaded by default

HDD::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId, eSECONDARY_VIEWER)
   , mDataContainer()
   , mHdds()
   , mPrefWidgetPtr(new PrefWidget)
{
   mInterfacePtr->SetEnabled(false);
   mPrefObjectPtr = mPrefWidgetPtr.data()->GetPreferenceObject();
}

void HDD::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      wkf::Platform* platPtr = dynamic_cast<wkf::Platform*>(aEntityPtr);
      if ((mInterfacePtr->HasSituationAwarenessProcessor(platPtr->GetIndex())) &&
          mHdds.find(QString::fromStdString(platPtr->GetName())) == mHdds.end())
      {
         auto airCombatMenu = aMenu->findChild<QMenu*>("Air Combat");
         if (!airCombatMenu)
         {
            airCombatMenu = ut::qt::make_qt_ptr<QMenu>(QString("Air Combat"), aMenu);
            aMenu->addMenu(airCombatMenu);
            airCombatMenu->setObjectName("Air Combat");
         }

         wkf::Action* connectAction =
            new wkf::Action(QIcon::fromTheme("headsdown"), QString("Head Down View"), airCombatMenu);
         connectAction->setData(QString(aEntityPtr->GetName().c_str()));
         connect(connectAction, &QAction::triggered, this, &HDD::Plugin::ConnectToPlatformActionHandler);
      }
   }
}

void HDD::Plugin::ConnectToPlatform(const QString& aPlatformName, QMainWindow* aParent)
{
   // If another HDD window has been opened for the platform with aPlatformName, delete the old one
   if (mHdds.find(aPlatformName) != mHdds.end())
   {
      delete mHdds[aPlatformName];
      mHdds.erase(aPlatformName);
   }

   mInterfacePtr->SetEnabled(true);
   mInterfacePtr->AddConnectedPlatform(aPlatformName);
   if (mHdds.find(aPlatformName) == mHdds.end())
   {
      auto* scenarioPtr = vaEnv.GetStandardScenario();
      if (scenarioPtr)
      {
         vespa::VaEntity*    attached = nullptr;
         vespa::VaEntityList entitylist;
         scenarioPtr->GetEntityList(entitylist);
         vespa::VaEntityList::iterator it;
         for (it = entitylist.begin(); it != entitylist.end(); ++it)
         {
            if (QString::fromStdString((*it)->GetName()).compare(aPlatformName) == 0)
            {
               attached = (*it);
            }
         }
         wkf::VtkEnvironment::InitializeOSG_Path();
         QPointer<HDD::Dockable> hdd = new HDD::Dockable(attached,
                                                         mPrefObjectPtr->GetResolutionX(),
                                                         mPrefObjectPtr->GetResolutionY(),
                                                         mPrefObjectPtr->GetPageList(),
                                                         aParent);
         connect(hdd.data(), &HDD::Dockable::HDD_Closed, [this, hdd]() { ReleasePlatform(hdd->GetName()); });
         QString title = "HeadDownView: ";
         title += aPlatformName;
         hdd->setWindowTitle(title);

         if (hdd->isHidden())
         {
            hdd->setHidden(false);
         }
         hdd->show();
         hdd->raise();

         mHdds.emplace(aPlatformName, hdd);
      }
   }
}

void HDD::Plugin::ConnectToPlatformActionHandler()
{
   QAction* sender       = dynamic_cast<QAction*>(QObject::sender());
   QString  platformName = sender->data().toString();

   // Check if our sender is valid.
   if (sender)
   {
      // If the platform has a hud, release the old one.
      if (mHdds.find(platformName) != mHdds.end())
      {
         if (mHdds.at(platformName))
         {
            ReleasePlatform(platformName);
         }
      }

      // Connect the platform to a new HDD window.
      ConnectToPlatform(platformName, wkfEnv.GetMainWindow());
   }
}

QList<wkf::PrefWidget*> HDD::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void HDD::Plugin::GuiUpdate()
{
   GetInterface()->ProcessEvents(mDataContainer);

   // Check if the connection has been destroyed for our hdds
   for (auto hdd : mHdds)
   {
      if (GetInterface()->IsConnectionDestroyed(hdd.second.data()->GetName()))
      {
         hdd.second.data()->SetConnectionDestroyed(true);
      }
   }

   Update(mDataContainer);
}

void HDD::Plugin::ReleasePlatform(const QString& aPlatformName)
{
   mHdds.erase(aPlatformName);
   GetInterface()->ReleaseConnectedPlatform(aPlatformName);
}

// This function will not be needed in the future.
bool HDD::Plugin::RequestPlatformResourceInstance(const QString& aResourceName, const QString& aPlatformName)
{
   if (aResourceName == "hdd")
   {
      ConnectToPlatform(aPlatformName, wkfEnv.GetMainWindow()->centralWidget());
      return true;
   }
   return false;
}

void HDD::Plugin::Update(const DataContainer& aDataContainer)
{
   // If we have hdds, push current datacontainer data
   // into them so they can draw new data.
   if (mHdds.size() > 0)
   {
      for (auto hdd : mHdds)
      {
         // If we have data for this HDD.
         if (mDataContainer.GetHDDData().find(hdd.first) != mDataContainer.GetHDDData().end())
         {
            if (!mHdds.at(hdd.first).isNull())
            {
               const std::map<QString, HDD::HDD_Data>::mapped_type& headDownData =
                  mDataContainer.GetHDDData().at(hdd.first);

               const HDD::SimData::PlatformData&               platformData = headDownData.platformData;
               const HDD::SimData::TargetTrackItemList&        trackdata    = headDownData.masterTrackListData;
               const HDD::SimData::FuelData&                   fuelData     = headDownData.fuelData;
               const HDD::SimData::WeaponDataSummary&          weaponData   = headDownData.weaponData;
               const HDD::SimData::FlightControlsData&         controlsData = headDownData.flightControlsData;
               const HDD::SimData::NavData&                    navData      = headDownData.navData;
               const std::vector<HDD::SimData::TrackProcessor> tpData       = headDownData.trackProcessorList;
               const std::string                               esmTrackProcessorString = headDownData.esmTrackprocessor;
               // Note: These will be used when additional trackers are supported.
               // const std::string mwsTrackProcessorString   = headDownData.mwsTrackprocessor;
               // const std::string radarTrackProcessorString = headDownData.radarTrackprocessor;
               // const std::string irstTrackProcessorString  = headDownData.irstTrackprocessor;
               // const std::string dasTrackProcessorString   = headDownData.dasTrackprocessor;
               // const std::string flirTrackProcessorString  = headDownData.flirTrackprocessor;
               // const std::string eyeTrackProcessorString   = headDownData.eyeTrackprocessor;
               const HDD::SimData::AssetsList& assetData = headDownData.assetsList;

               mHdds[hdd.first]->SetPlatformData(platformData); // Pages needing platform data will use this for updating
               mHdds[hdd.first]->SetTrackData(
                  trackdata); // Pages needing track data (master track list) will use this for updating
               mHdds[hdd.first]->SetFuelData(fuelData);     // Pages needing fuel data will use this for updating
               mHdds[hdd.first]->SetWeaponData(weaponData); // Pages needing weapons data will use this for updating
               mHdds[hdd.first]->SetFlightControlsData(
                  controlsData); // Pages needing flight controls data will use this for updating

               // Pages needing secondary track processor data (not master track list) will use this for updating
               mHdds[hdd.first]->SetTrackProcessorData(tpData, esmTrackProcessorString);
               // Note: This will be used when additional trackers are supported.
               // mHdds[hdd.first]->SetTrackProcessorData(tpData,
               //                                         esmTrackProcessorString,
               //                                         mwsTrackProcessorString,
               //                                         radarTrackProcessorString,
               //                                         irstTrackProcessorString,
               //                                         dasTrackProcessorString,
               //                                         flirTrackProcessorString,
               //                                         eyeTrackProcessorString);

               mHdds[hdd.first]->SetNavData(navData);          // Pages needing nav data will use this for updating
               mHdds[hdd.first]->SetAssetsListData(assetData); // Pages needing assets list data will use this for updating
            }
         }
      }
   }
}
