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

#include "SensorVolumesSimInterface.hpp"

#include "UtMemory.hpp"
#include "WkPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPlatform.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorMode.hpp"
#include "WsfSensorModeList.hpp"
#include "WsfSimulation.hpp"
#include "sensor_volume/WkfSensorVolumesPrefObject.hpp"

WkSensorVolumes::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<SensorVolumeEvent>(aPluginName)
{
   qRegisterMetaType<std::map<std::string, UtVec3d>>("std::map<std::string, UtVec3d>");
}

void WkSensorVolumes::SimInterface::EnableBoresightUpdates(wkf::ImmersiveViewDockWidget* aWidget,
                                                           const std::string&            aSensorName,
                                                           bool                          aEnabled)
{
   std::string platformName = aWidget->GetConnectedPlatformName().toStdString();
   if (aEnabled)
   {
      auto sensor = dynamic_cast<WsfSensor*>(mPlatforms[platformName]->FindComponent(aSensorName));
      if (sensor)
      {
         mUpdateWidgets[aWidget].emplace_back(aSensorName);
         auto orientation = GetOrientation(sensor);
         emit ShowBoresight(aWidget, aSensorName, orientation.first, orientation.second);

         connect(aWidget, &QObject::destroyed, [=](QObject* obj) { mUpdateWidgets.clear(); });
      }
   }
   else
   {
      auto it = mUpdateWidgets.find(aWidget);
      if (it != mUpdateWidgets.end())
      {
         it->second.erase(std::find(it->second.begin(), it->second.end(), aSensorName));
      }
   }
}

void WkSensorVolumes::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   const std::size_t count = aSimulation.GetPlatformCount();
   if (count > 0)
   {
      std::map<unsigned int, VolumeUpdateEvent::PlatformEntry> data;

      for (std::size_t i = 0; i < count; i++)
      {
         std::size_t index = aSimulation.GetPlatformEntry(i)->GetIndex();
         ReadData(aSimulation, static_cast<unsigned int>(index), eSENSOR | eWEAPON, data);
      }

      AddSimEvent(ut::make_unique<VolumeUpdateEvent>(std::move(data), Platform::Source::SimInterface));
   }
}


void WkSensorVolumes::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   static double lastSimTime = -1;
   if (lastSimTime == aSimulation.GetSimTime() || mUpdateWidgets.empty())
   {
      return;
   }
   lastSimTime = aSimulation.GetSimTime();

   std::map<std::string, UtVec3d> sensorOrientations;
   auto                           widgetIter = mUpdateWidgets.begin();
   while (widgetIter != mUpdateWidgets.end())
   {
      std::string platformName = widgetIter->first->GetConnectedPlatformName().toStdString();

      if (mPlatforms.find(platformName) != mPlatforms.end())
      {
         auto plat = mPlatforms[platformName];

         auto sensorIter = widgetIter->second.begin();
         while (sensorIter != widgetIter->second.end())
         {
            WsfSensor* sensor = dynamic_cast<WsfSensor*>(plat->FindComponent(*sensorIter));
            if (sensor && sensor->IsTurnedOn())
            {
               auto azEl = GetOrientation(sensor);
               emit UpdateBoresight(widgetIter->first, sensor->GetName(), azEl.first, azEl.second);
               ++sensorIter;
            }
            else
            {
               if (sensor)
               {
                  emit HideBoresight(widgetIter->first, sensor->GetName());
               }
               sensorIter = widgetIter->second.erase(sensorIter);
            }
         }

         if (widgetIter->second.empty())
         {
            widgetIter = mUpdateWidgets.erase(widgetIter);
         }
         else
         {
            ++widgetIter;
         }
      }
   }
}

void WkSensorVolumes::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mPlatforms.clear();
   mUpdateWidgets.clear();
}

void WkSensorVolumes::SimInterface::ReadData(const WsfSimulation&                                      aSimulation,
                                             unsigned int                                              aPlatformIndex,
                                             char                                                      aComponents,
                                             std::map<unsigned int, VolumeUpdateEvent::PlatformEntry>& aData) const
{
   WsfPlatform* platform = aSimulation.GetPlatformByIndex(aPlatformIndex);
   if (platform)
   {
      VolumeUpdateEvent::PlatformEntry entry;

      if (aComponents & eSENSOR) // is the sensor bit positive
      {
         ReadSensorData(aSimulation, *platform, entry);
      }
      if (aComponents & eWEAPON) // is the weapon bit positive
      {
         ReadWeaponData(aSimulation, *platform, entry);
      }
      aData[aPlatformIndex] = std::move(entry);
   }
}

void WkSensorVolumes::SimInterface::ReadSensorData(const WsfSimulation&              aSimulation,
                                                   const WsfPlatform&                aPlatform,
                                                   VolumeUpdateEvent::PlatformEntry& aEntry) const
{
   unsigned int sensorCount = aPlatform.GetComponentCount<WsfSensor>();
   // double ECS[3] = { 0 };
   for (unsigned int sc = 0; sc < sensorCount; ++sc)
   {
      WsfSensor* sensor = aPlatform.GetComponentEntry<WsfSensor>(sc);
      if (sensor && sensor->IsTurnedOn() && !sensor->IsBroken())
      {
         if (sensor->IsClassPassive())
         {
            aEntry.mPassiveSensors.insert(sensor->GetName());
         }

         // ------ mode ------ //

         WsfSensorModeList* modeList = sensor->GetModeList();
         if (modeList)
         {
            std::vector<WsfSensorMode*> modeVec;
            modeList->GetDerivedModeList(modeVec);
            for (auto& m : modeVec)
            {
               if (m->IsSelected())
               {
                  aEntry.mSensorMap[sensor->GetName()].emplace_back(*m);
               }
            }
         }
         else
         {
            WsfSensorMode* wsfMode = sensor->GetCurrentMode();
            aEntry.mSensorMap[sensor->GetName()].emplace_back(*wsfMode);
         }

         // ------ articulation ------ //
         // taken from WsfEventPipeInterface::UpdateArticulation

         VolumeUpdateEvent::PartData sensorInfo;
         if (sensor->GetEM_XmtrCount() != 0)
         {
            if ((sensor->GetSlewMode() != WsfArticulatedPart::cSLEW_FIXED) && (!sensor->IsCued()) &&
                (!sensor->TransientCueActive()))
            {
               sensor->GetCurrentCuedOrientation(sensorInfo.mCue[0], sensorInfo.mCue[1]);
            }
         }
         sensor->GetCurrentOrientationECS(sensorInfo.mSlew[0], sensorInfo.mSlew[1], sensorInfo.mSlew[2]);

         double* sensorTrans = sensorInfo.mTranslation;
         sensor->GetLocation(sensorTrans);

         aEntry.mSensorArticulationMap[sensor->GetName()] = sensorInfo;
      }
   }
}

void WkSensorVolumes::SimInterface::ReadWeaponData(const WsfSimulation&              aSimulation,
                                                   const WsfPlatform&                aPlatform,
                                                   VolumeUpdateEvent::PlatformEntry& aEntry) const
{
   unsigned int weaponCount = aPlatform.GetComponentCount<WsfWeapon>();
   for (unsigned int wc = 0; wc < weaponCount; ++wc)
   {
      WsfWeapon* weapon = aPlatform.GetComponentEntry<WsfWeapon>(wc);
      if (weapon && weapon->IsA_TypeOf("WSF_RF_JAMMER") && weapon->IsTurnedOn() && !weapon->IsBroken())
      {
         // ------ mode ------ //
         int mc = (int)weapon->GetModeCount();
         if (mc > 0)
         {
            for (int i = 0; i < mc; ++i)
            {
               WsfWeapon::WsfWeaponMode* m = weapon->GetModeEntry(i);
               aEntry.mWeaponMap[weapon->GetName()].emplace_back(*m);
            }
         }
         else
         {
            WsfWeapon::WsfWeaponMode* wsfMode = weapon->GetCurrentMode();
            aEntry.mWeaponMap[weapon->GetName()].emplace_back(*wsfMode);
         }

         // ------ articulation ------ //
         // taken from WsfEventPipeInterface::UpdateArticulation

         VolumeUpdateEvent::PartData weaponInfo;
         if (weapon->GetEM_XmtrCount() != 0)
         {
            if ((weapon->GetSlewMode() != WsfArticulatedPart::cSLEW_FIXED) && (!weapon->IsCued()) &&
                (!weapon->TransientCueActive()))
            {
               weapon->GetCurrentCuedOrientation(weaponInfo.mCue[0], weaponInfo.mCue[2]);
            }
         }
         weapon->GetCurrentOrientationECS(weaponInfo.mSlew[0], weaponInfo.mSlew[1], weaponInfo.mSlew[2]);

         double* weaponTrans = weaponInfo.mTranslation;
         weapon->GetLocation(weaponTrans);

         aEntry.mWeaponArticulationMap[weapon->GetName()] = weaponInfo;
      }
   }
}

std::pair<double, double> WkSensorVolumes::SimInterface::GetOrientation(WsfSensor* aSensorPtr)
{
   double az, el;
   aSensorPtr->GetCurrentCuedOrientation(az, el);

   auto slewMode = aSensorPtr->GetSlewMode();
   if (!(slewMode & WsfSensor::cSLEW_AZ))
   {
      az = aSensorPtr->GetYaw();
   }
   if (!(slewMode & WsfSensor::cSLEW_EL))
   {
      el = aSensorPtr->GetPitch();
   }
   return std::make_pair(az, el);
}
