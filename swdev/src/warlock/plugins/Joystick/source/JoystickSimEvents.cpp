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

#include "JoystickSimEvents.hpp"

#include "WsfPlatform.hpp"

void Joystick::SimulationInitializingEvent::Process(JoystickDataContainer& aData)
{
   aData.SetSimulationInitialized(true);
}

void Joystick::PlatformAddedEvent::Process(JoystickDataContainer& aData)
{
   aData.PlatformAdded(mPlatform);
}

void Joystick::PlatformDeletedEvent::Process(JoystickDataContainer& aData)
{
   aData.PlatformDeleted(mPlatform);
}
