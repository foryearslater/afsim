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

#ifndef MAP_UTILS_GHOSTING_INTERFACE_HPP
#define MAP_UTILS_GHOSTING_INTERFACE_HPP

#include <string>
#include <vector>

#include <QColor>

#include "PluginUtil.hpp"
#include "UtColor.hpp"
#include "VaEntity.hpp"
#include "VaPosition.hpp"
#include "WkfAttachmentModel.hpp"
#include "WkfVtkEnvironment.hpp"
#include "route/WkfAttachmentRoute.hpp"

//! @brief namespace for the VESPA Toolkit
namespace vespa
{
class VaAttachmentModel;
using VaEntityPtr = std::unique_ptr<VaEntity>;
} // namespace vespa

//! @brief namespace for the Warlock Framework
namespace wkf
{
class AttachmentRangeRing;
class AttachmentRoute;
class Bullseye;
using BullseyePtr = std::unique_ptr<Bullseye>;
class Entity;
using EntityPtr = std::unique_ptr<Entity>;
class Platform;
using PlatformPtr = std::unique_ptr<Platform>;
class PointOfInterest;
using PointOfInterestPtr = std::unique_ptr<PointOfInterest>;
class Viewer;
class AttachmentZone;
class AttachmentZoneCircle;
class AttachmentZoneEllipse;
class AttachmentZonePolygon;
class AttachmentZoneSphere;
} // namespace wkf

//! @brief convenient preprocessor macro for the ghost::Manager instance
#define ghostMan MapUtils::ghost::Manager::Instance()

//! @brief namespace for things related to the Wizard application
namespace wizard
{
class Platform;
using PlatformPtr = std::unique_ptr<Platform>;
} // namespace wizard

namespace MapUtils
{
//! @brief namespace of common/helper objects, functions, and variables for the ghosting interface
//! @par details
//!    The ghosting interface is used by the MapUtils::ScenarioTransformation dialogs to create a
//!    temporary scenario or selection of entities to show the state of the scenario or selection
//!    of entities after the transformation.  This is safer than modifying the actual scenario or
//!    selection of entities directly because it prevents any unexpected change to the scenario
//!    or selection of entities if a problem within the dialog would occur.
namespace ghost
{

//! @name color constants
//@{

//! @brief denotes the color of a ghost entity
const UtColor cGHOST_ENTITY_COLOR(1, 1, 1, 0.5f);

//! @brief denotes the color of a ghost zone's lines
const UtColor cGHOST_ZONE_LINE_COLOR(1, 1, 1);

//! @brief denotes the color of a ghost zone's fill
const UtColor cGHOST_ZONE_FILL_COLOR(0.75, 0.75, 0.75, 0.5f);

//! @brief denotes the color of a ghost range ring's lines
const QColor cGHOST_RANGE_RING_LINE_COLOR(127, 127, 0);

//! @brief denotes the color of a ghost range ring's radial lines
const QColor cGHOST_RANGE_RING_RADIAL_COLOR(255, 255, 0);

//@}

//! @brief super light-weight templated subclass for ghost attachments
template<typename ATTACHMENT>
class AttachmentT final : public ATTACHMENT
{
public:
   AttachmentT(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr)
      : ATTACHMENT(aParent, aViewerPtr)
   {
   }
   ~AttachmentT() override = default;

   unsigned int GetPickMode() const override { return vespa::VaAttachment::cPICK_NOTHING; }
};

//! @name some handy aliases for specific types of ghost attachments
//@{

using AttachmentModel = AttachmentT<wkf::AttachmentModel>;
using AttachmentRoute = AttachmentT<wkf::AttachmentRoute>;

//@}

//! @brief singleton that manages the "temporary" (ghost) scenario/selection of entities
class Manager final
{
public:
   using EntityList     = std::vector<std::string>;
   using RangeRingList  = std::vector<wkf::AttachmentRangeRing*>;
   using VaZoneList     = std::vector<wkf::AttachmentZone*>;
   using VaPositionList = std::vector<vespa::VaPosition>;

   //! @brief accesses the ghost::Manager instance
   //! @pre the ghost::Manager DOES exist
   //! @return a pointer to the ghost::Manager instance
   static Manager* Instance();

   //! @brief creates the ghost::Manager instance
   //! @pre the ghost::Manager DOES NOT exist
   //! @post the ghost::Manager exists and there is one and only one
   static void Create();

   Manager(const Manager& /* aSrc */) = delete;
   Manager(Manager&& /* aSrc */)      = delete;
   ~Manager()                         = default;

   Manager& operator=(const Manager& /* aSrc */) = delete;
   Manager& operator=(Manager&& /* aSrc */) = delete;

   //! @name methods to manage a single ghost
   //@{

   //! @brief construct a ghost entity and attachments for a given real entity and attachments
   void ConstructGhostEntityAndAttachments(const vespa::VaEntity* aRealEntityPtr);

   //! @brief reset a ghost entity and attachments given the real entity and attachments
   //! @param aRealEntity is the name of the real entity with which to reset the ghost entity
   //! @param aGhostEntity is the name of the ghost entity to reset
   void ResetGhostEntityAndAttachments(const std::string& aRealEntity, const std::string& aGhostEntity);

   //! @brief destruct a ghost entity and attachments
   //! @param aGhostEntity is the name of the ghost entity to destruct
   //! @return an iterator to the next element
   Manager::EntityList::iterator DestructGhostEntityAndAttachments(const std::string aGhostEntity);

   //@}

   //! @name methods to manage multiple ghosts
   //@{

   //! @brief constructs the ghost entities and attachments
   //! @param aEntitiesToGhost is the set of entities to ghost
   //! @note these are used to preview the scenario transformation
   //! @see PluginUtil::WhatEntities for more information
   void ConstructGhostEntitiesAndAttachments(const PluginUtil::WhatEntities aEntitiesToGhost);

   //! @brief resets the positions of the ghost entities and attachments
   void ResetGhostEntitiesAndAttachments();

   //! @brief destructs the ghost entities and attachments
   void DestructGhostEntitiesAndAttachments();

   //@}

   //! @name ghost accessors
   //@{

   //! @brief computes the centroid from a list of entities
   //! @param aEntityList is a list containing the names of the ghost entities
   //! @return the centroid of the list of entities
   vespa::VaPosition GetGhostCentroid(const EntityList& aEntityList);

   //! @brief computes the centroid from a list of entities
   //! @return the centroid of the list of entities
   //! @note this uses the list of names inside the manager
   vespa::VaPosition GetGhostCentroid();

   //! @brief gets a list of ghost entities
   //! @param aEntityList is a list containing the names of the ghost entities
   //! @return a list containing pointers to the ghost entities
   vespa::VaEntityList GetGhostEntities(const EntityList& aEntityList);

   //! @brief gets a list of ghost entities
   //! @return a list containing pointers to the ghost entities
   //! @note this uses the list of names inside the manager
   vespa::VaEntityList GetGhostEntities();

   //@}

   //! @brief gets the ghost entity name from the real entity name
   //! @param aRealEntity is the name of the real entity
   //! @return the name of the ghost entity
   static std::string GhostEntityName(const std::string& aRealEntity);

   //! @brief gets the real entity name from the ghost entity name
   //! @param aGhostEntity is the name of the ghost entity
   //! @return the name of the real entity
   static std::string RealEntityName(const std::string& aGhostEntity);

private:
   Manager() = default;

   //! @name ghost accessors
   //@{

   //! @brief gets the color of the ghost attachment model
   //! @param aEntityPtr is the entity from which to extract the color
   //! @return the ghost model color
   UtColor GetGhostModelColor(const vespa::VaEntity* aEntityPtr);

   //! @brief gets the color of the ghost attachment route
   //! @param aEntityPtr is the entity from which to extract the color
   //! @return the ghost route color
   UtColor GetGhostRouteColor(const vespa::VaEntity* aEntityPtr);

   //@}

   //! @brief adds the ghost entity to the scenario
   //! @param aGhostEntityPtr is the ghost entity to add
   //! @return whether or not it successfully added the ghost entity to the scenario
   template<typename VA_ENTITY>
   bool AddGhostToScenario(const std::unique_ptr<VA_ENTITY>& aGhostEntityPtr) const
   {
      static_assert(std::is_base_of<vespa::VaEntity, VA_ENTITY>::value, "VA_ENTITY must derive from vespa::VaEntity");
      return AddGhostToScenario(aGhostEntityPtr.get());
   }

   //! @brief adds the ghost entity to the scenario
   //! @param aGhostEntityPtr is the ghost entity to add
   //! @return whether or not it successfully added the ghost entity to the scenario
   //! This is necessary before the entity gets its attachments to avoid deletion of the attachments.
   bool AddGhostToScenario(vespa::VaEntity* aGhostEntityPtr) const;

   //! @brief safely releases ownership of the ghost entity to the scenario
   //! @param aGhostEntityPtr is the ghost entity to transfer ownership to the scenario
   //! @return whether or not it successfully released the ghost to the scenario
   bool TransferGhostToScenario(vespa::VaEntityPtr aGhostEntityPtr) const;

   //! @name ghost entity construction functions
   //@{

   //! @brief constructs a ghost vespa::VaEntity
   //! @param aViewerPtr is the current viewer
   //! @param aRealEntityPtr is the real entity to ghost
   //! @param aRealZoneList is the list of real zones to ghost
   //! @param aRealRangeRingList is the list of real range rings to ghost
   //! @param aGhostEntityName is the name of the ghost entity
   //! @return a pointer to the newly created ghost vespa::VaEntity
   vespa::VaEntityPtr ConstructGhostVaEntity(wkf::Viewer*           aViewerPtr,
                                             const vespa::VaEntity* aRealEntityPtr,
                                             const VaZoneList&      aRealZoneList,
                                             const RangeRingList&   aRealRangeRingList,
                                             const std::string&     aGhostEntityName);

   //! @brief constructs a ghost wkf::Entity
   //! @param aViewerPtr is the current viewer
   //! @param aRealEntityPtr is the real entity to ghost
   //! @param aRealZoneList is the list of real zones to ghost
   //! @param aRealRangeRingList is the list of real range rings to ghost
   //! @param aGhostEntityName is the name of the ghost entity
   //! @return a pointer to the newly created ghost wkf::Entity
   wkf::EntityPtr ConstructGhostWkfEntity(wkf::Viewer*         aViewerPtr,
                                          const wkf::Entity*   aRealEntityPtr,
                                          const VaZoneList&    aRealZoneList,
                                          const RangeRingList& aRealRangeRingList,
                                          const std::string&   aGhostEntityName);

   //! @brief constructs a ghost wkf::PointOfInterest with attachments
   //! @param aViewerPtr is the current viewer
   //! @param aRealEntityPtr is the real entity to ghost
   //! @param aRealZoneList is the list of real zones to ghost
   //! @param aRealRangeRingList is the list of real range rings to ghost
   //! @param aGhostEntityName is the name of the ghost entity
   //! @return a pointer to the newly created ghost wkf::PointOfInterest
   wkf::PointOfInterestPtr ConstructGhostWkfPointOfInterest(wkf::Viewer*                aViewerPtr,
                                                            const wkf::PointOfInterest* aRealEntityPtr,
                                                            const VaZoneList&           aRealZoneList,
                                                            const RangeRingList&        aRealRangeRingList,
                                                            const std::string&          aGhostEntityName);

   //! @brief constructs a ghost wkf::Bullseye
   //! @param aViewerPtr is the current viewer
   //! @param aRealEntityPtr is the real entity to ghost
   //! @param aRealZoneList is the list of real zones to ghost
   //! @param aRealRangeRingList is the list of real range rings to ghost
   //! @param aGhostEntityName is the name of the ghost entity
   //! @return a pointer to the newly created ghost wkf::Bullseye
   wkf::BullseyePtr ConstructGhostWkfBullseye(wkf::Viewer*         aViewerPtr,
                                              const wkf::Bullseye* aRealEntityPtr,
                                              const VaZoneList&    aRealZoneList,
                                              const RangeRingList& aRealRangeRingList,
                                              const std::string&   aGhostEntityName);

   //! @brief constructs a ghost wkf::Platform
   //! @param aViewerPtr is the current viewer
   //! @param aRealEntityPtr is the real entity to ghost
   //! @param aRealZoneList is the list of real zones to ghost
   //! @param aRealRangeRingList is the list of real range rings to ghost
   //! @param aGhostEntityName is the name of the ghost entity
   //! @return a pointer to the newly created ghost wkf::Platform
   //! @note used also when ghosting a wizard::Platform because the WsfPM_Platform information does not need to be there
   wkf::PlatformPtr ConstructGhostWkfPlatform(wkf::Viewer*         aViewerPtr,
                                              const wkf::Platform* aRealEntityPtr,
                                              const VaZoneList&    aRealZoneList,
                                              const RangeRingList& aRealRangeRingList,
                                              const std::string&   aGhostEntityName);

   //@}

   //! @brief constructs the ghost entity's model attachment
   //! @param aViewerPtr is the viewer for the scenario/selection
   //! @param aRealModelPtr is the model attachment from the real entity
   //! @param aGhostEntityPtr is the ghost entity
   void ConstructGhostAttachmentModel(wkf::Viewer*                    aViewerPtr,
                                      const vespa::VaAttachmentModel* aRealModelPtr,
                                      vespa::VaEntity*                aGhostEntityPtr);

   //! @name ghost attachment model construction functions
   //@{

   //! @brief constructs a ghost vespa::VaAttachmentModel for a ghost wkf::PointOfInterest
   //! @param aViewerPtr is the current viewer
   //! @param aRealModelPtr is the real model to ghost
   //! @param aModelColor is the color to set the ghost model
   //! @param aGhostPointOfInterestPtr is the ghost wkf::PointOfInterest on which to attach the model
   void ConstructGhostAttachmentModelWkfPointOfInterest(wkf::Viewer*                    aViewerPtr,
                                                        const vespa::VaAttachmentModel* aRealModelPtr,
                                                        const UtColor&                  aModelColor,
                                                        wkf::PointOfInterest*           aGhostPointOfInterestPtr);

   //! @brief constructs a ghost vespa::VaAttachmentModel for a ghost wkf::Bullseye
   //! @param aViewerPtr is the current viewer
   //! @param aRealModelPtr is the real model to ghost
   //! @param aModelColor is the color to set the ghost model
   //! @param aGhostBullseyePtr is the ghost wkf::Bullseye on which to attach the model
   void ConstructGhostAttachmentModelWkfBullseye(wkf::Viewer*                    aViewerPtr,
                                                 const vespa::VaAttachmentModel* aRealModelPtr,
                                                 const UtColor&                  aModelColor,
                                                 wkf::Bullseye*                  aGhostBullseyePtr);

   //! @brief constructs a ghost vespa::VaAttachmentModel for a ghost wkf::Platform
   //! @param aViewerPtr is the current viewer
   //! @param aRealModelPtr is the real model to ghost
   //! @param aModelColor is the color to set the ghost model
   //! @param aGhostPlatformPtr is the ghost wkf::Platform on which to attach the model
   //! @note used also when cloning a wizard::Platform because the WsfPM_Platform information does not need to be there
   void ConstructGhostAttachmentModelWkfPlatform(wkf::Viewer*                    aViewerPtr,
                                                 const vespa::VaAttachmentModel* aRealModelPtr,
                                                 const UtColor&                  aModelColor,
                                                 wkf::Platform*                  aGhostPlatformPtr);

   //@}

   //! @brief constructs the ghost entity's route attachment
   //! @param aViewerPtr is the viewer for the scenario/selection
   //! @param aRealRoutePtr is the route attachment from the real entity
   //! @param aGhostEntityPtr is the ghost entity
   void ConstructGhostAttachmentRoute(wkf::Viewer*                aViewerPtr,
                                      const wkf::AttachmentRoute* aRealRoutePtr,
                                      vespa::VaEntity*            aGhostEntityPtr);

   //! @name ghost attachment route construction functions
   //@{

   //! @brief constructs a ghost wkf::AttachmentRoute for a ghost vespa::VaEntity
   //! @param aViewerPtr is the current viewer
   //! @param aRealRoutePtr is the real route to ghost
   //! @param aRouteColor is the color to set the ghost route
   //! @param aGhostEntityPtr is the ghost vespa::VaEntity on which to attach the route
   void ConstructGhostAttachmentRouteVaEntity(wkf::Viewer*                aViewerPtr,
                                              const wkf::AttachmentRoute* aRealRoutePtr,
                                              const UtColor&              aRouteColor,
                                              vespa::VaEntity*            aGhostEntityPtr);

   //! @brief constructs a ghost wkf::AttachmentRoute for a ghost wkf::Platform
   //! @param aViewerPtr is the current viewer
   //! @param aRealRoutePtr is the real route to ghost
   //! @param aRouteColor is the color to set the ghost route
   //! @param aGhostPlatformPtr is the ghost wkf::Platform on which to attach the route
   void ConstructGhostAttachmentRouteWkfPlatform(wkf::Viewer*                aViewerPtr,
                                                 const wkf::AttachmentRoute* aRealRoutePtr,
                                                 const UtColor&              aRouteColor,
                                                 wkf::Platform*              aGhostPlatformPtr);

   //@}

   //! @brief constructs the ghost entity's zone attachments
   //! @param aViewerPtr is the viewer for the scenario/selection
   //! @param aRealZonePtr is the zone attachment from the real entity
   //! @param aGhostEntityPtr is the ghost entity
   void ConstructGhostAttachmentZone(wkf::Viewer*               aViewerPtr,
                                     const wkf::AttachmentZone* aRealZonePtr,
                                     vespa::VaEntity*           aGhostEntityPtr);

   //! @name ghost attachment zone construction functions
   //@{

   //! @brief constructs a ghost wkf::AttachmentZoneCircle for the ghost entity
   //! @param aViewerPtr is the current viewer
   //! @param aRealZoneCirclePtr is the real circular zone to ghost
   //! @param aGhostEntityPtr is the ghost entity on which to attach the circular zone
   void ConstructGhostAttachmentZoneCircle(wkf::Viewer*                     aViewerPtr,
                                           const wkf::AttachmentZoneCircle* aRealZoneCirclePtr,
                                           vespa::VaEntity*                 aGhostEntityPtr);

   //! @brief constructs a ghost wkf::AttachmentZoneEllipse for the ghost entity
   //! @param aViewerPtr is the current viewer
   //! @param aRealZoneEllipsePtr is the real elliptical zone to ghost
   //! @param aGhostEntityPtr is the ghost entity on which to attach the elliptical zone
   void ConstructGhostAttachmentZoneEllipse(wkf::Viewer*                      aViewerPtr,
                                            const wkf::AttachmentZoneEllipse* aRealZoneEllipsePtr,
                                            vespa::VaEntity*                  aGhostEntityPtr);

   //! @brief constructs a ghost wkf::AttachmentZonePolygon for the ghost entity
   //! @param aViewerPtr is the current viewer
   //! @param aRealZonePolygonPtr is the real polygonal zone to ghost
   //! @param aGhostEntityPtr is the ghost entity on which to attach the polygonal zone
   void ConstructGhostAttachmentZonePolygon(wkf::Viewer*                      aViewerPtr,
                                            const wkf::AttachmentZonePolygon* aRealZonePolygonPtr,
                                            vespa::VaEntity*                  aGhostEntityPtr);

   //! @brief constructs a ghost wkf::AttachmentZoneSphere for the ghost entity
   //! @param aViewerPtr is the current viewer
   //! @param aRealZoneSpherePtr is the real spherical zone to ghost
   //! @param aGhostEntityPtr is the ghost entity on which to attach the spherical zone
   void ConstructGhostAttachmentZoneSphere(wkf::Viewer*                     aViewerPtr,
                                           const wkf::AttachmentZoneSphere* aRealZoneSpherePtr,
                                           vespa::VaEntity*                 aGhostEntityPtr);

   //@}

   //! @brief constructs the ghost entity's range ring attachments
   //! @param aViewerPtr is the viewer for the scenario/selection
   //! @param aRealRangeRingPtr is the range ring attachment from the real entity
   //! @param aGhostEntityPtr is the ghost entity
   void ConstructGhostAttachmentRangeRing(wkf::Viewer*                    aViewerPtr,
                                          const wkf::AttachmentRangeRing* aRealRangeRingPtr,
                                          vespa::VaEntity*                aGhostEntityPtr);

   void ShowGhostEntityAndAttachments(vespa::VaEntity* aGhostEntityPtr) const;

   //@}

   //! @name real attachment accessors
   //@{

   //! @brief get the model associated with a certain real entity
   //! @param aRealEntityPtr is the real entity from which to get the model
   //! @return the model associated with the specified real entity
   const vespa::VaAttachmentModel* GetRealModel(const vespa::VaEntity* aRealEntityPtr);

   //! @brief get the route associated with a certain real entity
   //! @param aRealEntityPtr is the real entity from which to get the route
   //! @return the route associated with the specified real entity
   const wkf::AttachmentRoute* GetRealRoute(const vespa::VaEntity* aRealEntityPtr);

   //! @brief get the zones associated with a certain real entity
   //! @param aRealEntityPtr is the real entity from which to get the zones
   //! @return a list of zones associated with the specified real entity
   VaZoneList GetRealZones(const vespa::VaEntity* aRealEntityPtr);

   //! @brief gets the range rings associated with a certain real entity
   //! @param aRealEntityPtr is the real entity from which to get the range rings
   //! @return a list of range rings associated with the specified real entity
   RangeRingList GetRealRangeRings(const vespa::VaEntity* aRealEntityPtr);

   //@}

   //! @name functions to get entities
   //@{

   //! @brief finds the matching ghost entity from the real entity
   //! @param aRealEntity is the name of the real entity
   //! @return the ghost entity corresponding to the real entity
   vespa::VaEntity* FindGhostEntity(const std::string& aRealEntity);

   //! @brief finds the matching real entity from the ghost entity
   //! @param aGhostEntity is the name of the ghost entity
   //! @return the real entity corresponding to the ghost entity
   //! @note by default, the ghost entity name is in the format <real name>_ghost
   vespa::VaEntity* FindRealEntity(const std::string& aGhostEntity);

   //@}

   //! @brief denotes the instance of the singleton ghost::Manager
   static std::unique_ptr<Manager> mInstancePtr;

   //! @brief denotes the names of the ghosted entities
   EntityList mGhostEntityList;
};

} // namespace ghost
} // namespace MapUtils

#endif // MAP_UTILS_GHOSTING_INTERFACE_HPP
