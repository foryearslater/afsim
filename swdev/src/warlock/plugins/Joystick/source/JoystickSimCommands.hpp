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
#ifndef JOYSTICKSIMCOMMANDS_HPP
#define JOYSTICKSIMCOMMANDS_HPP

#include "JoystickSimInterface.hpp"

// This is a collection of SimCommands that are used in the P6DofController

namespace Joystick
{
// This defines a SimCommand for setting the active pilot controller
class ActivatePilotCommand : public warlock::SimCommand
{
public:
   enum PilotType
   {
      MANUAL,
      AUGMENTED,
      HARDWARE,
      SYNTHETIC,
      GUIDANCE
   };

   ActivatePilotCommand(const std::string& aConnectedPlatform, PilotType aPilotType)
      : mConnectedPlatform(aConnectedPlatform)
      , mPilotType(aPilotType)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mConnectedPlatform;
   PilotType   mPilotType;
};

// This defines a SimCommand for setting the flight controls input data,
// such as stick, rudder, and throttle (plus other control inputs).
class ControlCommand : public warlock::SimCommand
{
public:
   struct ControlData
   {
      float roll;
      float pitch;
      float rudder;
      float throttle;
      float spdBrake;
      float landingGear;
      float trimNoseUp;
      float trimRollRight;
      float trimYawRight;

      ControlData()
         : roll(0.0f)
         , pitch(0.0f)
         , rudder(0.0f)
         , throttle(0.0f)
         , spdBrake(0.0f)
         , landingGear(0.0f)
         , trimNoseUp(0.0f)
         , trimRollRight(0.0f)
         , trimYawRight(0.0f)
      {
      }
   };

   ControlCommand(const std::string& aConnectedPlatform, const ControlData& aControlData);

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mConnectedPlatform;
   ControlData mControlData;

   static double mLastSimulationTime_sec;
};

// This is a command for sending hud data over the event pipe.
class HUD_DataCommand : public warlock::SimCommand
{
public:
   // Using a struct here even though it's only one variable
   // at the moment.  More things will get added later.
   struct HUD_Data
   {
      JoystickDataContainer::eHudMode hudMode;
   };

   HUD_DataCommand(const std::string& aConnectedPlatform, const HUD_Data& aHUDData);

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mConnectedPlatform;
   HUD_Data    mHUD_Data;
};
} // namespace Joystick
#endif // JOYSTICKSIMCOMMANDS_HPP
