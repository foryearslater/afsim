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
#include "SimControllerSimInterface.hpp"

#include "DisControlEnums.hpp"
#include "SimControllerDataContainer.hpp"
#include "SimControllerSimCommands.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WsfClockSource.hpp"
#include "WsfDateTime.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationObserver.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisStartResume.hpp"
#include "dis/WsfDisStopFreeze.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"
#include "xio_sim/WsfXIO_Simulation.hpp"

WkSimController::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<SimControllerEvent>(aPluginName)
{
}

void WkSimController::SimInterface::OnFallingBehindThresholdChanged(double aThreshold)
{
   mFallingBehindThreshold.store(aThreshold);
}

void WkSimController::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   WallClockRead(aSimulation);
   AddSimEvent(ut::make_unique<SimCompleteEvent>(aSimulation.GetSimTime()));
}

void WkSimController::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimStartingEvent>(aSimulation.IsExternallyStarted()));
}

void WkSimController::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);

   mCallbacks.Clear();

   mCallbacks.Add(
      WsfObserver::SimulationPausing(&aSimulation).Connect([this]() { AddSimEvent(ut::make_unique<SimPausingEvent>()); }));

   mCallbacks.Add(WsfObserver::SimulationResuming(&aSimulation)
                     .Connect([this]() { AddSimEvent(ut::make_unique<SimResumingEvent>()); }));

   mCallbacks.Add(WsfObserver::SimulationClockRateChange(&aSimulation)
                     .Connect([this](double aClockRate) { AddSimEvent(ut::make_unique<SimClockRateEvent>(aClockRate)); }));

   auto* xioPtr = WsfXIO_Extension::Find(aSimulation);
   if (xioPtr)
   {
      xioPtr->mFallingBehindThreshold = mFallingBehindThreshold.load();

      mCallbacks.Add(xioPtr->OnApplicationBehind.Connect(
         [this](bool aRemoteBehind, const std::string& aApplicationId)
         { AddSimEvent(ut::make_unique<TimeBehindEvent>(aRemoteBehind, aApplicationId)); }));
   }
}

void WkSimController::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   SimControllerDataContainer::SimState state;
   state.mSimTime = aSimulation.GetSimTime();
   // GetCurrentTime() expects mWallTime to a valid UtCalendar with the last correct value stored in it.
   //  This is why we have to have mWallTime be a member variable vs a local variable.
   aSimulation.GetDateTime().GetCurrentTime(state.mSimTime, mWallTime);
   state.mWallTime   = mWallTime;
   state.mEndTime    = aSimulation.GetEndTime();
   state.mSimState   = aSimulation.GetState();
   state.mPaused     = aSimulation.IsActive() && aSimulation.GetClockSource()->IsStopped();
   state.mClockRate  = aSimulation.GetClockRate();
   state.mTimeBehind = aSimulation.GetTimeBehind();
   state.mIsRealTime = aSimulation.IsRealTime();
   AddSimEvent(ut::make_unique<SimStateEvent>(state));
}
