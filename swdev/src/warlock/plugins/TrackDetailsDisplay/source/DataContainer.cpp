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

#include "DataContainer.hpp"

void WkTrackDetailsDisplay::DataContainer::AddPlatform(const std::string& aPlatformName)
{
   emit PlatformAdded(aPlatformName);
}

void WkTrackDetailsDisplay::DataContainer::RemovePlatform(const std::string& aPlatformName)
{
   emit PlatformRemoved(aPlatformName);
}

void WkTrackDetailsDisplay::DataContainer::SetData(const PlatformsTrackData& aPlatformTrackData)
{
   mData = aPlatformTrackData;
   emit DataUpdated();
}

WkTrackDetailsDisplay::PlatformsTrackData WkTrackDetailsDisplay::DataContainer::GetData() const
{
   return mData;
}
