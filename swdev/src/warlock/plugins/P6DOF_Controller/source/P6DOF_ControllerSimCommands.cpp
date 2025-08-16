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

#include "P6DOF_ControllerSimCommands.hpp"

#include "P6DofGuidanceAutopilot.hpp"
#include "P6DofHardwareAutopilot.hpp"
#include "P6DofPilotManager.hpp"
#include "P6DofPilotObject.hpp"
#include "P6DofVehicle.hpp"
#include "WkObserver.hpp"
#include "WsfMover.hpp"
#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

double WkP6DOF_Controller::ControlCommand::mLastSimulationTime_sec = 0.0;

WkP6DOF_Controller::ControlCommand::ControlCommand(const std::string& aConnectedPlatform, const ControlData& aControlData)
   : mConnectedPlatform(aConnectedPlatform)
   , mControlData(aControlData)
{
}

void WkP6DOF_Controller::ControlCommand::Process(WsfSimulation& aSimulation)
{
   double currentTime_sec = aSimulation.GetSimTime();
   double deltaT_sec      = -1.0;
   if (mLastSimulationTime_sec >= 0.0)
   {
      deltaT_sec = currentTime_sec - mLastSimulationTime_sec;
   }
   mLastSimulationTime_sec = currentTime_sec;

   WsfPlatform* plat = aSimulation.GetPlatformByName(mConnectedPlatform);
   if (nullptr != plat)
   {
      WsfMover* mover = plat->GetMover();
      if (nullptr != mover)
      {
         if (mover->IsA_TypeOf("WSF_P6DOF_MOVER"))
         {
            WsfP6DOF_Mover* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(mover);

            P6DofVehicle* vehicle = p6dofMover->GetP6DofVehicle();
            if (vehicle != nullptr)
            {
               P6DofPilotManager* pilotMgr = vehicle->GetPilotManager();
               if (pilotMgr != nullptr)
               {
                  // We need an active manual pilot to proceed
                  P6DofPilotObject* pilot = pilotMgr->GetActiveManualPilot();
                  if (pilot != nullptr)
                  {
                     p6dofMover->EnableControls(true);
                     p6dofMover->EnableAutopilot(false);

                     if (pilot->ControlAugmentationModeIsActive())
                     {
                        p6dofMover->SetRollRateMax(180.0);
                        p6dofMover->SetPitchGLoadMax(8.0);
                     }

                     pilot->TakeManualControl();

                     double stickRightPosition       = mControlData.roll;
                     double stickAftPosition         = mControlData.pitch;
                     double rudderRightPosition      = mControlData.rudder;
                     double throttleLeverPosition    = mControlData.throttle;
                     double spoilerLeverPosition     = 0.0;
                     double flapsLeverPosition       = 0.0;
                     double landingGearLeverPosition = mControlData.landingGear;
                     double noseUpTrimValue          = mControlData.trimNoseUp;
                     double noseUpTrimDeltaT_sec     = 0.0;

                     if (deltaT_sec > 0.0)
                     {
                        if (noseUpTrimValue > 0.001)
                        {
                           noseUpTrimDeltaT_sec = deltaT_sec;
                        }
                        if (noseUpTrimValue < -0.001)
                        {
                           noseUpTrimDeltaT_sec = -deltaT_sec;
                        }
                     }

                     double rollRightTrimValue      = mControlData.trimRollRight;
                     double rollRightTrimDeltaT_sec = 0.0;

                     if (deltaT_sec > 0.0)
                     {
                        if (rollRightTrimValue > 0.001)
                        {
                           rollRightTrimDeltaT_sec = deltaT_sec;
                        }
                        if (rollRightTrimValue < -0.001)
                        {
                           rollRightTrimDeltaT_sec = -deltaT_sec;
                        }
                     }

                     double yawRightTrimValue      = mControlData.trimYawRight;
                     double yawRightTrimDeltaT_sec = 0.0;

                     if (deltaT_sec > 0.0)
                     {
                        if (yawRightTrimValue > 0.001)
                        {
                           yawRightTrimDeltaT_sec = deltaT_sec;
                        }
                        if (yawRightTrimValue < -0.001)
                        {
                           yawRightTrimDeltaT_sec = -deltaT_sec;
                        }
                     }

                     double currentSpdBrkInputPosition = pilot->GetSpeedBrakesControllerPosition();

                     double speedBrakeValue      = mControlData.spdBrake;
                     double speedBrakeDeltaT_sec = 0.0;

                     if (deltaT_sec > 0.0)
                     {
                        if (speedBrakeValue > 0.001)
                        {
                           speedBrakeDeltaT_sec = deltaT_sec;
                        }
                        if (speedBrakeValue < -0.001)
                        {
                           speedBrakeDeltaT_sec = -deltaT_sec;
                        }
                     }

                     currentSpdBrkInputPosition += speedBrakeDeltaT_sec;

                     if (currentSpdBrkInputPosition < 0.0)
                     {
                        currentSpdBrkInputPosition = 0.0;
                     }
                     else if (currentSpdBrkInputPosition > 1.0)
                     {
                        currentSpdBrkInputPosition = 1.0;
                     }

                     double spdBrakeLeverPosition = currentSpdBrkInputPosition;


                     pilot->SetTrimManualControlData(noseUpTrimDeltaT_sec, rollRightTrimDeltaT_sec, yawRightTrimDeltaT_sec);

                     pilot->SetManualControlData(stickRightPosition,
                                                 stickAftPosition,
                                                 rudderRightPosition,
                                                 throttleLeverPosition,
                                                 spdBrakeLeverPosition,
                                                 spoilerLeverPosition,
                                                 flapsLeverPosition,
                                                 landingGearLeverPosition);
                  }
                  else // pilot == nullptr
                  {
                     // We do not have an active manual pilot. Let's try for another pilot type.

                     // Try for a hardware autopilot
                     P6DofPilotObject* tempPilot = pilotMgr->GetActiveHardwareAutopilot();
                     if (tempPilot != nullptr)
                     {
                        // The hardware autopilot is active
                        P6DofHardwareAutopilot* hwAutopilot = dynamic_cast<P6DofHardwareAutopilot*>(tempPilot);
                        if (hwAutopilot != nullptr)
                        {
                           // Not sure about this check   if (mActivateHardwareAutopilot)
                           {
                              hwAutopilot->HoldPitchAngle();
                              hwAutopilot->HoldBankAngle();
                              hwAutopilot->HoldSpeedKCAS();
                              // hwAutopilot->HoldAltitude();
                              // hwAutopilot->HoldHeading();
                              // hwAutopilot->HoldSpeedKTAS();
                           }
                        }
                     }
                     else
                     {
                        // Try for a guidance autopilot pilot
                        P6DofPilotObject* tempPilot = pilotMgr->GetActiveGuidanceAutopilot();
                        if (tempPilot != nullptr)
                        {
                           // The guidance autopilot is active
                           P6DofGuidanceAutopilot* guidanceAutopilot = dynamic_cast<P6DofGuidanceAutopilot*>(tempPilot);
                           if (guidanceAutopilot != nullptr)
                           {
                              // Not sure about this check  if (mActivateGuidanceAutopilot)
                              {
                                 guidanceAutopilot->HoldAltitude();
                                 guidanceAutopilot->HoldHeading();
                                 guidanceAutopilot->HoldSpeedKTAS();
                              }
                           }
                        }
                        else
                        {
                           // Try for a synthetic pilot
                           P6DofPilotObject* tempPilot = pilotMgr->GetActiveSyntheticPilot();
                           if (tempPilot != nullptr)
                           {
                              // The synthetic pilot is active
                           }
                        }
                     }
                  }
               }
            }
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Enabled Manual Control of a P6DOF Platform", plat);
   }
}

void WkP6DOF_Controller::ActivatePilotCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* plat = aSimulation.GetPlatformByName(mConnectedPlatform);
   if (nullptr != plat)
   {
      WsfMover* mover = plat->GetMover();
      if (nullptr != mover)
      {
         if (mover->IsA_TypeOf("WSF_P6DOF_MOVER"))
         {
            WsfP6DOF_Mover* p6dofMover = dynamic_cast<WsfP6DOF_Mover*>(mover);

            P6DofVehicle* vehicle = p6dofMover->GetP6DofVehicle();
            if (vehicle != nullptr)
            {
               P6DofPilotManager* pilotMgr = vehicle->GetPilotManager();
               if (pilotMgr != nullptr)
               {
                  if (mPilotType == SYNTHETIC)
                  {
                     if (pilotMgr->MakeSyntheticPilotActive())
                     {
                        WkObserver::SimulationUserAction(aSimulation)("Changed Active P6DOF Pilot Type to Synthetic", plat);
                     }
                     else
                     {
                        std::cout << "Unable to MakeSyntheticPilotActive() for " << vehicle->GetName() << std::endl;
                     }
                  }
                  else if (mPilotType == HARDWARE)
                  {
                     if (pilotMgr->MakeHardwareAutopilotActive())
                     {
                        WkObserver::SimulationUserAction(
                           aSimulation)("Changed Active P6DOF Pilot Type to Hardware Autopilot", plat);
                     }
                     else
                     {
                        std::cout << "Unable to MakeHardwareAutopilotActive() for " << vehicle->GetName() << std::endl;
                     }
                  }
                  else if (mPilotType == GUIDANCE)
                  {
                     if (pilotMgr->MakeGuidanceAutopilotActive())
                     {
                        WkObserver::SimulationUserAction(
                           aSimulation)("Changed Active P6DOF Pilot Type to Guidance Autopilot", plat);
                     }
                     else
                     {
                        std::cout << "Unable to MakeGuidanceAutopilotActive() for " << vehicle->GetName() << std::endl;
                     }
                  }
                  else if (mPilotType == MANUAL)
                  {
                     if (pilotMgr->MakeManualPilotActive())
                     {
                        WkObserver::SimulationUserAction(aSimulation)("Changed Active P6DOF Pilot Type to Manual", plat);
                     }
                     else
                     {
                        std::cout << "Unable to MakeManualPilotActive() for " << vehicle->GetName() << std::endl;
                     }
                  }
               }
            }
         }
      }
   }
}
