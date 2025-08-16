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

#ifndef ZONESET_HPP
#define ZONESET_HPP

#include <memory>
#include <string>
#include <vector>

#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "WsfPM_Zone.hpp"

namespace wkf
{
class AttachmentZone;
}

namespace wizard
{
class Zone;
class ProxyChange;

class VI_EXPORT ZoneSet
{
public:
   ZoneSet(const ZoneSet& aRhs) = delete; // vector<unique_ptr> requires deletion of default copy constructor
   ZoneSet(const WsfPM_Zone& aZone);
   ~ZoneSet() = default;

   //! Visually updates the zone set to match the given zone node.
   //! @param aZone is the zone proxy node to refer to when updating the zone
   void UpdateZoneSet(const WsfPM_Zone& aZone);

   //! Visually removes the zone from the map display.
   void RemoveZoneSet();

   //! Finds all subzones with the given reference platform.
   //! @param aRefPlatName is the name of a platform
   //! @returns a vector containing zones that have the given platform as a reference platform
   std::vector<Zone*> GetZonesWithReferencePlatform(const std::string& aRefPlatName) const;

   //! Returns whether any subzone uses the given platform as a reference platform.
   //! @param aRefPlatName is the name of a platform
   //! @returns whether any subzone uses the given platform as a reference platform
   bool SubzoneReferencesPlatform(const std::string& aRefPlatName) const;

   //! Finds a zone by checking attachment equality.
   //! @param aAttachmentZone is an attachment zone to use as an equality check
   //! @returns a non-owning pointer to the corresponding zone, or nullptr if there is no zone
   Zone* GetZoneByAttachment(const wkf::AttachmentZone* aAttachmentZone) const;

   //! Returns the name of the zone/zone set.
   std::string GetZoneSetName() const { return mZoneSetName; }

   //! Returns the name of the parent platform of this zone/zone set.
   std::string GetParentPlatformName() const { return mParentPlatformName; }

   //! Returns the node corresponding to this zone/zone set.
   WsfPM_Zone GetZoneSetNode() const { return mZoneNode; }

   //! Returns whether this zone/zone set is defined globally (i.e., not on a platform).
   bool IsGlobal() const { return mParentPlatformName == "Global Zones"; }

private:
   //! Adds this zone to Wizard's map display.
   //! @param aZone is the zone proxy node to which this zone/zone set refers to
   void AddZoneSet(const WsfPM_Zone& aZone);

   //! Handles proxy updates for the proxy nodes associated with this zone/zone set.
   //! @param aProxyChange is the change occuring to this zone's corresponding proxy node
   void OnProxyModified(const ProxyChange& aProxyChange);

   std::string mZoneSetName;

   std::string mParentPlatformName;

   WsfPM_Zone mZoneNode;

   std::vector<std::unique_ptr<Zone>> mSubzones;

   UtCallbackHolder mProxyCallbacks;
   UtCallback*      mUpdateCallbackPtr{nullptr};
};
}; // namespace wizard

#endif
