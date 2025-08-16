// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TrackDetailsSimEvents.hpp"

WkTrackDetailsDisplay::AddPlatformEvent::AddPlatformEvent(const std::string& aName)
   : EventBase(false)
   , mPlatformName(aName)
{
}

void WkTrackDetailsDisplay::AddPlatformEvent::Process(DataContainer& aDataContainer)
{
   aDataContainer.AddPlatform(mPlatformName);
}

WkTrackDetailsDisplay::DeletePlatformEvent::DeletePlatformEvent(const std::string& aName)
   : EventBase(false)
   , mPlatformName(aName)
{
}

void WkTrackDetailsDisplay::DeletePlatformEvent::Process(DataContainer& aDataContainer)
{
   aDataContainer.RemovePlatform(mPlatformName);
}

WkTrackDetailsDisplay::UpdateEvent::UpdateEvent(const PlatformsTrackData& aPlatform)
   : EventBase(true)
   , mPlatformData(aPlatform)
{
}

void WkTrackDetailsDisplay::UpdateEvent::Process(DataContainer& aDataContainer)
{
   aDataContainer.SetData(mPlatformData);
}
