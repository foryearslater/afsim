// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef ROUTETERRAINQUERY_HPP
#define ROUTETERRAINQUERY_HPP

#include <QString>

#include "UtCallbackHolder.hpp"
#include "UtMemory.hpp"
#include "WsfPM_Waypoint.hpp"

namespace wizard
{
class ProxyChange;
}

namespace MapRoute
{
class RouteTerrainQuery
{
public:
   //! Queries the wkf::ResourceManager for the elevation data at the given latitude and longitude
   //! @param aLat is the given latitude
   //! @param aLon is the given longitude
   //! @return the elevation at the given latitude and longitude
   double GetElevationData(double aLat, double aLon);

   //! Gets the waypoint's altitude
   //! @param aWaypoint is the proxy waypoint
   //! @pre aWaypoint's Altitude property is set
   //! @return the altitude of the waypoint
   //! @note AGL altitude = ground level + MSL altitude
   double GetWaypointAltitude(const WsfPM_Waypoint& aWaypoint);

   //! This will get called when the terrain is modified via the proxy
   //! @param aProxyChange a change recorded by the proxy
   void TerrainProxyModified(const wizard::ProxyChange& aProxyChange);

   //! Call this to set the name of the platform associated with this route
   //! @param aPlatformName name of the platform associated with this route
   void SetPlatformName(const QString& aPlatformName) { mPlatformName = aPlatformName; }

private:
   //! Handler for querying the wkf::ResourceManager at periodic intervals to see if the terrain data for a route is ready
   //! @param aDtedId is the terrain request ID of the local route's waypoint
   void DelayedRouteUpdate(size_t aId);

   //! name of the platform associated with this route.  If it's a global route, the name should have '_anchor' at the end.
   QString mPlatformName{""};

   //! Keep a cache of found elevations for faster retrieval on route rebuilds
   std::map<std::pair<double, double>, double> mWaypointCache;
   //! Keep a cache of requestIds to prevent multiple requests for the same lat/lon
   std::map<std::pair<double, double>, size_t> mWaypointRequestCache;

   UtCallbackHolder mCallbacks;
};
} // namespace MapRoute

#endif
