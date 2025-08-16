// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "SensorVolumesPlugin.hpp"

#include <QAction>
#include <QMenu>
#include <QString>

#include "SensorVolumesNetwork.hpp"
#include "SensorVolumesSimCommands.hpp"
#include "VaViewer.hpp"
#include "WkAppEnvironment.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfSensorMode.hpp"
#include "sensor_volume/WkfSensorVolumesOptionHistoryManager.hpp"
#include "sensor_volume/WkfSensorVolumesPrefObject.hpp"
#include "sensor_volume/WkfSensorVolumesPrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkSensorVolumes::Plugin,
   "Sensor Volumes",
   "The Sensor Volumes plugin is responsible for drawing sensor volumes on the main map display.",
   "warlock")

static const WkSensorVolumes::Clock::duration cNETWORK_UPDATE_INTERVAL = std::chrono::milliseconds(5000);
static const WkSensorVolumes::Clock::duration cSENSOR_TIMEOUT          = std::chrono::milliseconds(12000);

WkSensorVolumes::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new wkf::SensorVolumesPrefWidget(wkf::SensorVolumesPrefWidget::cLINEEDIT))
   , mIndividualIndex(0)
{
   SetOptionHistoryManager(new wkf::SensorVolumesOptionHistoryManager(aPluginName, this));
   mPrefObjectPtr = mPrefWidgetPtr->GetPreferenceObject();

   mTopSensorItem = RegisterOption(nullptr, "Sensor Volumes");
   mTopWeaponItem = RegisterOption(nullptr, "Jammer Volumes");

   mCallbacks.Add(wkf::Observer::EntityDeleted.Connect(&Plugin::PlatformDeletedHandler, this));

   wkEnv.GetNetwork().RegisterPacket<SensorVolumePacket>();
   mNetworkCallbacks += wkEnv.GetNetwork().Subscribe<SensorVolumePacket>(&Plugin::OnSensorVolumePacketReceived, this);

   connect(mPrefWidgetPtr->GetPreferenceObject(), &wkf::SensorVolumesPrefObject::ModesReset, this, &Plugin::ModesReset);
   connect(mPrefWidgetPtr->GetPreferenceObject(), &wkf::SensorVolumesPrefObject::DrawModeChanged, this, &Plugin::DrawModeChanged);
   connect(&wkfEnv,
           &wkf::Environment::PlatformVisibilityChanged,
           this,
           [this]()
           {
              auto* scenarioPtr = vaEnv.GetStandardScenario();
              if (scenarioPtr)
              {
                 for (auto& i : mSensorVolumeMap)
                 {
                    auto* platformPtr = scenarioPtr->FindPlatformByIndex(i.first.first);
                    if (platformPtr)
                    {
                       i.second->SetPlatformVisibility(wkfEnv.IsPlatformVisible(platformPtr));
                    }
                 }
              }
           });
}

void WkSensorVolumes::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   UtVariant v = aViewerPtr->GetData("type");
   // Add boresight vector support for any wkf::ImmersiveViewDockWidget
   if (!v.IsUnset() && v.GetString() == "immersive")
   {
      QMenu* addMenu = aMenu->findChild<QMenu*>("Vectors");
      if (!addMenu)
      {
         addMenu = aMenu->addMenu("Vectors");
         addMenu->setObjectName("Vectors");
      }

      // Find the current dockWidget that is immersive
      QMainWindow*                  centralWidget = wkfEnv.GetMainWindow()->centralWidget();
      wkf::ImmersiveViewDockWidget* currDockPtr   = nullptr;
      for (auto& i : centralWidget->children())
      {
         auto dockWidget = dynamic_cast<wkf::ImmersiveViewDockWidget*>(i);
         if (dockWidget != nullptr && dockWidget->IsMyViewer(aViewerPtr))
         {
            currDockPtr = dockWidget;
         }
      }
      if (currDockPtr == nullptr)
      {
         return;
      }

      // Find action to insert boresight actions after
      QAction* previousAction = nullptr;
      for (auto action : addMenu->actions())
      {
         if (action->text() == "New Vector...")
         {
            previousAction = action;
            break;
         }
      }

      auto plat = mInterfacePtr->GetPlatform(currDockPtr->GetConnectedPlatformName().toStdString());

      if (previousAction == nullptr || plat == nullptr)
      {
         return;
      }

      for (auto component : plat->GetComponents())
      {
         WsfSensor* sensor = dynamic_cast<WsfSensor*>(component);
         if (sensor != nullptr && sensor->IsTurnedOn())
         {
            QAction* action = new QAction(QString::fromStdString(sensor->GetName()), aMenu);
            action->setCheckable(true);
            action->setChecked(currDockPtr->GetBoresightShown(sensor->GetName()));
            addMenu->insertAction(previousAction, action);

            // On click of action
            connect(action,
                    &QAction::triggered,
                    [this, currDockPtr, sensor]()
                    {
                       // If boresights are not shown yet, show them
                       if (!currDockPtr->GetBoresightShown(sensor->GetName()))
                       {
                          WkSensorVolumes::SimInterface* interfacePtr =
                             dynamic_cast<WkSensorVolumes::SimInterface*>(mInterfacePtr.get());
                          if (interfacePtr != nullptr)
                          {
                             // On each clock update until scenario ends, update boresight positions
                             connect(interfacePtr,
                                     &WkSensorVolumes::SimInterface::ShowBoresight,
                                     [=](wkf::ImmersiveViewDockWidget* aWidgetPtr,
                                         const std::string&            aSensorName,
                                         double                        aAz,
                                         double                        aEl)
                                     {
                                        if (currDockPtr == aWidgetPtr)
                                        {
                                           currDockPtr->ShowBoresight(aSensorName, aAz, aEl);
                                        }
                                     });
                             connect(interfacePtr,
                                     &WkSensorVolumes::SimInterface::UpdateBoresight,
                                     [=](wkf::ImmersiveViewDockWidget* aWidgetPtr,
                                         const std::string&            aSensorName,
                                         double                        aAz,
                                         double                        aEl)
                                     {
                                        if (currDockPtr == aWidgetPtr)
                                        {
                                           currDockPtr->UpdateBoresight(aSensorName, aAz, aEl);
                                        }
                                     });
                             connect(interfacePtr,
                                     &WkSensorVolumes::SimInterface::HideBoresight,
                                     [=](wkf::ImmersiveViewDockWidget* aWidgetPtr, const std::string& aSensorName)
                                     {
                                        if (currDockPtr == aWidgetPtr)
                                        {
                                           currDockPtr->HideBoresight(aSensorName);
                                        }
                                     });
                          }

                          // Show boresight of each sensor on specific platform
                          this->mInterfacePtr->EnableBoresightUpdates(currDockPtr, sensor->GetName());
                       }
                       // If boresights are already shown, hide them
                       else
                       {
                          this->mInterfacePtr->EnableBoresightUpdates(currDockPtr, sensor->GetName(), false);
                          currDockPtr->HideBoresight(sensor->GetName());
                       }
                    });
         }
      }
   }
}


void WkSensorVolumes::Plugin::PlatformDeletedHandler(vespa::VaEntity* aPlatformPtr)
{
   wkf::Platform* derivedPtr = dynamic_cast<wkf::Platform*>(aPlatformPtr);
   if (derivedPtr)
   {
      mSensorVolumeMap.erase(std::make_pair(derivedPtr->GetIndex(), false));
      mSensorVolumeMap.erase(std::make_pair(derivedPtr->GetIndex(), true));
   }
}

QList<wkf::PrefWidget*> WkSensorVolumes::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> list;
   list.push_back(mPrefWidgetPtr);
   return list;
}

void WkSensorVolumes::Plugin::ResetOptionStates()
{
   for (auto& it : mSensorVolumeMap)
   {
      it.second->RemoveAttachments();
   }
   mSensorVolumeMap.clear();
}

void WkSensorVolumes::Plugin::SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr)
{
   unsigned int index        = aPlatformPtr->GetIndex();
   int          p            = GetOptionHistoryManager()->GetParentOption(aOptionId);
   bool         isWeaponType = p == mTopWeaponItem->type();
   auto         mp           = std::make_pair(index, isWeaponType);
   if (aState)
   {
      auto jt = mSensorVolumeMap.find(mp);
      if (jt == mSensorVolumeMap.end())
      {
         // Create a new entry in the SensorVolumeMap
         auto retPair = mSensorVolumeMap.emplace(mp, ut::make_unique<Platform>(mPrefObjectPtr, isWeaponType));
         jt           = retPair.first;
         // Default the PlatformVisibility to be the same as the WKF environment
         jt->second->SetPlatformVisibility(wkfEnv.IsPlatformVisible(aPlatformPtr));
         if (aOptionId != p)
         {
            jt->second->SetVolumeVisibility(false);
         }
      }

      if (aOptionId == p)
      {
         jt->second->SetVolumeVisibility(true);
      }
      else
      {
         jt->second->SetSensorVisible(OptionName(aOptionId).toStdString(), true);
      }
   }
   else
   {
      auto jt = mSensorVolumeMap.find(mp);
      if (jt != mSensorVolumeMap.end())
      {
         if (aOptionId == p)
         {
            jt->second->SetVolumeVisibility(false);
         }
         else
         {
            jt->second->SetSensorVisible(OptionName(aOptionId).toStdString(), false);
         }
      }
   }
}

Qt::CheckState WkSensorVolumes::Plugin::GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const
{
   if (aPlatformPtr)
   {
      unsigned int index        = aPlatformPtr->GetIndex();
      int          p            = GetOptionHistoryManager()->GetParentOption(aOptionId);
      bool         isWeaponType = p == mTopWeaponItem->type();
      auto         mp           = std::make_pair(index, isWeaponType);

      auto it = mSensorVolumeMap.find(mp);
      if (it != mSensorVolumeMap.end())
      {
         if (aOptionId == p)
         {
            bool all  = true;
            bool some = true;
            it->second->CheckSensorVisibility(all, some);
            return (all ? Qt::Checked : some ? Qt::PartiallyChecked : Qt::Unchecked);
         }
         else if (it->second->SensorVisible(OptionName(aOptionId).toStdString(), false))
         {
            return Qt::Checked;
         }
      }
   }
   return Qt::Unchecked;
}

void WkSensorVolumes::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mSensorVolumeMap, this);
   RemoveAttachmentsPastTimeout();
}

void WkSensorVolumes::Plugin::AddMode(WsfSensorMode& aSensorMode)
{
   mPrefWidgetPtr->AddSensorModeName(aSensorMode.GetName());
}

void WkSensorVolumes::Plugin::ModesReset()
{
   // notify the volumes
   for (auto& it : mSensorVolumeMap)
   {
      it.second->ModesReset();
   }
}

void WkSensorVolumes::Plugin::DrawModeChanged(const wkf::SensorVolumesPrefData::DrawMode& aState)
{
   // notify the volumes
   for (auto& jt : mSensorVolumeMap)
   {
      jt.second->RemoveAttachments();
   }
}

void WkSensorVolumes::Plugin::ClearIndividualOptions()
{
   wkf::Plugin::ClearIndividualOptions();
   mIndividualsSensors.clear();
   mIndividualsWeapons.clear();
}

WkSensorVolumes::Platform* WkSensorVolumes::Plugin::GetOrAddSensorVolumeEntry(unsigned int aPlatform, bool aWeapon)
{
   const auto mp = std::make_pair(aPlatform, aWeapon);
   auto       it = mSensorVolumeMap.find(mp);
   if (it == mSensorVolumeMap.end())
   {
      auto      ptr  = ut::make_unique<Platform>(mPrefObjectPtr, aWeapon);
      Platform* plat = ptr.get();
      mSensorVolumeMap.emplace(mp, std::move(ptr));
      return plat;
   }
   return it->second.get();
}

void WkSensorVolumes::Plugin::SendSensorVolumePacket(const warlock::Platform*                aPlatform,
                                                     const VolumeUpdateEvent::PlatformEntry& aEntry)
{
   Platform* plat = GetOrAddSensorVolumeEntry(aPlatform->GetIndex(), false);

   const auto now = Clock::now();
   if (plat && !aEntry.mPassiveSensors.empty())
   {
      for (const auto& sensor : aEntry.mSensorMap)
      {
         const bool isPassive = (aEntry.mPassiveSensors.find(sensor.first) != aEntry.mPassiveSensors.end());
         if (isPassive && plat->LastPacketBefore(sensor.first, cNETWORK_UPDATE_INTERVAL, now))
         {
            SensorVolumePacket packet;
            packet.mPlatform->SetPlatform(aPlatform);
            packet.mSensor->Set(QString::fromStdString(sensor.first));
            for (const auto& mode : sensor.second)
            {
               auto& modeData = packet.mModeList->Append();
               modeData.SetData(mode);
            }

            plat->SetLastPacketTime(sensor.first, now);
            wkEnv.GetNetwork().Publish(packet);
         }
      }
   }
}

void WkSensorVolumes::Plugin::OnSensorVolumePacketReceived(const SensorVolumePacket& aPacket)
{
   warlock::Platform* platform = aPacket.mPlatform->GetPlatform();
   if (platform && platform->GetExternallyControlled())
   {
      std::map<unsigned int, VolumeUpdateEvent::PlatformEntry> platformInfo;
      const std::string                                        sensorName = aPacket.mSensor->Get().toStdString();
      auto&                                                    entry      = platformInfo[platform->GetIndex()];
      auto&                                                    modeList   = entry.mSensorMap[sensorName];
      modeList                                                            = aPacket.GetModeList();
      VolumeUpdateEvent evt{std::move(platformInfo), Platform::Source::Network};
      evt.Process(mSensorVolumeMap, this);
   }
}

void WkSensorVolumes::Plugin::RegisterIndividualOptions(const std::string& aStringId)
{
   auto* plat = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(aStringId));
   if (plat)
   {
      mIndividualIndex = plat->GetIndex();

      auto plat = GetOrAddSensorVolumeEntry(mIndividualIndex, false);
      if (plat)
      {
         for (const auto& sensor : plat->GetSensorList())
         {
            ProcessSensor(mIndividualIndex, sensor);
         }
      }

      mInterfacePtr->AddSimCommand(ut::make_unique<SensorListRequest>(GetInterface(), mIndividualIndex));
      mInterfacePtr->AddSimCommand(ut::make_unique<WeaponListRequest>(GetInterface(), mIndividualIndex));
   }
   else
   {
      mIndividualIndex = 0;
   }
   // add individuals sensors
}

void WkSensorVolumes::Plugin::ProcessSensor(unsigned int aEntityId, const std::string& aSensorName)
{
   if (aEntityId == mIndividualIndex)
   {
      if (mIndividualsSensors.find(aSensorName) == mIndividualsSensors.end())
      {
         mIndividualsSensors.insert(aSensorName);
         bool check = (mTopSensorItem->checkState(0) != Qt::Unchecked);
         auto plat  = GetOrAddSensorVolumeEntry(mIndividualIndex, false);
         if (plat)
         {
            check = plat->IsDefaultVisible();
            check = plat->SensorVisible(aSensorName, check);
         }
         RegisterOption(mTopSensorItem, aSensorName.c_str(), "", true, check);
      }
   }
}

void WkSensorVolumes::Plugin::ProcessWeapon(unsigned int aEntityId, const std::string& aWeaponName)
{
   if (aEntityId == mIndividualIndex)
   {
      if (mIndividualsWeapons.find(aWeaponName) == mIndividualsWeapons.end())
      {
         mIndividualsWeapons.insert(aWeaponName);
         bool check = (mTopWeaponItem->checkState(0) != Qt::Unchecked);
         auto plat  = GetOrAddSensorVolumeEntry(mIndividualIndex, true);
         if (plat)
         {
            check = plat->IsDefaultVisible();
            check = plat->SensorVisible(aWeaponName, check);
         }
         RegisterOption(mTopWeaponItem, aWeaponName.c_str(), "", true, check);
      }
   }
}

void WkSensorVolumes::Plugin::RemoveAttachmentsPastTimeout()
{
   auto now = Clock::now();
   for (auto& it : mSensorVolumeMap)
   {
      it.second->RemoveAttachmentsPastTimeout(cSENSOR_TIMEOUT, now);
   }
}
