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

#include "SA_DisplayEvents.hpp"

void WkSA_Display::UpdateSA_DataEvent::Process(wkf::SA_Display::DataContainer& aData)
{
   aData.SetSA_Data(mData.GetSA_Data());
   aData.SetSA_TruthPlatforms(mData.GetSA_TruthPlatforms());
}

void WkSA_Display::SimulationInitializingEvent::Process(wkf::SA_Display::DataContainer& aData)
{
   aData.SetSimulationInitialized(true);
}
