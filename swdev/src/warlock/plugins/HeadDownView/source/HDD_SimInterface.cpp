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

#include "HDD_SimInterface.hpp"

#include <QTime>

#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WsfAttributeContainer.hpp"
#include "WsfFieldOfView.hpp"
#include "WsfMoverObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfSA_Perceive.hpp"
#include "WsfSA_Processor.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackManager.hpp"


HDD::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<HDDEvent>(aPluginName)
   , mConnectedPlatforms()
{
}

HDD::SimInterface::~SimInterface()
{
   mConnectedPlatforms.clear();
}

void HDD::SimInterface::AddConnectedPlatform(const QString& aPlatformName)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);

   mConnectedPlatforms.emplace(aPlatformName);
}

void HDD::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);

   QString platformName = QString::fromStdString(aPlatform.GetName());

   mConnectedPlatforms.erase(platformName);
}

void HDD::SimInterface::ReleaseConnectedPlatform(const QString& aPlatformName)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);

   mConnectedPlatforms.erase(aPlatformName);
}

void HDD::SimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
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

void HDD::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);

   mConnectedPlatforms.clear();
   mPlatformsWithSAP.clear();
}

bool HDD::SimInterface::HasSituationAwarenessProcessor(size_t aIndex)
{
   QMutexLocker locker(&mMutex);
   return (mPlatformsWithSAP.find(aIndex) != mPlatformsWithSAP.end());
}

void HDD::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);

   std::map<QString, HDD::HDD_Data> tempHddData;

   for (QString platformName : mConnectedPlatforms)
   {
      HDD::HDD_Data tempData;

      WsfPlatform* platform = aSimulation.GetPlatformByName(platformName.toStdString());
      if (platform != nullptr)
      {
         tempData.platformData.platformIndex = platform->GetIndex();
         tempData.platformData.simTime       = aSimulation.GetSimTime();

         // First, we get general data -- if there is other data, it will override this data
         // Lat/Lon, Altitude
         double lat, lon, alt;
         platform->GetLocationLLA(lat, lon, alt);
         tempData.platformData.lat_deg = lat;
         tempData.platformData.lon_deg = lon;
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
                  HDD::SimData::WaypointLatLon wayPt;
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

            // Update controls data
            const WsfSA_Processor::SA_FlightControlsDataSummary& controls = saProc->GetFlightControlsDataSummary();
            tempData.flightControlsData.platformIndex                     = platform->GetIndex();
            tempData.flightControlsData.stickRgtInput                     = controls.stickRgtInput;
            tempData.flightControlsData.stickBackInput                    = controls.stickBackInput;
            tempData.flightControlsData.rudderRgtInput                    = controls.rudderRgtInput;
            tempData.flightControlsData.speedBrakeInput                   = controls.speedBrakeInput;
            tempData.flightControlsData.throttleInput                     = controls.throttleInput;
            tempData.flightControlsData.aileronRgtNormalized              = controls.aileronRgtNormalized;
            tempData.flightControlsData.aileronLftNormalized              = controls.aileronLftNormalized;
            tempData.flightControlsData.flapRgtNormalized                 = controls.flapRgtNormalized;
            tempData.flightControlsData.flapLftNormalized                 = controls.flapLftNormalized;
            tempData.flightControlsData.stabilizerRgtNormalized           = controls.stabilizerRgtNormalized;
            tempData.flightControlsData.stabilizerLftNormalized           = controls.stabilizerLftNormalized;
            tempData.flightControlsData.rudderRgtNormalized               = controls.rudderRgtNormalized;
            tempData.flightControlsData.rudderLftNormalized               = controls.rudderLftNormalized;
            tempData.flightControlsData.rudderNormalized                  = controls.rudderNormalized;
            tempData.flightControlsData.speedBrakeNormalized              = controls.speedBrakeNormalized;
            tempData.flightControlsData.thrustNormalized                  = controls.thrustNormalized;
            tempData.flightControlsData.landingGear                       = controls.landingGear;
            tempData.flightControlsData.throttleValid                     = controls.throttleValid;
            tempData.flightControlsData.aileronsValid                     = controls.aileronsValid;
            tempData.flightControlsData.flapsValid                        = controls.flapsValid;
            tempData.flightControlsData.twinRuddersValid                  = controls.twinRuddersValid;
            tempData.flightControlsData.singleRudderValid                 = controls.singleRudderValid;

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
               HDD::SimData::WeaponNameQty wpn;
               wpn.weaponName = weapon.weaponName;
               wpn.quantity   = weapon.quantity;

               tempData.weaponData.weaponList.push_back(wpn);
            }

            for (auto& target : weapons.engagedTargetList)
            {
               HDD::SimData::EngagedTargetData tgt;
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

            for (auto& trkProc : trackProcessors)
            {
               HDD::SimData::TrackProcessor proc;
               proc.isMasterProcessor  = trkProc.isMasterProcessor;
               proc.trackProcessorName = trkProc.name;

               for (auto& trk : trkProc.trackList)
               {
                  HDD::SimData::TargetTrackItem track;

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
                  track.identification = static_cast<HDD::SimData::Identification>(static_cast<int>(trk.identification));
                  track.latLonValid    = trk.latLonValid;
                  track.altitudeValid  = trk.altitudeValid;
                  track.bearingValid   = trk.bearingValid;
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
            for (auto& trkProc : trackProcessors)
            {
               if (trkProc.isMasterProcessor)
               {
                  for (auto& trk : trkProc.trackList)
                  {
                     HDD::SimData::TargetTrackItem track;

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
                        static_cast<HDD::SimData::Identification>(static_cast<int>(trk.identification));
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
            tempData.assetsList.platformIndex           = platform->GetIndex();

            for (auto& asset : assets)
            {
               HDD::SimData::EntityPerceptionData entity;

               entity.lat_deg        = asset->GetLat_deg();
               entity.lon_deg        = asset->GetLon_deg();
               entity.altitude_ft    = asset->GetAltitude_ft();
               entity.bearing_deg    = asset->GetBearing_deg();
               entity.speed_kts      = asset->GetSpeed_kts();
               entity.heading_deg    = asset->GetHeading_deg();
               entity.threatLevel    = asset->GetThreatLevel();
               entity.targetValue    = asset->GetTargetValue();
               entity.risk           = asset->GetRisk();
               entity.defensiveness  = asset->GetDefensiveness();
               entity.urgency        = asset->GetUrgency();
               entity.idString       = asset->GetIdString();
               entity.perceivedIndex = asset->GetPerceivedIndex();
               entity.perceivedName  = asset->GetPerceivedName();
               entity.perceivedType  = asset->GetPerceivedType();
               entity.flightId       = static_cast<unsigned short>(asset->GetFlightId());
               entity.idFlag         = asset->GetIdFlag();
               entity.identification = static_cast<HDD::SimData::Identification>(asset->GetIdentification());
               entity.friendlyAsset  = asset->GetFriendlyAsset();
               entity.angleOnly      = asset->GetAngleOnly();
               entity.idIffFriend    = asset->GetIdIffFriend();
               entity.idIffFoe       = asset->GetIdIffFoe();
               entity.idIffNeutral   = asset->GetIdIffNeutral();
               entity.idIffUnknown   = asset->GetIdIffUnknown();
               entity.idAuxFriend    = asset->GetIdAuxFriend();
               entity.idAuxFoe       = asset->GetIdAuxFoe();
               entity.idAuxNeutral   = asset->GetIdAuxNeutral();
               entity.idAuxUnknown   = asset->GetIdAuxUnknown();
               entity.idSideFriend   = asset->GetIdSideFriend();
               entity.idSideFoe      = asset->GetIdSideFoe();
               entity.idSideNeutral  = asset->GetIdSideNeutral();
               entity.idSideUnknown  = asset->GetIdSideUnknown();
               entity.idTypeFriend   = asset->GetIdTypeFriend();
               entity.idTypeFoe      = asset->GetIdTypeFoe();
               entity.idTypeNeutral  = asset->GetIdTypeNeutral();
               entity.idTypeUnknown  = asset->GetIdTypeUnknown();
               entity.isHostile      = asset->GetIsHostile();
               entity.altitudeValid  = asset->GetAltitudeValid();
               entity.speedValid     = asset->GetSpeedValid();
               entity.headingValid   = asset->GetHeadingValid();
               entity.isMissile      = asset->GetIsMissile();

               // Add to the list
               tempData.assetsList.assetList.emplace_back(entity);
            }
         }
      } // if (0 != platform)
      // Once we've finished populating the data, update our container
      tempHddData.emplace(platformName, tempData);

   } // for (QString platformName : mConnectedPlatforms)

   if (mConnectedPlatforms.size() > 0)
   {
      HDD::DataContainer tempDataContainer;
      tempDataContainer.SetHDDData(tempHddData);
      AddSimEvent(ut::make_unique<UpdateHDDDataEvent>(tempDataContainer));
   }
}

bool HDD::SimInterface::IsConnectionDestroyed(const QString& aPlatformName)
{
   if (!IsEnabled())
   {
      return true;
   }

   QMutexLocker locker(&mMutex);

   if (mConnectedPlatforms.find(aPlatformName) != mConnectedPlatforms.end())
   {
      return false;
   }
   return true;
}
