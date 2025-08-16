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
#include "TunerSimInterface.hpp"

#include <QMutexLocker>

#include "P6DofAtmosphere.hpp"
#include "P6DofCommonController.hpp"
#include "P6DofFreezeFlags.hpp"
#include "P6DofJetEngineObject.hpp"
#include "P6DofLandingGear.hpp"
#include "P6DofPilotObject.hpp"
#include "P6DofPropulsionSystem.hpp"
#include "P6DofScenario.hpp"
#include "P6DofTerrain.hpp"
#include "P6DofThrustProducerObject.hpp"
#include "P6DofVehicle.hpp"
#include "UtMath.hpp"
#include "UtUnitTypes.hpp"
#include "WsfClockSource.hpp"
#include "WsfMover.hpp"
#include "WsfP6DOF_Mover.hpp"
#include "WsfP6DOF_TypeManager.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

WkTuner::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceBase(aPluginName)
   , mPlatformOfInterest(-1)
   , mPidOfInterest(P6DOF::Pid::Unknown)
   , mFirstP6DofMoverFound(false)
   , mLogData(false)
{
}

void WkTuner::SimInterface::GuiUpdate()
{
   // Check for new platforms
   std::map<int, std::string> newMap = GetAndClearNewPlatformMap();
   for (auto& iter : newMap)
   {
      emit NewP6DofPlatform(iter.second, iter.first);
   }

   // Check for platforms that have been removed
   std::map<int, std::string> removeMap = GetAndClearDeletedPlatformMap();
   for (auto& iter : removeMap)
   {
      emit DeletedP6DofPlatform(iter.second, iter.first);
   }
}

void WkTuner::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   if (mMutex.tryLock())
   {
      mSimData.simDataValid = true;
      mSimData.simTime      = aSimulation.GetSimTime();
      mSimData.simRate      = aSimulation.GetClockRate();
      mSimData.isPaused     = aSimulation.GetClockSource()->IsStopped();

      // Loop through all platforms
      for (unsigned int i = 0; i < aSimulation.GetPlatformCount(); ++i)
      {
         WsfPlatform* platformPtr = aSimulation.GetPlatformEntry(i);
         int          platformId  = (int)platformPtr->GetIndex();

         // Check to see if this is a new platform
         bool newPlatform = false;
         if (mDiscoveredPlatforms.count(platformId) == 0)
         {
            mDiscoveredPlatforms.insert(platformId);
            newPlatform = true;
         }

         // If platform contains a P6DOF mover
         auto* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(platformPtr->GetMover());
         if (p6dofMover != nullptr)
         {
            if (!mFirstP6DofMoverFound)
            {
               // This is dirty, but it is a work-around to avoid rewriting the plugin to conform
               //  to the new SimInterface/SimEvents/SimCommands paradigm
               auto& simulation = const_cast<WsfSimulation&>(aSimulation);
               simulation.AddWallEvent(ut::make_unique<WriteDataEvent>(this));
               mFirstP6DofMoverFound = true;
            }

            Platform&          platformData = mSimData.platformMap[platformId];
            const std::string& platformName = platformPtr->GetName();
            P6DofVehicle*      object       = p6dofMover->GetP6DofVehicle();

            platformData.alpha_deg      = object->GetAlpha_deg();
            platformData.beta_deg       = object->GetBeta_deg();
            platformData.heading_deg    = object->GetHeading_deg();
            platformData.pitch_deg      = object->GetPitch_deg();
            platformData.roll_deg       = object->GetRoll_deg();
            platformData.crashed        = object->Crashed();
            platformData.hasLandingGear = object->HasLandingGear();

            double alt_m;
            object->GetLocationLLA(platformData.latitude_deg, platformData.longitude_deg, alt_m);
            platformData.altitude_ft = alt_m * UtMath::cFT_PER_M;
            platformData.speed_fps   = object->GetSpeed_fps();

            if (object->GetScenario()->GetAtmosphere() != nullptr)
            {
               platformData.airDensityAtCurrentAlt =
                  object->GetScenario()->GetAtmosphere()->AirDensityAtAltitude(platformData.altitude_ft);
            }
            else
            {
               platformData.airDensityAtCurrentAlt = 0.0;
            }

            P6DofPropulsionSystem* propulsionSystem = object->GetPropulsionSystem();
            if (propulsionSystem != nullptr)
            {
               P6DofThrustProducerObject* engine = propulsionSystem->GetThrustProducerByIndex(0);
               if (engine != nullptr)
               {
                  platformData.thrustPitchVector_deg = engine->GetThrustVectoringPitch_deg();
                  platformData.engineThrust_lbs      = engine->GetForwardThrust_lbs();

                  auto* jetPtr = dynamic_cast<P6DofJetEngineObject*>(engine);
                  if (jetPtr != nullptr)
                  {
                     platformData.afterburnerOn                  = jetPtr->AfterburnerOn();
                     platformData.engineThrottleLeverMilitary    = jetPtr->GetEnginePercentRPM() * 0.01;
                     platformData.engineThrottleLeverAfterburner = jetPtr->GetNozzlePosition();
                  }
                  else
                  {
                     platformData.afterburnerOn                  = false;
                     platformData.engineThrottleLeverMilitary    = 0.0;
                     platformData.engineThrottleLeverAfterburner = 0.0;
                  }
               }
            }

            P6DofPilotObject* pilot = object->GetPilotObject();
            if (pilot != nullptr)
            {
               P6DofCommonController* autopilot = pilot->GetCommonController();
               if (autopilot != nullptr)
               {
                  platformData.controlMethod = autopilot->GetControlMethod();
                  autopilot->GetAutopilotPidValues(&platformData.pidValues);
                  AFSIM_LoadPidGains(autopilot, platformData.pidGains, platformData.pidFlags);
                  platformData.limits = autopilot->GetCurrentLimitsAndSettings();

                  if (mLogData && mPidOfInterest != P6DOF::Pid::Unknown && platformId == mPlatformOfInterest)
                  {
                     LogPidOfInterest(platformData.pidValues, mSimData.simTime, object);
                  }
               }
            }

            // If this is a new platform and it is a P6DOF mover, add to the mNewPlatformMap
            if (newPlatform)
            {
               mNewPlatformMap[platformId] = platformName;
            }
         }
      }
      mMutex.unlock();
   }
}

void WkTuner::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest   = -1;
   mSimData              = SimData();
   mSimData.initializing = true;
   mDiscoveredPlatforms.clear();
   mNewPlatformMap.clear();
   mDeletedPlatformMap.clear();
   mFirstP6DofMoverFound = false;
}

void WkTuner::SimInterface::WriteData(WsfSimulation& aSimulation)
{
   if (mMutex.tryLock())
   {
      mSimData.simDataValid = true;
      mSimData.simTime      = aSimulation.GetSimTime();
      mSimData.simRate      = aSimulation.GetClockRate();
      mSimData.isPaused     = aSimulation.GetClockSource()->IsStopped();
      // loop through all platforms
      for (unsigned int i = 0; i < aSimulation.GetPlatformCount(); ++i)
      {
         WsfPlatform* platformPtr = aSimulation.GetPlatformEntry(i);
         int          platformId  = (int)platformPtr->GetIndex();

         if (platformId == mPlatformOfInterest)
         {
            // If platform contains a P6DOF mover
            auto* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(platformPtr->GetMover());
            if (p6dofMover != nullptr)
            {
               P6DofVehicle* object = p6dofMover->GetP6DofVehicle();
               if (object != nullptr)
               {
                  // Prevent crashing: object->SetTestingIgnoreAllCrashes(true);

                  P6DofPilotObject* pilot = object->GetPilotObject();
                  if (pilot != nullptr)
                  {
                     // Check for pending Freeze commands to issue to the sim
                     for (auto& pendingFreezeCommand : mGui.mPendingFreezeCommands)
                     {
                        switch (pendingFreezeCommand.first)
                        {
                        case FRZ_ALTITUDE:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeAltitude(pendingFreezeCommand.second);
                           break;
                        case FRZ_SPEED:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeSpeed(pendingFreezeCommand.second);
                           break;
                        case FRZ_POSITION:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeLocation(pendingFreezeCommand.second);
                           break;
                        case FRZ_FUEL:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeFuelBurn(pendingFreezeCommand.second);
                           break;
                        case FRZ_YAW:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeYaw(pendingFreezeCommand.second);
                           break;
                        case FRZ_PITCH:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezePitch(pendingFreezeCommand.second);
                           break;
                        case FRZ_ROLL:
                           object->GetScenario()->GetFreezeFlags()->SetMasterFreezeRoll(pendingFreezeCommand.second);
                           break;
                        default:
                           break;
                        }
                     }

                     // Clear Freeze Pending commands list after handling the commands
                     mGui.mPendingFreezeCommands.clear();

                     if (mGui.taxiControlPending)
                     {
                        mGui.taxiControlPending = false;

                        // Put the gear down since we'll be taxiing
                        object->ForceLandingGearDownInstantly("Landing_Gear_Extended");

                        // Pre-position to the ground, but include the nominal offset height
                        if (mGui.taxiControl)
                        {
                           P6DofVehicle* vehicle = pilot->GetParentVehicle();
                           if (vehicle != nullptr)
                           {
                              P6DofLandingGear* gear = vehicle->GetLandingGear();
                              if (gear != nullptr)
                              {
                                 double        hgt_nominal_m = gear->GetNominalHeightAboveGroundOnGear();
                                 double        hgt_terrain_m = 0.0;
                                 P6DofTerrain* terrain       = vehicle->GetScenario()->GetTerrain();
                                 if (terrain != nullptr)
                                 {
                                    double startLat, startLon;
                                    vehicle->GetStartingLatLon(startLat, startLon);
                                    hgt_terrain_m = terrain->HeightOfTerrain_m(startLat, startLon);
                                 }
                                 double hgt_m = hgt_terrain_m + hgt_nominal_m;
                                 AFSIM_SendPrePositionCommand(hgt_m, Data::KTAS, 0, pilot);
                              }
                              else
                              {
                                 std::cout << "Warning -- Cannot prepo vehicle on ground without landing gear in "
                                              "Tuner::SimInterface::SimulationUpdateWrite()"
                                           << std::endl;
                              }
                           }
                        }
                     }

                     if (mGui.setPositionPending)
                     {
                        mGui.setPositionPending = false;
                        AFSIM_SendPrePositionCommand(mGui.setPositionAltitude_m,
                                                     mGui.setPositionSpeedType,
                                                     mGui.setPositionSpeed,
                                                     pilot);
                        // Make sure gear is up b/c we aren't taxiing
                        object->ForceLandingGearUpInstantly("Landing_Gear_Extended");
                     }

                     if (mGui.command.pending)
                     {
                        mGui.command.pending = false;
                        AFSIM_SendCommandBasedOnPid(mGui.command.pid, mGui.command.value, pilot);
                     }

                     if (mGui.customCommand.pending)
                     {
                        mGui.customCommand.pending = false;
                        AFSIM_SendCustomCommand(mGui.customCommand.commandType, mGui.customCommand.value, pilot);
                     }

                     if (mGui.p.pending)
                     {
                        mGui.p.pending                   = false;
                        P6DofCommonController* autopilot = pilot->GetCommonController();
                        if (autopilot != nullptr)
                        {
                           autopilot->SetPidGainKP(mGui.p.pid, mGui.p.controllingValue, mGui.p.value);
                        }
                     }

                     if (mGui.i.pending)
                     {
                        mGui.i.pending                   = false;
                        P6DofCommonController* autopilot = pilot->GetCommonController();
                        if (autopilot != nullptr)
                        {
                           autopilot->SetPidGainKI(mGui.i.pid, mGui.i.controllingValue, mGui.i.value);
                        }
                     }

                     if (mGui.d.pending)
                     {
                        mGui.d.pending                   = false;
                        P6DofCommonController* autopilot = pilot->GetCommonController();
                        if (autopilot != nullptr)
                        {
                           autopilot->SetPidGainKD(mGui.d.pid, mGui.d.controllingValue, mGui.d.value);
                        }
                     }

                     if (mGui.maxAccum.pending)
                     {
                        mGui.maxAccum.pending            = false;
                        P6DofCommonController* autopilot = pilot->GetCommonController();
                        if (autopilot != nullptr)
                        {
                           autopilot->SetPidMaxAccum(mGui.maxAccum.pid, mGui.maxAccum.controllingValue, mGui.maxAccum.value);
                        }
                     }

                     if (mGui.runCommandPending)
                     {
                        mGui.runCommandPending = false;
                        aSimulation.Resume();
                     }

                     if (mGui.simRateCommandPending)
                     {
                        mGui.simRateCommandPending = false;
                        aSimulation.SetClockRate(mGui.simRate);
                     }

                     if (mGui.pauseCommandPending)
                     {
                        mGui.pauseCommandPending = false;
                        aSimulation.Pause();
                     }

                     if (mGui.advanceToTimeCommandPending)
                     {
                        mGui.advanceToTimeCommandPending = false;
                        aSimulation.AddEvent(ut::make_unique<AdvanceTimeEvent>(mGui.advanceTime, true, this));
                        if (!mGui.advanceRealTime)
                        {
                           aSimulation.SetRealtime(aSimulation.GetSimTime(), false);
                        }
                     }

                     if (mGui.guiAutoTuneCommand.pending || mGui.guiAutoTuneCommand.running)
                     {
                        AFSIM_AutoTune(pilot, aSimulation);
                        mGui.guiAutoTuneCommand.pending = false;
                     }
                  }
               }
            }
         }
      }
      mMutex.unlock();
   }
}

void WkTuner::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (mMutex.tryLock())
   {
      auto* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(aPlatform.GetMover());
      if (p6dofMover != nullptr)
      {
         mDeletedPlatformMap[(int)aPlatform.GetIndex()] = aPlatform.GetName();
      }
      mMutex.unlock();
   }
}

double WkTuner::SimInterface::GetSimTime()
{
   QMutexLocker locker(&mMutex);
   return mSimData.simTime;
}

bool WkTuner::SimInterface::GetSimInitializing()
{
   bool returnVal = false;

   QMutexLocker locker(&mMutex);
   returnVal             = mSimData.initializing;
   mSimData.initializing = false;
   return returnVal;
}

void WkTuner::SimInterface::SetLogData(bool aVal)
{
   QMutexLocker locker(&mMutex);
   mLogData = aVal;
}

void WkTuner::SimInterface::RunAutoTuneCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.pending = true;
}

void WkTuner::SimInterface::InitializeAutoTuneCommand(int                 aPlatformId,
                                                      P6DOF::Pid::Type    aPid,
                                                      double              aAltitude_m,
                                                      Data::CustomCommand aSpeedType,
                                                      double              aSpeed)
{
   P6DOF::AutopilotLimitsAndSettings limitsAndSettings = GetLimitsAndSettings(aPlatformId);

   QMutexLocker locker(&mMutex);

   mGui.guiAutoTuneCommand.tuningPid             = aPid;
   mGui.guiAutoTuneCommand.setPositionAltitude_m = aAltitude_m;
   mGui.guiAutoTuneCommand.setPositionSpeedType  = aSpeedType;
   mGui.guiAutoTuneCommand.setPositionSpeed      = aSpeed;

   // Default commanded values (Values determined through testing)
   double commandedAlpha_deg     = limitsAndSettings.alpha_Max * 0.25;
   double commandedBeta_deg      = limitsAndSettings.beta_Max * 0.25;
   double commandedVertSpd_fpm   = limitsAndSettings.vertSpd_Max * 0.10;
   double commandedYawRate_dps   = limitsAndSettings.yawRate_Max * 0.05;
   double commandedPitchRate_dps = limitsAndSettings.pitchRate_Max * 0.05;
   double commandedRollRate_dps  = limitsAndSettings.rollRate_Max * 0.20;
   double commandedBankAngle_deg = limitsAndSettings.bankAngle_Max * 0.50;
   double commandedGamma_deg     = 10.0;
   double commandedHeading_deg   = 30.0;
   double commandedAlt_m         = aAltitude_m + 1500;
   double commandedSpeed         = aSpeed + 20;

   switch (aPid)
   {
   case P6DOF::Pid::Alpha:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedAlpha_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;
   case P6DOF::Pid::VertSpeed:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitSpeed::ConvertToStandard(commandedVertSpd_fpm, UtUnitSpeed::cFEET_PER_MINUTE);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::PitchAngle:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedAlpha_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::PitchRate:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngularRate::ConvertToStandard(commandedPitchRate_dps, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::FltPathAngle:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedGamma_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::DeltaPitch:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedAlpha_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::Altitude:
      mGui.guiAutoTuneCommand.commandVal = commandedAlt_m;
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::Beta:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedBeta_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case P6DOF::Pid::YawRate:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngularRate::ConvertToStandard(commandedYawRate_dps, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case P6DOF::Pid::YawHeading:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedHeading_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case P6DOF::Pid::TaxiHeading:
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case P6DOF::Pid::RollRate:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngularRate::ConvertToStandard(commandedRollRate_dps, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::DeltaRoll:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedBankAngle_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::BankAngle:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedBankAngle_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::RollHeading:
      mGui.guiAutoTuneCommand.commandVal =
         UtUnitAngle::ConvertToStandard(commandedHeading_deg, UtUnitAngle::BaseUnit::cDEGREES);
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case P6DOF::Pid::Speed:
      mGui.guiAutoTuneCommand.commandVal = commandedSpeed;
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::TaxiSpeed:
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case P6DOF::Pid::TaxiYawRate:
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   default:
      break;
   }
}

void WkTuner::SimInterface::CancelAutoTuneCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.pending = false;
   mGui.guiAutoTuneCommand.running = false;
}

void WkTuner::SimInterface::SetAutoTunePopSize(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetPopulationSize(aVal);
}

void WkTuner::SimInterface::SetAutoTuneMaxGenerations(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetMaxGenerationCount(aVal);
}

void WkTuner::SimInterface::SetAutoTuneCarryOverFrac(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetCarryOverFraction(aVal);
}

void WkTuner::SimInterface::SetAutoTuneCrossoverProb(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetCrossOverProbability(aVal);
}

void WkTuner::SimInterface::SetAutoTuneMutationProb(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetMutationProbability(aVal);
}

void WkTuner::SimInterface::SetAutoTuneKpRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKpRange(aMin, aMax);
}

void WkTuner::SimInterface::SetAutoTuneKiRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKiRange(aMin, aMax);
}

void WkTuner::SimInterface::SetAutoTuneKdRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKdRange(aMin, aMax);
}

void WkTuner::SimInterface::SetAutoTuneRunTime(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.runTime = aVal;
}

void WkTuner::SimInterface::SetAutoTuneErrorWeight(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetErrorWeight(aVal);
}

void WkTuner::SimInterface::SetAutoTuneOvershootWeight(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetOvershootWeight(aVal);
}

void WkTuner::SimInterface::SendFreezeCommand(FreezeEnum aFreezeCommand, bool aEnabled)
{
   QMutexLocker locker(&mMutex);
   mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(aFreezeCommand, aEnabled));
}

void WkTuner::SimInterface::SendSetPositionCommand(double aAltitude_m, Data::CustomCommand aSpeedType, double aSpeed)
{
   QMutexLocker locker(&mMutex);
   mGui.setPositionPending    = true;
   mGui.setPositionAltitude_m = aAltitude_m;
   mGui.setPositionSpeedType  = aSpeedType;
   mGui.setPositionSpeed      = aSpeed;
}

void WkTuner::SimInterface::SendTaxiCommand(bool aTaxiControl)
{
   QMutexLocker locker(&mMutex);
   mGui.taxiControlPending = true;
   mGui.taxiControl        = aTaxiControl;
}

void WkTuner::SimInterface::SendCommandBasedOnPID(P6DOF::Pid::Type aPID, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.command.pending = true;
   mGui.command.pid     = aPID;
   mGui.command.value   = value;
}

void WkTuner::SimInterface::SendCustomCommand(Data::CustomCommand aCommandType, double aValue)
{
   QMutexLocker locker(&mMutex);
   mGui.customCommand.pending     = true;
   mGui.customCommand.commandType = aCommandType;
   mGui.customCommand.value       = aValue;
}

void WkTuner::SimInterface::SendPValue(P6DOF::Pid::Type aPID, double aControllingValue, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.p.pending          = true;
   mGui.p.pid              = aPID;
   mGui.p.controllingValue = aControllingValue;
   mGui.p.value            = value;

   if (mSimData.platformMap.find(mPlatformOfInterest) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.find(aPID) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).find(aControllingValue) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).end())
   {
      P6DOF::PidGainData* gains = &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KpGain             = value;
   }
}

void WkTuner::SimInterface::SendIValue(P6DOF::Pid::Type aPID, double aControllingValue, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.i.pending          = true;
   mGui.i.pid              = aPID;
   mGui.i.controllingValue = aControllingValue;
   mGui.i.value            = value;

   if (mSimData.platformMap.find(mPlatformOfInterest) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.find(aPID) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).find(aControllingValue) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).end())
   {
      P6DOF::PidGainData* gains = &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KiGain             = value;
   }
}

void WkTuner::SimInterface::SendDValue(P6DOF::Pid::Type aPID, double aControllingValue, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.d.pending          = true;
   mGui.d.pid              = aPID;
   mGui.d.controllingValue = aControllingValue;
   mGui.d.value            = value;

   if (mSimData.platformMap.find(mPlatformOfInterest) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.find(aPID) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).find(aControllingValue) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).end())
   {
      P6DOF::PidGainData* gains = &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KdGain             = value;
   }
}

void WkTuner::SimInterface::SendMaxAccumulation(P6DOF::Pid::Type aPID, bool aEnabled, double aControllingValue, double aValue)
{
   QMutexLocker locker(&mMutex);
   mGui.maxAccum.pending          = true;
   mGui.maxAccum.pid              = aPID;
   mGui.maxAccumEnable            = aEnabled;
   mGui.maxAccum.controllingValue = aControllingValue;
   mGui.maxAccum.value            = aValue;
}

void WkTuner::SimInterface::SendRunCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.runCommandPending = true;
}

void WkTuner::SimInterface::SendPauseCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.pauseCommandPending = true;
}

void WkTuner::SimInterface::SendAdvanceToTimeCommand(double aTime, bool aRealTimeFlag)
{
   mGui.advanceToTimeCommandPending = true;
   mGui.advanceRealTime             = aRealTimeFlag;
   mGui.advanceTime                 = aTime;
}

void WkTuner::SimInterface::SendSimRateCommand(double aSimClockRate)
{
   QMutexLocker locker(&mMutex);
   mGui.simRateCommandPending = true;
   mGui.simRate               = aSimClockRate;
}

bool WkTuner::SimInterface::GetFreezeFlags(bool& aAltitude,
                                           bool& aSpeed,
                                           bool& aPosition,
                                           bool& aFuel,
                                           bool& aRoll,
                                           bool& aPitch,
                                           bool& aYaw)
{
   QMutexLocker locker(&mMutex);

   bool retVal = false;
   if (mSimData.simDataValid)
   {
      aAltitude = mSimData.freezeFlags.at(FRZ_ALTITUDE);
      aSpeed    = mSimData.freezeFlags.at(FRZ_SPEED);
      aPosition = mSimData.freezeFlags.at(FRZ_POSITION);
      aFuel     = mSimData.freezeFlags.at(FRZ_FUEL);
      aRoll     = mSimData.freezeFlags.at(FRZ_ROLL);
      aPitch    = mSimData.freezeFlags.at(FRZ_PITCH);
      aYaw      = mSimData.freezeFlags.at(FRZ_YAW);
      retVal    = true;
   }
   return retVal;
}

std::list<double> WkTuner::SimInterface::GetControllingValueList(int aPlatformId, P6DOF::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   std::list<double> retVal;

   if (mSimData.platformMap.count(aPlatformId) > 0 && mSimData.platformMap.at(aPlatformId).pidGains.count(aPid) > 0)
   {
      for (std::map<double, P6DOF::PidGainData>::const_iterator iter =
              mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).begin();
           iter != mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end();
           ++iter)
      {
         retVal.push_back(iter->first);
      }
   }

   return retVal;
}

double WkTuner::SimInterface::GetCurrentControllingValue(int aPlatformId)
{
   QMutexLocker locker(&mMutex);

   double retVal = 0.0;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      double speed_fps = mSimData.platformMap.at(aPlatformId).speed_fps;
      double density   = mSimData.platformMap.at(aPlatformId).airDensityAtCurrentAlt;

      // Calculate Q from density of the air and speed of the platform
      retVal = 0.5 * density * speed_fps * speed_fps;
   }
   return retVal;
}

bool WkTuner::SimInterface::GetOutputValuesForPid(int              aPlatformId,
                                                  P6DOF::Pid::Type aPid,
                                                  double&          aP,
                                                  double&          aI,
                                                  double&          aD,
                                                  double&          aFeedForward)
{
   QMutexLocker locker(&mMutex);

   bool retVal = false;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const P6DOF::SinglePidValueData& pidValue = GetPID_Value(aPid, mSimData.platformMap.at(aPlatformId).pidValues);
      aP                                        = pidValue.KpValue;
      aI                                        = pidValue.KiValue;
      aD                                        = pidValue.KdValue;
      aFeedForward                              = pidValue.FFValue;
      retVal                                    = true;
   }
   return retVal;
}

void WkTuner::SimInterface::GetInputValuesForPid(int              aPlatformId,
                                                 P6DOF::Pid::Type aPid,
                                                 double           aControllingValue,
                                                 double&          aP,
                                                 double&          aI,
                                                 double&          aD)
{
   QMutexLocker locker(&mMutex);
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).find(aControllingValue) !=
          mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end())
   {
      const P6DOF::PidGainData& gains = mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).at(aControllingValue);
      aP                              = gains.KpGain;
      aI                              = gains.KiGain;
      aD                              = gains.KdGain;
   }
   else
   {
      aP = 0;
      aI = 0;
      aD = 0;
   }
}

std::map<double, P6DOF::PidGainData> WkTuner::SimInterface::GetInputValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   std::map<double, P6DOF::PidGainData> retVal;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end())
   {
      retVal = mSimData.platformMap.at(aPlatformId).pidGains.at(aPid);
   }

   return retVal;
}

WkTuner::SimInterface::PID_Flags WkTuner::SimInterface::GetFlagsForPid(int aPlatformId, P6DOF::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   PID_Flags retVal = PID_Flags();
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidFlags.find(aPid) != mSimData.platformMap.at(aPlatformId).pidFlags.end())
   {
      retVal = mSimData.platformMap.at(aPlatformId).pidFlags.at(aPid);
   }

   return retVal;
}

bool WkTuner::SimInterface::GetStateValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid, double& aCurrentValue, double& aSetPoint)
{
   QMutexLocker locker(&mMutex);

   bool retVal = false;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const P6DOF::SinglePidValueData& pidValue = GetPID_Value(aPid, mSimData.platformMap.at(aPlatformId).pidValues);
      aCurrentValue                             = pidValue.CurrentValue;
      aSetPoint                                 = pidValue.SetPoint;
      retVal                                    = true;
   }
   return retVal;
}

void WkTuner::SimInterface::GetMaxAccumulationValues(int              aPlatformId,
                                                     P6DOF::Pid::Type aPid,
                                                     double           aControllingValue,
                                                     bool&            aUseMaxAccum,
                                                     double&          aMaxAccum)
{
   QMutexLocker locker(&mMutex);
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).find(aControllingValue) !=
          mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end() &&
       mSimData.platformMap.at(aPlatformId).pidFlags.find(aPid) != mSimData.platformMap.at(aPlatformId).pidFlags.end())
   {
      aUseMaxAccum = mSimData.platformMap.at(aPlatformId).pidFlags.at(aPid).limitMax;
      aMaxAccum    = mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).at(aControllingValue).MaxAccum;
   }
}

std::map<int, std::string> WkTuner::SimInterface::GetAndClearNewPlatformMap()
{
   std::map<int, std::string> retVal;

   QMutexLocker locker(&mMutex);

   retVal = mNewPlatformMap;
   mNewPlatformMap.clear();

   return retVal;
}

std::map<int, std::string> WkTuner::SimInterface::GetAndClearDeletedPlatformMap()
{
   std::map<int, std::string> retVal;

   QMutexLocker locker(&mMutex);

   retVal = mDeletedPlatformMap;
   mDeletedPlatformMap.clear();

   return retVal;
}

void WkTuner::SimInterface::GetRelevantLimits(int              aPlatformId,
                                              P6DOF::Pid::Type aPid,
                                              double&          aLowerLimit,
                                              double&          aUpperLimit,
                                              std::string&     aUnitName)
{
   P6DOF::AutopilotLimitsAndSettings limits = GetLimitsAndSettings(aPlatformId);

   switch (aPid)
   {
   case P6DOF::Pid::Type::Alpha:
      aLowerLimit = limits.alpha_Min;
      aUpperLimit = limits.alpha_Max;
      aUnitName   = "degrees";
      break;

   case P6DOF::Pid::VertSpeed:
      aLowerLimit = limits.vertSpd_Min;
      aUpperLimit = limits.vertSpd_Max;
      aUnitName   = "ft/s";
      break;

   case P6DOF::Pid::FltPathAngle:
   case P6DOF::Pid::PitchRate:
      aLowerLimit = limits.pitchRate_Min;
      aUpperLimit = limits.pitchRate_Max;
      aUnitName   = "deg/s";
      break;

   case P6DOF::Pid::Beta:
      aLowerLimit = limits.beta_Max;
      aUpperLimit = 0;
      aUnitName   = "degrees";
      break;

   case P6DOF::Pid::YawRate:
      aLowerLimit = limits.yawRate_Max;
      aUpperLimit = 0;
      aUnitName   = "deg/s";
      break;

   case P6DOF::Pid::RollRate:
      aLowerLimit = limits.rollRate_Max;
      aUpperLimit = 0;
      aUnitName   = "deg/s";
      break;

   case P6DOF::Pid::BankAngle:
      aLowerLimit = limits.bankAngle_Max;
      aUpperLimit = 0;
      aUnitName   = "degrees";
      break;

   case P6DOF::Pid::TaxiSpeed:
      aLowerLimit = limits.taxiSpeed_Max;
      aUpperLimit = 0;
      aUnitName   = "m/s";
      break;
   case P6DOF::Pid::TaxiYawRate:
      aLowerLimit = limits.taxiYawRate_Max;
      aUpperLimit = 0;
      aUnitName   = "deg/s";
      break;
   default:
      aLowerLimit = 0;
      aUpperLimit = 0;
      break;
   }
}

P6DOF::AutopilotLimitsAndSettings WkTuner::SimInterface::GetLimitsAndSettings(int aPlatformId)
{
   P6DOF::AutopilotLimitsAndSettings retVal;

   QMutexLocker locker(&mMutex);

   const auto iter = mSimData.platformMap.find(aPlatformId);
   if (iter != mSimData.platformMap.end())
   {
      retVal = iter->second.limits;
   }

   return retVal;
}

P6DOF::Control::Method WkTuner::SimInterface::GetControlMethod(int aPlatformId)
{
   P6DOF::Control::Method retVal = P6DOF::Control::Undefined;

   QMutexLocker locker(&mMutex);

   const auto iter = mSimData.platformMap.find(aPlatformId);
   if (iter != mSimData.platformMap.end())
   {
      retVal = iter->second.controlMethod;
   }

   return retVal;
}

bool WkTuner::SimInterface::IsDataValid()
{
   QMutexLocker locker(&mMutex);
   return mSimData.simDataValid;
}

bool WkTuner::SimInterface::IsSimPaused()
{
   QMutexLocker locker(&mMutex);
   return mSimData.isPaused;
}

bool WkTuner::SimInterface::GetRollPitchYaw(int aPlatformId, double& aRoll_deg, double& aPitch_deg, double& aYaw_deg)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aRoll_deg            = plat.roll_deg;
      aPitch_deg           = plat.pitch_deg;
      aYaw_deg             = plat.beta_deg;
      retVal               = true;
   }
   return retVal;
}

bool WkTuner::SimInterface::GetPosition(int aPlatformId, double& aLatitude_deg, double& aLongitude_deg, double& aAltitude_ft)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aLatitude_deg        = plat.latitude_deg;
      aLongitude_deg       = plat.longitude_deg;
      aAltitude_ft         = plat.altitude_ft;
      retVal               = true;
   }
   return retVal;
}

bool WkTuner::SimInterface::GetHeading(int aPlatformId, double& aHeading_deg)
{
   bool retVal = false;

   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aHeading_deg         = plat.heading_deg;
      retVal               = true;
   }
   return retVal;
}

bool WkTuner::SimInterface::GetSpeed(int aPlatformId, double& aSpeed_fps)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aSpeed_fps           = plat.speed_fps;
      retVal               = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetThrustVectorAngle(int aPlatformId, double& aThrustVectorPitch_deg)
{
   bool retVal = false;

   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat   = mSimData.platformMap.at(aPlatformId);
      aThrustVectorPitch_deg = plat.thrustPitchVector_deg;
      retVal                 = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetEngineThrottleLeverMilitary(int aPlatformId, double& aEngineThrottleLeverMilitary)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat         = mSimData.platformMap.at(aPlatformId);
      aEngineThrottleLeverMilitary = plat.engineThrottleLeverMilitary;
      retVal                       = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetEngineThrottleLeverAfterburner(int aPlatformId, double& aEngineThrottleLeverAfterburner)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat            = mSimData.platformMap.at(aPlatformId);
      aEngineThrottleLeverAfterburner = plat.engineThrottleLeverAfterburner;
      retVal                          = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetEngineThrust_lbs(int aPlatformId, double& aEngineThrust_lbs)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aEngineThrust_lbs    = plat.engineThrust_lbs;
      retVal               = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetAfterburnerOn(int aPlatformId, bool& aAfterburnerOn)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aAfterburnerOn       = plat.afterburnerOn;
      retVal               = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetPlatformCrashed(int aPlatformId, bool& aCrashed)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const Platform& plat = mSimData.platformMap.at(aPlatformId);
      aCrashed             = plat.crashed;
      retVal               = true;
   }

   return retVal;
}

bool WkTuner::SimInterface::GetRelevantPids(int aPlatformId, std::vector<P6DOF::Pid::Type>& aPidVector)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      // Longitudinal control PIDs
      aPidVector.push_back(P6DOF::Pid::Type::Alpha);
      aPidVector.push_back(P6DOF::Pid::Type::VertSpeed);
      aPidVector.push_back(P6DOF::Pid::Type::Altitude);
      aPidVector.push_back(P6DOF::Pid::Type::PitchAngle);
      aPidVector.push_back(P6DOF::Pid::Type::DeltaPitch);
      aPidVector.push_back(P6DOF::Pid::Type::PitchRate);
      aPidVector.push_back(P6DOF::Pid::Type::FltPathAngle);

      // Thrust control PIDs
      aPidVector.push_back(P6DOF::Pid::Type::Speed);

      // Yaw control PIDs, some of which depend on control mode
      aPidVector.push_back(P6DOF::Pid::Type::Beta);
      aPidVector.push_back(P6DOF::Pid::Type::YawRate);
      if (mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::BankToTurnWithYaw ||
          mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::YawToTurnNoRoll ||
          mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::YawToTurnRollRate ||
          mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::YawToTurnZeroBank)
      {
         aPidVector.push_back(P6DOF::Pid::Type::YawHeading);
      }

      // Roll control PIDs, some of which depend on control mode
      aPidVector.push_back(P6DOF::Pid::Type::RollRate);
      aPidVector.push_back(P6DOF::Pid::Type::BankAngle);
      if (mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::BankToTurnNoYaw ||
          mSimData.platformMap[aPlatformId].controlMethod == P6DOF::Control::BankToTurnWithYaw)
      {
         aPidVector.push_back(P6DOF::Pid::Type::DeltaRoll);
         aPidVector.push_back(P6DOF::Pid::Type::RollHeading);
      }

      // These PIDs are only relevant if the vehicle can actually land
      if (mSimData.platformMap[aPlatformId].hasLandingGear)
      {
         aPidVector.push_back(P6DOF::Pid::Type::TaxiYawRate);
         aPidVector.push_back(P6DOF::Pid::Type::TaxiHeading);
         aPidVector.push_back(P6DOF::Pid::Type::TaxiSpeed);
      }
      retVal = true;
   }
   return retVal;
}

void WkTuner::SimInterface::GetOptimumSpecimen(P6DofTunerGASpecimen& aOptimum)
{
   QMutexLocker locker(&mMutex);
   aOptimum = mGui.guiAutoTuneCommand.optimum;
}

void WkTuner::SimInterface::SetPlatformOfInterest(int aPlatformId)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformId;
}

void WkTuner::SimInterface::SetPidOfInterest(P6DOF::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);
   mPidOfInterest = aPid;
}

P6DOF::Pid::Type WkTuner::SimInterface::GetPidOfInterest()
{
   QMutexLocker locker(&mMutex);
   return mPidOfInterest;
}

void WkTuner::SimInterface::GetPidLog(std::vector<P6DofTunerGASpecimen::PidLogElement>& aPidDataVector)
{
   QMutexLocker locker(&mMutex);
   aPidDataVector = mPidLog;
}

void WkTuner::SimInterface::ClearPidLog()
{
   QMutexLocker locker(&mMutex);
   mPidLog.clear();
}

const P6DOF::SinglePidValueData& WkTuner::SimInterface::GetPID_Value(P6DOF::Pid::Type                         aPid,
                                                                     const P6DOF::AutopilotPidGroupValueData& aPID_Values) const
{
   switch (aPid)
   {
   case P6DOF::Pid::Alpha:
      return aPID_Values.alphaPID;
   case P6DOF::Pid::VertSpeed:
      return aPID_Values.vertSpeedPID;
   case P6DOF::Pid::PitchAngle:
      return aPID_Values.pitchAnglePID;
   case P6DOF::Pid::PitchRate:
      return aPID_Values.pitchRatePID;
   case P6DOF::Pid::FltPathAngle:
      return aPID_Values.fltpathAnglePID;
   case P6DOF::Pid::DeltaPitch:
      return aPID_Values.deltaPitchPID;
   case P6DOF::Pid::Altitude:
      return aPID_Values.altitudePID;
   case P6DOF::Pid::Beta:
      return aPID_Values.betaPID;
   case P6DOF::Pid::YawRate:
      return aPID_Values.yawRatePID;
   case P6DOF::Pid::YawHeading:
      return aPID_Values.yawheadingPID;
   case P6DOF::Pid::TaxiHeading:
      return aPID_Values.taxiHeadingPID;
   case P6DOF::Pid::RollRate:
      return aPID_Values.rollRatePID;
   case P6DOF::Pid::DeltaRoll:
      return aPID_Values.deltaRollPID;
   case P6DOF::Pid::BankAngle:
      return aPID_Values.bankAnglePID;
   case P6DOF::Pid::RollHeading:
      return aPID_Values.rollHeadingPID;
   case P6DOF::Pid::ForwardAccel:
      return aPID_Values.forwardAccelPID;
   case P6DOF::Pid::Speed:
      return aPID_Values.speedPID;
   // case P6DOF::Pid::TaxiForwardAccel:
   // return aPID_Values.taxiForwardAccelPID;
   case P6DOF::Pid::TaxiSpeed:
      return aPID_Values.taxiSpeedPID;
   case P6DOF::Pid::TaxiYawRate:
      return aPID_Values.taxiYawRatePID;
   default:
      // Return alpha PID as default
      return aPID_Values.alphaPID;
   }
}

void WkTuner::SimInterface::LogPidOfInterest(const P6DOF::AutopilotPidGroupValueData& aPID_Values,
                                             double                                   aSimTime,
                                             const P6DofVehicle*                      aObject)
{
   P6DofTunerGASpecimen::PidLogElement temp;
   temp.pidValues = GetPID_Value(mPidOfInterest, aPID_Values);
   temp.timeStamp = aSimTime;

   mPidLog.push_back(temp);
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkTuner::SimInterface::AFSIM_SendCommandBasedOnPid(P6DOF::Pid::Type aPid, double aValue, P6DofPilotObject* aPilotObject)
{
   if (aPilotObject->GetCommonController() == nullptr)
   {
      return;
   }

   switch (aPid)
   {
   case P6DOF::Pid::Alpha:
      aPilotObject->SetAutopilotAlpha(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::VertSpeed:
      aPilotObject->SetAutopilotVerticalSpeed(
         UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cFEET_PER_MINUTE));
      break;

   case P6DOF::Pid::PitchAngle:
      aPilotObject->SetAutopilotPitchAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::PitchRate:
      aPilotObject->SetAutopilotPitchRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case P6DOF::Pid::FltPathAngle:
      aPilotObject->SetAutopilotFlightPathAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::DeltaPitch:
      aPilotObject->SetAutopilotDeltaPitch(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::Altitude:
      aPilotObject->SetAutopilotAltitude(UtUnitLength::ConvertFromStandard(aValue, UtUnitLength::BaseUnit::cFEET));
      break;

   case P6DOF::Pid::Beta:
      aPilotObject->SetAutopilotBeta(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::YawRate:
      aPilotObject->SetAutopilotYawRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case P6DOF::Pid::YawHeading:
      aPilotObject->SetAutopilotYawHeading(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::TaxiHeading:
      break;
   case P6DOF::Pid::RollRate:
      aPilotObject->SetAutopilotRollRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case P6DOF::Pid::DeltaRoll:
      aPilotObject->SetAutopilotDeltaRoll(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::BankAngle:
      aPilotObject->SetAutopilotRollAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case P6DOF::Pid::RollHeading:
      aPilotObject->SetAutopilotRollHeading(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   // case P6DOF::Pid::FORWARDACCEL:
   case P6DOF::Pid::Speed:
      aPilotObject->SetAutopilotSpeedKTAS(UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   // case P6DOF::Pid::TAXIFORWARDACCEL:
   case P6DOF::Pid::TaxiSpeed:
      break;
   case P6DOF::Pid::TaxiYawRate:
      break;
   default:
      break;
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkTuner::SimInterface::AFSIM_LoadPidGains(P6DofCommonController*                                            aPilot,
                                               std::map<P6DOF::Pid::Type, std::map<double, P6DOF::PidGainData>>& aPidGains,
                                               std::map<P6DOF::Pid::Type, PID_Flags>& aPidFlags)
{
   for (P6DOF::Pid::Type pidType = (P6DOF::Pid::Type)(1); pidType != P6DOF::Pid::LastPidType;
        pidType                  = (P6DOF::Pid::Type)(pidType + 1))
   {
      unsigned char                    flags;
      std::vector<P6DOF::PidGainData>* data = aPilot->GetAutopilotPidGainData(pidType, flags);

      if (data != nullptr)
      {
         for (std::vector<P6DOF::PidGainData>::const_iterator iter = data->begin(); iter != data->end(); ++iter)
         {
            aPidGains[pidType][iter->ControllingValue] = (*iter);
         }

         aPidFlags[pidType].useAlpha  = flags & P6DOF::Pid::UseAlpha;
         aPidFlags[pidType].limitMax  = flags & P6DOF::Pid::LimitMax;
         aPidFlags[pidType].zeroGtMax = flags & P6DOF::Pid::ZeroGtMax;
         aPidFlags[pidType].zeroLtMin = flags & P6DOF::Pid::ZeroLtMin;
         aPidFlags[pidType].useKt     = flags & P6DOF::Pid::UseKt;
      }
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkTuner::SimInterface::AFSIM_SendCustomCommand(Data::CustomCommand aCommandType,
                                                    double              aValue,
                                                    P6DofPilotObject*   aPilotObject)
{
   if (aPilotObject->GetCommonController() == nullptr)
   {
      return;
   }

   switch (aCommandType)
   {
   case Data::CustomCommand::KCAS:
      aPilotObject->SetAutopilotSpeedKCAS(UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   case Data::CustomCommand::KTAS:
      aPilotObject->SetAutopilotSpeedKTAS(UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   case Data::CustomCommand::Mach:
      // unitless, no conversion necessary
      aPilotObject->SetAutopilotSpeedMach(aValue);
      break;

   case Data::CustomCommand::PitchG:
      aPilotObject->SetPitchGLoad(aValue);
      break;

   case Data::CustomCommand::YawG:
      aPilotObject->SetAutopilotYawGLoad(aValue);
      break;

   default:
      break;
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkTuner::SimInterface::AFSIM_SendPrePositionCommand(double              aAltitude,
                                                         Data::CustomCommand aSpeedType,
                                                         double              aSpeed,
                                                         P6DofPilotObject*   aPilotObject)
{
   switch (aSpeedType)
   {
   case Data::KCAS:
      aPilotObject->SetPrePositionCAS(UtUnitLength::ConvertFromStandard(aAltitude, UtUnitLength::BaseUnit::cFEET),
                                      UtUnitSpeed::ConvertFromStandard(aSpeed, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   case Data::KTAS:
      aPilotObject->SetPrePositionTAS(UtUnitLength::ConvertFromStandard(aAltitude, UtUnitLength::BaseUnit::cFEET),
                                      UtUnitSpeed::ConvertFromStandard(aSpeed, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   case Data::Mach:
      aPilotObject->SetPrePositionMach(UtUnitLength::ConvertFromStandard(aAltitude, UtUnitLength::BaseUnit::cFEET), aSpeed);
      break;

   case Data::DynamicPressure:
      aPilotObject->SetPrePositionQ(UtUnitLength::ConvertFromStandard(aAltitude, UtUnitLength::BaseUnit::cFEET),
                                    UtUnitPressure::ConvertFromStandard(aSpeed, UtUnitPressure::BaseUnit::cPSF));
      break;

   default:
      break;
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkTuner::SimInterface::AFSIM_AutoTune(P6DofPilotObject* aPilotController, WsfSimulation& aSimulation)
{
   if (!mGui.guiAutoTuneCommand.running)
   {
      P6DofVehicle* vehicle = aPilotController->GetParentVehicle();
      if (vehicle)
      {
         // Disable crashes, which remove the vehicle from the sim and halt tuning
         vehicle->SetTestingIgnoreAllCrashes(true);
      }

      mGui.guiAutoTuneCommand.GAManager.InitializePopulation(mGui.guiAutoTuneCommand.parentSpecimen);
      mGui.guiAutoTuneCommand.running = true;

      emit AutoTuneMaximum((int)(mGui.guiAutoTuneCommand.GAManager.GetMaxGenerationCount() *
                                 mGui.guiAutoTuneCommand.GAManager.GetPopulationSize()));

      P6DofCommonController* autopilot = aPilotController->GetCommonController();
      if (autopilot != nullptr)
      {
         autopilot->SetPidGainKP(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKp());
         autopilot->SetPidGainKI(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKi());
         autopilot->SetPidGainKD(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKd());
      }
      AFSIM_SendPrePositionCommand(mGui.guiAutoTuneCommand.setPositionAltitude_m,
                                   mGui.guiAutoTuneCommand.setPositionSpeedType,
                                   mGui.guiAutoTuneCommand.setPositionSpeed,
                                   aPilotController);
      AFSIM_SendCommandBasedOnPid(mGui.guiAutoTuneCommand.tuningPid, mGui.guiAutoTuneCommand.commandVal, aPilotController);

      aSimulation.AddEvent(
         ut::make_unique<AdvanceTimeEvent>(aSimulation.GetSimTime() + mGui.guiAutoTuneCommand.runTime, true, this));
      aSimulation.SetRealtime(aSimulation.GetSimTime(), false);
      aSimulation.Resume();
   }

   if (mSimData.isPaused)
   {
      if (mGui.guiAutoTuneCommand.GAManager.IsComplete())
      {
         mGui.guiAutoTuneCommand.optimum = mGui.guiAutoTuneCommand.GAManager.GetOptimum();
         mGui.guiAutoTuneCommand.running = false;
         emit AutoTuneComplete((int)mGui.guiAutoTuneCommand.tuningPid);
         return;
      }

      emit AutoTuneProgress((int)(mGui.guiAutoTuneCommand.GAManager.GetProgress()));
      mGui.guiAutoTuneCommand.GAManager.Advance(mPidLog);
      mPidLog.clear();

      P6DofCommonController* autopilot = aPilotController->GetCommonController();
      if (autopilot != nullptr)
      {
         autopilot->SetPidGainKP(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKp());
         autopilot->SetPidGainKI(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKi());
         autopilot->SetPidGainKD(mGui.guiAutoTuneCommand.tuningPid,
                                 mGui.guiAutoTuneCommand.controllingValue,
                                 mGui.guiAutoTuneCommand.GAManager.GetCurrentSpecimen().GetKd());
      }
      AFSIM_SendPrePositionCommand(mGui.guiAutoTuneCommand.setPositionAltitude_m,
                                   mGui.guiAutoTuneCommand.setPositionSpeedType,
                                   mGui.guiAutoTuneCommand.setPositionSpeed,
                                   aPilotController);
      AFSIM_SendCommandBasedOnPid(mGui.guiAutoTuneCommand.tuningPid, mGui.guiAutoTuneCommand.commandVal, aPilotController);

      aSimulation.AddEvent(
         ut::make_unique<AdvanceTimeEvent>(aSimulation.GetSimTime() + mGui.guiAutoTuneCommand.runTime, true, this));
      aSimulation.SetRealtime(aSimulation.GetSimTime(), false);
      aSimulation.Resume();
   }
}

WsfEvent::EventDisposition WkTuner::SimInterface::AdvanceTimeEvent::Execute()
{
   WsfSimulation* simulationPtr = GetSimulation();
   simulationPtr->SetRealtime(GetTime(), true);

   if (mPauseOnExecute)
   {
      simulationPtr->Pause();
   }

   emit mSimInterface->AdvanceToTimeAchieved();

   return WsfEvent::cDELETE;
}

WsfEvent::EventDisposition WkTuner::WriteDataEvent::Execute()
{
   if (mSimInterface)
   {
      mSimInterface->WriteData(*GetSimulation());

      SetTime(GetTime() + .05); // 20hz
      return WsfEvent::cRESCHEDULE;
   }
   return WsfEvent::cDELETE;
}
