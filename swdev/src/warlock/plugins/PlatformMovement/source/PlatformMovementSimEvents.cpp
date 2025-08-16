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

#include "PlatformMovementSimEvents.hpp"

#include "PlatformMovementUtils.hpp"
#include "RouteBrowserInterface.hpp"
#include "RouteContainer.hpp"
#include "UtSphericalEarth.hpp"
#include "VaUtils.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserDockWidget.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

void WkPlatformMovement::RouteEvent::Process(RouteBrowserInterface* aRouteInterface)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   wkf::Viewer*   viewer   = vaEnv.GetStandardViewer();

   if (scenario != nullptr && viewer != nullptr)
   {
      int selectedWaypointIndex = aRouteInterface->GetCurrentWaypoint();

      wkf::Platform* entityPtr = scenario->FindPlatformByIndex(mPlatformIndex);
      if (entityPtr != nullptr)
      {
         bool visible  = false;
         bool selected = false;

         wkf::AttachmentRoute* prevRouteAttachment =
            dynamic_cast<wkf::AttachmentRoute*>(entityPtr->FindAttachment("route"));
         if (prevRouteAttachment != nullptr)
         {
            visible  = prevRouteAttachment->GetStateVisibility();
            selected = prevRouteAttachment->GetSelected(viewer);
            entityPtr->RemoveAttachment(prevRouteAttachment->GetUniqueId());
         }

         if (mRoutePtr != nullptr)
         {
            wkf::AttachmentRoute* routeAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*entityPtr, viewer);

            routeAttachmentPtr->ShowIndices(false);
            vespa::VaAttachment::LoadAttachment(*routeAttachmentPtr);

            vespa::VaPosition                      pos         = entityPtr->GetPosition();
            double                                 previousLat = pos.GetLat();
            double                                 previousLon = pos.GetLon();
            double                                 previousAlt = pos.GetAlt();
            double                                 heading     = entityPtr->GetHeading();
            double                                 lat, lon, alt;
            wkf::AttachmentRoute::WaypointDataList wpd;

            for (auto iter = mRoutePtr->Begin(); iter != mRoutePtr->End(); ++iter)
            {
               if (iter->GetPointType() & WsfWaypoint::cLATITUDE_AND_LONGITUDE ||
                   iter->GetPointType() & WsfWaypoint::cHEADING_MASK)
               {
                  heading = 0;
                  if (iter->GetHeading() != WsfPath::cDOUBLE_NOT_SET)
                  {
                     heading = iter->GetHeading();
                  }

                  iter->GetLocationLLA(lat, lon, alt);

                  if (alt == WsfPath::cUSE_PREVIOUS)
                  {
                     alt = previousAlt;
                  }
                  int goToIndex = -1;
                  if (!iter->GetGotoId().Empty())
                  {
                     auto wypt = std::find_if(mRoutePtr->Begin(),
                                              mRoutePtr->End(),
                                              [&](const WsfWaypoint& waypoint)
                                              { return iter->GetGotoId() == waypoint.GetLabelId(); });

                     if (wypt != mRoutePtr->End())
                     {
                        goToIndex = std::distance(mRoutePtr->Begin(), wypt);
                     }
                  }
                  wpd.emplace_back(lat, lon, alt, goToIndex, iter->GetLabelId());
                  previousAlt = alt;
               }
               else if (iter->GetPointType() & WsfWaypoint::cRELATIVE_OFFSET)
               {
                  double x          = iter->GetLat();
                  double y          = iter->GetLon();
                  double range      = sqrt(x * x + y * y);
                  double relBearing = atan2(y, x);
                  double headingDeg = (relBearing + heading) * UtMath::cDEG_PER_RAD;
                  UtSphericalEarth::ExtrapolateGreatCirclePosition(previousLat, previousLon, headingDeg, range, lat, lon);

                  int goToIndex = -1;
                  if (!iter->GetGotoId().Empty())
                  {
                     auto wypt = std::find_if(mRoutePtr->Begin(),
                                              mRoutePtr->End(),
                                              [&](const WsfWaypoint& waypoint)
                                              { return iter->GetGotoId() == waypoint.GetLabelId(); });

                     if (wypt != mRoutePtr->End())
                     {
                        goToIndex = std::distance(mRoutePtr->Begin(), wypt);
                     }
                  }

                  wpd.emplace_back(lat, lon, previousAlt, goToIndex, iter->GetLabelId());
                  previousLat = lat;
                  previousLon = lon;
               }
            }
            routeAttachmentPtr->AddWaypoints(wpd);

            routeAttachmentPtr->SetTeamColor(
               wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(entityPtr->GetSide()));
            // Do not set the route as visible unless the platform is visible
            routeAttachmentPtr->SetStateVisibility(visible && wkfEnv.IsPlatformVisible(entityPtr));

            // If the route had a selected waypoint before we redrew, select the waypoint.
            if (selected && selectedWaypointIndex >= 0 &&
                static_cast<size_t>(selectedWaypointIndex) < routeAttachmentPtr->GetWaypoints().size())
            {
               wkf::Waypoint* selectedWaypoint = routeAttachmentPtr->GetWaypoint(selectedWaypointIndex);
               routeAttachmentPtr->SetSelected(viewer, true, selectedWaypoint->GetId());
            }
         }
      }
   }

   wkf::RouteBrowserInterface::RouteInfo routeInfo = GetRouteInfo(mRoutePtr.get());
   routeInfo.mPlatformName                         = QString::fromStdString(mPlatformName);
   aRouteInterface->GetDockWidget()->EditRoute(routeInfo);
}

void WkPlatformMovement::RouteSelectEvent::Process(RouteBrowserInterface* aRouteInterface)
{
   // If this event contains info for the current Platform Of Interest.
   if (wkfEnv.GetPlatformOfInterest() &&
       wkfEnv.GetPlatformOfInterest()->GetName() == mRouteInfo.mPlatformName.toStdString())
   {
      aRouteInterface->PlatformOfInterestAddedHandler(mRouteInfo);
   }
}

void WkPlatformMovement::RouteDialogEvent::Process(RouteBrowserInterface* aRouteInterface)
{
   aRouteInterface->GetDockWidget()->GetEditDialog()->PopulateDialog(mRouteInfo);
   aRouteInterface->GetDockWidget()->GetEditDialog()->show();
}

void WkPlatformMovement::GlobalRouteListEvent::Process(RouteBrowserInterface* aRouteInterface)
{
   aRouteInterface->Clear();

   RouteContainer& rcAll  = aRouteInterface->GetGlobalRoutes();
   RouteContainer& rcUser = aRouteInterface->GetUserGlobalRoutes();

   rcAll.RoutesInitializing();

   for (auto& rc : rcUser.GetRoutes()) // new routes that were added by user
   {
      rcAll.CreateRoute(rc.second);
   }

   for (auto& route : mRoutes) // global routes already defined
   {
      rcAll.CreateRoute(route, false);
   }

   std::vector<RouteBrowserInterface::RouteInfo> routeInfoVector;
   for (auto& routePair : rcAll.GetRoutes())
   {
      routeInfoVector.push_back(routePair.second);
      aRouteInterface->HideGlobalRoute(routePair.second.mName);
   }
   aRouteInterface->GetDockWidget()->PopulateGlobal(routeInfoVector);
}
