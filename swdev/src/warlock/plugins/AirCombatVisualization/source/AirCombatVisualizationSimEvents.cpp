// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AirCombatVisualizationSimEvents.hpp"

void WkAirCombat::UpdateEvent::Process(wkf::DataContainer& aState)
{
   aState.SetEngagementDataList(mData);
}

void WkAirCombat::PlatformAddedEvent::Process(wkf::DataContainer& aState)
{
   aState.AddViablePlatform(mPlatform);
}

void WkAirCombat::PlatformDeletedEvent::Process(wkf::DataContainer& aState)
{
   aState.RemoveViablePlatform(mPlatform);
}

void WkAirCombat::SimulationCompleteEvent::Process(wkf::DataContainer& aDataContainer)
{
   aDataContainer.Clear();
}
