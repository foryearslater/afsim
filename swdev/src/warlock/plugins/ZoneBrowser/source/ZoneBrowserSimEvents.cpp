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
#include "ZoneBrowserSimEvents.hpp"

#include "UtOptional.hpp"
#include "UtQt.hpp"
#include "zone_browser/WkfZoneBrowserDataContainer.hpp"

void WkZoneBrowser::UpdateZoneColor::Process(wkf::ZoneBrowserDataContainer& aData)
{
   wkf::ZoneSetData* zoneSetPtr{nullptr};
   zoneSetPtr = mPlatform == "Global Zones" ? aData.GetGlobalZone(mZone) : aData.GetLocalZone(mPlatform, mZone);
   if (zoneSetPtr)
   {
      auto                  zoneSet = zoneSetPtr;
      ut::optional<UtColor> fillColor(UtQtColor(mFillColor));
      ut::optional<UtColor> lineColor(UtQtColor(mLineColor));
      zoneSetPtr->SetFillColor(fillColor);
      zoneSetPtr->SetLineColor(lineColor);
      aData.UpdateColor(*zoneSet);
   }
}

void WkZoneBrowser::UpdateFillColor::Process(wkf::ZoneBrowserDataContainer& aData)
{
   wkf::ZoneSetData* zoneSetPtr{nullptr};
   zoneSetPtr = mPlatform == "Global Zones" ? aData.GetGlobalZone(mZone) : aData.GetLocalZone(mPlatform, mZone);
   if (zoneSetPtr)
   {
      auto                  zoneSet = zoneSetPtr;
      ut::optional<UtColor> fillColor(UtQtColor(mColor));
      zoneSetPtr->SetFillColor(fillColor);
      aData.UpdateColor(*zoneSet);
   }
}

void WkZoneBrowser::UpdateLineColor::Process(wkf::ZoneBrowserDataContainer& aData)
{
   wkf::ZoneSetData* zoneSetPtr{nullptr};
   zoneSetPtr = mPlatform == "Global Zones" ? aData.GetGlobalZone(mZone) : aData.GetLocalZone(mPlatform, mZone);
   if (zoneSetPtr)
   {
      auto                  zoneSet = zoneSetPtr;
      ut::optional<UtColor> lineColor(UtQtColor(mColor));
      zoneSetPtr->SetLineColor(lineColor);
      aData.UpdateColor(*zoneSet);
   }
}
