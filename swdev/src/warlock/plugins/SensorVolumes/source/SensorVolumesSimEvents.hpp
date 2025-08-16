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
#ifndef SENSORVOLUMESSIMEVENTS_HPP
#define SENSORVOLUMESSIMEVENTS_HPP

#include <map>
#include <string>

#include "SensorVolumesPlatform.hpp"
#include "WkSimInterface.hpp"

namespace WkSensorVolumes
{
class Plugin;

using PlatformSensorMap = std::map<std::pair<unsigned int, bool>, std::unique_ptr<Platform>>;

class SensorVolumeEvent : public warlock::SimEvent
{
public:
   SensorVolumeEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr) = 0;
};

class SimInitializingEvent : public warlock::SimEvent
{
public:
   SimInitializingEvent()
      : warlock::SimEvent()
   {
   }

   virtual void Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr) = 0;
};

class VolumeUpdateEvent : public SensorVolumeEvent
{
public:
   struct PartData
   {
      double mSlew[3]{0.0, 0.0, 0.0};
      double mCue[3]{0.0, 0.0, 0.0};
      double mTranslation[3];
   };


   struct PlatformEntry
   {
      // If a sensor is in the list, that means it's passive
      std::set<std::string> mPassiveSensors;
      // The key is the name of the sensor/weapon
      std::map<std::string, std::vector<Platform::Mode>> mSensorMap;
      std::map<std::string, PartData>                    mSensorArticulationMap;
      std::map<std::string, std::vector<Platform::Mode>> mWeaponMap;
      std::map<std::string, PartData>                    mWeaponArticulationMap;
   };

   VolumeUpdateEvent(std::map<unsigned int, PlatformEntry>&& aPlatformInfo, Platform::Source aSource)
      : SensorVolumeEvent(true)
      , mPlatformInfo(std::move(aPlatformInfo))
      , mSource(aSource)
   {
   }

   void Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr) override;

private:
   //! Used by Process() to create network traffic for passive sensor volumes.
   void NotifyPlugin(Plugin* aPluginPtr, unsigned int aIndex, PlatformEntry& aEntry) const;
   //! Used by Process() to update individual components.
   void UpdateSensorVolumes(PlatformSensorMap& aPlatformSensorMap, unsigned int aIndex, PlatformEntry& aEntry) const;
   void UpdateWeaponVolumes(PlatformSensorMap& aPlatformSensorMap, unsigned int aIndex, PlatformEntry& aEntry) const;

   std::map<unsigned int, PlatformEntry> mPlatformInfo;
   Platform::Source                      mSource;
};

class ActiveSensorListEvent : public SensorVolumeEvent
{
public:
   ActiveSensorListEvent(const unsigned int aPlatformIndex, const QStringList& aSensorList)
      : mPlatformIndex(aPlatformIndex)
      , mSensorList(aSensorList)
   {
   }

   void Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr) override;

private:
   unsigned int mPlatformIndex;
   QStringList  mSensorList;
};

class ActiveWeaponListEvent : public SensorVolumeEvent
{
public:
   ActiveWeaponListEvent(const unsigned int aPlatformIndex, const QStringList& aWeaponList)
      : mPlatformIndex(aPlatformIndex)
      , mWeaponList(aWeaponList)
   {
   }

   void Process(PlatformSensorMap& aPlatformSensorMap, Plugin* aPluginPtr) override;

private:
   unsigned int mPlatformIndex;
   QStringList  mWeaponList;
};
} // namespace WkSensorVolumes

#endif
