// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "P6DOF_DataSimInterface.hpp"

#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WsfMover.hpp"
#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

WkP6DOF_Data::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<P6DOF_DataEvent>(aPluginName)
   , mPlatformOfInterest("")
{
}

void WkP6DOF_Data::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

void WkP6DOF_Data::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   std::string platformName = "";
   if (mMutex.tryLock())
   {
      platformName = mPlatformOfInterest;
      mMutex.unlock();
   }

   WsfPlatform* platform = aSimulation.GetPlatformByName(platformName);
   if (platform != nullptr)
   {
      WsfP6DOF_Mover* mover = dynamic_cast<WsfP6DOF_Mover*>(platform->GetMover());
      if (mover != nullptr)
      {
         DataContainer::PlatformData data;
         data.name = platformName;

         data.altitude_m           = mover->GetAlt_m();
         data.vertSpeed_fpm        = mover->GetVerticalSpeed() * UtMath::cFT_PER_M * 60.0;
         data.KCAS                 = mover->GetSpeed_KCAS();
         data.mach                 = mover->GetSpeed_Mach();
         data.dynamicPressure_psf  = mover->GetDynamicPressure_psf();
         data.heading_rad          = mover->GetP6DofVehicle()->GetHeading_deg() * UtMath::cRAD_PER_DEG;
         data.pitch_rad            = mover->GetP6DofVehicle()->GetPitch_deg() * UtMath::cRAD_PER_DEG;
         data.roll_rad             = mover->GetP6DofVehicle()->GetRoll_deg() * UtMath::cRAD_PER_DEG;
         data.nx                   = mover->GetNx_g();
         data.ny                   = mover->GetNy_g();
         data.gLoad                = -mover->GetNz_g(); // G-load is negative Nz
         data.gAvail               = mover->GetP6DofVehicle()->MaxPotentialManeuverGLoad();
         data.alpha_rad            = mover->GetAlpha_deg() * UtMath::cRAD_PER_DEG;
         data.beta_rad             = mover->GetBeta_deg() * UtMath::cRAD_PER_DEG;
         data.alphaDot_rps         = mover->GetP6DofVehicle()->GetKinematicState().GetAlphaDot_rps();
         data.betaDot_rps          = mover->GetP6DofVehicle()->GetKinematicState().GetBetaDot_rps();
         data.yawRate_rps          = mover->GetYawRate();
         data.pitchRate_rps        = mover->GetPitchRate();
         data.rollRate_rps         = mover->GetRollRate();
         data.lift_nt              = mover->GetLift_lbs() * UtMath::cNT_PER_LB;
         data.drag_nt              = mover->GetDrag_lbs() * UtMath::cNT_PER_LB;
         data.thrust_nt            = mover->GetTotalForwardThrust();
         data.fuelFlow_pph         = mover->GetTotalFuelFlow() * UtMath::cLB_PER_KG * 3600.0; // Convert from kg/sec
         data.fuelTotalCapacity_kg = mover->GetTotalFuelCapacity();
         data.fuelRemaining_kg     = mover->GetTotalFuelRemaining();
         data.fuelInternal_kg      = mover->GetInternalFuelRemaining();
         data.fuelExternal_kg      = mover->GetExternalFuelRemaining();

         P6DofPilotObject* pilotObj = mover->GetP6DofVehicle()->GetPilotObject();
         if (pilotObj != nullptr)
         {
            data.activePilot       = pilotObj->GetPilotType();
            data.stickBack         = pilotObj->GetStickBackControllerPosition();
            data.stickRight        = pilotObj->GetStickRightControllerPosition();
            data.rudderRight       = pilotObj->GetRudderRightControllerPosition();
            data.throttle          = pilotObj->GetThrottleControllerPosition();
            data.speedbrakeCommand = pilotObj->GetSpeedBrakesControllerPosition();
         }
         else
         {
            data.stickBack         = 0.0;
            data.stickRight        = 0.0;
            data.rudderRight       = 0.0;
            data.throttle          = 0.0;
            data.speedbrakeCommand = 0.0;
            data.activePilot       = "None";
         }

         P6DofFlightControlSystem* fltControls = mover->GetP6DofVehicle()->GetFlightControls();
         if (fltControls != nullptr)
         {
            size_t speedbrakeHandle = fltControls->GetControlSurfaceHandle("Speed_Brake_Extended");

            if (speedbrakeHandle > 0)
            {
               data.speedbrakeAngle_rad = fltControls->GetControlSurfaceAngle_deg(speedbrakeHandle) * UtMath::cRAD_PER_DEG;
            }
            else
            {
               data.speedbrakeAngle_rad = 0.0;
            }
         }
         else
         {
            data.speedbrakeAngle_rad = 0.0;
         }

         data.afterburner_on = mover->GetAfterburnerOn();
         data.totalWeight_kg = mover->GetTotalWeight_kg();
         data.weightOnWheels = mover->GetWeightOnWheels();

         AddSimEvent(ut::make_unique<UpdateEvent>(data));
      }
   }
}

void WkP6DOF_Data::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<PlatformClearEvent>());
}

void WkP6DOF_Data::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   if (dynamic_cast<WsfP6DOF_Mover*>(aPlatform.GetMover()) != nullptr)
   {
      AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
   }
}

void WkP6DOF_Data::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (dynamic_cast<WsfP6DOF_Mover*>(aPlatform.GetMover()) != nullptr)
   {
      AddSimEvent(ut::make_unique<PlatformRemovedEvent>(aPlatform.GetName()));
   }
}
