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
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "RouteTerrainQuery.hpp"
#include "UtCallbackHolder.hpp"
#include "UtMemory.hpp"
#include "UtOrbitalElements.hpp"
#include "VaCallbackHolder.hpp"
#include "WsfPM_Mover.hpp"
#include "WsfPM_Waypoint.hpp"
#include "WsfPathList.hpp"
#include "route/WkfAttachmentRoute.hpp"

class WsfPathList;
struct WsfPathTarget;
struct WsfPathState;
class WsfPM_WaypointMover;

namespace vespa
{
class VaEntity;
}

namespace wkf
{
class AttachmentOrbit;
}

namespace wizard
{
class Platform;
class ProxyChange;
} // namespace wizard

namespace MapRoute
{
class Route
{
public:
   Route(wizard::Platform* aPlatformPtr);

   Route(const Route&) = delete;
   Route& operator=(const Route&) = delete;

   ~Route() = default;

   void Clear();
   //! Represents the route building state.
   //! @par Mixed Routes
   //!   Routes can have four types of waypoints:  position, MGRS, offset, and turn.
   //!   Currently, WSF does not allow offset waypoints after the route defines a second position waypoint.
   //!   Wizard will abort the creation of a route if this occurs.
   enum class BuildStatus
   {
      //! BuildRoute has not processed any waypoint yet.
      NoPosition,
      //! BuildRoute has processed the initial position waypoint.
      InitialPosition,
      //! BuildRoute has processed a second position waypoint.
      //! @note When in this state, succeeding waypoints MUST be position waypoints.
      SecondPosition,
      //! BuildRoute has processed an offset waypoint succeeding a second position waypoint.
      //! @warning BuildRoute will abort if it reaches this state.
      MixedWaypointError
   };
   //! Builds a wkf::AttachmentRoute from the proxy mover's route
   //! @param aMover is the proxy mover containing the proxy route
   //! @param aVisible is whether to show the route
   //! @retval BuildStatus::NoPosition if the route-building with a offset-only route
   //! @retval BuildStatus::InitialPosition if the route-building with a offset-only route containing an initial
   //! position waypoint
   //! @retval BuildStatus::SecondPosition if the route-building succeeded with a position-only route
   //! @retval BuildStatus::MixedWaypointError if the route-building failed
   BuildStatus BuildRoute(WsfPM_Mover aMover, bool aVisible);
   void        SetPositionByTime(double aTime);
   void        SetDirty() { mDirty = true; }

   void ResetColor();
   void UpdateEpoch(const UtCalendar& aEpoch);

   void SetOrbitLineWidth(int aWidth);
   void SetOrbitPeriods(unsigned int aPeriods);

private:
   void AttachmentDeletedCB(vespa::VaAttachment* aAttachmentPtr, const vespa::VaAttachmentSubId& aSubpartId);
   void PlatformDeletedCB(vespa::VaEntity* aEntityPtr);

   void SetPositionByDefaultState();
   void OnProxyModified(const wizard::ProxyChange& aProxyChange);

   //! Clears out the callback list and re-adds the terrain modified callback.
   void ResetCallbacks();

   //! Build a wkf::AttachmentRoute from a type of WSF_ROUTE_MOVER
   //! @param aMover is the proxy mover object whose type is of WSF_ROUTE_MOVER
   //! @param aHeading is the initial heading of the platform
   //! @retval BuildStatus::NoPosition if the route-building with a offset-only route
   //! @retval BuildStatus::InitialPosition if the route-building with a offset-only route containing an initial
   //! position waypoint
   //! @retval BuildStatus::SecondPosition if the route-building succeeded with a position-only route
   //! @retval BuildStatus::MixedWaypointError if the route-building failed
   BuildStatus BuildRoute(WsfPM_Mover aMover, double aHeading);

   //! Sets the altitude (ground level + altitude above ground level) of the target waypoint (the next waypoint to build)
   //! @param aTarget is the WsfPathTarget containing the position and altitude information for the target waypoint
   //! @param aProxyRoute is the proxy route from which to extract the proxy waypoint's position and altitude information
   //! @param aIndex is the index of the proxy waypoint
   void SetTargetAltitude(WsfPathTarget& aTarget, const WsfPM_Route& aProxyRoute, size_t aIndex) noexcept;

   //! Sets the altitude of the path state
   //! @param aState is the path state whose altitude is to be set
   //! @param aWaypoint is the proxy waypoint containing the altitude information
   void SetStateAltitude(WsfPathState& aState, WsfPM_Waypoint aWaypoint) noexcept;

   wkf::AttachmentOrbit* BuildOrbit(const UtColor& aOrbitColor);
   void                  BuildOrbit(bool aVisible, const UtColor& aOrbitColor);
   bool                  SetOrbitSize(WsfPM_Mover aMover);
   UtColor               GetOrbitColor(const WsfPM_Mover& aMover);

   void ClearPathLists();

   //! Finds the index indecated of a waypoint indicated by the goto value in the given waypoint
   //! @param aRoute the route to search in
   //! @param aWaypoint the waypoint from which a goto command is to be followed
   //! @retval returns the index of the goto waypoint, -1 if not found
   int FindGotoIndex(const WsfPM_Route& aRoute, const WsfPM_Waypoint& aWaypoint) const;

   //! Finds a waypoint on the given route with the given label
   //! @param aRoute is the route to search in
   //! @param aLabel is the label to search for
   //! @param aWp contains the requested waypoint, if found
   //! @param aIndex contains the index of the requested waypoint, if found
   //! @retval returns true if a waypoint could be found, false otherwise
   bool FindWaypoint(const WsfPM_Route& aRoute, const std::string& aLabel, WsfPM_Waypoint& aWp, size_t& aIndex) const;

   void AddWaypoint(const WsfPM_Waypoint&                   aWP,
                    double                                  aLat,
                    double                                  aLon,
                    double                                  aAlt,
                    wkf::AttachmentRoute::WaypointDataList& aWaypionts,
                    int                                     aGoToIndex = -1) const;

   wizard::Platform*     mPlatformPtr;
   wkf::AttachmentRoute* mAttachmentPtr; // We do not own this
   WsfPM_Mover           mProxyMover;

   UtCallbackHolder                          mCallbacks;
   vespa::VaCallbackHolder                   mV_Callbacks;
   std::vector<std::unique_ptr<WsfPathList>> mPathLists;
   WsfPM_Waypoint::EndOfPathOption           mEndOfPath;
   double                                    mSimTime;
   bool                                      mDirty;
   UtOrbitalElements                         mOrbit;
   UtCalendar                                mEpoch;
   RouteTerrainQuery                         mTerrainQuery;

   //! Used to keep track of the last waypoint that defined the "altitude" attribute
   //! @see SetTargetAltitudeP for usage
   size_t mLastWaypointWithDefinedAltitude{0};
};
} // namespace MapRoute

#endif
