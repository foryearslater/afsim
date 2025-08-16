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

#include "SA_DisplaySimInterface.hpp"

#include <QString>

#include "UtMemory.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfSA_Assess.hpp"
#include "WsfSA_Group.hpp"
#include "WsfSA_Perceive.hpp"
#include "WsfSA_Processor.hpp"
#include "WsfSimulation.hpp"

WkSA_Display::SimInterface::SimInterface(const QString& aPluginName)
{
   // mCallbacks.Add(PerceptionUpdated.Connect(&OnPerceptionUpdated, this));
}

void WkSA_Display::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   std::set<size_t> platformsofInterest;
   if (mMutex.try_lock())
   {
      platformsofInterest = mPlatformsOfInterest;
      mMutex.unlock();
   }

   // Get the current time.
   const double simTime = aSimulation.GetSimTime();

   // Gather truth data of every platform that is valid(not deleted/removed).
   std::vector<wkf::SA_Display::EntityTruth> tempSA_TruthPlatforms;
   size_t                                    platformCount = aSimulation.GetPlatformCount();

   // Loop through each platform.
   for (size_t platformEntry = 0; platformEntry < platformCount; ++platformEntry)
   {
      WsfPlatform* platformPtr = aSimulation.GetPlatformEntry(platformEntry);

      // If we have a valid platform, populate using WsfPlatform* data.
      // Ignore deleted platforms.
      if (platformPtr != nullptr && !platformPtr->IsDeleted())
      {
         wkf::SA_Display::EntityTruth entity;

         // Fill in the general data needed to draw here.
         double lat, lon, alt;
         platformPtr->GetLocationLLA(lat, lon, alt);
         entity.lat_deg = lat;
         entity.lon_deg = lon;

         double heading_rad, dummypitch, dummyroll;
         platformPtr->GetOrientationNED(heading_rad, dummypitch, dummyroll);
         entity.heading_deg = heading_rad * UtMath::cDEG_PER_RAD;
         entity.altitude_ft = platformPtr->GetAltitude() * UtMath::cFT_PER_M;
         entity.speed_kts   = platformPtr->GetSpeed() * UtMath::cMPS_PER_NMPH;

         entity.index = platformPtr->GetIndex();
         entity.name  = platformPtr->GetName();
         entity.type  = platformPtr->GetType();

         // Get the domain.
         switch (platformPtr->GetSpatialDomain())
         {
         default:
         case WSF_SPATIAL_DOMAIN_UNKNOWN:
            entity.domain = "unknown";
            break;
         case WSF_SPATIAL_DOMAIN_LAND:
            entity.domain = "land";
            break;
         case WSF_SPATIAL_DOMAIN_AIR:
            entity.domain = "air";
            break;
         case WSF_SPATIAL_DOMAIN_SURFACE:
            entity.domain = "surface";
            break;
         case WSF_SPATIAL_DOMAIN_SUBSURFACE:
            entity.domain = "subsurface";
            break;
         case WSF_SPATIAL_DOMAIN_SPACE:
            entity.domain = "space";
            break;
         }

         // Set missile flag to false, but it may become true below
         entity.isMissile = false;

         WsfCategoryList catList = platformPtr->GetCategories();
         if (catList.IsCategoryMember(WsfStringId("missile")))
         {
            entity.isMissile = true;
         }

         entity.altitudeValid = true;
         entity.speedValid    = true;
         entity.headingValid  = true;

         tempSA_TruthPlatforms.emplace_back(entity);
      }
   }
   // Truth data end

   // SA data start
   // Create a temp container for the SA Data
   std::map<size_t, wkf::SA_Display::SA_Data> tempSA_Data;

   // Loop through each platform of interest
   for (size_t i : platformsofInterest)
   {
      // Create a temporary data container
      wkf::SA_Display::SA_Data tempData;

      WsfPlatform* platform = aSimulation.GetPlatformByIndex(i);
      // If the platform is valid, fill in data for it
      if (platform != nullptr && !platform->IsDeleted())
      {
         tempData.platformData.platformIndex = platform->GetIndex();
         tempData.platformData.simTime       = simTime;

         // First, we get general data -- if there is other data, it will override this data
         double heading_rad, pitch_rad, roll_rad;
         platform->GetOrientationNED(heading_rad, pitch_rad, roll_rad);
         tempData.platformData.heading_deg = heading_rad * UtMath::cDEG_PER_RAD;
         tempData.platformData.pitch_deg   = pitch_rad * UtMath::cDEG_PER_RAD;
         tempData.platformData.roll_deg    = roll_rad * UtMath::cDEG_PER_RAD;
         tempData.platformData.altBaro_ft  = platform->GetAltitude() * UtMath::cFT_PER_M;

         // KTAS
         double vned[3];
         platform->GetVelocityNED(vned);
         UtVec3<double> nedSpeed    = UtVec3<double>(vned);
         tempData.platformData.ktas = nedSpeed.Magnitude() * UtMath::cNMPH_PER_MPS;

         // Most data comes from the SA Processor
         WsfSA_Processor* saProc = WsfSA_Processor::GetSA_Processor(*platform);

         // If the platform has an SA processor, fill in SA data
         if (saProc != nullptr)
         {
            // Update kinematics/platform data
            const WsfSA_Processor::SA_AircraftKinematics& kinematics = saProc->GetAircraftKinematics();
            tempData.platformData.flightId                           = saProc->FlightId();
            tempData.platformData.idFlag                             = saProc->IdFlag();
            tempData.platformData.lat_deg                            = kinematics.lat_deg;
            tempData.platformData.lon_deg                            = kinematics.lon_deg;
            tempData.platformData.kcas                               = kinematics.kcas;
            tempData.platformData.ktas                               = kinematics.ktas;
            tempData.platformData.mach                               = kinematics.mach;
            tempData.platformData.altBaro_ft                         = kinematics.altBaro_ft;
            tempData.platformData.altRadar_ft                        = kinematics.altRadar_ft;
            tempData.platformData.vertSpd_fpm                        = kinematics.vertSpd_fpm;
            tempData.platformData.alpha_deg                          = kinematics.alpha_deg;
            tempData.platformData.beta_deg                           = kinematics.beta_deg;
            tempData.platformData.gLoad                              = kinematics.gLoad;
            tempData.platformData.gAvail                             = kinematics.gAvail;
            tempData.platformData.heading_deg                        = kinematics.heading_deg;
            tempData.platformData.pitch_deg                          = kinematics.pitch_deg;
            tempData.platformData.roll_deg                           = kinematics.roll_deg;
            tempData.platformData.gX                                 = kinematics.gX;
            tempData.platformData.gY                                 = kinematics.gY;
            tempData.platformData.pilotType                          = kinematics.pilotType;
            tempData.platformData.afterburnerOn                      = kinematics.afterburnerOn;
            tempData.platformData.contrailing                        = kinematics.contrailing;
            tempData.platformData.stallWarning                       = kinematics.stallWarning;
            tempData.platformData.alphaValid                         = kinematics.alphaValid;
            tempData.platformData.betaValid                          = kinematics.betaValid;
            tempData.platformData.kcasValid                          = kinematics.kcasValid;
            tempData.platformData.nxValid                            = kinematics.nxValid;
            tempData.platformData.nyValid                            = kinematics.nyValid;
            tempData.platformData.nzValid                            = kinematics.nzValid;
            tempData.platformData.gAvailValid                        = kinematics.gAvailValid;

            // Update fuel system data
            const WsfSA_Processor::SA_FuelSystemData& fuel = saProc->GetFuelSystemData();
            tempData.fuelData.platformIndex                = platform->GetIndex();
            tempData.fuelData.fuelInternal_lbs             = fuel.fuelInternal_lbs;
            tempData.fuelData.fuelExternal_lbs             = fuel.fuelExternal_lbs;
            tempData.fuelData.fuelCapInt_lbs               = fuel.fuelCapInt_lbs;
            tempData.fuelData.fuelCapExt_lbs               = fuel.fuelCapExt_lbs;
            tempData.fuelData.grossWgt_lbs                 = fuel.grossWgt_lbs;
            tempData.fuelData.joker_lbs                    = fuel.joker_lbs;
            tempData.fuelData.bingo_lbs                    = fuel.bingo_lbs;
            tempData.fuelData.fuelFlow_pph                 = fuel.fuelFlow_pph;
            tempData.fuelData.normalizedFuelQty            = fuel.normalizedFuelQty;
            tempData.fuelData.numExtTanks                  = fuel.numExtTanks;
            tempData.fuelData.dropTanksPresent             = fuel.dropTanksPresent;
            tempData.fuelData.fuelValid                    = fuel.fuelValid;

            // Update nav system data
            const WsfSA_Processor::SA_NavigationDataSummary& nav = saProc->GetNavigationDataSummary();
            tempData.navData.platformIndex                       = platform->GetIndex();

            if (nav.dataValid)
            {
               tempData.navData.wayptDataValid                     = true;
               tempData.navData.waypointData.numWaypoints          = nav.numWaypoints;
               tempData.navData.waypointData.currentWaypoint       = nav.currentWaypoint;
               tempData.navData.waypointData.headingBug_deg        = nav.headingBug_deg;
               tempData.navData.waypointData.distanceToWaypoint_nm = nav.distanceToWaypoint_nm;
               tempData.navData.waypointData.timeToWaypoint_sec    = nav.timeToWaypoint_sec;

               for (auto& wp : nav.waypoints)
               {
                  wkf::SA_Display::WaypointLatLon wayPt;
                  wayPt.lat_deg = wp.lat_deg;
                  wayPt.lon_deg = wp.lon_deg;
                  tempData.navData.waypointData.waypoints.push_back(wayPt);
               }
            }
            else
            {
               tempData.navData.wayptDataValid                     = false;
               tempData.navData.waypointData.numWaypoints          = 0;
               tempData.navData.waypointData.currentWaypoint       = 0;
               tempData.navData.waypointData.headingBug_deg        = 0.0;
               tempData.navData.waypointData.distanceToWaypoint_nm = 0.0;
               tempData.navData.waypointData.timeToWaypoint_sec    = 0.0;
            }

            // Update weapons data
            const WsfSA_Processor::SA_WeaponsDataSummary& weapons = saProc->GetWeaponsDataSummary();
            tempData.weaponData.platformIndex                     = platform->GetIndex();
            tempData.weaponData.selectedWeapon                    = weapons.selectedWeapon;
            tempData.weaponData.numSelectedWeapon                 = weapons.numSelectedWeapon;
            tempData.weaponData.numChaff                          = weapons.numChaff;
            tempData.weaponData.numFlares                         = weapons.numFlares;
            tempData.weaponData.numDecoys                         = weapons.numDecoys;
            tempData.weaponData.weaponBayDoorsOpen                = weapons.weaponBayDoorsOpen;
            tempData.weaponData.supportingWeapon                  = weapons.supportingWeapon;
            tempData.weaponData.masterWarning                     = weapons.masterWarning;
            tempData.weaponData.masterCaution                     = weapons.masterCaution;
            tempData.weaponData.shootCueActive                    = weapons.shootCueActive;
            tempData.weaponData.shootCueBlink                     = weapons.shootCueBlink;
            tempData.weaponData.masterArmActive                   = weapons.masterArmActive;
            tempData.weaponData.jammingDetected                   = weapons.jammingDetected;

            for (auto& weapon : weapons.weaponList)
            {
               wkf::SA_Display::WeaponNameQty wpn;
               wpn.weaponName = weapon.weaponName;
               wpn.quantity   = weapon.quantity;

               tempData.weaponData.weaponList.push_back(wpn);
            }

            for (auto& target : weapons.engagedTargetList)
            {
               wkf::SA_Display::EngagedTargetData tgt;
               tgt.targetIndex          = static_cast<size_t>(target.targetIndex);
               tgt.rangeMax_nm          = target.rangeMax_nm;
               tgt.rangeNe_nm           = target.rangeNe_nm;
               tgt.rangeMin_nm          = target.rangeMin_nm;
               tgt.rangeCurrent_nm      = target.rangeCurrent_nm;
               tgt.rangeClosureRate_kts = target.rangeClosureRate_kts;
               tgt.timeToIntercept_sec  = target.timeToIntercept_sec;
               tgt.timeToActive_sec     = target.timeToActive_sec;
               tgt.targetKTAS           = target.targetKTAS;
               tgt.targetAspect_deg     = target.targetAspect_deg;
               tgt.targetAz_deg         = target.targetAz_deg;
               tgt.targetEl_deg         = target.targetEl_deg;

               tempData.weaponData.engagedTargetList.push_back(tgt);
            }

            // Update tracks data
            const std::vector<::WsfSA_Processor::SA_TrackManagerData>& trackProcessors = saProc->GetTrackManagerList();

            // Get data for the (first) master track processor
            for (auto& trkProc : trackProcessors)
            {
               if (trkProc.isMasterProcessor)
               {
                  for (auto& trk : trkProc.trackList)
                  {
                     wkf::SA_Display::TargetTrackItem track;

                     track.tgtPlatformIndex = trk.targetPlatformIndex;
                     track.tgtLat_deg       = trk.lat_deg;
                     track.tgtLon_deg       = trk.lon_deg;
                     track.tgtAlt_ft        = trk.altitude_ft;
                     track.tgtBearing_rad   = trk.bearing_deg * UtMath::cRAD_PER_DEG;
                     track.tgtElevation_rad = trk.elevation_deg * UtMath::cRAD_PER_DEG;
                     track.tgtRange_nm      = trk.range_nm;
                     track.tgtRangeRate_kts = trk.rangeRate_kts;
                     track.tgtSpeed_kts     = trk.targetSpeed_kts;
                     track.tgtHdg_deg       = trk.targetHdg_deg;
                     track.tgtAspect_deg    = trk.targetAspect_deg;
                     track.tgtIdString      = trk.idString;
                     track.tgtPlatformName  = trk.tgtPlatformName;
                     track.tgtTypeName      = trk.tgtTypeName;
                     track.identification =
                        static_cast<wkf::SA_Display::Identification>(static_cast<int>(trk.identification));
                     track.latLonValid       = trk.latLonValid;
                     track.altitudeValid     = trk.altitudeValid;
                     track.bearingValid      = trk.bearingValid;
                     track.elevationValid    = trk.elevationValid;
                     track.rangeValid        = trk.rangeValid;
                     track.rangeRateValid    = trk.rangeRateValid;
                     track.targetSpeedValid  = trk.targetSpeedValid;
                     track.targetHdgValid    = trk.targetHdgValid;
                     track.targetAspectValid = trk.targetAspectValid;
                     track.idIffValid        = trk.idIffValid;
                     track.idAuxValid        = trk.idAuxValid;
                     track.idSideValid       = trk.idSideValid;
                     track.idTypeValid       = trk.idTypeValid;
                     track.isMissile         = trk.isMissile;

                     // Add to the list of tracks for this track processor
                     tempData.trackList.emplace_back(track);
                  }

                  // We found our master track processor, break out of the loop
                  break;
               }
            }

            // Update assets data
            std::vector<WsfSA_EntityPerception*> assets = saProc->Perceive()->PerceivedAssets(aSimulation.GetSimTime());
            for (auto& asset : assets)
            {
               wkf::SA_Display::EntityPerception entity;
               PopulateEntityPerception(entity, *asset);

               // Add to the list
               tempData.assetsList.emplace_back(entity);
            }

            // Update bogies data
            std::vector<WsfSA_EntityPerception*> bogies = saProc->Perceive()->PerceivedBogies(aSimulation.GetSimTime());
            for (auto& bogie : bogies)
            {
               wkf::SA_Display::EntityPerception entity;
               PopulateEntityPerception(entity, *bogie);

               // Add to the list
               tempData.bogiesList.emplace_back(entity);
            }

            // Update bandits data
            std::vector<WsfSA_EntityPerception*> bandits = saProc->Perceive()->PerceivedBandits(aSimulation.GetSimTime());
            for (auto& bandit : bandits)
            {
               wkf::SA_Display::EntityPerception entity;
               PopulateEntityPerception(entity, *bandit);

               // Add to the list
               tempData.banditsList.emplace_back(entity);
            }

            // Update prioritized threats data
            const auto& prioritizedThreats = saProc->Assess()->GetPrioritizedThreatEntities();
            for (auto& threat : prioritizedThreats)
            {
               if (threat.IsValid())
               {
                  wkf::SA_Display::EntityPerception entity;
                  PopulateEntityPerception(entity, *threat.Get());

                  // Add to the list
                  tempData.prioritizedThreatsList.emplace_back(entity);
               }
            }

            // Update prioritized targets data
            const auto& prioritizedTargets = saProc->Assess()->GetPrioritizedTargetEntities();
            for (auto& target : prioritizedTargets)
            {
               if (target.IsValid())
               {
                  wkf::SA_Display::EntityPerception entity;
                  PopulateEntityPerception(entity, *target.Get());

                  // Add to the list
                  tempData.prioritizedTargetsList.emplace_back(entity);
               }
            }

            // Update groups data
            const std::list<WsfSA_Group*> groups = saProc->Assess()->PerceivedGroups();
            for (auto group : groups)
            {
               wkf::SA_Display::PerceivedGroup perceivedGroup;

               perceivedGroup.groupName           = group->GetPerceivedName();
               perceivedGroup.centroidLat_deg     = group->GetLat_deg();
               perceivedGroup.centroidLon_deg     = group->GetLon_deg();
               perceivedGroup.centroidAlt_ft      = group->GetAltitude_ft();
               perceivedGroup.centroidHeading_deg = group->GetHeading_deg();
               perceivedGroup.centroidSpeed_kts   = group->GetSpeed_kts();
               perceivedGroup.radiusCurrent_nm    = group->GetRadiusCurrent_nm();
               perceivedGroup.radiusMinimum_nm    = group->GetRadiusMinimum_nm();
               perceivedGroup.simtimeAtCoast_sec  = 0.0;
               perceivedGroup.maxCoastingTime_sec = 0.0;

               for (auto& target : group->GetElementList())
               {
                  wkf::SA_Display::EntityPerception entity;
                  PopulateEntityPerception(entity, *target);

                  // Add to the list
                  perceivedGroup.elementList.emplace_back(entity);
               }

               // Add to the list
               tempData.groupsList.emplace_back(perceivedGroup);
            }

            // Add this platform's temp data to the data container
            tempSA_Data.emplace(i, tempData);
         } // saProc
      }    // platform != nullptr && !platform->IsDeleted()
   }       // for (size_t i : mPlatformsOfInterest)

   // If we have container data, fill it in
   if (tempSA_Data.size() > 0 || tempSA_TruthPlatforms.size() > 0)
   {
      wkf::SA_Display::DataContainer tempDataContainer;
      tempDataContainer.SetSA_Data(tempSA_Data);
      tempDataContainer.SetSA_TruthPlatforms(tempSA_TruthPlatforms);
      AddSimEvent(ut::make_unique<UpdateSA_DataEvent>(tempDataContainer));
   }
}

void WkSA_Display::SimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
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

bool WkSA_Display::SimInterface::HasSituationAwarenessProcessor(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   return (mPlatformsWithSAP.find(aIndex) != mPlatformsWithSAP.end());
}

void WkSA_Display::SimInterface::AddPlatformOfInterest(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   mPlatformsOfInterest.insert(aIndex);
}

void WkSA_Display::SimInterface::RemovePlatformOfInterest(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   mPlatformsOfInterest.erase(aIndex);
}

void WkSA_Display::SimInterface::PopulateEntityPerception(wkf::SA_Display::EntityPerception& aEntityPerception,
                                                          const WsfSA_EntityPerception&      aData)
{
   aEntityPerception.lat_deg        = aData.GetLat_deg();
   aEntityPerception.lon_deg        = aData.GetLon_deg();
   aEntityPerception.altitude_ft    = aData.GetAltitude_ft();
   aEntityPerception.bearing_deg    = aData.GetBearing_deg();
   aEntityPerception.speed_kts      = aData.GetSpeed_kts();
   aEntityPerception.heading_deg    = aData.GetHeading_deg();
   aEntityPerception.threatLevel    = aData.GetThreatLevel();
   aEntityPerception.targetValue    = aData.GetTargetValue();
   aEntityPerception.risk           = aData.GetRisk();
   aEntityPerception.defensiveness  = aData.GetDefensiveness();
   aEntityPerception.urgency        = aData.GetUrgency();
   aEntityPerception.idString       = aData.GetIdString();
   aEntityPerception.perceivedIndex = aData.GetPerceivedIndex();
   aEntityPerception.perceivedName  = aData.GetPerceivedName();
   aEntityPerception.perceivedType  = aData.GetPerceivedType();
   aEntityPerception.flightId       = static_cast<unsigned short>(aData.GetFlightId());
   aEntityPerception.idFlag         = aData.GetIdFlag();
   aEntityPerception.identification = static_cast<wkf::SA_Display::Identification>(aData.GetIdentification());
   aEntityPerception.friendlyAsset  = aData.GetFriendlyAsset();
   aEntityPerception.angleOnly      = aData.GetAngleOnly();
   aEntityPerception.idIffFriend    = aData.GetIdIffFriend();
   aEntityPerception.idIffFoe       = aData.GetIdIffFoe();
   aEntityPerception.idIffNeutral   = aData.GetIdIffNeutral();
   aEntityPerception.idIffUnknown   = aData.GetIdIffUnknown();
   aEntityPerception.idAuxFriend    = aData.GetIdAuxFriend();
   aEntityPerception.idAuxFoe       = aData.GetIdAuxFoe();
   aEntityPerception.idAuxNeutral   = aData.GetIdAuxNeutral();
   aEntityPerception.idAuxUnknown   = aData.GetIdAuxUnknown();
   aEntityPerception.idSideFriend   = aData.GetIdSideFriend();
   aEntityPerception.idSideFoe      = aData.GetIdSideFoe();
   aEntityPerception.idSideNeutral  = aData.GetIdSideNeutral();
   aEntityPerception.idSideUnknown  = aData.GetIdSideUnknown();
   aEntityPerception.idTypeFriend   = aData.GetIdTypeFriend();
   aEntityPerception.idTypeFoe      = aData.GetIdTypeFoe();
   aEntityPerception.idTypeNeutral  = aData.GetIdTypeNeutral();
   aEntityPerception.idTypeUnknown  = aData.GetIdTypeUnknown();
   aEntityPerception.isHostile      = aData.GetIsHostile();
   aEntityPerception.altitudeValid  = aData.GetAltitudeValid();
   aEntityPerception.speedValid     = aData.GetSpeedValid();
   aEntityPerception.headingValid   = aData.GetHeadingValid();
   aEntityPerception.isMissile      = aData.GetIsMissile();
}

void WkSA_Display::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mPlatformsOfInterest.clear();
   mPlatformsWithSAP.clear();

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>());
}

void WkSA_Display::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   if (mPlatformsOfInterest.find(aPlatform.GetIndex()) != mPlatformsOfInterest.end())
   {
      mPlatformsOfInterest.erase(aPlatform.GetIndex());
   }
}

bool WkSA_Display::SimInterface::IsConnectionDestroyed(const size_t aPlatformIndex)
{
   if (!IsEnabled())
   {
      return true;
   }

   QMutexLocker locker(&mMutex);

   if (mPlatformsOfInterest.find(aPlatformIndex) != mPlatformsOfInterest.end())
   {
      return false;
   }
   return true;
}
