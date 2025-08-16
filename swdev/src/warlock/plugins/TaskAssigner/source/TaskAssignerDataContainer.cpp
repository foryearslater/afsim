// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TaskAssignerDataContainer.hpp"

// ============================================================================
void WkTaskAssigner::DataContainer::SetPlatformOfInterest(const std::string& aPlatformName)
{
   mCommandChainData.Clear();
   mCommunicationDevices.clear();
   mJammers.clear();
   mProcessors.clear();
   mTaskProcessors.clear();
   mSensors.clear();
   mWeapons.clear();
   mValid              = false;
   mPlatformOfInterest = aPlatformName;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetData(const std::string&                          aPlatformName,
                                            const CommandChainData&                     aCommandChainData,
                                            const ObjectList&                           aCommunicationDevicesData,
                                            const std::map<std::string, JammerList>&    aJammerData,
                                            const std::map<std::string, ProcessorList>& aProcessorData,
                                            const std::map<std::string, ProcessorList>& aTaskProcessorData,
                                            const std::map<std::string, SensorList>&    aSensorData,
                                            const std::map<std::string, WeaponList>&    aWeaponData)
{
   mValid = false;
   if (mPlatformOfInterest == aPlatformName)
   {
      SetCommandChainData(aCommandChainData);
      SetCommunicationDevicesData(aCommunicationDevicesData);
      SetJammerData(aJammerData);
      SetProcessorData(aProcessorData);
      SetTaskProcessorData(aTaskProcessorData);
      SetSensorData(aSensorData);
      SetWeaponData(aWeaponData);
      mValid = true;
   }
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetCommandChainData(const CommandChainData& aCommandChainData)
{
   mCommandChainData = aCommandChainData;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetCommunicationDevicesData(const ObjectList& aCommunicationDevicesData)
{
   mCommunicationDevices = aCommunicationDevicesData;
}

// ============================================================================
const WkTaskAssigner::JammerList* WkTaskAssigner::DataContainer::GetJammerData(const std::string& aPlatformName) const
{
   auto data = mJammers.find(aPlatformName);
   if (data != mJammers.end())
   {
      return &(data->second);
   }
   return nullptr;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetJammerData(const std::map<std::string, JammerList>& aJammerData)
{
   mJammers = aJammerData;
}

// ============================================================================
const WkTaskAssigner::ProcessorList* WkTaskAssigner::DataContainer::GetProcessorData(const std::string& aPlatformName) const
{
   auto data = mProcessors.find(aPlatformName);
   if (data != mProcessors.end())
   {
      return &(data->second);
   }
   return nullptr;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetProcessorData(const std::map<std::string, ProcessorList>& aProcessorData)
{
   mProcessors = aProcessorData;
}

// ============================================================================
const WkTaskAssigner::ProcessorList* WkTaskAssigner::DataContainer::GetTaskProcessorData(const std::string& aPlatformName) const
{
   auto data = mTaskProcessors.find(aPlatformName);
   if (data != mTaskProcessors.end())
   {
      return &(data->second);
   }
   return nullptr;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetTaskProcessorData(const std::map<std::string, ProcessorList>& aTaskProcessorData)
{
   mTaskProcessors = aTaskProcessorData;
}

// ============================================================================
const WkTaskAssigner::SensorList* WkTaskAssigner::DataContainer::GetSensorData(const std::string& aPlatformName) const
{
   auto data = mSensors.find(aPlatformName);
   if (data != mSensors.end())
   {
      return &(data->second);
   }
   return nullptr;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetSensorData(const std::map<std::string, SensorList>& aSensorData)
{
   mSensors = aSensorData;
}

// ============================================================================
const WkTaskAssigner::WeaponList* WkTaskAssigner::DataContainer::GetWeaponData(const std::string& aPlatformName) const
{
   auto data = mWeapons.find(aPlatformName);
   if (data != mWeapons.end())
   {
      return &(data->second);
   }
   return nullptr;
}

// ============================================================================
void WkTaskAssigner::DataContainer::SetWeaponData(const std::map<std::string, WeaponList>& aWeaponData)
{
   mWeapons = aWeaponData;
}

// ============================================================================
void WkTaskAssigner::DataContainer::RemovePlatform(const std::string& aPlatformName)
{
   emit PlatformDeleted(aPlatformName);
}
