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

#include "JoystickSimInterface.hpp"

#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSixDOF_Mover.hpp"

Joystick::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<WkJoystickEvent>(aPluginName)
{
}

void Joystick::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimulationInitializingEvent>());
}

void Joystick::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   // Verify that we have a P6DOF or SixDOF mover
   if (dynamic_cast<wsf::six_dof::Mover*>(aPlatform.GetMover()) || dynamic_cast<WsfP6DOF_Mover*>(aPlatform.GetMover()))
   {
      AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
   }
}

void Joystick::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform.GetName()));
}
