// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "TunerSimInterface.hpp"

#include <QMutexLocker>

#include "UtMath.hpp"
#include "UtUnitTypes.hpp"
#include "WsfClockSource.hpp"
#include "WsfMover.hpp"
#include "WsfPlatform.hpp"
#include "WsfRigidBodySixDOF_LandingGear.hpp"
#include "WsfRigidBodySixDOF_Mover.hpp"
#include "WsfRigidBodySixDOF_PropulsionSystem.hpp"
#include "WsfRigidBodySixDOF_ThrustProducerObject.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfSixDOF_CommonController.hpp"
#include "WsfSixDOF_Environment.hpp"
#include "WsfSixDOF_JetEngine.hpp"
#include "WsfSixDOF_Mover.hpp"
#include "WsfSixDOF_PilotObject.hpp"
#include "WsfSixDOF_Terrain.hpp"

WkSixDOF_Tuner::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceBase(aPluginName)
{
}

void WkSixDOF_Tuner::SimInterface::GuiUpdate()
{
   // Check for new platforms
   std::map<int, std::string> newMap = GetAndClearNewPlatformMap();
   for (auto& iter : newMap)
   {
      emit NewPlatform(iter.second, iter.first);
   }

   // Check for platforms that have been removed
   std::map<int, std::string> removeMap = GetAndClearDeletedPlatformMap();
   for (auto& iter : removeMap)
   {
      emit DeletedPlatform(iter.second, iter.first);
   }
}

void WkSixDOF_Tuner::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
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

         // If platform contains a SixDOF mover
         auto* mover = dynamic_cast<wsf::six_dof::Mover*>(platformPtr->GetMover());
         if (mover != nullptr)
         {
            if (!mFirstMoverFound)
            {
               // This is dirty, but it is a work-around to avoid rewriting the plugin to conform
               //  to the new SimInterface/SimEvents/SimCommands paradigm
               auto& simulation = const_cast<WsfSimulation&>(aSimulation);
               simulation.AddWallEvent(ut::make_unique<WriteDataEvent>(this));
               mFirstMoverFound = true;
            }

            Platform&          platformData = mSimData.platformMap[platformId];
            const std::string& platformName = platformPtr->GetName();

            platformData.alpha_deg      = mover->GetAlpha_deg();
            platformData.beta_deg       = mover->GetBeta_deg();
            platformData.heading_deg    = mover->GetHeading_deg();
            platformData.pitch_deg      = mover->GetPitch_deg();
            platformData.roll_deg       = mover->GetRoll_deg();
            platformData.crashed        = mover->Crashed();
            platformData.hasLandingGear = mover->HasLandingGear();

            if (mover->IsA_TypeOf("WSF_POINT_MASS_SIX_DOF_MOVER"))
            {
               platformData.type = PointMass;
            }
            else if (mover->IsA_TypeOf("WSF_RIGID_BODY_SIX_DOF_MOVER"))
            {
               platformData.type = RigidBody;
            }

            double alt_m;
            mover->GetLocationLLA(platformData.latitude_deg, platformData.longitude_deg, alt_m);
            platformData.altitude_ft = alt_m * UtMath::cFT_PER_M;
            platformData.speed_fps   = mover->GetSpeed_fps();

            if (mover->GetEnvironment() != nullptr)
            {
               platformData.airDensityAtCurrentAlt =
                  mover->GetEnvironment()->CalcAirDensity_slug_ft3(platformData.altitude_ft);
            }
            else
            {
               platformData.airDensityAtCurrentAlt = 0.0;
            }

            wsf::six_dof::PropulsionSystem* propulsionSystem = mover->GetPropulsionSystem();
            if (propulsionSystem != nullptr)
            {
               wsf::six_dof::ThrustProducerObject* engine = propulsionSystem->GetThrustProducerByIndex(0);
               if (engine != nullptr)
               {
                  auto sixdofEngine = dynamic_cast<wsf::six_dof::RigidBodyThrustProducerObject*>(engine);
                  if (sixdofEngine != nullptr)
                  {
                     platformData.thrustPitchVector_deg = sixdofEngine->GetThrustVectoringPitch_deg();
                  }
                  platformData.engineThrust_lbs = engine->GetForwardThrust_lbs();

                  auto* jetPtr = dynamic_cast<wsf::six_dof::JetEngine*>(engine);
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

            wsf::six_dof::PilotObject* pilot = mover->GetActivePilotObject();
            if (pilot != nullptr)
            {
               wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
               if (autopilot != nullptr)
               {
                  platformData.controlMethod = autopilot->GetControlMethod();
                  autopilot->GetAutopilotPidValues(&platformData.pidValues);
                  AFSIM_LoadPidGains(autopilot, platformData.pidGains, platformData.pidIntervals);
                  platformData.limits = autopilot->GetCurrentLimitsAndSettings();

                  if (mLogData && mPidOfInterest != wsf::six_dof::Pid::Unknown && platformId == mPlatformOfInterest)
                  {
                     LogPidOfInterest(platformData.pidValues, mSimData.simTime, mover);
                  }
               }
            }

            // If this is a new platform and it is an appropriate mover, add to the mNewPlatformMap
            if (newPlatform)
            {
               mNewPlatformMap[platformId] = platformName;
            }
         }
      }
      mMutex.unlock();
   }
}

void WkSixDOF_Tuner::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest   = -1;
   mSimData              = SimData();
   mSimData.initializing = true;
   mDiscoveredPlatforms.clear();
   mNewPlatformMap.clear();
   mDeletedPlatformMap.clear();
   mFirstMoverFound = false;
}

void WkSixDOF_Tuner::SimInterface::WriteData(WsfSimulation& aSimulation)
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
            // If platform contains a SixDOF mover
            auto* mover = dynamic_cast<wsf::six_dof::Mover*>(platformPtr->GetMover());
            if (mover != nullptr)
            {
               // Prevent crashing: object->SetTestingIgnoreAllCrashes(true);

               wsf::six_dof::PilotObject* pilot = mover->GetActivePilotObject();
               if (pilot != nullptr)
               {
                  // Check for pending Freeze commands to issue to the sim
                  for (auto& pendingFreezeCommand : mGui.mPendingFreezeCommands)
                  {
                     switch (pendingFreezeCommand.first)
                     {
                     case FRZ_ALTITUDE:
                        mover->GetFreezeFlags()->altitude = pendingFreezeCommand.second;
                        break;
                     case FRZ_SPEED:
                        mover->GetFreezeFlags()->speed = pendingFreezeCommand.second;
                        break;
                     case FRZ_POSITION:
                        mover->GetFreezeFlags()->location = pendingFreezeCommand.second;
                        break;
                     case FRZ_FUEL:
                        mover->GetFreezeFlags()->fuelBurn = pendingFreezeCommand.second;
                        break;
                     case FRZ_YAW:
                        mover->GetFreezeFlags()->yaw = pendingFreezeCommand.second;
                        break;
                     case FRZ_PITCH:
                        mover->GetFreezeFlags()->pitch = pendingFreezeCommand.second;
                        break;
                     case FRZ_ROLL:
                        mover->GetFreezeFlags()->roll = pendingFreezeCommand.second;
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

                     // Pre-position to the ground, but include the nominal offset height
                     if (mGui.taxiControl)
                     {
                        wsf::six_dof::RigidBodyMover* vehicle =
                           wsf::six_dof::utils::CastToRigidBody(pilot->GetParentVehicle());
                        if (vehicle != nullptr)
                        {
                           wsf::six_dof::RigidBodyLandingGear* gear = vehicle->GetLandingGear();
                           if (gear != nullptr)
                           {
                              // Put the gear down since we'll be taxiing
                              vehicle->ForceLandingGearDownInstantly("Landing_Gear_Extended");

                              double startLat, startLon;
                              vehicle->GetStartingLatLon(startLat, startLon);
                              double hgt_terrain_m = vehicle->GetEnvironment()->GetTerrainElevation(startLat, startLon);
                              double hgt_nominal_m = gear->GetNominalHeightAboveGroundOnGear();
                              double hgt_m         = hgt_terrain_m + hgt_nominal_m;
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
                     wsf::six_dof::RigidBodyMover* vehicle =
                        wsf::six_dof::utils::CastToRigidBody(pilot->GetParentVehicle());
                     if (vehicle != nullptr)
                     {
                        wsf::six_dof::RigidBodyLandingGear* gear = vehicle->GetLandingGear();
                        if (gear != nullptr)
                        {
                           // Put the gear down since we'll be taxiing
                           vehicle->ForceLandingGearUpInstantly("Landing_Gear_Extended");
                        }
                     }
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
                     mGui.p.pending                            = false;
                     wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
                     if (autopilot != nullptr)
                     {
                        autopilot->SetPidGainKP(mGui.p.pid, mGui.p.controllingValue, mGui.p.value);
                     }
                  }

                  if (mGui.i.pending)
                  {
                     mGui.i.pending                            = false;
                     wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
                     if (autopilot != nullptr)
                     {
                        autopilot->SetPidGainKI(mGui.i.pid, mGui.i.controllingValue, mGui.i.value);
                     }
                  }

                  if (mGui.d.pending)
                  {
                     mGui.d.pending                            = false;
                     wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
                     if (autopilot != nullptr)
                     {
                        autopilot->SetPidGainKD(mGui.d.pid, mGui.d.controllingValue, mGui.d.value);
                     }
                  }

                  if (mGui.alpha.pending)
                  {
                     mGui.alpha.pending                        = false;
                     wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
                     if (autopilot != nullptr)
                     {
                        autopilot->SetPidAlpha(mGui.alpha.pid, mGui.alpha.controllingValue, mGui.alpha.value);
                     }
                  }

                  if (mGui.maxAccum.pending)
                  {
                     mGui.maxAccum.pending                     = false;
                     wsf::six_dof::CommonController* autopilot = pilot->GetCommonController();
                     if (autopilot != nullptr)
                     {
                        if (mGui.maxAccumEnable)
                        {
                           autopilot->SetPidMaxAccum(mGui.maxAccum.pid, mGui.maxAccum.controllingValue, mGui.maxAccum.value);
                        }
                        else
                        {
                           autopilot->SetPidMaxAccum(mGui.maxAccum.pid,
                                                     mGui.maxAccum.controllingValue,
                                                     std::numeric_limits<float>::max());
                        }
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
      mMutex.unlock();
   }
}

void WkSixDOF_Tuner::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (mMutex.tryLock())
   {
      auto* mover = dynamic_cast<wsf::six_dof::Mover*>(aPlatform.GetMover());
      if (mover != nullptr)
      {
         mDeletedPlatformMap[(int)aPlatform.GetIndex()] = aPlatform.GetName();
      }
      mMutex.unlock();
   }
}

double WkSixDOF_Tuner::SimInterface::GetSimTime()
{
   QMutexLocker locker(&mMutex);
   return mSimData.simTime;
}

bool WkSixDOF_Tuner::SimInterface::GetSimInitializing()
{
   bool returnVal = false;

   QMutexLocker locker(&mMutex);
   returnVal             = mSimData.initializing;
   mSimData.initializing = false;
   return returnVal;
}

void WkSixDOF_Tuner::SimInterface::SetLogData(bool aVal)
{
   QMutexLocker locker(&mMutex);
   mLogData = aVal;
}

void WkSixDOF_Tuner::SimInterface::RunAutoTuneCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.pending = true;
}

void WkSixDOF_Tuner::SimInterface::InitializeAutoTuneCommand(int                     aPlatformId,
                                                             wsf::six_dof::Pid::Type aPid,
                                                             double                  aAltitude_m,
                                                             Data::CustomCommand     aSpeedType,
                                                             double                  aSpeed)
{
   wsf::six_dof::AutopilotLimitsAndSettings limitsAndSettings = GetLimitsAndSettings(aPlatformId);

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
   case wsf::six_dof::Pid::Alpha:
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
   case wsf::six_dof::Pid::VerticalSpeed:
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

   case wsf::six_dof::Pid::PitchAngle:
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

   case wsf::six_dof::Pid::PitchRate:
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

   case wsf::six_dof::Pid::FlightPathAngle:
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

   case wsf::six_dof::Pid::DeltaPitch:
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

   case wsf::six_dof::Pid::Altitude:
      mGui.guiAutoTuneCommand.commandVal = commandedAlt_m;
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case wsf::six_dof::Pid::Beta:
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

   case wsf::six_dof::Pid::YawRate:
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

   case wsf::six_dof::Pid::YawHeading:
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

   case wsf::six_dof::Pid::TaxiHeading:
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, false));
      break;

   case wsf::six_dof::Pid::RollRate:
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

   case wsf::six_dof::Pid::DeltaRoll:
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

   case wsf::six_dof::Pid::BankAngle:
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

   case wsf::six_dof::Pid::RollHeading:
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

   case wsf::six_dof::Pid::Speed:
      mGui.guiAutoTuneCommand.commandVal = commandedSpeed;
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case wsf::six_dof::Pid::TaxiSpeed:
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ALTITUDE, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_SPEED, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_POSITION, false));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_FUEL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_ROLL, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_PITCH, true));
      mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(FRZ_YAW, true));
      break;

   case wsf::six_dof::Pid::TaxiYawRate:
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

void WkSixDOF_Tuner::SimInterface::CancelAutoTuneCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.pending = false;
   mGui.guiAutoTuneCommand.running = false;
}

void WkSixDOF_Tuner::SimInterface::SetAutoTunePopSize(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetPopulationSize(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneMaxGenerations(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetMaxGenerationCount(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneCarryOverFrac(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetCarryOverFraction(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneCrossoverProb(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetCrossOverProbability(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneMutationProb(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.GAManager.SetMutationProbability(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneKpRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKpRange(aMin, aMax);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneKiRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKiRange(aMin, aMax);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneKdRange(double aMin, double aMax)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetKdRange(aMin, aMax);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneRunTime(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.runTime = aVal;
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneErrorWeight(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetErrorWeight(aVal);
}

void WkSixDOF_Tuner::SimInterface::SetAutoTuneOvershootWeight(double aVal)
{
   QMutexLocker locker(&mMutex);
   mGui.guiAutoTuneCommand.parentSpecimen.SetOvershootWeight(aVal);
}

void WkSixDOF_Tuner::SimInterface::SendFreezeCommand(FreezeEnum aFreezeCommand, bool aEnabled)
{
   QMutexLocker locker(&mMutex);
   mGui.mPendingFreezeCommands.emplace_back(std::pair<FreezeEnum, bool>(aFreezeCommand, aEnabled));
}

void WkSixDOF_Tuner::SimInterface::SendSetPositionCommand(double aAltitude_m, Data::CustomCommand aSpeedType, double aSpeed)
{
   QMutexLocker locker(&mMutex);
   mGui.setPositionPending    = true;
   mGui.setPositionAltitude_m = aAltitude_m;
   mGui.setPositionSpeedType  = aSpeedType;
   mGui.setPositionSpeed      = aSpeed;
}

void WkSixDOF_Tuner::SimInterface::SendTaxiCommand(bool aTaxiControl)
{
   QMutexLocker locker(&mMutex);
   mGui.taxiControlPending = true;
   mGui.taxiControl        = aTaxiControl;
}

void WkSixDOF_Tuner::SimInterface::SendCommandBasedOnPID(wsf::six_dof::Pid::Type aPID, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.command.pending = true;
   mGui.command.pid     = aPID;
   mGui.command.value   = value;
}

void WkSixDOF_Tuner::SimInterface::SendCustomCommand(Data::CustomCommand aCommandType, double aValue)
{
   QMutexLocker locker(&mMutex);
   mGui.customCommand.pending     = true;
   mGui.customCommand.commandType = aCommandType;
   mGui.customCommand.value       = aValue;
}

void WkSixDOF_Tuner::SimInterface::SendPValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double value)
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
      wsf::six_dof::PidGainData* gains =
         &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KpGain = value;
   }
}

void WkSixDOF_Tuner::SimInterface::SendIValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double value)
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
      wsf::six_dof::PidGainData* gains =
         &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KiGain = value;
   }
}

void WkSixDOF_Tuner::SimInterface::SendDValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double value)
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
      wsf::six_dof::PidGainData* gains =
         &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->KdGain = value;
   }
}

void WkSixDOF_Tuner::SimInterface::SendAlphaValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double value)
{
   QMutexLocker locker(&mMutex);
   mGui.alpha.pending          = true;
   mGui.alpha.pid              = aPID;
   mGui.alpha.controllingValue = aControllingValue;
   mGui.alpha.value            = value;

   if (mSimData.platformMap.find(mPlatformOfInterest) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.find(aPID) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.end() &&
       mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).find(aControllingValue) !=
          mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).end())
   {
      wsf::six_dof::PidGainData* gains =
         &mSimData.platformMap.at(mPlatformOfInterest).pidGains.at(aPID).at(aControllingValue);
      gains->LowpassAlpha = value;
   }
}

void WkSixDOF_Tuner::SimInterface::SendMaxAccumulation(wsf::six_dof::Pid::Type aPID,
                                                       bool                    aEnabled,
                                                       double                  aControllingValue,
                                                       double                  aValue)
{
   QMutexLocker locker(&mMutex);
   mGui.maxAccum.pending          = true;
   mGui.maxAccum.pid              = aPID;
   mGui.maxAccumEnable            = aEnabled;
   mGui.maxAccum.controllingValue = aControllingValue;
   mGui.maxAccum.value            = aValue;
}

void WkSixDOF_Tuner::SimInterface::SendRunCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.runCommandPending = true;
}

void WkSixDOF_Tuner::SimInterface::SendPauseCommand()
{
   QMutexLocker locker(&mMutex);
   mGui.pauseCommandPending = true;
}

void WkSixDOF_Tuner::SimInterface::SendAdvanceToTimeCommand(double aTime, bool aRealTimeFlag)
{
   mGui.advanceToTimeCommandPending = true;
   mGui.advanceRealTime             = aRealTimeFlag;
   mGui.advanceTime                 = aTime;
}

void WkSixDOF_Tuner::SimInterface::SendSimRateCommand(double aSimClockRate)
{
   QMutexLocker locker(&mMutex);
   mGui.simRateCommandPending = true;
   mGui.simRate               = aSimClockRate;
}

bool WkSixDOF_Tuner::SimInterface::GetFreezeFlags(bool& aAltitude,
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

std::list<double> WkSixDOF_Tuner::SimInterface::GetControllingValueList(int aPlatformId, wsf::six_dof::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   std::list<double> retVal;

   if (mSimData.platformMap.count(aPlatformId) > 0 && mSimData.platformMap.at(aPlatformId).pidGains.count(aPid) > 0)
   {
      for (std::map<double, wsf::six_dof::PidGainData>::const_iterator iter =
              mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).begin();
           iter != mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end();
           ++iter)
      {
         retVal.push_back(iter->first);
      }
   }

   return retVal;
}

double WkSixDOF_Tuner::SimInterface::GetCurrentControllingValue(int aPlatformId)
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

bool WkSixDOF_Tuner::SimInterface::GetOutputValuesForPid(int                     aPlatformId,
                                                         wsf::six_dof::Pid::Type aPid,
                                                         double&                 aP,
                                                         double&                 aI,
                                                         double&                 aD,
                                                         double&                 aFeedForward)
{
   QMutexLocker locker(&mMutex);

   bool retVal = false;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const wsf::six_dof::SinglePidValueData& pidValue =
         GetPID_Value(aPid, mSimData.platformMap.at(aPlatformId).pidValues);
      aP           = pidValue.KpValue;
      aI           = pidValue.KiValue;
      aD           = pidValue.KdValue;
      aFeedForward = pidValue.FFValue;
      retVal       = true;
   }
   return retVal;
}

void WkSixDOF_Tuner::SimInterface::GetInputValuesForPid(int                     aPlatformId,
                                                        wsf::six_dof::Pid::Type aPid,
                                                        double                  aControllingValue,
                                                        double&                 aP,
                                                        double&                 aI,
                                                        double&                 aD,
                                                        double&                 aLowpassAlpha)
{
   QMutexLocker locker(&mMutex);
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).find(aControllingValue) !=
          mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end())
   {
      const wsf::six_dof::PidGainData& gains =
         mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).at(aControllingValue);
      aP            = gains.KpGain;
      aI            = gains.KiGain;
      aD            = gains.KdGain;
      aLowpassAlpha = gains.LowpassAlpha;
   }
   else
   {
      aP            = 0;
      aI            = 0;
      aD            = 0;
      aLowpassAlpha = 0;
   }
}

std::map<double, wsf::six_dof::PidGainData> WkSixDOF_Tuner::SimInterface::GetInputValuesForPid(int aPlatformId,
                                                                                               wsf::six_dof::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   std::map<double, wsf::six_dof::PidGainData> retVal;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end())
   {
      retVal = mSimData.platformMap.at(aPlatformId).pidGains.at(aPid);
   }

   return retVal;
}

double WkSixDOF_Tuner::SimInterface::GetUpdateIntervalForPid(int aPlatformId, wsf::six_dof::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);

   double retVal = 0.0;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidIntervals.find(aPid) !=
          mSimData.platformMap.at(aPlatformId).pidIntervals.end())
   {
      retVal = mSimData.platformMap.at(aPlatformId).pidIntervals.at(aPid);
   }

   return retVal;
}

bool WkSixDOF_Tuner::SimInterface::GetStateValuesForPid(int                     aPlatformId,
                                                        wsf::six_dof::Pid::Type aPid,
                                                        double&                 aCurrentValue,
                                                        double&                 aSetPoint)
{
   QMutexLocker locker(&mMutex);

   bool retVal = false;
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      const wsf::six_dof::SinglePidValueData& pidValue =
         GetPID_Value(aPid, mSimData.platformMap.at(aPlatformId).pidValues);
      aCurrentValue = pidValue.CurrentValue;
      aSetPoint     = pidValue.SetPoint;
      retVal        = true;
   }
   return retVal;
}

void WkSixDOF_Tuner::SimInterface::GetMaxAccumulationValues(int                     aPlatformId,
                                                            wsf::six_dof::Pid::Type aPid,
                                                            double                  aControllingValue,
                                                            double&                 aMaxAccum)
{
   QMutexLocker locker(&mMutex);
   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.find(aPid) != mSimData.platformMap.at(aPlatformId).pidGains.end() &&
       mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).find(aControllingValue) !=
          mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).end())
   {
      aMaxAccum = mSimData.platformMap.at(aPlatformId).pidGains.at(aPid).at(aControllingValue).MaxAccum;
   }
}

std::map<int, std::string> WkSixDOF_Tuner::SimInterface::GetAndClearNewPlatformMap()
{
   std::map<int, std::string> retVal;

   QMutexLocker locker(&mMutex);

   retVal = mNewPlatformMap;
   mNewPlatformMap.clear();

   return retVal;
}

std::map<int, std::string> WkSixDOF_Tuner::SimInterface::GetAndClearDeletedPlatformMap()
{
   std::map<int, std::string> retVal;

   QMutexLocker locker(&mMutex);

   retVal = mDeletedPlatformMap;
   mDeletedPlatformMap.clear();

   return retVal;
}

void WkSixDOF_Tuner::SimInterface::GetRelevantLimits(int                     aPlatformId,
                                                     wsf::six_dof::Pid::Type aPid,
                                                     double&                 aLowerLimit,
                                                     double&                 aUpperLimit,
                                                     std::string&            aUnitName)
{
   wsf::six_dof::AutopilotLimitsAndSettings limits = GetLimitsAndSettings(aPlatformId);

   switch (aPid)
   {
   case wsf::six_dof::Pid::Type::Alpha:
      aLowerLimit = limits.alpha_Min;
      aUpperLimit = limits.alpha_Max;
      aUnitName   = "degrees";
      break;

   case wsf::six_dof::Pid::VerticalSpeed:
      aLowerLimit = limits.vertSpd_Min;
      aUpperLimit = limits.vertSpd_Max;
      aUnitName   = "ft/s";
      break;

   case wsf::six_dof::Pid::PitchRate:
      aLowerLimit = limits.pitchRate_Min;
      aUpperLimit = limits.pitchRate_Max;
      aUnitName   = "deg/s";
      break;

   case wsf::six_dof::Pid::Beta:
      aLowerLimit = limits.beta_Max;
      aUpperLimit = 0;
      aUnitName   = "degrees";
      break;

   case wsf::six_dof::Pid::YawRate:
      aLowerLimit = limits.yawRate_Max;
      aUpperLimit = 0;
      aUnitName   = "deg/s";
      break;

   case wsf::six_dof::Pid::RollRate:
      aLowerLimit = limits.rollRate_Max;
      aUpperLimit = 0;
      aUnitName   = "deg/s";
      break;

   case wsf::six_dof::Pid::BankAngle:
      aLowerLimit = limits.bankAngle_Max;
      aUpperLimit = 0;
      aUnitName   = "degrees";
      break;

   case wsf::six_dof::Pid::TaxiSpeed:
      aLowerLimit = limits.taxiSpeed_Max;
      aUpperLimit = 0;
      aUnitName   = "m/s";
      break;
   case wsf::six_dof::Pid::TaxiYawRate:
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

wsf::six_dof::AutopilotLimitsAndSettings WkSixDOF_Tuner::SimInterface::GetLimitsAndSettings(int aPlatformId)
{
   wsf::six_dof::AutopilotLimitsAndSettings retVal;

   QMutexLocker locker(&mMutex);

   const auto iter = mSimData.platformMap.find(aPlatformId);
   if (iter != mSimData.platformMap.end())
   {
      retVal = iter->second.limits;
   }

   return retVal;
}

wsf::six_dof::Control::Method WkSixDOF_Tuner::SimInterface::GetControlMethod(int aPlatformId)
{
   wsf::six_dof::Control::Method retVal = wsf::six_dof::Control::Undefined;

   QMutexLocker locker(&mMutex);

   const auto iter = mSimData.platformMap.find(aPlatformId);
   if (iter != mSimData.platformMap.end())
   {
      retVal = iter->second.controlMethod;
   }

   return retVal;
}

WkSixDOF_Tuner::SimInterface::VehicleType WkSixDOF_Tuner::SimInterface::GetType(int aPlatformId)
{
   VehicleType retVal = VehicleType::Unknown;

   QMutexLocker locker(&mMutex);

   const auto iter = mSimData.platformMap.find(aPlatformId);
   if (iter != mSimData.platformMap.end())
   {
      retVal = iter->second.type;
   }

   return retVal;
}

bool WkSixDOF_Tuner::SimInterface::IsDataValid()
{
   QMutexLocker locker(&mMutex);
   return mSimData.simDataValid;
}

bool WkSixDOF_Tuner::SimInterface::IsSimPaused()
{
   QMutexLocker locker(&mMutex);
   return mSimData.isPaused;
}

bool WkSixDOF_Tuner::SimInterface::GetRollPitchYaw(int aPlatformId, double& aRoll_deg, double& aPitch_deg, double& aYaw_deg)
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

bool WkSixDOF_Tuner::SimInterface::GetPosition(int aPlatformId, double& aLatitude_deg, double& aLongitude_deg, double& aAltitude_ft)
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

bool WkSixDOF_Tuner::SimInterface::GetHeading(int aPlatformId, double& aHeading_deg)
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

bool WkSixDOF_Tuner::SimInterface::GetSpeed(int aPlatformId, double& aSpeed_fps)
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

bool WkSixDOF_Tuner::SimInterface::GetThrustVectorAngle(int aPlatformId, double& aThrustVectorPitch_deg)
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

bool WkSixDOF_Tuner::SimInterface::GetEngineThrottleLeverMilitary(int aPlatformId, double& aEngineThrottleLeverMilitary)
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

bool WkSixDOF_Tuner::SimInterface::GetEngineThrottleLeverAfterburner(int aPlatformId, double& aEngineThrottleLeverAfterburner)
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

bool WkSixDOF_Tuner::SimInterface::GetEngineThrust_lbs(int aPlatformId, double& aEngineThrust_lbs)
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

bool WkSixDOF_Tuner::SimInterface::GetAfterburnerOn(int aPlatformId, bool& aAfterburnerOn)
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

bool WkSixDOF_Tuner::SimInterface::GetPlatformCrashed(int aPlatformId, bool& aCrashed)
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

bool WkSixDOF_Tuner::SimInterface::GetRelevantPids(int aPlatformId, std::vector<wsf::six_dof::Pid::Type>& aPidVector)
{
   bool         retVal = false;
   QMutexLocker locker(&mMutex);

   if (mSimData.platformMap.find(aPlatformId) != mSimData.platformMap.end())
   {
      // Some PIDs are only used for rigid-body controls.
      // Point-mass vehicles do not use PIDs for the inner loop stick controls, and
      // use the pitch and bank angle PIDs for delta pitch and roll control.
      VehicleType moverType = mSimData.platformMap[aPlatformId].type;

      // Longitudinal control PIDs
      if (moverType == VehicleType::RigidBody)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::Alpha);
      }
      aPidVector.push_back(wsf::six_dof::Pid::Type::VerticalSpeed);
      aPidVector.push_back(wsf::six_dof::Pid::Type::Altitude);
      aPidVector.push_back(wsf::six_dof::Pid::Type::PitchAngle);
      if (moverType == VehicleType::RigidBody)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::DeltaPitch);
      }
      aPidVector.push_back(wsf::six_dof::Pid::Type::PitchRate);
      aPidVector.push_back(wsf::six_dof::Pid::Type::FlightPathAngle);

      // Thrust control PIDs
      aPidVector.push_back(wsf::six_dof::Pid::Type::Speed);

      // Yaw control PIDs, some of which depend on control mode
      if (moverType == VehicleType::RigidBody)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::Beta);
      }
      aPidVector.push_back(wsf::six_dof::Pid::Type::YawRate);
      if (mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::BankToTurnWithYaw ||
          mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::YawToTurnNoRoll ||
          mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::YawToTurnRollRate ||
          mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::YawToTurnZeroBank)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::YawHeading);
      }

      // Roll control PIDs, some of which depend on control mode
      if (moverType == VehicleType::RigidBody)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::RollRate);
      }
      aPidVector.push_back(wsf::six_dof::Pid::Type::BankAngle);
      if (mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::BankToTurnNoYaw ||
          mSimData.platformMap[aPlatformId].controlMethod == wsf::six_dof::Control::BankToTurnWithYaw)
      {
         if (moverType == VehicleType::RigidBody)
         {
            aPidVector.push_back(wsf::six_dof::Pid::Type::DeltaRoll);
         }
         aPidVector.push_back(wsf::six_dof::Pid::Type::RollHeading);
      }

      // These PIDs are only relevant if the vehicle can actually land
      if (mSimData.platformMap[aPlatformId].hasLandingGear)
      {
         aPidVector.push_back(wsf::six_dof::Pid::Type::TaxiYawRate);
         aPidVector.push_back(wsf::six_dof::Pid::Type::TaxiHeading);
         aPidVector.push_back(wsf::six_dof::Pid::Type::TaxiSpeed);
      }
      retVal = true;
   }
   return retVal;
}

void WkSixDOF_Tuner::SimInterface::GetOptimumSpecimen(wsf::six_dof::TunerGASpecimen& aOptimum)
{
   QMutexLocker locker(&mMutex);
   aOptimum = mGui.guiAutoTuneCommand.optimum;
}

void WkSixDOF_Tuner::SimInterface::SetPlatformOfInterest(int aPlatformId)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformId;
}

void WkSixDOF_Tuner::SimInterface::SetPidOfInterest(wsf::six_dof::Pid::Type aPid)
{
   QMutexLocker locker(&mMutex);
   mPidOfInterest = aPid;
}

wsf::six_dof::Pid::Type WkSixDOF_Tuner::SimInterface::GetPidOfInterest()
{
   QMutexLocker locker(&mMutex);
   return mPidOfInterest;
}

void WkSixDOF_Tuner::SimInterface::GetPidLog(std::vector<wsf::six_dof::TunerGASpecimen::PidLogElement>& aPidDataVector)
{
   QMutexLocker locker(&mMutex);
   aPidDataVector = mPidLog;
}

void WkSixDOF_Tuner::SimInterface::ClearPidLog()
{
   QMutexLocker locker(&mMutex);
   mPidLog.clear();
}

const wsf::six_dof::SinglePidValueData&
WkSixDOF_Tuner::SimInterface::GetPID_Value(wsf::six_dof::Pid::Type                         aPid,
                                           const wsf::six_dof::AutopilotPidGroupValueData& aPID_Values) const
{
   switch (aPid)
   {
   case wsf::six_dof::Pid::Alpha:
      return aPID_Values.alphaPID;
   case wsf::six_dof::Pid::VerticalSpeed:
      return aPID_Values.verticalSpeedPID;
   case wsf::six_dof::Pid::PitchAngle:
      return aPID_Values.pitchAnglePID;
   case wsf::six_dof::Pid::PitchRate:
      return aPID_Values.pitchRatePID;
   case wsf::six_dof::Pid::FlightPathAngle:
      return aPID_Values.flightPathAnglePID;
   case wsf::six_dof::Pid::DeltaPitch:
      return aPID_Values.deltaPitchPID;
   case wsf::six_dof::Pid::Altitude:
      return aPID_Values.altitudePID;
   case wsf::six_dof::Pid::Beta:
      return aPID_Values.betaPID;
   case wsf::six_dof::Pid::YawRate:
      return aPID_Values.yawRatePID;
   case wsf::six_dof::Pid::YawHeading:
      return aPID_Values.yawheadingPID;
   case wsf::six_dof::Pid::TaxiHeading:
      return aPID_Values.taxiHeadingPID;
   case wsf::six_dof::Pid::RollRate:
      return aPID_Values.rollRatePID;
   case wsf::six_dof::Pid::DeltaRoll:
      return aPID_Values.deltaRollPID;
   case wsf::six_dof::Pid::BankAngle:
      return aPID_Values.bankAnglePID;
   case wsf::six_dof::Pid::RollHeading:
      return aPID_Values.rollHeadingPID;
   case wsf::six_dof::Pid::ForwardAccel:
      return aPID_Values.forwardAccelPID;
   case wsf::six_dof::Pid::Speed:
      return aPID_Values.speedPID;
   // case wsf::six_dof::Pid::TaxiForwardAccel:
   // return aPID_Values.taxiForwardAccelPID;
   case wsf::six_dof::Pid::TaxiSpeed:
      return aPID_Values.taxiSpeedPID;
   case wsf::six_dof::Pid::TaxiYawRate:
      return aPID_Values.taxiYawRatePID;
   default:
      // Return alpha PID as default
      return aPID_Values.alphaPID;
   }
}

void WkSixDOF_Tuner::SimInterface::LogPidOfInterest(const wsf::six_dof::AutopilotPidGroupValueData& aPID_Values,
                                                    double                                          aSimTime,
                                                    const wsf::six_dof::Mover*                      aObject)
{
   wsf::six_dof::TunerGASpecimen::PidLogElement temp;
   temp.pidValues = GetPID_Value(mPidOfInterest, aPID_Values);
   temp.timeStamp = aSimTime;

   mPidLog.push_back(temp);
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkSixDOF_Tuner::SimInterface::AFSIM_SendCommandBasedOnPid(wsf::six_dof::Pid::Type    aPid,
                                                               double                     aValue,
                                                               wsf::six_dof::PilotObject* aPilotObject)
{
   if (aPilotObject->GetCommonController() == nullptr)
   {
      return;
   }

   switch (aPid)
   {
   case wsf::six_dof::Pid::Alpha:
      aPilotObject->SetAutopilotAlpha(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::VerticalSpeed:
      aPilotObject->SetAutopilotVerticalSpeed(
         UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cFEET_PER_MINUTE));
      break;

   case wsf::six_dof::Pid::PitchAngle:
      aPilotObject->SetAutopilotPitchAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::PitchRate:
      aPilotObject->SetAutopilotPitchRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case wsf::six_dof::Pid::FlightPathAngle:
      aPilotObject->SetAutopilotFlightPathAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::DeltaPitch:
      aPilotObject->SetAutopilotDeltaPitch(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::Altitude:
      aPilotObject->SetAutopilotAltitude(UtUnitLength::ConvertFromStandard(aValue, UtUnitLength::BaseUnit::cFEET));
      break;

   case wsf::six_dof::Pid::Beta:
      aPilotObject->SetAutopilotBeta(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::YawRate:
      aPilotObject->SetAutopilotYawRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case wsf::six_dof::Pid::YawHeading:
      aPilotObject->SetAutopilotYawHeading(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::TaxiHeading:
      break;
   case wsf::six_dof::Pid::RollRate:
      aPilotObject->SetAutopilotRollRate(
         UtUnitAngularRate::ConvertFromStandard(aValue, UtUnitAngularRate::BaseUnit::cDEGREES_PER_SECOND));
      break;

   case wsf::six_dof::Pid::DeltaRoll:
      aPilotObject->SetAutopilotDeltaRoll(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::BankAngle:
      aPilotObject->SetAutopilotRollAngle(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   case wsf::six_dof::Pid::RollHeading:
      aPilotObject->SetAutopilotRollHeading(UtUnitAngle::ConvertFromStandard(aValue, UtUnitAngle::BaseUnit::cDEGREES));
      break;

   // case wsf::six_dof::Pid::FORWARDACCEL:
   case wsf::six_dof::Pid::Speed:
      aPilotObject->SetAutopilotSpeedKTAS(UtUnitSpeed::ConvertFromStandard(aValue, UtUnitSpeed::BaseUnit::cKNOTS));
      break;

   // case wsf::six_dof::Pid::TAXIFORWARDACCEL:
   case wsf::six_dof::Pid::TaxiSpeed:
      break;
   case wsf::six_dof::Pid::TaxiYawRate:
      break;
   default:
      break;
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkSixDOF_Tuner::SimInterface::AFSIM_LoadPidGains(
   wsf::six_dof::CommonController*                                                 aPilot,
   std::map<wsf::six_dof::Pid::Type, std::map<double, wsf::six_dof::PidGainData>>& aPidGains,
   std::map<wsf::six_dof::Pid::Type, double>&                                      aPidIntervals)
{
   for (wsf::six_dof::Pid::Type pidType = (wsf::six_dof::Pid::Type)(1); pidType != wsf::six_dof::Pid::LastPidType;
        pidType                         = (wsf::six_dof::Pid::Type)(pidType + 1))
   {
      std::vector<wsf::six_dof::PidGainData>* data = aPilot->GetAutopilotPidGainData(pidType);

      if (data != nullptr)
      {
         aPidIntervals[pidType] = aPilot->GetPID_ByType(pidType)->GetUpdateInterval_sec();

         for (std::vector<wsf::six_dof::PidGainData>::const_iterator iter = data->begin(); iter != data->end(); ++iter)
         {
            aPidGains[pidType][iter->ControllingValue] = (*iter);
         }
      }
   }
}

// This function is designed to execute on the Simulation thread only! There is no mutex locking within this function
// as it is suppose to be a helper function for other functions executing on the Simulation thread, those functions
// are responsible for locking the mutex prior to calling this function.
void WkSixDOF_Tuner::SimInterface::AFSIM_SendCustomCommand(Data::CustomCommand        aCommandType,
                                                           double                     aValue,
                                                           wsf::six_dof::PilotObject* aPilotObject)
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
void WkSixDOF_Tuner::SimInterface::AFSIM_SendPrePositionCommand(double                     aAltitude,
                                                                Data::CustomCommand        aSpeedType,
                                                                double                     aSpeed,
                                                                wsf::six_dof::PilotObject* aPilotObject)
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
void WkSixDOF_Tuner::SimInterface::AFSIM_AutoTune(wsf::six_dof::PilotObject* aPilotController, WsfSimulation& aSimulation)
{
   if (!mGui.guiAutoTuneCommand.running)
   {
      wsf::six_dof::RigidBodyMover* vehicle = wsf::six_dof::utils::CastToRigidBody(aPilotController->GetParentVehicle());
      if (vehicle)
      {
         // Disable crashes, which remove the vehicle from the sim and halt tuning
         vehicle->SetTestingIgnoreAllCrashes(true);
      }

      mGui.guiAutoTuneCommand.GAManager.InitializePopulation(mGui.guiAutoTuneCommand.parentSpecimen);
      mGui.guiAutoTuneCommand.running = true;

      emit AutoTuneMaximum((int)(mGui.guiAutoTuneCommand.GAManager.GetMaxGenerationCount() *
                                 mGui.guiAutoTuneCommand.GAManager.GetPopulationSize()));

      wsf::six_dof::CommonController* autopilot = aPilotController->GetCommonController();
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

      wsf::six_dof::CommonController* autopilot = aPilotController->GetCommonController();
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

WsfEvent::EventDisposition WkSixDOF_Tuner::SimInterface::AdvanceTimeEvent::Execute()
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

WsfEvent::EventDisposition WkSixDOF_Tuner::WriteDataEvent::Execute()
{
   if (mSimInterface)
   {
      mSimInterface->WriteData(*GetSimulation());

      SetTime(GetTime() + .05); // 20hz
      return WsfEvent::cRESCHEDULE;
   }
   return WsfEvent::cDELETE;
}
