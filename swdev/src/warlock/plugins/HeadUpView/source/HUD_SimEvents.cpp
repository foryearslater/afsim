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

#include "HUD_SimEvents.hpp"

void HeadsUpDisplay::SimulationInitializingEvent::Process(wkf::HUD_DataContainer& aData)
{
   aData.SetSimulationInitialized(true);
}

void HeadsUpDisplay::UpdateEvent::Process(wkf::HUD_DataContainer& aData)
{
   const std::map<std::string, wkf::HUD_DataContainer::PlatformData>& tempData = mData.GetPlatformData();
   aData.SetPlatformData(tempData);
}
