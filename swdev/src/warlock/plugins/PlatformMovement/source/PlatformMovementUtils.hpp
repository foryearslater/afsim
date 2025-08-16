// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WsfPlatform.hpp"
#include "WsfRoute.hpp"
#include "WsfRouteMover.hpp"
#include "WsfSimulation.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace WkPlatformMovement
{
wkf::RouteBrowserInterface::RouteInfo GetRouteInfo(const WsfRoute* aRoute);
const WsfRoute* FindLocalRoute(const WsfSimulation& aSimulation, const std::string& aPlatformName);
wkf::RouteBrowserInterface::RouteInfo FindLocalRouteInfo(const WsfSimulation& aSimulation,
                                                         const std::string&   aPlatformName);
void                                  UpdateWaypoint(WsfWaypoint&                                    aWaypoint,
                                                     const wkf::RouteBrowserInterface::WaypointInfo& aWaypointInfo,
                                                     WsfPath::EndPath                                aEndOfPathType);
void UpdateRoute(WsfRoute& aRoute, const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);
} // namespace WkPlatformMovement
