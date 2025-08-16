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

#include "SimControllerSimEvents.hpp"

void WkSimController::SimPausingEvent::Process(SimControllerDataContainer& aState)
{
   emit aState.SimPausing();
}

void WkSimController::SimResumingEvent::Process(SimControllerDataContainer& aState)
{
   emit aState.SimResuming();
}

void WkSimController::SimCompleteEvent::Process(SimControllerDataContainer& aState)
{
   emit aState.SimComplete(mSimTime);
}

void WkSimController::SimClockRateEvent::Process(SimControllerDataContainer& aState)
{
   emit aState.SimClockRateChanged(mClockRate);
}

void WkSimController::SimStartingEvent::Process(SimControllerDataContainer& aState)
{
   aState.SetStarting(mPaused);
}

void WkSimController::SimStateEvent::Process(SimControllerDataContainer& aState)
{
   aState.SetSimState(mState);
}

void WkSimController::TimeBehindEvent::Process(SimControllerDataContainer& aState)
{
   aState.SetApplicationBehind(mApplicationId, mIsBehind);
}
