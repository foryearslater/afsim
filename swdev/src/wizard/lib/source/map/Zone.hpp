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

#ifndef ZONE_HPP
#define ZONE_HPP

#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "WsfPM_Zone.hpp"

namespace vespa
{

class VaEntity;
} // namespace vespa
namespace wkf
{
class AttachmentZoneCircle;
class AttachmentZoneEllipse;
class AttachmentZonePolygon;
class AttachmentZone;
} // namespace wkf
namespace wizard
{
class ZoneSet;
class ProxyChange;
class AttachmentZoneCircle;
class AttachmentZoneEllipse;
class AttachmentZonePolygon;
class AttachmentZoneSphere;

class VI_EXPORT Zone
{
public:
   // Zone(WsfPM_Zone& aZoneNode);
   Zone(WsfPM_ZoneDefinition& aZoneNode, ZoneSet* aOwningZoneSetPtr, int aZoneSetIndex);
   ~Zone() = default;

   //! @brief Get the reference platform name
   //! @returns the reference platform name
   const std::string GetReferencePlatformName() const { return mReferencePlatformName; }

   //! @brief Remove the zone and its anchor from the scenario
   void RemoveZone();

   //! @brief Get whether or not this is a global zone
   //! @returns true if this is a global zone
   bool IsGlobal() { return mIsGlobal; }

   //! @brief Get the AttachmentZone associated with this zone
   //! @returns a pointer to the AttachmentZone associated with this zone
   wkf::AttachmentZone* GetZone() { return mZonePtr; }


   //! @brief Get the zone set name
   //! @returns the zone set name
   const std::string GetZoneSetName() const;

   //! @brief Get the parent platform name
   const std::string GetParentPlatformName() const;

   //!
   std::string GetDefinitionName() const { return mZoneDefName; }

   //! Get the definition's parent platform name
   std::string GetDefinitionParentPlatformName() const { return mZoneDefParentPlatName; }

   void LinkProxyCallbacks();

   void UnlinkProxyCallbacks();


   //! @brief Get the zone proxy node
   //! @returns the zone proxy node
   WsfPM_Zone& GetZoneProxyNode() { return mZoneProxyNode; }


   //! @brief Updates a zone along with it's anchor platform if necessary
   //! @param aZone is a proxy Zone node
   void UpdateZone(WsfPM_ZoneDefinition& aZone);

private:
   //! @brief This will get called when the zone is modified via the proxy
   //! @param aProxyChange is a change recorded by the proxy
   void OnProxyModified(const ProxyChange& aProxyChange);

   //! @brief Creates a zone along with it's anchor platform if necessary
   //! @param aZone is proxy Zone node
   void AddZone(WsfPM_ZoneDefinition& aZone);


   //! @brief Sets the data (position, name, height, etc.) of a circular zone
   //! @param aZonePtr is a pointer to a visual circular zone
   //! @param aZoneDef is the zone's proxy definition node
   void SetCircularZoneData(wizard::AttachmentZoneCircle* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef);

   //! @brief Sets the data (position, name, height, etc.) of an elliptical zone
   //! @param aZonePtr is a pointer to a visual elliptical zone
   //! @param aZoneDef is the zone's proxy definition node
   void SetEllipticalZoneData(wizard::AttachmentZoneEllipse* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef);

   //! @brief Sets the data (position, name, height, etc.) of a spherical zone
   //! @param aZonePtr is a pointer to a visual spherical zone
   //! @param aZoneDef is the zone's proxy definition node
   void SetSphericalZoneData(wizard::AttachmentZoneSphere* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef);

   //! @brief Sets the data (name, height, etc.) of a polygonal zone
   //! @param aZonePtr is a pointer to a visual polygonal zone
   //! @param aZoneDef is the zone's proxy definition node
   void SetPolygonalZoneData(wizard::AttachmentZonePolygon* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef);

   //! @brief Sets a the position and heading of the zone
   //! @param aZoneDefinition is a proxy Zone definition node
   void SetZonePositionAndHeading(const WsfPM_ZoneDefinition& aZoneDefinition);

   void SetZoneColors(const WsfPM_ZoneDefinition& aZoneDefinition);

   //! the name of the zone definition used to generate this zone
   std::string mZoneDefName;

   //! the name of the definition's parent platform
   std::string mZoneDefParentPlatName;

   //! @brief the zone set that owns this zone
   ZoneSet* mOwningZoneSetPtr;

   //! @brief the anchor entity for this zone
   vespa::VaEntity* mAnchorPtr = nullptr;

   //! @brief the name of the reference platform if one is defined
   std::string mReferencePlatformName = "";

   //! @brief the name of the zoneSet this zone belongs to
   int mZoneSetIndex = 1;

   //! @brief the global status of this zone
   bool mIsGlobal = false;

   //! @brief the proxy node for this zone
   WsfPM_ZoneDefinition mZoneProxyNode;

   //! @brief the visual zone attachment component of this zone
   wkf::AttachmentZone* mZonePtr = nullptr;

   //! @brief pointer to the update callback of this zone
   UtCallback* mUpdateCallbackPtr = nullptr;

   //! @brief the list of callbacks for this zone
   UtCallbackHolder mProxyCallbacks;
};
} // namespace wizard

#endif // ZONE_HPP
