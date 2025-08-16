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

#include "TaskAssignerSimInterface.hpp"

#include "UtMemory.hpp"
#include "WsfComm.hpp"
#include "WsfCommandChain.hpp"
#include "WsfPlatform.hpp"
#include "WsfProcessor.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorMode.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeapon.hpp"

// ============================================================================
WkTaskAssigner::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<TaskEvent>(aPluginName)
{
}

// ============================================================================
void WkTaskAssigner::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

// ============================================================================
void WkTaskAssigner::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   // Notify if platform is related to the platform-of-interest
   if (Related(aPlatform))
   {
      AddSimEvent(ut::make_unique<DeletePlatformEvent>(aPlatform.GetName()));
   }
}

// ============================================================================
void WkTaskAssigner::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   const WsfPlatform* platformPtr = nullptr;

   mMutex.lock();
   if (!mPlatformOfInterest.empty())
   {
      platformPtr = aSimulation.GetPlatformByName(mPlatformOfInterest);
   }
   mMutex.unlock();

   if (platformPtr)
   {
      CommandChainData                     commandChainData;
      std::map<std::string, JammerList>    jammersData;
      std::map<std::string, ProcessorList> processorsData;
      std::map<std::string, ProcessorList> taskProcessorsData;
      std::map<std::string, SensorList>    sensorsData;
      std::map<std::string, WeaponList>    weaponsData;

      // SELF
      commandChainData.mSelf = ObjectData(platformPtr->GetName(), platformPtr->GetType());
      GetComponentData(platformPtr, jammersData, processorsData, taskProcessorsData, sensorsData, weaponsData);

      // All command chains
      for (WsfComponentList::RoleIterator<WsfCommandChain> iter(*platformPtr); !iter.AtEnd(); ++iter)
      {
         WsfCommandChain* commandChainPtr = *iter;

         // Command chain names
         commandChainData.mCommandChainNames.emplace_back(commandChainPtr->GetName());

         // Commander
         WsfPlatform* commander = commandChainPtr->GetCommander();
         if (commander)
         {
            commandChainData.mCommanders.emplace(
               std::make_pair(commandChainPtr->GetName(), ObjectData(commander->GetName(), commander->GetType())));
         }
         // Peers
         ObjectList                           peersData;
         const WsfCommandChain::PlatformList& peerList = commandChainPtr->GetPeers();
         for (WsfPlatform* peerPtr : peerList)
         {
            peersData.emplace_back(peerPtr->GetName(), peerPtr->GetType());
            GetComponentData(peerPtr, jammersData, processorsData, taskProcessorsData, sensorsData, weaponsData);
         }
         commandChainData.mPeers.emplace(std::make_pair(commandChainPtr->GetName(), peersData));

         // Subordinates
         ObjectList                           subsData;
         const WsfCommandChain::PlatformList& subList = commandChainPtr->GetSubordinates();
         for (WsfPlatform* subPtr : subList)
         {
            subsData.emplace_back(subPtr->GetName(), subPtr->GetType());
            GetComponentData(subPtr, jammersData, processorsData, taskProcessorsData, sensorsData, weaponsData);
         }
         commandChainData.mSubordinates.emplace(std::make_pair(commandChainPtr->GetName(), subsData));
      }

      // Comms on assigner
      ObjectList communicationDevicesData;
      for (WsfComponentList::RoleIterator<wsf::comm::Comm> iter(*platformPtr); !iter.AtEnd(); ++iter)
      {
         communicationDevicesData.emplace_back((*iter)->GetName(), (*iter)->GetType());
      }

      AddSimEvent(ut::make_unique<TaskUpdateEvent>(platformPtr->GetName(),
                                                   commandChainData,
                                                   communicationDevicesData,
                                                   jammersData,
                                                   processorsData,
                                                   taskProcessorsData,
                                                   sensorsData,
                                                   weaponsData));
   }
}

// ============================================================================
void WkTaskAssigner::SimInterface::GetComponentData(const WsfPlatform*                    aPlatformPtr,
                                                    std::map<std::string, JammerList>&    aJammersData,
                                                    std::map<std::string, ProcessorList>& aProcessorsData,
                                                    std::map<std::string, ProcessorList>& aTaskProcessorsData,
                                                    std::map<std::string, SensorList>&    aSensorsData,
                                                    std::map<std::string, WeaponList>&    aWeaponsData)
{
   JammerList    jammerData;
   ProcessorList processorData;
   ProcessorList taskProcessorData;
   SensorList    sensorData;
   WeaponList    weaponData;
   GetComponentData(aPlatformPtr, jammerData, processorData, taskProcessorData, sensorData, weaponData);
   aJammersData.emplace(std::make_pair(aPlatformPtr->GetName(), jammerData));
   aProcessorsData.emplace(std::make_pair(aPlatformPtr->GetName(), processorData));
   aTaskProcessorsData.emplace(std::make_pair(aPlatformPtr->GetName(), taskProcessorData));
   aSensorsData.emplace(std::make_pair(aPlatformPtr->GetName(), sensorData));
   aWeaponsData.emplace(std::make_pair(aPlatformPtr->GetName(), weaponData));
}

// ============================================================================
void WkTaskAssigner::SimInterface::GetComponentData(const WsfPlatform* aPlatformPtr,
                                                    JammerList&        aJammerData,
                                                    ProcessorList&     aProcessorData,
                                                    ProcessorList&     aTaskProcessorData,
                                                    SensorList&        aSensorData,
                                                    WeaponList&        aWeaponData)
{
   // Processors
   for (WsfComponentList::RoleIterator<WsfProcessor> iter(*aPlatformPtr); !iter.AtEnd(); ++iter)
   {
      aProcessorData.emplace_back((*iter)->GetName(), (*iter)->GetType());
      if ((*iter)->IsA_TypeOf("WSF_TASK_PROCESSOR"))
      {
         aTaskProcessorData.emplace_back((*iter)->GetName(), (*iter)->GetType());
      }
   }

   // Sensors
   for (WsfComponentList::RoleIterator<WsfSensor> iter(*aPlatformPtr); !iter.AtEnd(); ++iter)
   {
      WsfSensor*               sensorPtr = (*iter);
      std::vector<std::string> modes;
      WsfSensorModeList*       modeList = sensorPtr->GetModeList();
      if (modeList != nullptr)
      {
         for (size_t i = 0; i < sensorPtr->GetModeCount(); ++i)
         {
            WsfSensorMode* mode = sensorPtr->GetModeEntry(i);
            if (mode != nullptr)
            {
               modes.push_back(mode->GetName());
            }
         }
      }

      aSensorData.emplace_back(sensorPtr->GetName(), sensorPtr->GetType(), modes);
   }

   // Weapons - both explicit and jammers
   for (WsfComponentList::RoleIterator<WsfWeapon> iter(*aPlatformPtr); !iter.AtEnd(); ++iter)
   {
      if ((*iter)->IsA_TypeOf("WSF_EXPLICIT_WEAPON"))
      {
         aWeaponData.emplace_back((*iter)->GetUniqueId(),
                                  (*iter)->GetName(),
                                  (*iter)->GetType(),
                                  (*iter)->GetQuantityRemaining());
      }
      else if ((*iter)->IsA_TypeOf("WSF_RF_JAMMER"))
      {
         aJammerData.emplace_back((*iter)->GetName(), (*iter)->GetType());
      }
   }
}

// ============================================================================
bool WkTaskAssigner::SimInterface::Related(const WsfPlatform& aPlatform)
{
   bool related(false);
   if (!mPlatformOfInterest.empty())
   {
      // All command chains
      for (WsfComponentList::RoleIterator<WsfCommandChain> iter(aPlatform); !iter.AtEnd(); ++iter)
      {
         WsfCommandChain* commandChainPtr = *iter;
         related                          = (commandChainPtr->GetCommanderName() == mPlatformOfInterest);
         if (!related)
         {
            // Check subordinate relationship
            const WsfCommandChain::PlatformList& subList = commandChainPtr->GetSubordinates();
            for (WsfPlatform* subPtr : subList)
            {
               if (subPtr->GetName() == mPlatformOfInterest)
               {
                  related = true;
                  break;
               }
            }

            if (!related)
            {
               // Check peer relationship
               const WsfCommandChain::PlatformList& peerList = commandChainPtr->GetPeers();
               for (WsfPlatform* peerPtr : peerList)
               {
                  if (peerPtr->GetName() == mPlatformOfInterest)
                  {
                     related = true;
                     break;
                  }
               }
            }
         }

         if (related)
         {
            break;
         }
      }
   }
   return related;
}
