// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMMOVEMENTSIMEVENTS_HPP
#define PLATFORMMOVEMENTSIMEVENTS_HPP

#include <memory>
#include <string>
#include <vector>

#include "WsfRoute.hpp"
namespace vespa
{
class VaViewer;
}

#include "WkSimInterface.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace WkPlatformMovement
{
class RouteBrowserInterface;

class PlatformMovementEvent : public warlock::SimEvent
{
public:
   virtual void Process(RouteBrowserInterface* aRouteInterface) = 0;
};

// A Route needs to be drawn (wkf::AttachmentRoute)
class RouteEvent : public PlatformMovementEvent
{
public:
   RouteEvent(size_t aPlatformIndex, const std::string& aPlatformName, const WsfRoute* aRoutePtr)
      : mPlatformIndex(aPlatformIndex)
      , mPlatformName(aPlatformName)
      , mRoutePtr(aRoutePtr != nullptr ? aRoutePtr->Clone() : nullptr)
   {
   }

   void Process(RouteBrowserInterface* aRouteInterface) override;

protected:
   size_t                    mPlatformIndex;
   std::string               mPlatformName;
   std::unique_ptr<WsfRoute> mRoutePtr;
};

// A Route needs to be selected in the Route Browser
class RouteSelectEvent : public PlatformMovementEvent
{
public:
   RouteSelectEvent(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
      : mRouteInfo(aRouteInfo)
   {
   }

   void Process(RouteBrowserInterface* aRouteInterface) override;

protected:
   wkf::RouteBrowserInterface::RouteInfo mRouteInfo;
};

// A Route dialog needs to be populated with the route's info
class RouteDialogEvent : public PlatformMovementEvent
{
public:
   RouteDialogEvent(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
      : mRouteInfo(aRouteInfo)
   {
   }

   void Process(RouteBrowserInterface* aRouteInterface) override;

protected:
   wkf::RouteBrowserInterface::RouteInfo mRouteInfo;
};

// A list of the global routes is needed
class GlobalRouteListEvent : public PlatformMovementEvent
{
public:
   GlobalRouteListEvent(const std::vector<WsfRoute*>& aRoutes)
   {
      for (auto& route : aRoutes)
      {
         mRoutes.emplace_back(WsfRoute(*route));
      }
   }

   void Process(RouteBrowserInterface* aRouteInterface) override;

protected:
   std::vector<WsfRoute> mRoutes;
};

} // namespace WkPlatformMovement
#endif
