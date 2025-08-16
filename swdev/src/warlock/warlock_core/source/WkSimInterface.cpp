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
#include "WkSimInterface.hpp"

#include "WkSimEnvironment.hpp"
#include "WkfExceptionMessage.hpp"

warlock::SimEvent::~SimEvent() = default;

warlock::SimInterfaceBase::SimInterfaceBase(const QString& aName)
   : QObject(nullptr)
   , mName(aName)
   , mEnabled(true)
{
   // Use a Direct connection to invoke the slot immediately, in the simulation thread
   connect(&simEnv,
           &SimEnvironment::PlatformAdded,
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, PlatformAdded, aName),
           Qt::DirectConnection);
   connect(&simEnv,
           &SimEnvironment::PlatformInitialized,
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, PlatformInitialized, aName),
           Qt::DirectConnection);
   connect(&simEnv,
           &SimEnvironment::PlatformDeleted,
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, PlatformDeleted, aName),
           Qt::DirectConnection);
   connect(&simEnv,
           QOverload<const WsfSimulation&>::of(&SimEnvironment::SimulationInitializing),
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, SimulationInitializing, aName),
           Qt::DirectConnection);
   connect(&simEnv,
           &SimEnvironment::SimulationStarting,
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, SimulationStarting, aName),
           Qt::DirectConnection);
   connect(&simEnv,
           &SimEnvironment::SimulationComplete,
           this,
           WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, SimulationComplete, aName),
           Qt::DirectConnection);
   UpdatePeriodicConnections();
}

warlock::SimInterfaceBase::~SimInterfaceBase() = default;

void warlock::SimInterfaceBase::SetEnabled(bool aEnabled)
{
   bool expected = !aEnabled;
   if (mEnabled.compare_exchange_strong(expected, aEnabled))
   {
      UpdatePeriodicConnections();
      ProcessEnableFlagChanged(aEnabled);
   }
}

void warlock::SimInterfaceBase::AddSimCommand(std::unique_ptr<SimCommand> aCommand)
{
   auto& queue = aCommand->UseWallClock() ? mSimCommandsWallClock : mSimCommandsSimClock;
   queue.Push(std::move(aCommand));
}

void warlock::SimInterfaceBase::WallClockCommands(WsfSimulation& aSimulation)
{
   ProcessCommands(aSimulation, mSimCommandsWallClock);
}

void warlock::SimInterfaceBase::SimulationClockCommands(WsfSimulation& aSimulation)
{
   ProcessCommands(aSimulation, mSimCommandsSimClock);
}

void warlock::SimInterfaceBase::ProcessCommands(WsfSimulation& aSimulation, SimCommandQueue& aCommands)
{
   while (!aCommands.Empty())
   {
      aCommands.Pop()->Process(aSimulation);
   }
}

void warlock::SimInterfaceBase::UpdatePeriodicConnections()
{
   while (!mConnections.empty())
   {
      disconnect(mConnections.takeFirst());
   }

   if (mEnabled)
   {
      mConnections << connect(&simEnv,
                              &SimEnvironment::WallClockRead,
                              this,
                              WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, WallClockRead, mName),
                              Qt::DirectConnection);
      mConnections << connect(&simEnv,
                              &SimEnvironment::WallClockWrite,
                              this,
                              WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, WallClockCommands, mName),
                              Qt::DirectConnection);
      mConnections << connect(&simEnv,
                              &SimEnvironment::SimulationClockRead,
                              this,
                              WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, SimulationClockRead, mName),
                              Qt::DirectConnection);
      mConnections << connect(&simEnv,
                              &SimEnvironment::SimulationClockWrite,
                              this,
                              WKF_EXCEPTION_HANDLER_QUEUED(SimInterfaceBase, SimulationClockCommands, mName),
                              Qt::DirectConnection);
   }
}
