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

#ifndef PLATFORMMOVEMENT_FOO_HPP
#define PLATFORMMOVEMENT_FOO_HPP

#include <vector>

#include "WsfRoute.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace WkPlatformMovement
{
class RouteContainer : public QObject
{
   Q_OBJECT

public:
   using RoutePairList = std::vector<std::pair<WsfRoute, wkf::RouteBrowserInterface::RouteInfo>>;

   RouteContainer() = default;

   void CreateRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);
   void CreateRoute(const WsfRoute& aRoute, bool aUserEditable = true);

   void EditRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);
   void DeleteRoute(const QString& aRouteName);

   const RoutePairList& GetRoutes() const { return mRoutes; };

   void Clear();

signals:
   void RoutesInitializing();
   void RouteCreated(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);
   void RouteDeleted(const QString& aRouteName);
   void RouteEdited(const QString& aRouteName, int aWaypointCount);

private:
   bool RouteExists(const QString& aRouteName);

   RoutePairList mRoutes;
};
} // namespace WkPlatformMovement
#endif
