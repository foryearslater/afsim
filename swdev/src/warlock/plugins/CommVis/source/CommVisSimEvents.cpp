// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CommVisSimEvents.hpp"

WkCommVis::CommVisUpdateEvent::CommVisUpdateEvent(const wkf::CommEvent& aCommEvent)
   : mCommEvent(aCommEvent)
{
}

void WkCommVis::SimulationCompleteEvent::Process(wkf::DataContainer& aData)
{
   aData.Reset();
}

void WkCommVis::SimInitializedEvent::Process(wkf::DataContainer& aData)
{
   aData.Reset();
}

void WkCommVis::CommVisUpdateEvent::Process(wkf::DataContainer& aData)
{
   aData.AddCommData(mCommEvent);
}
