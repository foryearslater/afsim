// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SensorVolumesSimEvents.hpp"

#include "SensorVolumesPlugin.hpp"
#include "WkPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

void WkSensorVolumes::SimInitializingEvent::Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr)
{
   aPlatformSensorMap.clear();
}

void WkSensorVolumes::VolumeUpdateEvent::Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr)
{
   if (vaEnv.GetStandardScenario() && vaEnv.GetStandardViewer())
   {
      for (auto& i : mPlatformInfo)
      {
         NotifyPlugin(aPluginPtr, i.first, i.second);
         UpdateSensorVolumes(aPlatformSensorMap, i.first, i.second);
         UpdateWeaponVolumes(aPlatformSensorMap, i.first, i.second);
      }
   }
}

void WkSensorVolumes::VolumeUpdateEvent::NotifyPlugin(Plugin* aPluginPtr, unsigned int aIndex, PlatformEntry& aEntry) const
{
   auto* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      auto* platform = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByIndex(aIndex));
      if (platform && !platform->GetExternallyControlled())
      {
         aPluginPtr->SendSensorVolumePacket(platform, aEntry);
      }
      else if (platform)
      {
         aPluginPtr->GetOrAddSensorVolumeEntry(platform->GetIndex(), false);
      }
   }
}

void WkSensorVolumes::VolumeUpdateEvent::UpdateSensorVolumes(PlatformSensorMap& aPlatformSensorMap,
                                                             unsigned int       aIndex,
                                                             PlatformEntry&     aEntry) const
{
   auto sit = aPlatformSensorMap.find(std::make_pair(aIndex, false));
   if (sit != aPlatformSensorMap.end())
   {
      sit->second->MarkAll(mSource);
      for (auto& jt : aEntry.mSensorMap)
      {
         sit->second->UpdateAndUnmark(jt.first,
                                      aEntry.mSensorArticulationMap[jt.first].mSlew,
                                      aEntry.mSensorArticulationMap[jt.first].mCue,
                                      aEntry.mSensorArticulationMap[jt.first].mTranslation,
                                      jt.second,
                                      vaEnv.GetStandardScenario()->FindPlatformByIndex(aIndex),
                                      vaEnv.GetStandardViewer(),
                                      mSource);
      }
      sit->second->HideMarked();
   }
}

void WkSensorVolumes::VolumeUpdateEvent::UpdateWeaponVolumes(PlatformSensorMap& aPlatformSensorMap,
                                                             unsigned int       aIndex,
                                                             PlatformEntry&     aEntry) const
{
   auto wit = aPlatformSensorMap.find(std::make_pair(aIndex, true));
   if (wit != aPlatformSensorMap.end())
   {
      wit->second->MarkAll(mSource);
      for (auto& jt : aEntry.mWeaponMap)
      {
         wit->second->UpdateAndUnmark(jt.first,
                                      aEntry.mWeaponArticulationMap[jt.first].mSlew,
                                      aEntry.mWeaponArticulationMap[jt.first].mCue,
                                      aEntry.mWeaponArticulationMap[jt.first].mTranslation,
                                      jt.second,
                                      vaEnv.GetStandardScenario()->FindPlatformByIndex(aIndex),
                                      vaEnv.GetStandardViewer(),
                                      mSource);
      }
      wit->second->HideMarked();
   }
}

void WkSensorVolumes::ActiveSensorListEvent::Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr)
{
   for (auto& it : mSensorList)
   {
      aPluginPtr->ProcessSensor(mPlatformIndex, it.toStdString());
   }
}

void WkSensorVolumes::ActiveWeaponListEvent::Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr)
{
   for (auto& it : mWeaponList)
   {
      aPluginPtr->ProcessWeapon(mPlatformIndex, it.toStdString());
   }
}
