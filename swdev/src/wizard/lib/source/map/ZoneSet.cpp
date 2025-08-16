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

#include "ZoneSet.hpp"

#include "ProxyChange.hpp"
#include "ProxyUtil.hpp"
#include "ProxyWatcher.hpp"
#include "WsfPM_Root.hpp"
#include "Zone.hpp"

wizard::ZoneSet::ZoneSet(const WsfPM_Zone& aZone)
   : mZoneSetName{aZone.GetName()}
   , mParentPlatformName{aZone.ParentPlatform()}
   , mZoneNode{aZone}
{
   AddZoneSet(aZone);
}

void wizard::ZoneSet::AddZoneSet(const WsfPM_Zone& aZone)
{
   if (aZone.IsZoneSet())
   {
      WsfPM_ZoneSet zoneSetNode{aZone};

      WsfPM_List               usedInclusionZones     = zoneSetNode.InclusionZones();
      WsfPM_List               usedExclusionZones     = zoneSetNode.ExclusionZones();
      WsfPM_ZoneDefinitionList embeddedInclusionZones = zoneSetNode.EmbeddedInclusionZones();
      WsfPM_ZoneDefinitionList embeddedExclusionZones = zoneSetNode.EmbeddedExclusionZones();

      int addedZoneIndex = 0;

      for (size_t i = 0; i < usedInclusionZones.size(); i++)
      {
         // the proxy node is just a list of strings, we need to look the zone up ourselves
         std::string          zoneName    = usedInclusionZones[i].GetValue().ValueToString();
         WsfPM_ZoneDefinition zoneNode    = ProxyUtil::FindZoneNodeByName(zoneName);
         std::string          zoneDefName = zoneNode.GetName();
         if (!zoneNode.ReferencePlatform().empty() && !zoneNode.IsReferencePlatformValid())
         {
            // don't render zones that have non-existent reference platforms
            continue;
         }
         else
         {
            addedZoneIndex++;
            auto zonePtr = ut::make_unique<Zone>(zoneNode, this, addedZoneIndex);
            zonePtr->LinkProxyCallbacks();
            mSubzones.push_back(std::move(zonePtr));
         }
      }

      for (size_t i = 0; i < usedExclusionZones.size(); i++)
      {
         // the proxy node is just a list of strings, we need to look the zone up ourselves
         std::string          zoneName = usedExclusionZones[i].GetValue().ValueToString();
         WsfPM_ZoneDefinition zoneNode = ProxyUtil::FindZoneNodeByName(zoneName);
         if (!zoneNode.ReferencePlatform().empty() && !zoneNode.IsReferencePlatformValid())
         {
            // don't render zones that have non-existent reference platforms
            continue;
         }
         else
         {
            addedZoneIndex++;
            auto zonePtr = ut::make_unique<Zone>(zoneNode, this, addedZoneIndex);
            zonePtr->LinkProxyCallbacks();
            mSubzones.push_back(std::move(zonePtr));
         }
      }

      for (size_t i = 0; i < embeddedInclusionZones.size(); i++)
      {
         WsfPM_ZoneDefinition zoneNode{embeddedInclusionZones[i]};
         if (!zoneNode.ReferencePlatform().empty() && !zoneNode.IsReferencePlatformValid())
         {
            // don't render zones that have non-existent reference platforms
            continue;
         }
         else
         {
            addedZoneIndex++;
            auto zonePtr = ut::make_unique<Zone>(zoneNode, this, addedZoneIndex);
            // don't link proxy callbacks for embedded zones
            mSubzones.push_back(std::move(zonePtr));
         }
      }

      for (size_t i = 0; i < embeddedExclusionZones.size(); i++)
      {
         WsfPM_ZoneDefinition zoneNode{embeddedExclusionZones[i]};
         if (!zoneNode.ReferencePlatform().empty() && !zoneNode.IsReferencePlatformValid())
         {
            // don't render zones that have non-existent reference platforms
            continue;
         }
         else
         {
            addedZoneIndex++;
            auto zonePtr = ut::make_unique<Zone>(zoneNode, this, addedZoneIndex);
            // don't link proxy updates for embedded zones
            mSubzones.push_back(std::move(zonePtr));
         }
      }

      // link proxy updates
      auto updateCallbackPtr =
         wizard::WatchProxy(zoneSetNode.GetPath()).AnyUpdate.Connect(&ZoneSet::OnProxyModified, this);
      mUpdateCallbackPtr = updateCallbackPtr.get();
      mProxyCallbacks.Add(std::move(updateCallbackPtr));
   }
   else if (aZone.IsDefinition())
   {
      WsfPM_ZoneDefinition zoneNode{aZone};

      auto zonePtr = ut::make_unique<Zone>(zoneNode, this, 1);
      zonePtr->LinkProxyCallbacks();
      mSubzones.push_back(std::move(zonePtr));
   }
}

void wizard::ZoneSet::OnProxyModified(const ProxyChange& aProxyChange)
{
   // only interested in updates
   if (aProxyChange.reason() != ProxyChange::Reason::cUPDATED)
   {
      return;
   }

   WsfPProxyNode node = aProxyChange.changed();
   WsfPM_Root    rootNode{node.Root()};
   std::string   type = node.GetTypeName();

   // move up proxy tree until we find the zone set that changed
   while (type != "WSF_ZONE_SET" && node.IsValid())
   {
      node = node.GetParent();
      type = node.GetTypeName();
      if (node == rootNode)
      {
         return;
      }
   }

   WsfPM_Zone zoneSetNode{node};
   UpdateZoneSet(zoneSetNode);
}

void wizard::ZoneSet::UpdateZoneSet(const WsfPM_Zone& aZone)
{
   // TODO: implement more efficient update
   RemoveZoneSet();
   AddZoneSet(aZone);
}

void wizard::ZoneSet::RemoveZoneSet()
{
   for (auto& zonePtr : mSubzones)
   {
      zonePtr->RemoveZone();
   }
   mSubzones.clear();
   mProxyCallbacks.Remove(mUpdateCallbackPtr);
   mUpdateCallbackPtr = nullptr;
}

bool wizard::ZoneSet::SubzoneReferencesPlatform(const std::string& aRefPlatName) const
{
   for (const auto& zone : mSubzones)
   {
      if (zone->GetReferencePlatformName() == aRefPlatName)
      {
         return true;
      }
   }

   return false;
}

std::vector<wizard::Zone*> wizard::ZoneSet::GetZonesWithReferencePlatform(const std::string& aRefPlatName) const
{
   std::vector<Zone*> referencingZones;

   for (const auto& zone : mSubzones)
   {
      if (zone->GetReferencePlatformName() == aRefPlatName)
      {
         referencingZones.push_back(zone.get());
      }
   }

   return referencingZones;
}

wizard::Zone* wizard::ZoneSet::GetZoneByAttachment(const wkf::AttachmentZone* aAttachmentZone) const
{
   for (const auto& zonePtr : mSubzones)
   {
      if (zonePtr->GetZone() == aAttachmentZone)
      {
         return zonePtr.get();
      }
   }

   return nullptr;
}
