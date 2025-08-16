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

#ifndef JOYSTICKSIMEVENTS_HPP
#define JOYSTICKSIMEVENTS_HPP

#include <string>

#include "JoystickDataContainer.hpp"
#include "WkSimInterface.hpp"

// This is a collection of SimEvents that are used for the Joystick

namespace Joystick
{
class JoystickEvent : public warlock::SimEvent
{
public:
   virtual void Process(JoystickDataContainer& aData) = 0;
};

// This defines a base class for SimEvents for the Common HUD
class WkJoystickEvent : public JoystickEvent
{
public:
   WkJoystickEvent(bool aRecurring = false) {}
   virtual void Process(JoystickDataContainer& aData) = 0;
};

// This is the SimEvent for sim initialization
class SimulationInitializingEvent : public WkJoystickEvent
{
public:
   SimulationInitializingEvent()
      : WkJoystickEvent(false)
   {
   }

   void Process(JoystickDataContainer& aData) override;
};

// This is the SimEvent for adding a platform
class PlatformAddedEvent : public WkJoystickEvent
{
public:
   PlatformAddedEvent(const std::string aPlatform)
      : WkJoystickEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(JoystickDataContainer& aData) override;

private:
   std::string mPlatform;
};

// This is the SimEvent for deleting a platform
class PlatformDeletedEvent : public WkJoystickEvent
{
public:
   PlatformDeletedEvent(const std::string& aPlatform)
      : WkJoystickEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(JoystickDataContainer& aData) override;

private:
   std::string mPlatform;
};
} // namespace Joystick

#endif // JOYSTICKSIMEVENTS_HPP
