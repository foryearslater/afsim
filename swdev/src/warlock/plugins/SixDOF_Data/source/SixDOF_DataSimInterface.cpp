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

#include "SixDOF_DataSimInterface.hpp"

#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WsfMover.hpp"
#include "WsfPlatform.hpp"
#include "WsfRigidBodySixDOF_Mover.hpp"
#include "WsfSimulation.hpp"

WkSixDOF_Data::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<SixDOF_DataEvent>(aPluginName)
{
}

void WkSixDOF_Data::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

void WkSixDOF_Data::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   std::string platformName;
   if (mMutex.tryLock())
   {
      platformName = mPlatformOfInterest;
      mMutex.unlock();
   }

   WsfPlatform* platform = aSimulation.GetPlatformByName(platformName);
   if (platform != nullptr)
   {
      auto mover = dynamic_cast<wsf::six_dof::Mover*>(platform->GetMover());
      if (mover != nullptr)
      {
         DataContainer::PlatformData data;
         data.name = platformName;

         data.altitude_m           = mover->GetAlt_m();
         data.vertSpeed_fpm        = mover->GetVerticalSpeed() * UtMath::cFT_PER_M * 60.0;
         data.KCAS                 = mover->GetSpeed_KCAS();
         data.mach                 = mover->GetSpeed_Mach();
         data.dynamicPressure_psf  = mover->GetDynamicPressure_psf();
         data.heading_rad          = mover->GetHeading_rad();
         data.pitch_rad            = mover->GetPitch_rad();
         data.roll_rad             = mover->GetRoll_rad();
         data.nx                   = mover->GetNx_g();
         data.ny                   = mover->GetNy_g();
         data.gLoad                = -mover->GetNz_g(); // G-load is negative Nz
         data.gAvail               = mover->GetPotentialManeuverGLoadMax();
         data.alpha_rad            = mover->GetAlpha_deg() * UtMath::cRAD_PER_DEG;
         data.beta_rad             = mover->GetBeta_deg() * UtMath::cRAD_PER_DEG;
         data.alphaDot_rps         = mover->GetAlphaDot_rps();
         data.betaDot_rps          = mover->GetBetaDot_rps();
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

         wsf::six_dof::PilotObject* pilotObj = mover->GetActivePilotObject();
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

         data.speedbrakeAngle_rad = mover->GetSpeedBrakePosition();

         data.afterburner_on = mover->GetAfterburnerOn();
         data.totalWeight_kg = mover->GetTotalWeight_kg();
         data.weightOnWheels = mover->GetWeightOnWheels();

         AddSimEvent(ut::make_unique<UpdateEvent>(data));
      }
   }
}

void WkSixDOF_Data::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<PlatformClearEvent>());
}

void WkSixDOF_Data::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   if (dynamic_cast<wsf::six_dof::Mover*>(aPlatform.GetMover()) != nullptr)
   {
      AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
   }
}

void WkSixDOF_Data::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (dynamic_cast<wsf::six_dof::Mover*>(aPlatform.GetMover()) != nullptr)
   {
      AddSimEvent(ut::make_unique<PlatformRemovedEvent>(aPlatform.GetName()));
   }
}
