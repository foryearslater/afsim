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

#include "AirCombatVisualizationSimInterface.hpp"

#include "AirCombatVisualizationSimEvents.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfExplicitWeapon.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSA_Assess.hpp"
#include "WsfSA_Processor.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationObserver.hpp"
#include "WsfTrackManager.hpp"

WkAirCombat::SimInterface::SimInterface(const QString& aPluginName)
   : SimInterfaceT<AirCombatSimEvent>(aPluginName)
{
}

void WkAirCombat::SimInterface::RemoveTargetPlatform(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   auto         foundPlatform = std::find(mTargetPlatformNames.begin(), mTargetPlatformNames.end(), aPlatformName);
   if (foundPlatform != mTargetPlatformNames.end())
   {
      mTargetPlatformNames.erase(foundPlatform);
   }
}

void WkAirCombat::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   std::vector<wkf::AirCombatDataEngagementSummary> tempEngagementDataList;

   WsfPlatform*             sourcePlatform = nullptr;
   std::vector<std::string> targetPlatformNames;

   // Gather data for the source platform.
   if (mMutex.try_lock())
   {
      sourcePlatform      = aSimulation.GetPlatformByName(mSourcePlatformName);
      targetPlatformNames = mTargetPlatformNames;
      mMutex.unlock();
   }

   wkf::AirCombatDataEngagementSummary sourcePlatformData;
   if (sourcePlatform != nullptr)
   {
      for (WsfComponentList::RoleIterator<WsfProcessor> iter(*sourcePlatform); !iter.AtEnd(); ++iter)
      {
         WsfSA_Processor* processor = dynamic_cast<WsfSA_Processor*>(*iter);
         if (processor != nullptr)
         {
            PopulateData(aSimulation, *sourcePlatform, sourcePlatformData);
            tempEngagementDataList.emplace_back(sourcePlatformData);
         }
      }
   }

   // Target platforms
   for (const auto& targetName : targetPlatformNames)
   {
      WsfPlatform* targetPlatform = aSimulation.GetPlatformByName(targetName);
      if (targetPlatform != nullptr)
      {
         for (WsfComponentList::RoleIterator<WsfProcessor> iter(*targetPlatform); !iter.AtEnd(); ++iter)
         {
            WsfSA_Processor* processor = dynamic_cast<WsfSA_Processor*>(*iter);
            if (processor != nullptr)
            {
               wkf::AirCombatDataEngagementSummary data;
               PopulateData(aSimulation, *targetPlatform, data);
               tempEngagementDataList.emplace_back(data);
            }
         }
      }
   }

   // Prioritized targets
   for (const auto& prioritizedTargetName : sourcePlatformData.engagementData.mPrioritizedTargetNames)
   {
      WsfPlatform* targetPlatform = aSimulation.GetPlatformByName(prioritizedTargetName);
      if (targetPlatform != nullptr)
      {
         bool dataSuccess = false;
         for (WsfComponentList::RoleIterator<WsfProcessor> iter(*targetPlatform); !iter.AtEnd(); ++iter)
         {
            WsfSA_Processor* processor = dynamic_cast<WsfSA_Processor*>(*iter);
            if (processor != nullptr)
            {
               wkf::AirCombatDataEngagementSummary data;
               PopulateData(aSimulation, *targetPlatform, data);
               tempEngagementDataList.emplace_back(data);

               dataSuccess = true;
            }
         }

         if (!dataSuccess)
         {
            wkf::AirCombatDataEngagementSummary data;
            data.platformData.mPlatformName  = targetPlatform->GetName();
            data.platformData.mPlatformIndex = targetPlatform->GetIndex();
            data.platformData.mPlatformType  = targetPlatform->GetType();
            tempEngagementDataList.emplace_back(data);
         }
      }
   }

   // Prioritized threats
   for (const auto& prioritizedThreatName : sourcePlatformData.engagementData.mPrioritizedThreatNames)
   {
      WsfPlatform* targetPlatform = aSimulation.GetPlatformByName(prioritizedThreatName);
      if (targetPlatform != nullptr)
      {
         bool dataSuccess = false;
         for (WsfComponentList::RoleIterator<WsfProcessor> iter(*targetPlatform); !iter.AtEnd(); ++iter)
         {
            WsfSA_Processor* processor = dynamic_cast<WsfSA_Processor*>(*iter);
            if (processor != nullptr)
            {
               wkf::AirCombatDataEngagementSummary data;
               PopulateData(aSimulation, *targetPlatform, data);
               tempEngagementDataList.emplace_back(data);

               dataSuccess = true;
            }
         }

         if (!dataSuccess)
         {
            wkf::AirCombatDataEngagementSummary data;
            data.platformData.mPlatformName  = targetPlatform->GetName();
            data.platformData.mPlatformIndex = targetPlatform->GetIndex();
            data.platformData.mPlatformType  = targetPlatform->GetType();
            tempEngagementDataList.emplace_back(data);
         }
      }
   }

   AddSimEvent(ut::make_unique<UpdateEvent>(tempEngagementDataList));
}

void WkAirCombat::SimInterface::PopulateData(const WsfSimulation&                 aSimulation,
                                             WsfPlatform&                         aPlatform,
                                             wkf::AirCombatDataEngagementSummary& aData)
{
   // Most data comes from the SA Processor
   WsfSA_Processor* saProc = WsfSA_Processor::GetSA_Processor(aPlatform);

   if (saProc != nullptr)
   {
      aData.platformData.mPlatformName  = aPlatform.GetName();
      aData.platformData.mPlatformType  = aPlatform.GetType();
      aData.platformData.mPlatformIndex = aPlatform.GetIndex();

      // Update kinematics/platform data
      const WsfSA_Processor::SA_AircraftKinematics& kinematics = saProc->GetAircraftKinematics();
      aData.kinematicData.mAltitude_ft                         = kinematics.altBaro_ft;
      aData.kinematicData.mVerticalSpeed_fpm                   = kinematics.vertSpd_fpm;
      aData.kinematicData.mKCAS                                = kinematics.kcas;
      aData.kinematicData.mKTAS                                = kinematics.ktas;
      aData.kinematicData.mMach                                = kinematics.mach;
      aData.kinematicData.mNx                                  = kinematics.gX;
      aData.kinematicData.mAlpha_deg                           = kinematics.alpha_deg;
      aData.kinematicData.mGLoad                               = kinematics.gLoad;
      aData.kinematicData.mAvailG                              = kinematics.gAvail;
      aData.kinematicData.mHeading_deg                         = kinematics.heading_deg;
      aData.kinematicData.mPitch_deg                           = kinematics.pitch_deg;
      aData.kinematicData.mRoll_deg                            = kinematics.roll_deg;

      // Update fuel system data
      const WsfSA_Processor::SA_FuelSystemData& fuel = saProc->GetFuelSystemData();
      aData.fuelData.mCurrentInternalFuel_lbs        = fuel.fuelInternal_lbs;
      aData.fuelData.mCurrentExternalFuel_lbs        = fuel.fuelExternal_lbs;
      aData.fuelData.mInternalFuelCapacity_lbs       = fuel.fuelCapInt_lbs;
      aData.fuelData.mExternalFuelCapacity_lbs       = fuel.fuelCapExt_lbs;
      aData.fuelData.mTotalFuel_lbs                  = fuel.fuelInternal_lbs + fuel.fuelExternal_lbs;
      aData.fuelData.mNormalizedFuelLevel            = fuel.normalizedFuelQty;
      aData.fuelData.mJokerFuel_lbs                  = fuel.joker_lbs;
      aData.fuelData.mBingoFuel_lbs                  = fuel.bingo_lbs;

      if (aData.fuelData.mTotalFuel_lbs < aData.fuelData.mJokerFuel_lbs)
      {
         aData.fuelData.mJokerReached = true;
      }
      else
      {
         aData.fuelData.mJokerReached = false;
      }

      if (aData.fuelData.mTotalFuel_lbs < aData.fuelData.mBingoFuel_lbs)
      {
         aData.fuelData.mBingoReached = true;
      }
      else
      {
         aData.fuelData.mBingoReached = false;
      }

      // No data for this at this time:  aData.mTimeToJoker_sec

      // Update weapons data
      const WsfSA_Processor::SA_WeaponsDataSummary& weapons = saProc->GetWeaponsDataSummary();
      aData.weaponsData.mInitialTotalWeaponCount            = weapons.initialNumWeapons;
      aData.weaponsData.mSelectedWeapon                     = weapons.selectedWeapon;
      aData.weaponsData.mSupportingWeapon                   = weapons.supportingWeapon;
      aData.weaponsData.mWeaponsQtyMap.clear();
      for (auto& wpn : weapons.weaponList)
      {
         // Add weapon to the list
         aData.weaponsData.mWeaponsQtyMap.emplace(wpn.weaponName, wpn.quantity);
      }

      // Emissions data
      aData.emissionsData.mRadarEmitting        = saProc->RadarEmitting();
      aData.emissionsData.mJammerEmitting       = saProc->JammerEmitting();
      aData.emissionsData.mOtherSystemsEmitting = saProc->OtherSystemsEmitting();

      // Signature data
      aData.signatureData.mAfterburnerOn      = saProc->AfterburnerOn();
      aData.signatureData.mContrailing        = saProc->IsContrailing();
      aData.signatureData.mWeaponBayDoorsOpen = saProc->Assess()->WeaponBayDoorsOpen();

      // Overall SA data
      aData.saData.mMissionTask    = saProc->Assess()->MissionTask();
      aData.saData.mRiskAcceptable = saProc->Assess()->AcceptableLevelOfRiskByte();
      aData.saData.mRiskCurrent    = saProc->Assess()->Risk();
      aData.saData.mDefensiveness  = saProc->Assess()->Defensiveness();
      aData.saData.mUrgency        = saProc->Assess()->Urgency();
      aData.saData.mSelfRisk       = saProc->Assess()->SelfRisk();
      aData.saData.mFlightRisk     = saProc->Assess()->FlightRisk();
      aData.saData.mPackageRisk    = saProc->Assess()->PackageRisk();
      aData.saData.mMissionRisk    = saProc->Assess()->MissionRisk();

      // Get the list of prioritized target names
      std::vector<std::string> prioritizedTargetList = saProc->Assess()->PrioritizedTargetNames();

      // Next, add each prioritized target to the sim interface's list, as well as the data list.
      for (const auto& target : prioritizedTargetList)
      {
         aData.engagementData.mPrioritizedTargetNames.emplace_back(target);
      }

      // Get the list of prioritized threat names.
      std::vector<std::string> prioritizedThreatList = saProc->Assess()->PrioritizedThreatNames();

      // Next, add each prioritized threat to the sim interface's list, as well as the data list.
      for (const auto& threat : prioritizedThreatList)
      {
         aData.engagementData.mPrioritizedThreatNames.emplace_back(threat);
      }

      // Get the list of tracked platform names
      std::vector<std::string> trackedPlatformsList = saProc->Assess()->CurrentlyTrackedPlatformNames();
      for (const auto& threat : trackedPlatformsList)
      {
         aData.engagementData.mTrackedPlatformNames.emplace_back(threat);
      }

      // Clear any old engagement data
      aData.engagementData.mAngleOffMap_deg.clear();
      aData.engagementData.mAspectAngleMap_deg.clear();
      aData.engagementData.mCurrentRangeMap_nm.clear();
      aData.engagementData.mCurrentDeltaAltMap_ft.clear();
      aData.engagementData.mTrackQuality.clear();
      aData.engagementData.mTargetCID.clear();
      aData.engagementData.mTargetThreatLevel.clear();
      aData.engagementData.mRawDetectionAgainstTargetMap_nm.clear();
      aData.engagementData.mRawWEZAgainstTargetMap_nm.clear();
      aData.engagementData.mWEZMinRangeMap_nm.clear();
      aData.engagementData.mWEZMaxRangeMap_nm.clear();
      aData.engagementData.mWEZNoEscapeMap_nm.clear();
      aData.engagementData.mRiskMap.clear();
      aData.engagementData.mDefensivenessMap.clear();
      aData.engagementData.mUrgencyMap.clear();
      aData.engagementData.mTargetLocked.clear();
      aData.engagementData.mAcceptableWEZ.clear();

      // Loop through filtered targets
      const std::vector<size_t> platformIdList = saProc->GetFilteredPlatformIdList();
      for (const auto& platformId : platformIdList)
      {
         // Get the current platform in the loop
         WsfPlatform* tmpPlatform = aSimulation.GetPlatformByIndex(platformId);

         if (tmpPlatform != nullptr)
         {
            std::string tmpPlatformName = tmpPlatform->GetName();

            if (tmpPlatformName != aPlatform.GetName()) // Be sure to exclude "self"
            {
               if (tmpPlatform->GetSideId() != aPlatform.GetSideId()) // Also exclude friendly forces
               {
                  aData.engagementData.mAngleOffMap_deg.emplace(tmpPlatform->GetName(),
                                                                saProc->Assess()->AngleOffTarget2D(*tmpPlatform) *
                                                                   UtMath::cDEG_PER_RAD);
                  aData.engagementData.mAspectAngleMap_deg.emplace(tmpPlatform->GetName(),
                                                                   saProc->Assess()->AspectAngleForTarget2D(*tmpPlatform) *
                                                                      UtMath::cDEG_PER_RAD);

                  double currentRange_nm = saProc->Assess()->CalcSlantRange(*tmpPlatform) * UtMath::cNM_PER_M;
                  aData.engagementData.mCurrentRangeMap_nm.emplace(tmpPlatform->GetName(), currentRange_nm);

                  aData.engagementData.mCurrentDeltaAltMap_ft.emplace(tmpPlatform->GetName(),
                                                                      saProc->Assess()->CalcDeltaAltitude(*tmpPlatform) *
                                                                         UtMath::cFT_PER_M);

                  // This will be added in the future: aData.mTrackQuality();
                  // This will be added in the future: aData.mTargetCID();
                  // This will be added in the future: aData.mThreatLevel();
                  // This will be added in the future: aData.mTargetValue();

                  aData.engagementData.mRawDetectionAgainstTargetMap_nm.emplace(
                     tmpPlatform->GetName(),
                     saProc->Assess()->ExpectedDetectionRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);

                  double wezDesiredRange_nm =
                     saProc->Assess()->ExpectedWezRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M;
                  aData.engagementData.mRawWEZAgainstTargetMap_nm.emplace(tmpPlatform->GetName(), wezDesiredRange_nm);

                  aData.engagementData.mWEZMinRangeMap_nm.emplace(
                     tmpPlatform->GetName(),
                     saProc->Assess()->ExpectedMinimumEngagementRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);
                  aData.engagementData.mWEZMaxRangeMap_nm.emplace(
                     tmpPlatform->GetName(),
                     saProc->Assess()->ExpectedMaximumEngagementRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);
                  aData.engagementData.mWEZNoEscapeMap_nm.emplace(
                     tmpPlatform->GetName(),
                     saProc->Assess()->ExpectedNoEscapeRangeAgainstTarget(*tmpPlatform) * UtMath::cNM_PER_M);

                  // This will be added in the future: aData.mRisk();
                  // This will be added in the future: aData.mDefensiveness();
                  // This will be added in the future: aData.mUrgency();

                  if (saProc->Assess()->IsCurrentlyBeingTracked(platformId))
                  {
                     aData.engagementData.mTargetLocked.emplace(tmpPlatform->GetName(), true);
                  }
                  else
                  {
                     aData.engagementData.mTargetLocked.emplace(tmpPlatform->GetName(), false);
                  }

                  if (currentRange_nm < wezDesiredRange_nm)
                  {
                     aData.engagementData.mAcceptableWEZ.emplace(tmpPlatform->GetName(), true);
                  }
                  else
                  {
                     aData.engagementData.mAcceptableWEZ.emplace(tmpPlatform->GetName(), false);
                  }
               }
            }
         }
      }
   }
}

void WkAirCombat::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   for (WsfComponentList::RoleIterator<WsfProcessor> iter(aPlatform); !iter.AtEnd(); ++iter)
   {
      if (dynamic_cast<WsfSA_Processor*>(*iter) != nullptr)
      {
         AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
         break;
      }
   }
}

void WkAirCombat::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform.GetName()));
}

void WkAirCombat::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
}
