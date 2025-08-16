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

#include "ZoneBrowserSimCommands.hpp"

#include <vector>

#include "UtColor.hpp"
#include "UtOptional.hpp"
#include "UtQt.hpp"
#include "WkObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfZone.hpp"
#include "WsfZoneReference.hpp"
#include "WsfZoneTypes.hpp"
#include "ZoneBrowserSimEvents.hpp"


void WkZoneBrowser::UpdateZoneColorCommand::Process(WsfSimulation& aSimulation)
{
   auto isEqual = [&](WsfZone* aZone) { return (aZone->GetName() == mZone); };

   WsfPlatform* platformPtr{nullptr};
   if (mPlatform == "Global Zones")
   {
      std::vector<WsfZone*> globalZoneData;
      WsfZoneTypes&         globalZones = aSimulation.GetScenario().GetZoneTypes();
      globalZones.GetCurrentTypes(globalZoneData);
      auto it = std::find_if(globalZoneData.begin(), globalZoneData.end(), isEqual);
      if (it != globalZoneData.end())
      {
         auto fillColor((*it)->GetFillColor());
         if (fillColor.has_value())
         {
            mFillColor = UtQtColor(fillColor.value());
         }

         auto lineColor((*it)->GetLineColor());
         if (lineColor.has_value())
         {
            mLineColor = UtQtColor(lineColor.value());
         }
      }
   }
   else
   {
      platformPtr = aSimulation.GetPlatformByName(mPlatform);
      if (platformPtr)
      {
         auto zonePtr = dynamic_cast<WsfZone*>(platformPtr->FindComponent(mZone));
         if (zonePtr)
         {
            auto fillColor(zonePtr->GetFillColor());
            auto lineColor(zonePtr->GetLineColor());

            auto refPtr = dynamic_cast<WsfZoneReference*>(zonePtr);
            if (refPtr)
            {
               if (!fillColor.has_value())
               {
                  fillColor = refPtr->GetReferencedZone()->GetFillColor();
               }
               if (!lineColor.has_value())
               {
                  lineColor = refPtr->GetReferencedZone()->GetLineColor();
               }
            }

            if (fillColor.has_value())
            {
               mFillColor = UtQtColor(fillColor.value());
            }
            if (lineColor.has_value())
            {
               mLineColor = UtQtColor(lineColor.value());
            }
         }
      }
   }
   AddSimEvent(ut::make_unique<UpdateZoneColor>(mPlatform, mZone, mFillColor, mLineColor));
}

void WkZoneBrowser::SetFillColorCommand::Process(WsfSimulation& aSimulation)
{
   auto isEqual = [&](WsfZone* aZone) { return (aZone->GetName() == mZone); };

   WsfPlatform* platformPtr{nullptr};
   if (mPlatform == "Global Zones")
   {
      std::vector<WsfZone*> globalZoneData;
      WsfZoneTypes&         globalZones = aSimulation.GetScenario().GetZoneTypes();
      globalZones.GetCurrentTypes(globalZoneData);
      auto it = std::find_if(globalZoneData.begin(), globalZoneData.end(), isEqual);
      if (it != globalZoneData.end())
      {
         (*it)->SetFillColor(UtQtColor(mFillColor));
      }
   }
   else
   {
      platformPtr = aSimulation.GetPlatformByName(mPlatform);
      if (platformPtr)
      {
         auto zonePtr = dynamic_cast<WsfZone*>(platformPtr->FindComponent(mZone));
         if (zonePtr)
         {
            zonePtr->SetFillColor(UtQtColor(mFillColor));
         }
      }
   }
   AddSimEvent(ut::make_unique<UpdateFillColor>(mPlatform, mZone, mFillColor));

   WkObserver::SimulationUserAction(aSimulation)("Zone fill color changed", platformPtr);
}

void WkZoneBrowser::SetLineColorCommand::Process(WsfSimulation& aSimulation)
{
   auto isEqual = [&](WsfZone* aZone) { return (aZone->GetName() == mZone); };

   WsfPlatform* platformPtr{nullptr};
   if (mPlatform == "Global Zones")
   {
      std::vector<WsfZone*> globalZoneData;
      WsfZoneTypes&         globalZones = aSimulation.GetScenario().GetZoneTypes();
      globalZones.GetCurrentTypes(globalZoneData);
      auto it = std::find_if(globalZoneData.begin(), globalZoneData.end(), isEqual);
      if (it != globalZoneData.end())
      {
         (*it)->SetLineColor(UtQtColor(mLineColor));
      }
   }
   else
   {
      platformPtr = aSimulation.GetPlatformByName(mPlatform);
      if (platformPtr)
      {
         auto zonePtr = dynamic_cast<WsfZone*>(platformPtr->FindComponent(mZone));
         if (zonePtr)
         {
            zonePtr->SetLineColor(UtQtColor(mLineColor));
         }
      }
   }
   AddSimEvent(ut::make_unique<UpdateLineColor>(mPlatform, mZone, mLineColor));

   WkObserver::SimulationUserAction(aSimulation)("Zone line color changed", platformPtr);
}
