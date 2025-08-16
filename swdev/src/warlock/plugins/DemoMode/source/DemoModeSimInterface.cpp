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
#include "DemoModeSimInterface.hpp"

#include "DemoModeSimCommands.hpp"
#include "UtMemory.hpp"
#include "WsfSimulation.hpp"

WkDemoMode::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<DemoModeEvent>(aPluginName)
   , mRunSpeedMultiplier(1.0)
{
   SetEnabled(false); // Default to disabled
}

void WkDemoMode::SimInterface::SetRunSpeedMultiplier(double aRunSpeedMultiplier)
{
   QMutexLocker locker(&mMutex);
   mRunSpeedMultiplier = aRunSpeedMultiplier;
}

void WkDemoMode::SimInterface::ProcessEnableFlagChanged(bool aEnabled)
{
   if (aEnabled)
   {
      AddSimCommand(ut::make_unique<RunSpeedCommand>(mRunSpeedMultiplier));
   }
}

void WkDemoMode::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimCompleteEvent>());
   }
}

void WkDemoMode::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimCommand(ut::make_unique<RunSpeedCommand>(mRunSpeedMultiplier));
   }
}
