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

#include "RouteContainer.hpp"

#include "PlatformMovementUtils.hpp"

void WkPlatformMovement::RouteContainer::CreateRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   if (!RouteExists(aRouteInfo.mName))
   {
      WsfRoute route;
      for (auto& w : aRouteInfo.mWaypoints)
      {
         WsfWaypoint waypoint;
         UpdateWaypoint(waypoint, w, static_cast<WsfPath::EndPath>(aRouteInfo.mEndPathType));
         route.Append(waypoint);
      }
      route.SetName(aRouteInfo.mName.toStdString());
      mRoutes.emplace_back(route, aRouteInfo);
      emit RouteCreated(aRouteInfo);
   }
}

void WkPlatformMovement::RouteContainer::CreateRoute(const WsfRoute& aRoute, bool aUserDefined)
{
   wkf::RouteBrowserInterface::RouteInfo routeInfo = GetRouteInfo(&aRoute);
   routeInfo.mUserDefined                          = aUserDefined;
   CreateRoute(routeInfo);
}

void WkPlatformMovement::RouteContainer::EditRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   for (auto& r : mRoutes)
   {
      if (aRouteInfo.mName.toStdString() == r.first.GetName())
      {
         UpdateRoute(r.first, aRouteInfo);
         r.second = aRouteInfo;
         break;
      }
   }
   emit RouteEdited(aRouteInfo.mName, aRouteInfo.mWaypoints.size());
}

void WkPlatformMovement::RouteContainer::DeleteRoute(const QString& aRouteName)
{
   auto r = std::find_if(mRoutes.begin(),
                         mRoutes.end(),
                         [&](const std::pair<WsfRoute, wkf::RouteBrowserInterface::RouteInfo>& item)
                         { return item.second.mName == aRouteName; });
   if (r != mRoutes.end())
   {
      mRoutes.erase(r);
      emit RouteDeleted(aRouteName);
   }
}

void WkPlatformMovement::RouteContainer::Clear()
{
   mRoutes.clear();
}

bool WkPlatformMovement::RouteContainer::RouteExists(const QString& aRouteName)
{
   auto r = std::find_if(mRoutes.begin(),
                         mRoutes.end(),
                         [&](const std::pair<WsfRoute, wkf::RouteBrowserInterface::RouteInfo>& item)
                         { return item.second.mName == aRouteName; });

   return r != mRoutes.end();
}
