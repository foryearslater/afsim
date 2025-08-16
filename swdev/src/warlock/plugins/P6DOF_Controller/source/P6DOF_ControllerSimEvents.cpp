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

#include "P6DOF_ControllerSimEvents.hpp"

#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"

void WkP6DOF_Controller::SimulationInitializingEvent::Process(P6DOF_ControllerDataContainer& aData)
{
   aData.SetSimulationInitialized(true);
}

void WkP6DOF_Controller::PlatformAddedEvent::Process(P6DOF_ControllerDataContainer& aData)
{
   aData.PlatformAdded(mPlatform);
}

void WkP6DOF_Controller::PlatformDeletedEvent::Process(P6DOF_ControllerDataContainer& aData)
{
   aData.PlatformDeleted(mPlatform);
}

void WkP6DOF_Controller::UpdateEvent::Process(P6DOF_ControllerDataContainer& aData)
{
   const P6DOF_ControllerDataContainer::PlatformData& tempData = mData.GetPlatformData();
   aData.SetPlatformData(tempData);
}
