// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserSimEvents.hpp"

#include "PlatformPartBrowserDataContainer.hpp"

void WkPlatformPartBrowser::PartUpdateEvent::Process(DataContainer& aData)
{
   aData.UpdateParts(mParts);
}

void WkPlatformPartBrowser::PartPropertyUpdateEvent::Process(DataContainer& aData)
{
   aData.UpdateProperty(mUniqueId, mPropertyName, mPropertyValue);
}

void WkPlatformPartBrowser::PartListEvent::Process(DataContainer& aData)
{
   aData.SetParts(mPlatformName, mParts);
}