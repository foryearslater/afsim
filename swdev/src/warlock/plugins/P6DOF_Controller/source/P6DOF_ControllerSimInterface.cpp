// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "P6DOF_ControllerSimInterface.hpp"

#include "P6DofFlightControlSystem.hpp"
#include "P6DofGroundReactionPoint.hpp"
#include "P6DofGuidanceAutopilot.hpp"
#include "P6DofHardwareAutopilot.hpp"
#include "P6DofJetEngineObject.hpp"
#include "P6DofLandingGear.hpp"
#include "P6DofPilotManager.hpp"
#include "P6DofPilotObject.hpp"
#include "P6DofPropulsionSystem.hpp"
#include "P6DofSyntheticPilot.hpp"
#include "P6DofVehicle.hpp"
#include "UtCast.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Rcvr.hpp"
#include "WsfFieldOfView.hpp"
#include "WsfGeometricSensor.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfMover.hpp"
#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorBeam.hpp"
#include "WsfSensorMode.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"
#include "WsfTrackProcessor.hpp"
#include "WsfWeapon.hpp"
#include "script/WsfScriptArticulatedPartClass.hpp"

WkP6DOF_Controller::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<P6DOF_ControllerEvent>(aPluginName)
{
}

void WkP6DOF_Controller::SimInterface::SetConnectedPlatform(const std::string& aPlatformName)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mConnectedPlatform   = aPlatformName;
   mConnectionDestroyed = false;
}

void WkP6DOF_Controller::SimInterface::ReleaseConnectedPlatform()
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mConnectedPlatform   = "";
   mConnectionDestroyed = true;
}

void WkP6DOF_Controller::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mConnectedPlatform   = "";
   mConnectionDestroyed = false;

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>());
}

void WkP6DOF_Controller::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   // Verify that we have a P6DOF mover
   WsfP6DOF_Mover* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(aPlatform.GetMover());
   if (p6dofMover != nullptr)
   {
      AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
   }
}

void WkP6DOF_Controller::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   if (aPlatform.GetName() == mConnectedPlatform)
   {
      mConnectionDestroyed = true;
      mConnectedPlatform   = "";
      AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform.GetName()));
   }
}

void WkP6DOF_Controller::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   // Setup a temporary platform data structure to load with data
   P6DOF_ControllerDataContainer::PlatformData tempPlatData;

   if (!mConnectedPlatform.empty() && !mConnectionDestroyed)
   {
      WsfPlatform* platform = aSimulation.GetPlatformByName(mConnectedPlatform);
      if (nullptr != platform)
      {
         // Note:  What happens if platform->IsBroken() ?
         //        Should we mark mConnectionDestroyed as true here?

         WsfMover* mover = platform->GetMover();
         if (nullptr != mover)
         {
            if (mover->IsA_TypeOf("WSF_P6DOF_MOVER"))
            {
               WsfP6DOF_Mover* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(mover);

               if (p6dofMover != nullptr)
               {
                  tempPlatData.simTime = aSimulation.GetSimTime();

                  P6DofVehicle* vehicle = p6dofMover->GetP6DofVehicle();
                  if (vehicle != nullptr)
                  {
                     tempPlatData.heading_deg    = vehicle->GetHeading_deg();
                     tempPlatData.pitch_deg      = vehicle->GetPitch_deg();
                     tempPlatData.roll_deg       = vehicle->GetRoll_deg();
                     tempPlatData.yawRate_dps    = vehicle->GetYawRate_dps();
                     tempPlatData.pitchRate_dps  = vehicle->GetPitchRate_dps();
                     tempPlatData.rollRate_dps   = vehicle->GetRollRate_dps();
                     tempPlatData.alpha_deg      = vehicle->GetAlpha_deg();
                     tempPlatData.beta_deg       = vehicle->GetBeta_deg();
                     tempPlatData.alphaDot_dps   = vehicle->GetAlphaDot_dps();
                     tempPlatData.betaDot_dps    = vehicle->GetBetaDot_dps();
                     tempPlatData.lat_deg        = vehicle->GetLat();
                     tempPlatData.lon_deg        = vehicle->GetLon();
                     tempPlatData.altitudeMSL_ft = vehicle->GetAltitudeMSL_ft();
                     // Planned improvement: This is a placeholder until actual radar altitude is available
                     tempPlatData.radarAltitude_ft                  = vehicle->GetAltitudeMSL_ft();
                     tempPlatData.verticalSpeed_fpm                 = vehicle->GetVertSpeed_fpm();
                     tempPlatData.kcas                              = vehicle->GetKIAS();
                     tempPlatData.ktas                              = vehicle->GetKTAS();
                     tempPlatData.mach                              = vehicle->GetMach();
                     tempPlatData.internalTotalFuelTankCapacity_lbs = vehicle->GetInternalFuelTankCapacity_lbs();
                     tempPlatData.totalFuelTankCapacity_lbs         = vehicle->GetTotalFuelTankCapacity_lbs();
                     tempPlatData.currentInternalTotalFuelTankQuantity_lbs =
                        vehicle->GetCurrentInternalFuelTankQuantity_lbs();
                     tempPlatData.currentTotalFuelTankQuantity_lbs = vehicle->GetCurrentTotalFuelTankQuantity_lbs();
                     // Planned improvement:
                     // tempPlatData.bingoFuelTankQuantity_lbs;
                     tempPlatData.currentWeight_lbs = vehicle->GetCurrentWeight_lbs();
                     tempPlatData.emptyWeight_lbs   = vehicle->GetEmptyWeight_lbs();
                     tempPlatData.nx_g              = vehicle->GetNx_g();
                     tempPlatData.ny_g              = vehicle->GetNy_g();
                     tempPlatData.nz_g              = vehicle->GetNz_g();
                     tempPlatData.lat               = vehicle->GetLat();
                     tempPlatData.lon               = vehicle->GetLon();

                     // Zero waypoint data (it may be filled later, but ensure the default values are cleared)
                     tempPlatData.waypoints.clear();
                     tempPlatData.numWaypoints          = 0;
                     tempPlatData.currentWaypoint       = 0;
                     tempPlatData.headingBugValid       = false;
                     tempPlatData.headingBug_deg        = 0.0;
                     tempPlatData.wayptDataValid        = false;
                     tempPlatData.distanceToWaypoint_nm = 0.0;
                     tempPlatData.timeToWaypoint_sec    = 0.0;

                     P6DofPilotManager* pilotMgr = vehicle->GetPilotManager();
                     if (pilotMgr != nullptr)
                     {
                        P6DofPilotObject* pilot = pilotMgr->GetActivePilot();
                        if (pilot != nullptr)
                        {
                           tempPlatData.stickBackControllerPosition   = pilot->GetStickBackControllerPosition();
                           tempPlatData.stickRightControllerPosition  = pilot->GetStickRightControllerPosition();
                           tempPlatData.rudderRightControllerPosition = pilot->GetRudderRightControllerPosition();
                           tempPlatData.throttleMilitaryControllerPosition =
                              pilot->GetThrottleMilitaryControllerPosition();
                           tempPlatData.throttleAfterburnerControllerPosition =
                              pilot->GetThrottleAfterburnerControllerPosition();
                           tempPlatData.speedBrakesControllerPosition = pilot->GetSpeedBrakesControllerPosition();
                           tempPlatData.landingGearControllerPosition = pilot->GetLandingGearControllerPosition();
                           tempPlatData.flapsControllerPosition       = pilot->GetFlapsControllerPosition();
                           tempPlatData.spoilersControllerPosition    = pilot->GetSpoilersControllerPosition();

                           tempPlatData.pilotType = pilot->GetPilotType();

                           P6DofFlightControlSystem* flightControlSystem = vehicle->GetFlightControls();
                           if (flightControlSystem != nullptr)
                           {
                              size_t stabLftHandle =
                                 flightControlSystem->GetControlSurfaceHandle("Horizontal_Stab_Left");
                              size_t stabRgtHandle =
                                 flightControlSystem->GetControlSurfaceHandle("Horizontal_Stab_Right");
                              size_t rudderHandle = flightControlSystem->GetControlSurfaceHandle("Vertical_Tail");
                              size_t rudderLftHandle =
                                 flightControlSystem->GetControlSurfaceHandle("Vertical_Tail_Left");
                              size_t rudderRgtHandle =
                                 flightControlSystem->GetControlSurfaceHandle("Vertical_Tail_Right");
                              size_t aileronLftHandle = flightControlSystem->GetControlSurfaceHandle("Aileron_Left");
                              size_t aileronRgtHandle = flightControlSystem->GetControlSurfaceHandle("Aileron_Right");
                              size_t flapLftHandle    = flightControlSystem->GetControlSurfaceHandle("Flap_Left");
                              size_t flapRgtHandle    = flightControlSystem->GetControlSurfaceHandle("Flap_Right");
                              size_t spoilerLftHandle = flightControlSystem->GetControlSurfaceHandle("Spoiler_Left");
                              size_t spoilerRgtHandle = flightControlSystem->GetControlSurfaceHandle("Spoiler_Right");

                              // Find the handle of the first control surface that contains "SpeedBrake"
                              size_t speedBrakesHandle =
                                 flightControlSystem->GetControlSurfaceHandleContainingString("SpeedBrake");

                              // Find the handle of the first control surface that contains "LandingGear"
                              size_t landingGearHandle = flightControlSystem->GetControlSurfaceHandle("LandingGear");

                              size_t noseWheelAngleHandle =
                                 flightControlSystem->GetControlSurfaceHandle("Nose_Wheel_Angle_Rgt");
                              size_t nwsAngleHandle = flightControlSystem->GetControlSurfaceHandle("NWS_Angle_Rgt");
                              size_t wheelBrakeRgtHandle =
                                 flightControlSystem->GetControlValueHandle("Wheel_Brake_Rgt");
                              size_t wheelBrakeLftHandle =
                                 flightControlSystem->GetControlValueHandle("Wheel_Brake_Lft");

                              // Planned improvement: remove trigger and button, which are obsolete
                              // size_t triggerPulledHandle    =
                              // flightControlSystem->GetBooleanControlHandle("TriggerPulled"); size_t
                              // buttonPressedHandle    = flightControlSystem->GetBooleanControlHandle("ButtonPressed");

                              tempPlatData.stabLeft_deg = flightControlSystem->GetControlSurfaceAngle_deg(stabLftHandle);
                              tempPlatData.stabRight_deg = flightControlSystem->GetControlSurfaceAngle_deg(stabRgtHandle);
                              tempPlatData.rudder_deg = flightControlSystem->GetControlSurfaceAngle_deg(rudderHandle);
                              tempPlatData.rudderLft_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(rudderLftHandle);
                              tempPlatData.rudderRgt_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(rudderRgtHandle);
                              tempPlatData.aileronLft_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(aileronLftHandle);
                              tempPlatData.aileronRgt_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(aileronRgtHandle);
                              tempPlatData.flapLft_deg = flightControlSystem->GetControlSurfaceAngle_deg(flapLftHandle);
                              tempPlatData.flapRgt_deg = flightControlSystem->GetControlSurfaceAngle_deg(flapRgtHandle);
                              tempPlatData.spoilerLft_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(spoilerLftHandle);
                              tempPlatData.spoilerRgt_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(spoilerRgtHandle);
                              tempPlatData.landingGear_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(landingGearHandle);
                              tempPlatData.speedBrakes_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(speedBrakesHandle);
                              tempPlatData.noseWheel_deg =
                                 flightControlSystem->GetControlSurfaceAngle_deg(noseWheelAngleHandle);
                              tempPlatData.nws_angle_deg = flightControlSystem->GetControlSurfaceAngle_deg(nwsAngleHandle);
                              tempPlatData.wheelBrakeRight = flightControlSystem->GetControlValue(wheelBrakeRgtHandle);
                              tempPlatData.wheelBrakeLeft  = flightControlSystem->GetControlValue(wheelBrakeLftHandle);

                              if (rudderLftHandle != 0)
                              {
                                 tempPlatData.twinRuddersValid = true;
                              }
                              if (rudderHandle != 0)
                              {
                                 tempPlatData.singleRudderValid = true;
                              }
                              if (aileronLftHandle != 0)
                              {
                                 tempPlatData.aileronsValid = true;
                              }
                              if (flapLftHandle != 0)
                              {
                                 tempPlatData.flapsValid = true;
                              }
                              if (spoilerLftHandle != 0)
                              {
                                 tempPlatData.spoilersValid = true;
                              }
                              if (speedBrakesHandle != 0)
                              {
                                 tempPlatData.speedbrakesValid = true;
                              }

                              // Planned improvement: remove trigger and button, which are obsolete
                              // tempPlatData.triggerPulled       =
                              // flightControlSystem->GetControlBoolean(triggerPulledHandle); tempPlatData.buttonPressed
                              // = flightControlSystem->GetControlBoolean(buttonPressedHandle);
                           }

                           // Landing gear data
                           P6DofLandingGear* gear = vehicle->GetLandingGear();
                           if (gear != nullptr)
                           {
                              P6DofGroundReactionPoint* noseGear  = gear->GetNoseGear();
                              P6DofGroundReactionPoint* rightGear = gear->GetRightMainGear();
                              P6DofGroundReactionPoint* leftGear  = gear->GetLeftMainGear();

                              if (noseGear != nullptr)
                              {
                                 tempPlatData.noseGearSteeringAngle_deg = noseGear->GetSteeringAngle_deg();
                              }
                              else
                              {
                                 tempPlatData.noseGearSteeringAngle_deg = 0.0;
                              }

                              tempPlatData.nwsEnabled            = gear->NWS_IsEnabled();
                              tempPlatData.weightOnWheelsFlag    = gear->WeightOnWheels();
                              tempPlatData.weightOnNoseWheelFlag = gear->WeightOnNoseWheel();

                              double uncompressedLength_ft  = 0.0;
                              double maximumCompression_ft  = 0.0;
                              double currentCompression_ft  = 0.0;
                              double normalizedBrakingValue = 0.0;
                              double steeringAngle_deg      = 0.0;
                              bool   inContactWithGround    = false;
                              bool   maxCompressionExceeded = false;

                              if (noseGear != nullptr)
                              {
                                 noseGear->GetCompressionData(uncompressedLength_ft,
                                                              maximumCompression_ft,
                                                              currentCompression_ft,
                                                              normalizedBrakingValue,
                                                              steeringAngle_deg,
                                                              inContactWithGround,
                                                              maxCompressionExceeded);

                                 tempPlatData.noseGearUncompressedLength_ft  = uncompressedLength_ft;
                                 tempPlatData.noseGearMaximumCompression_ft  = maximumCompression_ft;
                                 tempPlatData.noseGearCurrentCompression_ft  = currentCompression_ft;
                                 tempPlatData.noseGearNormalizedBrakingValue = normalizedBrakingValue;
                                 tempPlatData.noseGearSteeringAngle_deg      = steeringAngle_deg;
                                 tempPlatData.noseGearInContactWithGround    = inContactWithGround;
                                 tempPlatData.noseGearMaxCompressionExceeded = maxCompressionExceeded;
                              } // if (noseGear != 0)

                              if (leftGear != nullptr)
                              {
                                 uncompressedLength_ft  = 0.0;
                                 maximumCompression_ft  = 0.0;
                                 currentCompression_ft  = 0.0;
                                 normalizedBrakingValue = 0.0;
                                 steeringAngle_deg      = 0.0;
                                 inContactWithGround    = false;
                                 maxCompressionExceeded = false;

                                 leftGear->GetCompressionData(uncompressedLength_ft,
                                                              maximumCompression_ft,
                                                              currentCompression_ft,
                                                              normalizedBrakingValue,
                                                              steeringAngle_deg,
                                                              inContactWithGround,
                                                              maxCompressionExceeded);

                                 tempPlatData.leftGearUncompressedLength_ft  = uncompressedLength_ft;
                                 tempPlatData.leftGearMaximumCompression_ft  = maximumCompression_ft;
                                 tempPlatData.leftGearCurrentCompression_ft  = currentCompression_ft;
                                 tempPlatData.leftGearNormalizedBrakingValue = normalizedBrakingValue;
                                 tempPlatData.leftGearSteeringAngle_deg      = steeringAngle_deg;
                                 tempPlatData.leftGearInContactWithGround    = inContactWithGround;
                                 tempPlatData.leftGearMaxCompressionExceeded = maxCompressionExceeded;
                              } // if (leftGear != 0)

                              if (rightGear != nullptr)
                              {
                                 uncompressedLength_ft  = 0.0;
                                 maximumCompression_ft  = 0.0;
                                 currentCompression_ft  = 0.0;
                                 normalizedBrakingValue = 0.0;
                                 steeringAngle_deg      = 0.0;
                                 inContactWithGround    = false;
                                 maxCompressionExceeded = false;

                                 rightGear->GetCompressionData(uncompressedLength_ft,
                                                               maximumCompression_ft,
                                                               currentCompression_ft,
                                                               normalizedBrakingValue,
                                                               steeringAngle_deg,
                                                               inContactWithGround,
                                                               maxCompressionExceeded);

                                 tempPlatData.rightGearUncompressedLength_ft  = uncompressedLength_ft;
                                 tempPlatData.rightGearMaximumCompression_ft  = maximumCompression_ft;
                                 tempPlatData.rightGearCurrentCompression_ft  = currentCompression_ft;
                                 tempPlatData.rightGearNormalizedBrakingValue = normalizedBrakingValue;
                                 tempPlatData.rightGearSteeringAngle_deg      = steeringAngle_deg;
                                 tempPlatData.rightGearInContactWithGround    = inContactWithGround;
                                 tempPlatData.rightGearMaxCompressionExceeded = maxCompressionExceeded;
                              } // if (rightGear != 0)
                           }
                           else
                           {
                              // No gear
                              tempPlatData.noseGearSteeringAngle_deg = 0.0;
                              tempPlatData.nwsEnabled                = false;
                              tempPlatData.weightOnWheelsFlag        = false;
                              tempPlatData.weightOnNoseWheelFlag     = false;

                              tempPlatData.noseGearUncompressedLength_ft  = 0.0;
                              tempPlatData.noseGearMaximumCompression_ft  = 0.0;
                              tempPlatData.noseGearCurrentCompression_ft  = 0.0;
                              tempPlatData.noseGearNormalizedBrakingValue = 0.0;
                              tempPlatData.noseGearSteeringAngle_deg      = 0.0;
                              tempPlatData.noseGearInContactWithGround    = false;
                              tempPlatData.noseGearMaxCompressionExceeded = false;

                              tempPlatData.leftGearUncompressedLength_ft  = 0.0;
                              tempPlatData.leftGearMaximumCompression_ft  = 0.0;
                              tempPlatData.leftGearCurrentCompression_ft  = 0.0;
                              tempPlatData.leftGearNormalizedBrakingValue = 0.0;
                              tempPlatData.leftGearSteeringAngle_deg      = 0.0;
                              tempPlatData.leftGearInContactWithGround    = false;
                              tempPlatData.leftGearMaxCompressionExceeded = false;

                              tempPlatData.rightGearUncompressedLength_ft  = 0.0;
                              tempPlatData.rightGearMaximumCompression_ft  = 0.0;
                              tempPlatData.rightGearCurrentCompression_ft  = 0.0;
                              tempPlatData.rightGearNormalizedBrakingValue = 0.0;
                              tempPlatData.rightGearSteeringAngle_deg      = 0.0;
                              tempPlatData.rightGearInContactWithGround    = false;
                              tempPlatData.rightGearMaxCompressionExceeded = false;
                           }

                        } // if (pilot != 0)
                     }    // if (pilotMgr != 0)

                     // Propulsion system data
                     P6DofPropulsionSystem* propulsionSystem = vehicle->GetPropulsionSystem();
                     if (propulsionSystem != nullptr)
                     {
                        P6DofThrustProducerObject* engine = propulsionSystem->GetThrustProducerByIndex(0);
                        if (engine != nullptr)
                        {
                           tempPlatData.thrustVectorYaw_deg   = engine->GetThrustVectoringYaw_deg();
                           tempPlatData.thrustVectorPitch_deg = engine->GetThrustVectoringPitch_deg();
                           tempPlatData.engineThrust_lbs      = engine->GetForwardThrust_lbs();
                           tempPlatData.engineFuelFlow_pph    = engine->GetFuelBurnRate_pph();

                           P6DofJetEngineObject* jetPtr = dynamic_cast<P6DofJetEngineObject*>(engine);
                           if (jetPtr != nullptr)
                           {
                              tempPlatData.afterburnerOn                  = jetPtr->AfterburnerOn();
                              tempPlatData.engineThrottleLeverMilitary    = jetPtr->GetEnginePercentRPM() * 0.01;
                              tempPlatData.engineThrottleLeverAfterburner = jetPtr->GetNozzlePosition();
                           }
                           else
                           {
                              tempPlatData.afterburnerOn                  = false;
                              tempPlatData.engineThrottleLeverMilitary    = 0.0;
                              tempPlatData.engineThrottleLeverAfterburner = 0.0;
                           } // if (jetPtr != 0)
                        }    // if (engine != 0)
                     }       // if (propulsionSystem != 0)

                     // Get waypoint, navigation, and autopilot data

                     if (pilotMgr != nullptr)
                     {
                        P6DofPilotObject* pilot = pilotMgr->GetActivePilot();
                        if (pilot != nullptr)
                        {
                           P6DOF::AutopilotData apData;
                           pilot->GetAutopilotData(&apData);

                           double aimHdg_deg =
                              apData.navWaypointParameters.navWaypointAimHeading_rad * UtMath::cDEG_PER_RAD;

                           // First, check for temp waypoints. If present, use those. Otherwise,
                           // check for planned waypoints. If present, use those. Otherwise we have no way points.
                           P6DOF::AutopilotWaypointDataList autopilotWaypoints;
                           bool                             waypointListValid = false;

                           // Check for temp waypoints
                           pilot->GetTempWaypoints(&autopilotWaypoints);
                           if (autopilotWaypoints.waypointListNumItems != 0)
                           {
                              waypointListValid = true;
                           }
                           else
                           {
                              // We lack a temp way point list, so use the planned list, if available
                              pilot->GetPlannedWaypoints(&autopilotWaypoints);
                              if (autopilotWaypoints.waypointListNumItems != 0)
                              {
                                 waypointListValid = true;
                              }
                           }

                           if (waypointListValid)
                           {
                              int    currentIndex    = autopilotWaypoints.currentWaypointIndex;
                              double currentIndexLat = 0.0;
                              double currentIndexLon = 0.0;

                              for (int i = 0; i < autopilotWaypoints.waypointListNumItems; ++i)
                              {
                                 P6DOF_ControllerDataContainer::WaypointData wayPt;
                                 wayPt.lat   = autopilotWaypoints.waypointListData[i].lat;
                                 wayPt.lon   = autopilotWaypoints.waypointListData[i].lon;
                                 wayPt.alt_m = autopilotWaypoints.waypointListData[i].alt_m;
                                 tempPlatData.waypoints.push_back(wayPt);

                                 if (i == currentIndex)
                                 {
                                    currentIndexLat = autopilotWaypoints.waypointListData[i].lat;
                                    currentIndexLon = autopilotWaypoints.waypointListData[i].lon;
                                 }
                              }
                              tempPlatData.numWaypoints    = autopilotWaypoints.waypointListNumItems;
                              tempPlatData.currentWaypoint = ut::cast_to_size_t(currentIndex);
                              tempPlatData.headingBugValid = true;
                              tempPlatData.headingBug_deg  = aimHdg_deg;

                              double heading_rad = 0.0;
                              double dist_m      = 0.0;
                              UtSphericalEarth::GreatCircleHeadingAndDistance(tempPlatData.lat,
                                                                              tempPlatData.lon,
                                                                              currentIndexLat,
                                                                              currentIndexLon,
                                                                              heading_rad,
                                                                              dist_m);

                              tempPlatData.wayptDataValid        = true;
                              tempPlatData.distanceToWaypoint_nm = dist_m * UtMath::cNM_PER_M;
                              tempPlatData.timeToWaypoint_sec =
                                 (tempPlatData.distanceToWaypoint_nm / tempPlatData.ktas) * 3600.0; // Convert to seconds
                           }
                        }
                     }

                     // Get sensor data
                     GetRadarData(aSimulation, platform, tempPlatData);
                     GetRWR_Data(aSimulation, platform, tempPlatData);
                     GetIRST_Data(aSimulation, platform, tempPlatData);
                     GetFLIR_Data(aSimulation, platform, tempPlatData);
                     GetTrackDataTrackManager(aSimulation, platform, tempPlatData);

                     // Current (Primary) Target
                     const WsfTrack* currentTargetTrack = platform->GetTrackManager().GetCurrentTarget();
                     if (currentTargetTrack != nullptr)
                     {
                        tempPlatData.currentTargetValid = true;
                        CalcTrackDataForTrack(aSimulation,
                                              platform,
                                              currentTargetTrack,
                                              tempPlatData.currentTarget,
                                              "ID_MASTER",
                                              true,
                                              true,
                                              true,
                                              true,
                                              true);
                     }
                     else
                     {
                        tempPlatData.currentTargetValid = false;
                     }

                     // Weapon data
                     tempPlatData.weaponList.clear();
                     WsfWeapon* weapon = nullptr;

                     weapon = platform->template GetComponent<WsfWeapon>("mrm");
                     if (weapon != nullptr)
                     {
                        P6DOF_ControllerDataContainer::WeaponData wpnData;
                        wpnData.weaponTypeName     = "mrm";
                        wpnData.displayName        = "120C";
                        wpnData.numRoundsRemaining = static_cast<int>(weapon->GetQuantityRemaining());
                        tempPlatData.weaponList.push_back(wpnData);
                     }

                     weapon = platform->template GetComponent<WsfWeapon>("srm");
                     if (weapon != nullptr)
                     {
                        P6DOF_ControllerDataContainer::WeaponData wpnData;
                        wpnData.weaponTypeName     = "srm";
                        wpnData.displayName        = "9M";
                        wpnData.numRoundsRemaining = static_cast<int>(weapon->GetQuantityRemaining());
                        tempPlatData.weaponList.push_back(wpnData);
                     }

                     weapon = platform->template GetComponent<WsfWeapon>("srmtvc");
                     if (weapon != nullptr)
                     {
                        P6DOF_ControllerDataContainer::WeaponData wpnData;
                        wpnData.weaponTypeName     = "srmtvc";
                        wpnData.displayName        = "9X";
                        wpnData.numRoundsRemaining = static_cast<int>(weapon->GetQuantityRemaining());
                        tempPlatData.weaponList.push_back(wpnData);
                     }


                  } // if (vehicle != 0)
               }    // if (p6dofMover != nullptr)
            }       // if (mover->IsA_TypeOf("WSF_P6DOF_MOVER"))
         }          // if (0 != mover)
      }             // if (0 != platform)
   }                // if (!mConnectedPlatform.empty() && !mConnectionDestroyed)

   // Use a temporary data container for the sim event below
   P6DOF_ControllerDataContainer tempData;

   // Set the platform data within the temporary data container data,
   // using a const reference to the temporary platform data
   const P6DOF_ControllerDataContainer::PlatformData& platData = tempPlatData;
   tempData.SetPlatformData(platData);

   // Add a sim event using the temporary data container data
   AddSimEvent(ut::make_unique<UpdateEvent>(tempData));
}

void WkP6DOF_Controller::SimInterface::GetRadarData(const WsfSimulation&                         aSimulation,
                                                    WsfPlatform*                                 aPlatform,
                                                    P6DOF_ControllerDataContainer::PlatformData& aPlatData)
{
   WsfLocalTrackList* localTrackListPtr = nullptr;
   WsfProcessor*      procPtr           = aPlatform->template GetComponent<WsfProcessor>("radar_track_processor");
   WsfTrackProcessor* trackProcessorPtr = dynamic_cast<WsfTrackProcessor*>(procPtr);
   if (trackProcessorPtr != nullptr)
   {
      localTrackListPtr = &trackProcessorPtr->GetTrackManager().GetTrackList();

      unsigned int numTracks = localTrackListPtr->GetTrackCount();

      // Clear vector
      aPlatData.radarTracks.clear();

      for (unsigned int index = 0; index < numTracks; ++index)
      {
         WsfLocalTrack* track = localTrackListPtr->GetTrackEntry(index);
         if (track != nullptr)
         {
            P6DOF_ControllerDataContainer::TrackData trkData;

            CalcTrackDataForTrack(aSimulation, aPlatform, track, trkData, "ID_RADAR", true, true, true, true, true);

            aPlatData.radarTracks.push_back(trkData);
         }
      }
   }
   // Note that we do not call delete on localTrackListPtr -- this is intentional

   // Supplemental data
   WsfSensor* sensorPtr = aPlatform->template GetComponent<WsfSensor>("radar");
   if (sensorPtr != nullptr)
   {
      // double minAz = 0.0;
      // double maxAz = 0.0;
      // double minEl = 0.0;
      // double maxEl = 0.0;
      // GetFOV_Limits(sensorPtr, minAz, maxAz, minEl, maxEl);
      // aPlatData.radarMinAz_deg = minAz * UtMath::cDEG_PER_RAD;
      // aPlatData.radarMaxAz_deg = maxAz * UtMath::cDEG_PER_RAD;
      // aPlatData.radarMinEl_deg = minEl * UtMath::cDEG_PER_RAD;
      // aPlatData.radarMaxEl_deg = maxEl * UtMath::cDEG_PER_RAD;

      double cuedAz_rad = 0.0;
      double cuedEl_rad = 0.0;
      sensorPtr->GetCurrentCuedOrientation(cuedAz_rad, cuedEl_rad);
      aPlatData.radarCuedAz_deg = cuedAz_rad * UtMath::cDEG_PER_RAD;
      aPlatData.radarCuedEl_deg = cuedEl_rad * UtMath::cDEG_PER_RAD;

      double minRange = 0.0;
      double maxRange = 0.0;
      GetRangeLimits(sensorPtr, minRange, maxRange);
      aPlatData.radarMaxRange_nm = maxRange * UtMath::cNM_PER_M;

      if (sensorPtr->IsTurnedOn() && sensorPtr->IsOperational())
      {
         aPlatData.radarMode = sensorPtr->GetCurrentModeName();
      }
      else
      {
         aPlatData.radarMode = "OFF";
      }
   }
   else
   {
      aPlatData.radarMode = "NO_SENSOR";
   }
}

void WkP6DOF_Controller::SimInterface::GetRWR_Data(const WsfSimulation&                         aSimulation,
                                                   WsfPlatform*                                 aPlatform,
                                                   P6DOF_ControllerDataContainer::PlatformData& aPlatData)
{
   WsfLocalTrackList* localTrackListPtr = nullptr;
   WsfProcessor*      procPtr           = aPlatform->template GetComponent<WsfProcessor>("rwr_track_processor");
   WsfTrackProcessor* trackProcessorPtr = dynamic_cast<WsfTrackProcessor*>(procPtr);
   if (trackProcessorPtr != nullptr)
   {
      localTrackListPtr = &trackProcessorPtr->GetTrackManager().GetTrackList();

      unsigned int numTracks = localTrackListPtr->GetTrackCount();

      // Clear vector
      aPlatData.rwrTracks.clear();

      for (unsigned int index = 0; index < numTracks; ++index)
      {
         WsfLocalTrack* track = localTrackListPtr->GetTrackEntry(index);
         if (track != nullptr)
         {
            P6DOF_ControllerDataContainer::TrackData trkData;
            CalcTrackDataForTrack(aSimulation, aPlatform, track, trkData, "ID_RWR", true, false, false, false, false);
            aPlatData.rwrTracks.push_back(trkData);
         }
      }
   }
   // Note that we do not call delete on localTrackListPtr -- this is intentional

   // Supplemental data
   WsfSensor* sensorPtr = aPlatform->template GetComponent<WsfSensor>("rwr");
   if (sensorPtr != nullptr)
   {
      if (sensorPtr->IsTurnedOn() && sensorPtr->IsOperational())
      {
         aPlatData.rwrMode = sensorPtr->GetCurrentModeName();
      }
      else
      {
         aPlatData.rwrMode = "OFF";
      }
   }
   else
   {
      aPlatData.rwrMode = "NO_SENSOR";
   }
}

void WkP6DOF_Controller::SimInterface::GetIRST_Data(const WsfSimulation&                         aSimulation,
                                                    WsfPlatform*                                 aPlatform,
                                                    P6DOF_ControllerDataContainer::PlatformData& aPlatData)
{
   WsfLocalTrackList* localTrackListPtr = nullptr;
   WsfProcessor*      procPtr           = aPlatform->template GetComponent<WsfProcessor>("irst_track_processor");
   WsfTrackProcessor* trackProcessorPtr = dynamic_cast<WsfTrackProcessor*>(procPtr);
   if (trackProcessorPtr != nullptr)
   {
      localTrackListPtr = &trackProcessorPtr->GetTrackManager().GetTrackList();

      unsigned int numTracks = localTrackListPtr->GetTrackCount();

      // Clear vector
      aPlatData.irstTracks.clear();

      for (unsigned int index = 0; index < numTracks; ++index)
      {
         WsfLocalTrack* track = localTrackListPtr->GetTrackEntry(index);
         if (track != nullptr)
         {
            P6DOF_ControllerDataContainer::TrackData trkData;
            CalcTrackDataForTrack(aSimulation, aPlatform, track, trkData, "ID_IRST", true, true, false, false, false);
            aPlatData.irstTracks.push_back(trkData);
         }
      }
   }
   // Note that we do not call delete on localTrackListPtr -- this is intentional

   // Supplemental data
   WsfSensor* sensorPtr = aPlatform->template GetComponent<WsfSensor>("irst");
   if (sensorPtr != nullptr)
   {
      if (sensorPtr->IsTurnedOn() && sensorPtr->IsOperational())
      {
         aPlatData.irstMode = sensorPtr->GetCurrentModeName();
      }
      else
      {
         aPlatData.irstMode = "OFF";
      }
   }
   else
   {
      aPlatData.irstMode = "NO_SENSOR";
   }
}

void WkP6DOF_Controller::SimInterface::GetFLIR_Data(const WsfSimulation&                         aSimulation,
                                                    WsfPlatform*                                 aPlatform,
                                                    P6DOF_ControllerDataContainer::PlatformData& aPlatData)
{
   WsfLocalTrackList* localTrackListPtr = nullptr;
   WsfProcessor*      procPtr           = aPlatform->template GetComponent<WsfProcessor>("flir_track_processor");
   WsfTrackProcessor* trackProcessorPtr = dynamic_cast<WsfTrackProcessor*>(procPtr);
   if (trackProcessorPtr != nullptr)
   {
      localTrackListPtr = &trackProcessorPtr->GetTrackManager().GetTrackList();

      unsigned int numTracks = localTrackListPtr->GetTrackCount();

      // Clear vector
      aPlatData.flirTracks.clear();

      for (unsigned int index = 0; index < numTracks; ++index)
      {
         WsfLocalTrack* track = localTrackListPtr->GetTrackEntry(index);
         if (track != nullptr)
         {
            P6DOF_ControllerDataContainer::TrackData trkData;
            CalcTrackDataForTrack(aSimulation, aPlatform, track, trkData, "ID_FLIR", true, true, false, false, false);
            aPlatData.flirTracks.push_back(trkData);
         }
      }
   }
   // Note that we do not call delete on localTrackListPtr -- this is intentional

   // Supplemental data
   WsfSensor* sensorPtr = aPlatform->template GetComponent<WsfSensor>("flir");
   if (sensorPtr != nullptr)
   {
      if (sensorPtr->IsTurnedOn() && sensorPtr->IsOperational())
      {
         aPlatData.flirMode = sensorPtr->GetCurrentModeName();
      }
      else
      {
         aPlatData.flirMode = "OFF";
      }
   }
   else
   {
      aPlatData.flirMode = "NO_SENSOR";
   }
}

void WkP6DOF_Controller::SimInterface::GetTrackDataTrackManager(const WsfSimulation& aSimulation,
                                                                WsfPlatform*         aPlatform,
                                                                P6DOF_ControllerDataContainer::PlatformData& aPlatData)
{
   WsfLocalTrackList* localTrackListPtr = nullptr;
   WsfProcessor*      procPtr           = aPlatform->template GetComponent<WsfProcessor>("master_track_processor");
   WsfTrackProcessor* trackProcessorPtr = dynamic_cast<WsfTrackProcessor*>(procPtr);
   if (trackProcessorPtr != nullptr)
   {
      localTrackListPtr = &trackProcessorPtr->GetTrackManager().GetTrackList();

      unsigned int numTracks = localTrackListPtr->GetTrackCount();

      // Clear vector
      aPlatData.masterTracks.clear();

      for (unsigned int index = 0; index < numTracks; ++index)
      {
         WsfLocalTrack* track = localTrackListPtr->GetTrackEntry(index);
         if (track != nullptr)
         {
            P6DOF_ControllerDataContainer::TrackData trkData;
            CalcTrackDataForTrack(aSimulation, aPlatform, track, trkData, "ID_MASTER", true, true, true, true, true);
            aPlatData.masterTracks.push_back(trkData);
         }
      }
   }
   // Note that we do not call delete on localTrackListPtr -- this is intentional
}

void WkP6DOF_Controller::SimInterface::CalcTrackDataForTrack(const WsfSimulation&                      aSimulation,
                                                             WsfPlatform*                              aPlatform,
                                                             const WsfTrack*                           aTrack,
                                                             P6DOF_ControllerDataContainer::TrackData& aTrackData)
{
   CalcTrackDataForTrack(aSimulation, aPlatform, aTrack, aTrackData, "");
}

void WkP6DOF_Controller::SimInterface::CalcTrackDataForTrack(const WsfSimulation&                      aSimulation,
                                                             WsfPlatform*                              aPlatform,
                                                             const WsfTrack*                           aTrack,
                                                             P6DOF_ControllerDataContainer::TrackData& aTrackData,
                                                             const std::string&                        aAuxData,
                                                             bool                                      aSupportsAz,
                                                             bool                                      aSupportsEl,
                                                             bool                                      aSupportsAlt,
                                                             bool                                      aSupportsHdg,
                                                             bool                                      aSupportsIFF)
{
   aTrackData.tgtPlatformName = aTrack->GetTargetName();

   if (aTrack->LocationValid())
   {
      aTrackData.tgtLLA_Valid = true;
      double tgtLat           = 0.0;
      double tgtLon           = 0.0;
      double tgtAlt_m         = 0.0;
      aTrack->GetLocationLLA(tgtLat, tgtLon, tgtAlt_m);

      aTrackData.tgtLat   = tgtLat;
      aTrackData.tgtLon   = tgtLon;
      aTrackData.tgtAlt_m = tgtAlt_m;
   }
   else
   {
      aTrackData.tgtLLA_Valid = false;
      aTrackData.tgtLat       = 0.0;
      aTrackData.tgtLon       = 0.0;
      aTrackData.tgtAlt_m     = 0.0;
   }

   if (aTrack->BearingValid())
   {
      aTrackData.tgtBearingValid = true;
      aTrackData.tgtBearing_rad  = aTrack->GetBearing();
   }
   else
   {
      aTrackData.tgtBearingValid = false;
      aTrackData.tgtBearing_rad  = 0.0;
   }

   if (aTrack->RangeValid())
   {
      aTrackData.tgtRngValid = true;
      aTrackData.tgtRng_m    = aTrack->GetRange();
   }
   else
   {
      aTrackData.tgtRngValid = false;
      aTrackData.tgtRng_m    = 0.0;
   }

   if (aTrack->VelocityValid())
   {
      aTrackData.tgtSpdValid = true;
      aTrackData.tgtSpd_kts  = aTrack->GetSpeed();
   }
   else
   {
      aTrackData.tgtSpdValid = false;
      aTrackData.tgtSpd_kts  = 0.0;
   }

   if (aTrack->RangeRateValid())
   {
      aTrackData.tgtRngRateValid  = true;
      aTrackData.tgtRangeRate_mps = aTrack->GetRangeRate();
   }
   else
   {
      aTrackData.tgtRngRateValid  = false;
      aTrackData.tgtRangeRate_mps = 0.0;
   }

   if (aSupportsAz || aSupportsEl)
   {
      // Planned improvement:  Previously, this extrapolated the location
      // double locWCS[3];
      // aPlatform->GetLocationWCS(locWCS);
      // double extrapLocWSC[3];
      // aTrack->GetExtrapolatedLocationWCS(mPlatData.simTime + 0.01, extrapLocWSC);
      //
      // double relLocWCS[3] = { extrapLocWSC[0] - locWCS[0],
      //                         extrapLocWSC[1] - locWCS[1],
      //                         extrapLocWSC[2] - locWCS[2] };
      //
      // double az_rad = 0.0;
      // double el_rad = 0.0;
      // aPlatform->ComputeAspect(relLocWCS, az_rad, el_rad);

      double locWCS[3];
      aPlatform->GetLocationWCS(locWCS);
      UtVec3d wcsVec3d     = aTrack->GetLocationWCS();
      double  relLocWCS[3] = {wcsVec3d.Get(0) - locWCS[0], wcsVec3d.Get(1) - locWCS[1], wcsVec3d.Get(2) - locWCS[2]};
      double  az_rad       = 0.0;
      double  el_rad       = 0.0;
      aPlatform->ComputeAspect(relLocWCS, az_rad, el_rad);

      if (aSupportsAz)
      {
         aTrackData.tgtAzValid = true;
         aTrackData.tgtAz_rad  = az_rad;
      }
      else
      {
         aTrackData.tgtAzValid = false;
         aTrackData.tgtAz_rad  = 0.0;
      }

      if (aSupportsEl)
      {
         aTrackData.tgtElValid = true;
         aTrackData.tgtEl_rad  = el_rad;
      }
      else
      {
         aTrackData.tgtElValid = false;
         aTrackData.tgtEl_rad  = 0.0;
      }
   }
   else
   {
      aTrackData.tgtAzValid = false;
      aTrackData.tgtAz_rad  = 0.0;
      aTrackData.tgtElValid = false;
      aTrackData.tgtEl_rad  = 0.0;
   }

   if (aSupportsAlt)
   {
      double tgtLat   = 0.0;
      double tgtLon   = 0.0;
      double tgtAlt_m = 0.0;
      aTrack->GetLocationLLA(tgtLat, tgtLon, tgtAlt_m);

      aTrackData.tgtAltValid = true;
      aTrackData.tgtAlt_ft   = tgtAlt_m * UtMath::cFT_PER_M;
   }
   else
   {
      aTrackData.tgtAltValid = false;
      aTrackData.tgtAlt_ft   = 0.0;
   }

   if (aSupportsHdg)
   {
      // Get the target name/platform
      std::string  targetName     = aTrack->GetTargetName();
      WsfPlatform* targetPlatform = aSimulation.GetPlatformByName(targetName);

      aTrackData.tgtHdgValid = true;
      aTrackData.tgtHdg_rad  = 0.0;

      if (targetPlatform != nullptr)
      {
         double tgtHdg_rad, tgtPitch_rad, tgtRoll_rad;
         targetPlatform->GetOrientationNED(tgtHdg_rad, tgtPitch_rad, tgtRoll_rad);
         aTrackData.tgtHdg_rad = tgtHdg_rad;
      }
      else
      {
         aTrackData.tgtHdgValid = false;
         aTrackData.tgtHdg_rad  = 0.0;
      }
   }
   else
   {
      aTrackData.tgtHdgValid = false;
      aTrackData.tgtHdg_rad  = 0.0;
   }

   if (!aSupportsIFF)
   {
      aTrackData.tgtIffValid    = false;
      aTrackData.tgtHasIffReply = false;
   }
   else
   {
      aTrackData.tgtIffValid = true;

      if (aPlatform->GetSideId() == aTrack->GetSideId())
      {
         aTrackData.tgtHasIffReply = true;
      }
      else
      {
         aTrackData.tgtHasIffReply = false;
      }
   }

   // Check for optional aux data on track
   aTrackData.tgtPackedData = 0;
   if (aTrack->HasAuxData())
   {
      if ((aTrack->GetAuxDataConst()).AttributeExists("PRIMARY"))
      {
         if (aTrack->GetAuxDataConst().GetBool("PRIMARY"))
         {
            aTrackData.tgtPackedData |= 0x00000001;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("RING"))
      {
         int value = aTrack->GetAuxDataConst().GetInt("RING");
         if (value == 1)
         {
            aTrackData.tgtPackedData |= 0x00000002;
         }
         else if (value == 2)
         {
            aTrackData.tgtPackedData |= 0x00000004;
         }
         else if (value == 3)
         {
            aTrackData.tgtPackedData |= 0x00000006;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("NEW_THREAT"))
      {
         if (aTrack->GetAuxDataConst().GetBool("NEW_THREAT"))
         {
            aTrackData.tgtPackedData |= 0x00000008;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("HIGH_THREAT"))
      {
         if (aTrack->GetAuxDataConst().GetBool("HIGH_THREAT"))
         {
            aTrackData.tgtPackedData |= 0x00000010;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("AIR"))
      {
         if (aTrack->GetAuxDataConst().GetBool("AIR"))
         {
            aTrackData.tgtPackedData |= 0x00000020;
         }
      }

      // Primary threat type

      // Note "AI" type uses zero values

      if ((aTrack->GetAuxDataConst()).AttributeExists("SAM"))
      {
         if (aTrack->GetAuxDataConst().GetBool("SAM"))
         {
            aTrackData.tgtPackedData |= 0x00000040;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("AAA"))
      {
         if (aTrack->GetAuxDataConst().GetBool("AAA"))
         {
            aTrackData.tgtPackedData |= 0x00000080;
         }
      }

      if ((aTrack->GetAuxDataConst()).AttributeExists("MSL"))
      {
         if (aTrack->GetAuxDataConst().GetBool("MSL"))
         {
            aTrackData.tgtPackedData |= 0x000000C0;
         }
      }
   }

   // Check for optional aux data on platform
   WsfPlatform* targetPlatform = aSimulation.GetPlatformByIndex(aTrack->GetTargetIndex());
   if (targetPlatform == nullptr)
   {
      aTrackData.tgtIdValid = false;
      aTrackData.tgtIdString.clear();
   }
   else
   {
      if (targetPlatform->HasAuxData())
      {
         if ((targetPlatform->GetAuxDataConst()).AttributeExists(aAuxData))
         {
            aTrackData.tgtIdValid  = true;
            aTrackData.tgtIdString = targetPlatform->GetAuxDataConst().GetString(aAuxData);
         }
         else
         {
            aTrackData.tgtIdValid = false;
            aTrackData.tgtIdString.clear();
         }
      }
      else
      {
         aTrackData.tgtIdValid = false;
         aTrackData.tgtIdString.clear();
      }
   }
}

void WkP6DOF_Controller::SimInterface::GetScanLimits(WsfSensor* aSensorPtr,
                                                     double&    aMinAz,
                                                     double&    aMaxAz,
                                                     double&    aMinEl,
                                                     double&    aMaxEl)
{
   aMinAz = std::numeric_limits<double>::max();
   aMaxAz = -aMinAz;
   aMinEl = std::numeric_limits<double>::max();
   aMaxEl = -aMinEl;

   double minValue, maxValue;
   size_t rcvrCount = aSensorPtr->GetEM_RcvrCount();
   for (size_t rcvrIndex = 0; rcvrIndex < rcvrCount; ++rcvrIndex)
   {
      WsfEM_Antenna* antennaPtr = aSensorPtr->GetEM_Rcvr(rcvrIndex).GetAntenna();
      if (antennaPtr)
      {
         WsfEM_Antenna::ScanMode scanMode = antennaPtr->GetScanMode();

         minValue = maxValue = 0.0;
         if ((scanMode & WsfEM_Antenna::cSCAN_AZ) != 0)
         {
            antennaPtr->GetAzimuthScanLimits(minValue, maxValue);
         }
         aMinAz = std::min(minValue, aMinAz);
         aMaxAz = std::max(maxValue, aMaxAz);

         minValue = maxValue = 0.0;
         if ((scanMode & WsfEM_Antenna::cSCAN_EL) != 0)
         {
            antennaPtr->GetElevationScanLimits(minValue, maxValue);
         }
         aMinEl = std::min(minValue, aMinEl);
         aMaxEl = std::max(maxValue, aMaxEl);
      }
   }
}

int WkP6DOF_Controller::SimInterface::GetScanMode(WsfSensor* aSensorPtr)
{
   int    scanMode  = 0;
   size_t rcvrCount = aSensorPtr->GetEM_RcvrCount();
   for (size_t rcvrIndex = 0; rcvrIndex < rcvrCount; ++rcvrIndex)
   {
      WsfEM_Antenna* antennaPtr = aSensorPtr->GetEM_Rcvr(rcvrIndex).GetAntenna();
      if (antennaPtr)
      {
         scanMode |= antennaPtr->GetScanMode();
      }
   }
   return scanMode;
}

void WkP6DOF_Controller::SimInterface::GetFOV_Limits(WsfSensor* aSensorPtr,
                                                     double&    aMinAz,
                                                     double&    aMaxAz,
                                                     double&    aMinEl,
                                                     double&    aMaxEl)
{
   aMinAz = std::numeric_limits<double>::max();
   aMaxAz = -aMinAz;
   aMinEl = std::numeric_limits<double>::max();
   aMaxEl = -aMinEl;

   double minValue, maxValue;
   size_t rcvrCount = aSensorPtr->GetEM_RcvrCount();
   for (size_t rcvrIndex = 0; rcvrIndex < rcvrCount; ++rcvrIndex)
   {
      WsfEM_Antenna* antennaPtr = aSensorPtr->GetEM_Rcvr(rcvrIndex).GetAntenna();
      if (antennaPtr != nullptr)
      {
         antennaPtr->GetAzimuthFieldOfView(minValue, maxValue);
         aMinAz = std::min(minValue, aMinAz);
         aMaxAz = std::max(maxValue, aMaxAz);
         antennaPtr->GetElevationFieldOfView(minValue, maxValue);
         aMinEl = std::min(minValue, aMinEl);
         aMaxEl = std::max(maxValue, aMaxEl);
      }
   }
}

void WkP6DOF_Controller::SimInterface::GetRangeLimits(WsfSensor* aSensorPtr, double& aMinRange, double& aMaxRange)
{
   aMinRange = std::numeric_limits<double>::max();
   aMaxRange = -aMinRange;

   double minValue, maxValue;
   size_t rcvrCount = aSensorPtr->GetEM_RcvrCount();
   for (size_t rcvrIndex = 0; rcvrIndex < rcvrCount; ++rcvrIndex)
   {
      WsfEM_Antenna* antennaPtr = aSensorPtr->GetEM_Rcvr(rcvrIndex).GetAntenna();
      if (antennaPtr != nullptr)
      {
         antennaPtr->GetRangeLimits(minValue, maxValue);
         aMinRange = std::min(minValue, aMinRange);
         aMaxRange = std::max(maxValue, aMaxRange);
      }
   }
}
