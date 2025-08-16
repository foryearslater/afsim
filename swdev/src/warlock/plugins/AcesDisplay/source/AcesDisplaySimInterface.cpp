// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AcesDisplaySimInterface.hpp"

#include <QString>

#include "UtMemory.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfSA_Assess.hpp"
#include "WsfSA_Group.hpp"
#include "WsfSA_Perceive.hpp"
#include "WsfSA_Processor.hpp"
#include "WsfSimulation.hpp"

WkAcesDisplay::SimInterface::SimInterface(const QString& aPluginName) {}

void WkAcesDisplay::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   std::set<size_t> platformsOfInterest;
   if (mMutex.try_lock())
   {
      platformsOfInterest = mPlatformsOfInterest;
      mMutex.unlock();
   }

   if (!platformsOfInterest.empty())
   {
      // Get the current time.
      const double simTime = aSimulation.GetSimTime();

      // Create a temp container for the Aces Data
      std::map<size_t, wkf::AcesDisplay::AcesData> tempAcesData;

      // Loop through each platform of interest
      for (const size_t i : platformsOfInterest)
      {
         // Create a temporary data container
         wkf::AcesDisplay::AcesData tempData;

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

            // Try to find an ABT for this platform to populate behavior history if possible
            WsfScriptProcessor*      processorPtr = nullptr;
            WsfAdvancedBehaviorTree* treePtr      = nullptr;
            for (WsfComponentList::RoleIterator<WsfProcessor> iter(*platform); !iter.AtEnd(); ++iter)
            {
               processorPtr = dynamic_cast<WsfScriptProcessor*>(*iter);
               if (processorPtr->AdvancedBehaviorTree() != nullptr)
               {
                  treePtr = processorPtr->AdvancedBehaviorTree();
                  break;
               }
            }

            // If we found an ABT to pull data from
            if (treePtr != nullptr)
            {
               std::vector<std::string>                  behaviorHistory;
               std::vector<WsfAdvancedBehaviorTreeNode*> lastExecuted;
               treePtr->FindLastExecuted(lastExecuted);
               for (const auto& node : lastExecuted)
               {
                  behaviorHistory.push_back(node->GetName());
                  if (behaviorHistory.size() >= wkf::AcesDisplay::cBEHAVIOR_HISTORY_COUNT)
                  {
                     break;
                  }
               }
               tempData.behaviorHistory = behaviorHistory;
            }

            // Most data comes from the SA Processor
            WsfSA_Processor* saProc = WsfSA_Processor::GetSA_Processor(*platform);

            // If the platform has an SA processor, fill in ACES data
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

                  for (const auto& wp : nav.waypoints)
                  {
                     wkf::AcesDisplay::WaypointLatLon wayPt;
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

               for (const auto& weapon : weapons.weaponList)
               {
                  wkf::AcesDisplay::WeaponNameQty wpn;
                  wpn.weaponName = weapon.weaponName;
                  wpn.quantity   = weapon.quantity;

                  tempData.weaponData.weaponList.push_back(wpn);
               }

               for (const auto& target : weapons.engagedTargetList)
               {
                  wkf::AcesDisplay::EngagedTargetData tgt;
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

               // Update engagement summary data
               // Populate the source platform data
               wkf::AcesDisplay::EngagementData sourcePlatformData;
               PopulateEngagementData(aSimulation, sourcePlatformData, *platform);
               tempData.engagementData.emplace(platform->GetIndex(), sourcePlatformData);

               // Get the indices to gather info for from each targets list of targets
               std::set<size_t> targetPlatforms;
               for (const auto& target : mTargetPlatforms)
               {
                  for (const auto& targetIndex : target.second)
                  {
                     targetPlatforms.emplace(targetIndex);
                  }
               }

               // Target platforms
               for (const auto& targetIndex : targetPlatforms)
               {
                  WsfPlatform* wsfTarget = aSimulation.GetPlatformByIndex(targetIndex);
                  if (wsfTarget != nullptr)
                  {
                     wkf::AcesDisplay::EngagementData targetData;
                     PopulateEngagementData(aSimulation, targetData, *wsfTarget);
                     tempData.engagementData.emplace(targetIndex, targetData);
                  }
               }

               // Prioritized Targets
               for (const auto& targetName : sourcePlatformData.mPrioritizedTargetNames)
               {
                  WsfPlatform* target = aSimulation.GetPlatformByName(targetName);
                  if (target != nullptr)
                  {
                     wkf::AcesDisplay::EngagementData targetData;
                     PopulateEngagementData(aSimulation, targetData, *target);
                     tempData.engagementData.emplace(target->GetIndex(), targetData);
                  }
               }

               // Prioritized Threats
               for (const auto& threatName : sourcePlatformData.mPrioritizedThreatNames)
               {
                  WsfPlatform* threat = aSimulation.GetPlatformByName(threatName);
                  if (threat != nullptr)
                  {
                     wkf::AcesDisplay::EngagementData threatData;
                     PopulateEngagementData(aSimulation, threatData, *threat);
                     tempData.engagementData.emplace(threat->GetIndex(), threatData);
                  }
               }

               // Update tracks data
               const std::vector<::WsfSA_Processor::SA_TrackManagerData>& trackProcessors = saProc->GetTrackManagerList();

               for (const auto& trkProc : trackProcessors)
               {
                  wkf::AcesDisplay::TrackProcessor proc;
                  proc.isMasterProcessor  = trkProc.isMasterProcessor;
                  proc.trackProcessorName = trkProc.name;

                  for (const auto& trk : trkProc.trackList)
                  {
                     wkf::AcesDisplay::TargetTrackItem track;

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
                        static_cast<wkf::AcesDisplay::Identification>(static_cast<int>(trk.identification));
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
                     proc.trackList.mTracks.emplace(track.tgtPlatformIndex, track);
                  }

                  // Add the track processor to the list
                  tempData.trackProcessorList.emplace_back(proc);
               }

               tempData.esmTrackprocessor   = saProc->ESM_TrackProcessor();
               tempData.mwsTrackprocessor   = saProc->MWS_TrackProcessor();
               tempData.radarTrackprocessor = saProc->RadarTrackProcessor();
               tempData.irstTrackprocessor  = saProc->IRST_TrackProcessor();
               tempData.dasTrackprocessor   = saProc->DAS_TrackProcessor();
               tempData.flirTrackprocessor  = saProc->FLIR_TrackProcessor();
               tempData.eyesTrackprocessor  = saProc->EyesTrackProcessor();

               // Get data for the (first) master track processor
               for (const auto& trkProc : trackProcessors)
               {
                  if (trkProc.isMasterProcessor)
                  {
                     for (auto& trk : trkProc.trackList)
                     {
                        wkf::AcesDisplay::TargetTrackItem track;

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
                           static_cast<wkf::AcesDisplay::Identification>(static_cast<int>(trk.identification));
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
                        tempData.masterTrackListData.mTracks.emplace(track.tgtPlatformIndex, track);
                     }

                     // We found our master track processor, break out of the loop
                     break;
                  }
               }

               // Update assets data
               std::vector<WsfSA_EntityPerception*> assets = saProc->Perceive()->PerceivedAssets(aSimulation.GetSimTime());
               for (const auto& asset : assets)
               {
                  wkf::AcesDisplay::EntityPerception entity;
                  PopulateEntityPerception(entity, *asset);

                  // Add to the list
                  tempData.assetsList.emplace_back(entity);
               }

               // Update bogies data
               std::vector<WsfSA_EntityPerception*> bogies = saProc->Perceive()->PerceivedBogies(aSimulation.GetSimTime());
               for (const auto& bogie : bogies)
               {
                  wkf::AcesDisplay::EntityPerception entity;
                  PopulateEntityPerception(entity, *bogie);

                  // Add to the list
                  tempData.bogiesList.emplace_back(entity);
               }

               // Update bandits data
               std::vector<WsfSA_EntityPerception*> bandits =
                  saProc->Perceive()->PerceivedBandits(aSimulation.GetSimTime());
               for (const auto& bandit : bandits)
               {
                  wkf::AcesDisplay::EntityPerception entity;
                  PopulateEntityPerception(entity, *bandit);

                  // Add to the list
                  tempData.banditsList.emplace_back(entity);
               }

               // Update prioritized threats data
               const auto& prioritizedThreats = saProc->Assess()->GetPrioritizedThreatEntities();
               for (const auto& threat : prioritizedThreats)
               {
                  if (threat.IsValid())
                  {
                     wkf::AcesDisplay::EntityPerception entity;
                     PopulateEntityPerception(entity, *threat.Get());

                     // Add to the list
                     tempData.prioritizedThreatsList.emplace_back(entity);
                  }
               }

               // Update prioritized targets data
               const auto& prioritizedTargets = saProc->Assess()->GetPrioritizedTargetEntities();
               for (const auto& target : prioritizedTargets)
               {
                  if (target.IsValid())
                  {
                     wkf::AcesDisplay::EntityPerception entity;
                     PopulateEntityPerception(entity, *target.Get());

                     // Add to the list
                     tempData.prioritizedTargetsList.emplace_back(entity);
                  }
               }

               // Update groups data
               const std::list<WsfSA_Group*> groups = saProc->Assess()->PerceivedGroups();
               for (const auto& group : groups)
               {
                  wkf::AcesDisplay::PerceivedGroup perceivedGroup;

                  perceivedGroup.groupName           = group->GetPerceivedName();
                  perceivedGroup.centroidLat_deg     = group->GetLat_deg();
                  perceivedGroup.centroidLon_deg     = group->GetLon_deg();
                  perceivedGroup.centroidAlt_ft      = group->GetAltitude_ft();
                  perceivedGroup.centroidHeading_deg = group->GetHeading_deg();
                  perceivedGroup.centroidSpeed_kts   = group->GetSpeed_kts();
                  perceivedGroup.radiusCurrent_nm    = group->GetRadiusCurrent_nm();
                  perceivedGroup.radiusMinimum_nm    = group->GetRadiusMinimum_nm();

                  for (const auto& target : group->GetElementList())
                  {
                     wkf::AcesDisplay::EntityPerception entity;
                     PopulateEntityPerception(entity, *target);

                     // Add to the list
                     perceivedGroup.elementList.emplace_back(entity);
                  }

                  perceivedGroup.focus       = group->GetFocus();
                  perceivedGroup.isImportant = group->GetImportance() == WsfSA_PerceivedItem::IMPORTANT;

                  // Add to the list
                  tempData.groupsList.emplace_back(perceivedGroup);
               }

               // Add this platform's temp data to the data container
               tempAcesData.emplace(i, tempData);
            } // saProc
         }    // platform != nullptr && !platform->IsDeleted()
      }       // for (size_t i : mPlatformsOfInterest)

      // If we have container data, fill it in
      if (tempAcesData.size() > 0)
      {
         AddSimEvent(ut::make_unique<UpdateAcesDataEvent>(tempAcesData));
      }
   }
}

void WkAcesDisplay::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform));

   const WsfSA_Processor* processor = nullptr;

   for (WsfComponentList::RoleIterator<WsfProcessor> iter(aPlatform); !iter.AtEnd(); ++iter)
   {
      processor = dynamic_cast<WsfSA_Processor*>(*iter);
      if (processor != nullptr)
      {
         AddSimEvent(ut::make_unique<PlatformWithSAP_AddedEvent>(aPlatform.GetIndex()));
         break;
      }
   }
}

void WkAcesDisplay::SimInterface::AddPlatformOfInterest(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   mPlatformsOfInterest.insert(aIndex);
}

void WkAcesDisplay::SimInterface::RemovePlatformOfInterest(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   mPlatformsOfInterest.erase(aIndex);
}

void WkAcesDisplay::SimInterface::PopulateEntityPerception(wkf::AcesDisplay::EntityPerception& aEntityPerception,
                                                           const WsfSA_EntityPerception&       aData)
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
   aEntityPerception.identification = static_cast<wkf::AcesDisplay::Identification>(aData.GetIdentification());
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
   aEntityPerception.focus          = aData.GetParentGroup() ? aData.GetParentGroup()->GetFocus() : true;
   aEntityPerception.isCoasting     = aData.GetIsCoasting();
   aEntityPerception.isImportant    = aData.GetImportance() == WsfSA_PerceivedItem::IMPORTANT;
}

void WkAcesDisplay::SimInterface::PopulateEngagementData(const WsfSimulation&              aSimulation,
                                                         wkf::AcesDisplay::EngagementData& aData,
                                                         WsfPlatform&                      aPlatform)
{
   // Most data comes from the SA Processor
   WsfSA_Processor* saProc = WsfSA_Processor::GetSA_Processor(aPlatform);

   if (saProc != nullptr)
   {
      // Get the list of prioritized target names
      std::vector<std::string> prioritizedTargetList = saProc->Assess()->PrioritizedTargetNames();

      // Next, add each prioritized target to the sim interface's list, as well as the data list.
      for (const auto& target : prioritizedTargetList)
      {
         aData.mPrioritizedTargetNames.emplace_back(target);
      }

      // Get the list of prioritized threat names.
      std::vector<std::string> prioritizedThreatList = saProc->Assess()->PrioritizedThreatNames();

      // Next, add each prioritized threat to the sim interface's list, as well as the data list.
      for (const auto& threat : prioritizedThreatList)
      {
         aData.mPrioritizedThreatNames.emplace_back(threat);
      }

      // Get the list of tracked platform names
      std::vector<std::string> trackedPlatformsList = saProc->Assess()->CurrentlyTrackedPlatformNames();
      for (const auto& threat : trackedPlatformsList)
      {
         aData.mTrackedPlatformNames.emplace_back(threat);
      }

      // Loop through filtered targets
      const std::vector<size_t> platformIdList = saProc->GetFilteredPlatformIdList();
      for (const auto& platformId : platformIdList)
      {
         // Get the current platform in the loop
         WsfPlatform* tmpPlatform = aSimulation.GetPlatformByIndex(platformId);

         if (tmpPlatform != nullptr)
         {
            size_t tmpPlatformIndex = tmpPlatform->GetIndex();

            if (tmpPlatformIndex != aPlatform.GetIndex()) // Be sure to exclude "self"
            {
               if (tmpPlatform->GetSideId() != aPlatform.GetSideId()) // Also exclude friendly forces
               {
                  aData.mAngleOffMap_deg.emplace(tmpPlatform->GetIndex(),
                                                 saProc->Assess()->AngleOffTarget2D(*tmpPlatform) * UtMath::cDEG_PER_RAD);
                  aData.mAspectAngleMap_deg.emplace(tmpPlatform->GetIndex(),
                                                    saProc->Assess()->AspectAngleForTarget2D(*tmpPlatform) *
                                                       UtMath::cDEG_PER_RAD);

                  double currentRange_nm = saProc->Assess()->CalcSlantRange(*tmpPlatform) * UtMath::cNM_PER_M;
                  aData.mCurrentRangeMap_nm.emplace(tmpPlatform->GetIndex(), currentRange_nm);

                  aData.mCurrentDeltaAltMap_ft.emplace(tmpPlatform->GetIndex(),
                                                       saProc->Assess()->CalcDeltaAltitude(*tmpPlatform) *
                                                          UtMath::cFT_PER_M);

                  // This will be added in the future: aData.mTrackQuality();
                  // This will be added in the future: aData.mTargetCID();
                  // This will be added in the future: aData.mThreatLevel();
                  // This will be added in the future: aData.mTargetValue();

                  aData.mRawDetectionAgainstTargetMap_nm.emplace(
                     tmpPlatform->GetIndex(),
                     saProc->Assess()->ExpectedDetectionRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);

                  double wezDesiredRange_nm =
                     saProc->Assess()->ExpectedWezRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M;
                  aData.mRawWEZAgainstTargetMap_nm.emplace(tmpPlatform->GetIndex(), wezDesiredRange_nm);

                  aData.mWEZMinRangeMap_nm.emplace(
                     tmpPlatform->GetIndex(),
                     saProc->Assess()->ExpectedMinimumEngagementRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);
                  aData.mWEZMaxRangeMap_nm.emplace(
                     tmpPlatform->GetIndex(),
                     saProc->Assess()->ExpectedMaximumEngagementRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);
                  aData.mWEZNoEscapeMap_nm.emplace(tmpPlatform->GetIndex(),
                                                   saProc->Assess()->ExpectedNoEscapeRangeAgainstTarget(*tmpPlatform) *
                                                      UtMath::cNM_PER_M);

                  // This will be added in the future: aData.mRisk();
                  // This will be added in the future: aData.mDefensiveness();
                  // This will be added in the future: aData.mUrgency();

                  if (saProc->Assess()->IsCurrentlyBeingTracked(platformId))
                  {
                     aData.mTargetLocked.emplace(tmpPlatform->GetIndex(), true);
                  }
                  else
                  {
                     aData.mTargetLocked.emplace(tmpPlatform->GetIndex(), false);
                  }

                  if (currentRange_nm < wezDesiredRange_nm)
                  {
                     aData.mAcceptableWEZ.emplace(tmpPlatform->GetIndex(), true);
                  }
                  else
                  {
                     aData.mAcceptableWEZ.emplace(tmpPlatform->GetIndex(), false);
                  }
               }
            }
         }
      }
   }
}

void WkAcesDisplay::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);

   mPlatformsOfInterest.clear();
   mCallbacks.Clear();
   mTargetPlatforms.clear();

   mCallbacks.Add(
      WsfObserver::Comment(&aSimulation)
         .Connect([this](double aSimTime, WsfPlatform* aPlatformPtr, const std::string& aMessage)
                  { AddSimEvent(ut::make_unique<CommentAddEvent>(aSimTime, aPlatformPtr->GetIndex(), aMessage)); }));

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>());
}

void WkAcesDisplay::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!IsEnabled())
   {
      return;
   }

   AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform));

   QMutexLocker locker(&mMutex);

   if (mPlatformsOfInterest.find(aPlatform.GetIndex()) != mPlatformsOfInterest.end())
   {
      mPlatformsOfInterest.erase(aPlatform.GetIndex());
   }
}

bool WkAcesDisplay::SimInterface::IsConnectionDestroyed(const size_t aPlatformIndex)
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

void WkAcesDisplay::SimInterface::SetTargetPlatforms(const std::map<size_t, std::vector<size_t>>& aPlatforms)
{
   QMutexLocker locker(&mMutex);

   mTargetPlatforms = aPlatforms;
}
