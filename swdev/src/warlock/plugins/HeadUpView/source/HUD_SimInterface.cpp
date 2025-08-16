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

#include "HUD_SimInterface.hpp"

#include "WsfSA_Processor.hpp"
#include "WsfSimulation.hpp"

HeadsUpDisplay::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<WkHeadsUpDisplayEvent>(aPluginName)
{
}

HeadsUpDisplay::SimInterface::~SimInterface()
{
   QMutexLocker locker(&mMutex);

   mConnectedPlatforms.clear();
}

void HeadsUpDisplay::SimInterface::AddConnectedPlatform(const std::string& aPlatformName)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);
   mConnectedPlatforms[aPlatformName] = false;
}

void HeadsUpDisplay::SimInterface::ReleaseConnectedPlatform(const std::string& aPlatformName)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   std::map<std::string, bool>::iterator it = mConnectedPlatforms.find(aPlatformName);

   if (it != mConnectedPlatforms.end())
   {
      mConnectedPlatforms.erase(aPlatformName);
   }
}

bool HeadsUpDisplay::SimInterface::IsConnectionDestroyed(const std::string& aPlatformName)
{
   if (!IsEnabled())
   {
      return true;
   }

   QMutexLocker locker(&mMutex);

   if (mConnectedPlatforms.find(aPlatformName) != mConnectedPlatforms.end())
   {
      return mConnectedPlatforms[aPlatformName];
   }
   return true;
}

bool HeadsUpDisplay::SimInterface::HasSituationAwarenessProcessor(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   return (mPlatformsWithSAP.find(aIndex) != mPlatformsWithSAP.end());
}

void HeadsUpDisplay::SimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
{
   const WsfSA_Processor* processor = nullptr;

   for (WsfComponentList::RoleIterator<WsfProcessor> iter(aPlatform); !iter.AtEnd(); ++iter)
   {
      processor = dynamic_cast<WsfSA_Processor*>(*iter);
      if (processor != nullptr)
      {
         QMutexLocker locker(&mMutex);
         mPlatformsWithSAP.insert(aPlatform.GetIndex());
         break;
      }
   }
}

void HeadsUpDisplay::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mConnectedPlatforms.clear();
   mPlatformsWithSAP.clear();

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>());
}

void HeadsUpDisplay::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   std::map<std::string, bool>::iterator it = mConnectedPlatforms.find(aPlatform.GetName());

   if (it != mConnectedPlatforms.end())
   {
      mConnectedPlatforms.erase(aPlatform.GetName());
   }
}

void HeadsUpDisplay::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   // Use a temporary data container for the sim event below
   std::map<std::string, wkf::HUD_DataContainer::PlatformData> tempPlatDatas;
   // Setup a temporary platform data structure to load with data
   wkf::HUD_DataContainer::PlatformData tempPlatData;

   for (std::map<std::string, bool>::iterator it = mConnectedPlatforms.begin(); it != mConnectedPlatforms.end(); it++)
   {
      const std::string& aConnectedPlatform   = it->first;
      bool               aConnectionDestroyed = it->second;

      if (!aConnectedPlatform.empty() && !aConnectionDestroyed)
      {
         WsfPlatform* platform = aSimulation.GetPlatformByName(aConnectedPlatform);

         if (nullptr != platform)
         {
            // Note:  What happens if platform->IsBroken() ?
            //        Should we mark mConnectionDestroyed as true here?

            ReadPlatformData(aSimulation, platform, aSimulation.GetSimTime(), tempPlatData);
         }
      } // if (!mConnectedPlatform.empty() && !mConnectionDestroyed)

      // Set the platform data within the temporary data container data,
      // using a const reference to the temporary platform data
      const wkf::HUD_DataContainer::PlatformData& platData = tempPlatData;
      tempPlatDatas.emplace(aConnectedPlatform, platData);
   }
   // Use a temporary data container for the sim event below
   wkf::HUD_DataContainer tempData;
   tempData.SetPlatformData(tempPlatDatas);
   // Add a sim event using the temporary data container data
   AddSimEvent(ut::make_unique<UpdateEvent>(tempData));
}

void HeadsUpDisplay::SimInterface::ReadPlatformData(const WsfSimulation&                  aSimulation,
                                                    WsfPlatform*                          aPlatform,
                                                    double                                aSimTime,
                                                    wkf::HUD_DataContainer::PlatformData& aPlatformData)
{
   if (nullptr != aPlatform)
   {
      // Get the SA Processor -- if none exists, return
      WsfSA_Processor* saProc = WsfSA_Processor::GetSA_Processor(*aPlatform);
      if (saProc == nullptr)
      {
         return;
      }

      // Get the flight data
      const WsfSA_Processor::SA_AircraftKinematics& tempFlightData = saProc->GetAircraftKinematics();

      aPlatformData.simTime           = aSimTime;
      aPlatformData.lat_deg           = tempFlightData.lat_deg;
      aPlatformData.lon_deg           = tempFlightData.lon_deg;
      aPlatformData.kcas              = tempFlightData.kcas;
      aPlatformData.ktas              = tempFlightData.ktas;
      aPlatformData.mach              = tempFlightData.mach;
      aPlatformData.altitudeMSL_ft    = tempFlightData.altBaro_ft;
      aPlatformData.radarAltitude_ft  = tempFlightData.altRadar_ft;
      aPlatformData.verticalSpeed_fpm = tempFlightData.vertSpd_fpm;
      aPlatformData.alpha_deg         = tempFlightData.alpha_deg;
      aPlatformData.beta_deg          = tempFlightData.beta_deg;
      aPlatformData.gLoad             = tempFlightData.gLoad;
      aPlatformData.gAvail            = tempFlightData.gAvail;
      aPlatformData.heading_deg       = tempFlightData.heading_deg;
      aPlatformData.pitch_deg         = tempFlightData.pitch_deg;
      aPlatformData.roll_deg          = tempFlightData.roll_deg;
      aPlatformData.gX                = tempFlightData.gX;
      aPlatformData.gY                = tempFlightData.gY;
      aPlatformData.pilotType         = tempFlightData.pilotType;
      aPlatformData.afterburnerOn     = tempFlightData.afterburnerOn;
      aPlatformData.contrailing       = tempFlightData.contrailing;
      aPlatformData.stallWarning      = tempFlightData.stallWarning;
      aPlatformData.alphaValid        = tempFlightData.alphaValid;
      aPlatformData.betaValid         = tempFlightData.betaValid;
      aPlatformData.kcasValid         = tempFlightData.kcasValid;
      aPlatformData.nxValid           = tempFlightData.nxValid;
      aPlatformData.nyValid           = tempFlightData.nyValid;
      aPlatformData.nzValid           = tempFlightData.nzValid;
      aPlatformData.gAvailValid       = tempFlightData.gAvailValid;

      // Get the flight controls data
      const WsfSA_Processor::SA_FlightControlsDataSummary& tempFlightControlsData = saProc->GetFlightControlsDataSummary();

      aPlatformData.speedBrakeInput      = tempFlightControlsData.speedBrakeInput;
      aPlatformData.speedBrakeNormalized = tempFlightControlsData.speedBrakeNormalized;
      aPlatformData.throttleInput        = tempFlightControlsData.throttleInput;
      aPlatformData.thrustNormalized     = tempFlightControlsData.thrustNormalized;
      aPlatformData.throttleValid        = tempFlightControlsData.throttleValid;

      if (aPlatformData.thrustNormalized > 1.0)
      {
         aPlatformData.engineThrottleNormalizedMilitary    = 1.0;
         aPlatformData.engineThrottleNormalizedAfterburner = aPlatformData.thrustNormalized - 1.0;
      }
      else
      {
         aPlatformData.engineThrottleNormalizedMilitary    = aPlatformData.thrustNormalized;
         aPlatformData.engineThrottleNormalizedAfterburner = 0.0;
      }

      if (aPlatformData.throttleInput > 1.0)
      {
         aPlatformData.throttleMilitaryControllerPosition    = 1.0;
         aPlatformData.throttleAfterburnerControllerPosition = aPlatformData.throttleInput - 1.0;
      }
      else
      {
         aPlatformData.throttleMilitaryControllerPosition    = aPlatformData.throttleInput;
         aPlatformData.throttleAfterburnerControllerPosition = 0.0;
      }

      // Get the nav data
      const WsfSA_Processor::SA_NavigationDataSummary& tempNavData = saProc->GetNavigationDataSummary();

      aPlatformData.numWaypoints          = tempNavData.numWaypoints;
      aPlatformData.currentWaypoint       = tempNavData.currentWaypoint;
      aPlatformData.headingBug_deg        = tempNavData.headingBug_deg;
      aPlatformData.distanceToWaypoint_nm = tempNavData.distanceToWaypoint_nm;
      aPlatformData.timeToWaypoint_sec    = tempNavData.timeToWaypoint_sec;
      aPlatformData.wayptDataValid        = tempNavData.dataValid;

      for (auto& wp : tempNavData.waypoints)
      {
         wkf::HUD_DataContainer::HudWaypointData pt;
         pt.lat_deg = wp.lat_deg;
         pt.lon_deg = wp.lon_deg;
         aPlatformData.waypoints.push_back(pt);
      }

      // Get the fuel data
      const WsfSA_Processor::SA_FuelSystemData& tempFuelData = saProc->GetFuelSystemData();

      aPlatformData.currentInternalFuelQuantity_lbs = tempFuelData.fuelInternal_lbs;
      aPlatformData.currentExternalFuelQuantity_lbs = tempFuelData.fuelExternal_lbs;
      aPlatformData.currentTotalFuelQuantity_lbs    = tempFuelData.fuelInternal_lbs + tempFuelData.fuelExternal_lbs;
      aPlatformData.internalFuelCapacity_lbs        = tempFuelData.fuelCapInt_lbs;
      aPlatformData.externalFuelCapacity_lbs        = tempFuelData.fuelCapExt_lbs;
      aPlatformData.totalFuelCapacity_lbs           = tempFuelData.fuelCapInt_lbs + tempFuelData.fuelCapExt_lbs;
      aPlatformData.grossWeight_lbs                 = tempFuelData.grossWgt_lbs;
      aPlatformData.joker_lbs                       = tempFuelData.joker_lbs;
      aPlatformData.bingo_lbs                       = tempFuelData.bingo_lbs;
      aPlatformData.fuelFlow_pph                    = tempFuelData.fuelFlow_pph;
      aPlatformData.fuelValid                       = tempFuelData.fuelValid;

      if (aPlatformData.currentTotalFuelQuantity_lbs < aPlatformData.bingo_lbs)
      {
         aPlatformData.bingoFlagActive = true;
      }
      else
      {
         aPlatformData.bingoFlagActive = false;
      }

      // Get the weapons data
      const WsfSA_Processor::SA_WeaponsDataSummary& tempWeaponsData = saProc->GetWeaponsDataSummary();

      aPlatformData.selectedWeapon    = tempWeaponsData.selectedWeapon;
      aPlatformData.numSelectedWeapon = tempWeaponsData.numSelectedWeapon;

      if (!aPlatformData.selectedWeapon.empty())
      {
         aPlatformData.selectedWeaponValid = true;
      }
      else
      {
         aPlatformData.selectedWeaponValid = false;
      }

      aPlatformData.weaponBayDoorsOpen = tempWeaponsData.weaponBayDoorsOpen;
      aPlatformData.supportingWeapon   = tempWeaponsData.supportingWeapon;
      aPlatformData.masterWarning      = tempWeaponsData.masterWarning;
      aPlatformData.masterCaution      = tempWeaponsData.masterCaution;
      aPlatformData.shootCueActive     = tempWeaponsData.shootCueActive;
      aPlatformData.shootCueBlink      = tempWeaponsData.shootCueBlink;
      aPlatformData.masterArmActive    = tempWeaponsData.masterArmActive;
      aPlatformData.jammingDetected    = tempWeaponsData.jammingDetected;
      aPlatformData.numChaff           = tempWeaponsData.numChaff;
      aPlatformData.numFlares          = tempWeaponsData.numFlares;
      aPlatformData.numDecoys          = tempWeaponsData.numDecoys;

      if (!tempWeaponsData.engagedTargetList.empty() && tempWeaponsData.engagedTargetList.at(0).targetIndex > 0)
      {
         aPlatformData.engagedTargetValid = true;

         aPlatformData.engagedTargetData.targetIndex     = tempWeaponsData.engagedTargetList.at(0).targetIndex;
         aPlatformData.engagedTargetData.rangeMax_nm     = tempWeaponsData.engagedTargetList.at(0).rangeMax_nm;
         aPlatformData.engagedTargetData.rangeNe_nm      = tempWeaponsData.engagedTargetList.at(0).rangeNe_nm;
         aPlatformData.engagedTargetData.rangeMin_nm     = tempWeaponsData.engagedTargetList.at(0).rangeMin_nm;
         aPlatformData.engagedTargetData.rangeCurrent_nm = tempWeaponsData.engagedTargetList.at(0).rangeCurrent_nm;
         aPlatformData.engagedTargetData.rangeClosureRate_kts =
            tempWeaponsData.engagedTargetList.at(0).rangeClosureRate_kts;
         aPlatformData.engagedTargetData.timeToIntercept_sec = tempWeaponsData.engagedTargetList.at(0).timeToIntercept_sec;
         aPlatformData.engagedTargetData.timeToActive_sec    = tempWeaponsData.engagedTargetList.at(0).timeToActive_sec;
         aPlatformData.engagedTargetData.targetKTAS          = tempWeaponsData.engagedTargetList.at(0).targetKTAS;
         aPlatformData.engagedTargetData.targetAspect_deg    = tempWeaponsData.engagedTargetList.at(0).targetAspect_deg;
         aPlatformData.engagedTargetData.targetAz_deg        = tempWeaponsData.engagedTargetList.at(0).targetAz_deg;
         aPlatformData.engagedTargetData.targetEl_deg        = tempWeaponsData.engagedTargetList.at(0).targetEl_deg;
      }
      else
      {
         aPlatformData.engagedTargetValid = false;

         aPlatformData.engagedTargetData.targetIndex          = 0;
         aPlatformData.engagedTargetData.rangeMax_nm          = 0.0;
         aPlatformData.engagedTargetData.rangeNe_nm           = 0.0;
         aPlatformData.engagedTargetData.rangeMin_nm          = 0.0;
         aPlatformData.engagedTargetData.rangeCurrent_nm      = 0.0;
         aPlatformData.engagedTargetData.rangeClosureRate_kts = 0.0;
         aPlatformData.engagedTargetData.timeToIntercept_sec  = 0.0;
         aPlatformData.engagedTargetData.timeToActive_sec     = 0.0;
         aPlatformData.engagedTargetData.targetKTAS           = 0.0;
         aPlatformData.engagedTargetData.targetAspect_deg     = 0.0;
         aPlatformData.engagedTargetData.targetAz_deg         = 0.0;
         aPlatformData.engagedTargetData.targetEl_deg         = 0.0;
      }
   }
}
