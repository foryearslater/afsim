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

#include "SimEvents.hpp"

WarlockTraining::UpdateEvent::UpdateEvent(const std::map<std::string, PlatformData>& aData)
// This is a recurring event, thus aRecurring = true in the EventBase constructor, 
// we inform the SimInterface class to only process the most recent event.
// This helps reduce computational time spent processing events.
   : EventBase(true)
   , mData(aData)
{
}

void WarlockTraining::UpdateEvent::Process(DataContainer& aDataContainer)
{
   // EXERCISE 1 TASK 1b
   // Add mData to the DataContainer
   aDataContainer.SetData(mData);
}
