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

#include "SimControllerDataContainer.hpp"

void WkSimController::SimControllerDataContainer::SetSimState(const SimState& aSimState)
{
   bool realTimeChanged = (mSimStateData.mIsRealTime != aSimState.mIsRealTime);
   mSimStateData        = aSimState;
   if (realTimeChanged)
   {
      emit RealTimeChanged(mSimStateData.mIsRealTime);
   }
}

void WkSimController::SimControllerDataContainer::SetStarting(bool aPaused)
{
   mSimStateData.mPaused   = aPaused;
   mSimStateData.mSimState = WsfSimulation::cSTARTING;
   mOthersBehindSet.clear();
   emit Initialized();
}

void WkSimController::SimControllerDataContainer::SetApplicationBehind(const std::string& aApplicationId, bool aRemoteBehind)
{
   // add or remove Ids from the set as remote simulations fall behind/catch up
   if (aRemoteBehind)
   {
      mOthersBehindSet.emplace(aApplicationId);
      emit RemoteSimulationBehind();
   }
   else
   {
      mOthersBehindSet.erase(aApplicationId);
   }
}
