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

#include "MapRoutePlugin.hpp"

#include <QCheckBox>
#include <QMenu>
#include <QTextCursor>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "Platform.hpp"
#include "Project.hpp"
#include "ProxyUtil.hpp"
#include "ProxyWatcher.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtQtMemory.hpp"
#include "UtSphericalEarth.hpp"
#include "VaAttachment.hpp"
#include "VaCamera.hpp"
#include "VaUtils.hpp"
#include "VaViewer.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "orbit/WkfAttachmentOrbit.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(MapRoute::Plugin, "Map Route", "Displays Platform Routes", "wizard")

MapRoute::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mOrbitPrefWidgetPtr{ut::qt::make_qt_ptr<wkf::OrbitPrefWidget>(false)}
{
   // right now these are created in wizard::VtkInterface, we'll just do option management here
   mOptionType = RegisterOption(nullptr, "Routes", "", false, false, true)->type();

   mVaCallbacks.Add(wkf::Observer::MapProjectionChanged.Connect(&Plugin::MapProjectionChangedCB, this));
   mVaCallbacks.Add(wkf::Observer::AttachmentSelected.Connect(&Plugin::AttachmentSelectionChangedCB, this));
   connect(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>(),
           &wkf::TeamVisibilityPrefObject::TeamColorChanged,
           this,
           &Plugin::TeamColorsChanged);
   connect(wizSignals, &wizard::Signals::EpochUpdated, this, &Plugin::UpdateEpoch);
   mDtedRegistrationId = wkfEnv.GetResourceManager().Register();
   connect(mOrbitPrefWidgetPtr->GetPreferenceObject(), &wkf::OrbitPrefObject::LineWidthChanged, this, &Plugin::LineWidthChanged);
   connect(mOrbitPrefWidgetPtr->GetPreferenceObject(), &wkf::OrbitPrefObject::PeriodsChanged, this, &Plugin::PeriodsChanged);

   mProxyAvailableCallback += wizard::ProxyWatcher::ProxyAvailable.Connect(&Plugin::OnProxyAvailable, this);
}

QList<wkf::PrefWidget*> MapRoute::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> retval{};
   retval.push_back(mOrbitPrefWidgetPtr);
   return retval;
}

void MapRoute::Plugin::ResetOptionStates()
{
   wkf::Scenario* scenPtr = vaEnv.GetStandardScenario();
   if (scenPtr)
   {
      const std::map<unsigned int, wkf::Platform*>& platMap = scenPtr->GetIndexPlatformMap();
      for (auto& it : platMap)
      {
         mState[it.second->GetUniqueId()] = false;
         std::set<int> idList;

         for (auto& jt : it.second->GetAttachments())
         {
            if (jt->IsA_TypeOf<wkf::AttachmentRoute>())
            {
               idList.insert(jt->GetUniqueId());
            }
         }
         for (auto& jt : idList)
         {
            it.second->RemoveAttachment(jt);
         }
      }
   }
}

void MapRoute::Plugin::SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      mState[aPlatformPtr->GetUniqueId()] = aState;
      aPlatformPtr->GetAttachments();
      for (auto&& it : aPlatformPtr->GetAttachments())
      {
         auto* rr = dynamic_cast<wkf::AttachmentRoute*>(it.get());
         if (rr)
         {
            rr->SetStateVisibility(aState);
            rr->SetSelectionEnabled(aState);
         }
         auto* orb = dynamic_cast<wkf::AttachmentOrbit*>(it.get());
         if (orb)
         {
            orb->SetStateVisibility(aState);
         }
      }
   }
}

Qt::CheckState MapRoute::Plugin::GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const
{
   if (aPlatformPtr)
   {
      std::map<unsigned int, bool>::const_iterator it = mState.find(aPlatformPtr->GetUniqueId());
      if (it != mState.end())
      {
         return it->second ? Qt::Checked : Qt::Unchecked;
      }
   }
   return Qt::Unchecked;
}

bool MapRoute::Plugin::GetVisibility(const wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      const auto& attachmentList = aPlatformPtr->GetAttachments();
      for (const auto& attachment : attachmentList)
      {
         auto* routeAttachmentPtr = dynamic_cast<wkf::AttachmentRoute*>(attachment.get());
         if (routeAttachmentPtr)
         {
            return routeAttachmentPtr->GetStateVisibility();
         }
      }
   }
   return true;
}

void MapRoute::Plugin::PlatformUpdated(const QString& aPlatformName)
{
   if (vaEnv.GetStandardScenario())
   {
      wizard::Platform* platform =
         dynamic_cast<wizard::Platform*>(vaEnv.GetStandardScenario()->FindPlatformByName(aPlatformName));
      if (platform)
      {
         WsfPM_Mover mover              = platform->GetPlatform().Mover();
         bool        previousVisibility = GetVisibility(platform);
         if (mRouteMap.find(aPlatformName) != mRouteMap.end())
         {
            mRouteMap[aPlatformName]->Clear();
            mRouteMap[aPlatformName]->SetDirty();
         }
         else
         {
            mRouteMap[aPlatformName] = new Route(platform);
         }

         bool visible = (Qt::Checked == GetPlatformOptionState(mOptionType, platform)) && previousVisibility;

         if (mRouteMap[aPlatformName]->BuildRoute(mover, visible) == Route::BuildStatus::MixedWaypointError)
         {
            if (!mover.UsingRoute().empty())
            {
               wizard::ProxyUtil::InvalidateInput(mover.InitialRoute());
            }
            else
            {
               // Need to access the WaypointList because the WsfPProxyIndexEntry is not on the InitialRoute...
               wizard::ProxyUtil::InvalidateInput(mover.InitialRoute().WaypointList());
            }
         }
         mRouteMap[aPlatformName]->SetPositionByTime(0.0);

         // reselect previously selected waypoints
         vespa::VaViewer* viewerPtr = vaEnv.Instance().GetStandardViewer();
         if (viewerPtr != nullptr)
         {
            vespa::VaEntity* entity         = vaEnv.FindEntity(aPlatformName.toStdString());
            wkf::Platform*   viewerPlatform = dynamic_cast<wkf::Platform*>(entity);
            // This entity is a wizard::Platform, which could contain a local route
            if (viewerPlatform)
            {
               const auto& attachmentList = viewerPlatform->GetAttachments();
               for (const auto& attachment : attachmentList)
               {
                  auto routeAttachmentPtr = dynamic_cast<wkf::AttachmentRoute*>(attachment.get());
                  if (routeAttachmentPtr)
                  {
                     routeAttachmentPtr->ClearSelection(vaEnv.Instance().GetStandardViewer());
                     if ((mWaypointSelectionRequest.has_value()) &&
                         (mWaypointSelectionRequest->first == aPlatformName.toStdString()))
                     {
                        platform->ClearSelectedWaypoints();
                        auto waypointPtr = routeAttachmentPtr->GetWaypoint(mWaypointSelectionRequest->second);
                        if (waypointPtr)
                        {
                           routeAttachmentPtr->SetSelected(viewerPtr, true, waypointPtr->GetId());
                        }
                        platform->UpdateSelectedWaypoints(routeAttachmentPtr);
                        mWaypointSelectionRequest.reset();
                     }
                     else
                     {
                        auto selectedWaypoints = platform->GetSelectedWaypoints();
                        for (const auto& ptIt : selectedWaypoints)
                        {
                           auto waypoint = routeAttachmentPtr->GetWaypoint(ptIt);
                           if (waypoint)
                           {
                              routeAttachmentPtr->SetSelected(viewerPtr, true, waypoint->GetId());
                           }
                        }

                        routeAttachmentPtr->SetSelectionEnabled(true);
                        platform->ClearSelectedWaypoints();
                        platform->ResetDeletedIndex();
                     }
                     routeAttachmentPtr->UpdateVisibility();
                  }
               }
            }
         }
      }
      // This entity is a vespa::VaEntity, which could be a global route
      else
      {
         auto vtkEnt = vaEnv.GetStandardScenario()->FindEntity(aPlatformName.toStdString());
         if (vtkEnt)
         {
            for (const auto& route : vtkEnt->FindAttachmentsOfType<wkf::AttachmentRoute>())
            {
               if (route->IsGlobalRoute())
               {
                  UpdateGlobalRoute(route);
               }
            }
         }
      }
   }
}

void MapRoute::Plugin::UpdateGlobalRoute(wkf::AttachmentRoute* aRoutePtr)
{
   auto routeName = aRoutePtr->GetName();
   // Find this route in the proxy
   WsfPM_RouteMap routeMap = mProxyRoot.routes();
   for (auto&& it = routeMap.begin(); it != routeMap.end(); ++it)
   {
      WsfPM_Route route          = routeMap.AtT(it);
      auto        proxyRouteName = route.GetName();
      if (route.GetName() == routeName)
      {
         // Found the route.
         // Now, loop through the waypoints and update their positions.
         size_t waypointCount = route.WaypointCount();
         aRoutePtr->DeleteWaypoints();
         wkf::AttachmentRoute::WaypointDataList waypoints;
         for (size_t i = 0; i < waypointCount; ++i)
         {
            auto   currentProxyWaypoint = route.GetWaypoint(i);
            double alt                  = mTerrainQuery.GetWaypointAltitude(currentProxyWaypoint);
            auto   proxyPosition        = currentProxyWaypoint.Position();
            auto   label                = currentProxyWaypoint.Label();
            int    gotoIndex            = GetWaypointGotoIndex(route, currentProxyWaypoint.GotoLabel());
            waypoints.emplace_back(proxyPosition.GetLatitude(), proxyPosition.GetLongitude(), alt, gotoIndex, label);
         }
         aRoutePtr->AddWaypoints(waypoints);
         aRoutePtr->SetDraggable(true);
         return;
      }
   }
}

void MapRoute::Plugin::PlatformDeleted(const QString& aPlatformName)
{
   if (mRouteMap.find(aPlatformName) != mRouteMap.end())
   {
      delete mRouteMap[aPlatformName];
      mRouteMap.erase(aPlatformName);
   }
}

void MapRoute::Plugin::MapProjectionChangedCB(unsigned int aViewerId, const UtoMapProjection* aProjection)
{
   if (vaEnv.GetStandardViewer())
   {
      if (aViewerId == vaEnv.GetStandardViewer()->GetUniqueId())
      {
         for (auto&& it : mRouteMap)
         {
            it.second->SetDirty();
            it.second->SetPositionByTime(0.0);
         }
      }
   }
}

void MapRoute::Plugin::AttachmentSelectionChangedCB(vespa::VaViewer*                aViewer,
                                                    int                             aSelected,
                                                    vespa::VaAttachment*            aAttachment,
                                                    const vespa::VaAttachmentSubId& aId)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wkf::Platform* platformPtr = dynamic_cast<wkf::Platform*>(&aAttachment->GetParent());
      if (platformPtr)
      {
         wizard::Platform* wizPlatformPtr =
            dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(platformPtr->GetName()));
         if (wizPlatformPtr)
         {
            // TODO: Distinguish between waypoints being de-selected when the route is destroyed (each time a route is
            // updated) and  the user triggering a Clear operation
            wkf::AttachmentRoute* route = dynamic_cast<wkf::AttachmentRoute*>(aAttachment);
            if (route)
            {
               wkf::Waypoint* waypoint = route->GetWaypoint(route->FindWaypointIndex(aId.mSubId));
               if (waypoint)
               {
                  waypoint->SetSelected(aSelected);
               }
               wizPlatformPtr->UpdateSelectedWaypoints(route);
               wizPlatformPtr->HandleQueuedDeleteWaypoint();
            }
         }
      }
   }
}

void MapRoute::Plugin::UpdateEpoch(const UtCalendar& aCalendar)
{
   for (auto&& it : mRouteMap)
   {
      it.second->UpdateEpoch(aCalendar);
   }
}

void MapRoute::Plugin::TeamColorsChanged()
{
   for (auto&& it : mRouteMap)
   {
      it.second->ResetColor();
   }
}


void MapRoute::Plugin::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (mProxyRoot.GetProxy() == aProxyPtr)
   {
      return;
   }
   mProxyRoot = aProxyPtr;
   if (aProxyPtr != nullptr)
   {
      WsfPM_RouteMap routeMap = mProxyRoot.routes();
      for (auto&& it = routeMap.begin(); it != routeMap.end(); ++it)
      {
         WsfPM_Route route = routeMap.AtT(it);
         GlobalRouteAdded(route);
      }
   }
}

void MapRoute::Plugin::GlobalRouteAdded(WsfPM_Route& aRoute)
{
   std::string routeName = "";
   WsfPM_Route route     = aRoute;
   if (aRoute.IsOfType("Route"))
   {
      routeName = aRoute.GetName();
   }
   else if (aRoute.GetParent().IsOfType("Route"))
   {
      routeName = aRoute.GetParent().GetName();
      route     = WsfPM_Route(aRoute.GetParent());
   }

   if (!routeName.empty())
   {
      vespa::VaAttachment* foundAttachment = vaEnv.FindAttachment(routeName);
      if (foundAttachment == nullptr)
      {
         vespa::VaScenario* scenario  = vaEnv.GetStandardScenario();
         wkf::Viewer*       viewerPtr = vaEnv.GetStandardViewer();
         if (scenario != nullptr && viewerPtr != nullptr)
         {
            std::string anchorName = routeName + "_anchor";
            auto        tempAnchor = ut::make_unique<vespa::VaEntity>(anchorName);
            mTerrainQuery.SetPlatformName(QString::fromStdString(anchorName));

            auto routeAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*tempAnchor, viewerPtr);
            routeAttachmentPtr->SetName(routeName);
            scenario->AddEntity(tempAnchor.release());
            routeAttachmentPtr->ShowIndices(false);

            if (GlobalRouteAddWaypoints(aRoute, routeAttachmentPtr) == Route::BuildStatus::MixedWaypointError)
            {
               wizard::ProxyUtil::InvalidateInput(aRoute);
            }

            if (!routeAttachmentPtr->GetWaypoints().empty())
            {
               wkf::Waypoint* firstWayPoint = routeAttachmentPtr->GetWaypoint(0);
               routeAttachmentPtr->GetParent().SetPosition(
                  vespa::VaPosition(firstWayPoint->GetLat(), firstWayPoint->GetLon(), firstWayPoint->GetAlt()));
            }
            routeAttachmentPtr->SetTeamColor(UtColor(0.75, 0.75, 0.75, 1));
            routeAttachmentPtr->UpdateTeamColor();
            routeAttachmentPtr->SetSelectionEnabled(true);
            routeAttachmentPtr->SetDraggable(true);
            routeAttachmentPtr->SetContextMenuEnabled(true);
         }
      }
   }
}

MapRoute::Route::BuildStatus MapRoute::Plugin::GlobalRouteAddWaypoints(const WsfPM_Route&    aRoute,
                                                                       wkf::AttachmentRoute* aRoutePtr)
{
   Route::BuildStatus                     status{Route::BuildStatus::NoPosition};
   size_t                                 waypointCount = aRoute.WaypointCount();
   wkf::AttachmentRoute::WaypointDataList waypoints;
   for (size_t i = 0; i < waypointCount; ++i)
   {
      if (i == 0)
      {
         status = GlobalRouteAddFirstWaypoint(aRoute.GetWaypoint(0), waypoints);
         continue;
      }

      auto currentProxyWaypoint  = aRoute.GetWaypoint(i);
      auto previousProxyWaypoint = aRoute.GetWaypoint(i - 1);

      int gotoIndex = GetWaypointGotoIndex(aRoute, currentProxyWaypoint.Label());

      if (!currentProxyWaypoint.Altitude().IsUnset())
      {
         mLastWaypointWithDefinedAltitude = i;
      }
      auto waypointAltitude = mTerrainQuery.GetWaypointAltitude(currentProxyWaypoint);
      if (currentProxyWaypoint.Altitude().IsUnset())
      {
         waypointAltitude = mTerrainQuery.GetWaypointAltitude(aRoute.GetWaypoint(mLastWaypointWithDefinedAltitude));
      }

      auto waypointType = currentProxyWaypoint.GetWaypointType();
      switch (waypointType)
      {
      case WsfPM_Waypoint::cPOSITION:
      {
         status             = Route::BuildStatus::SecondPosition;
         auto proxyPosition = currentProxyWaypoint.Position();
         waypoints.emplace_back(proxyPosition.GetLatitude(),
                                proxyPosition.GetLongitude(),
                                waypointAltitude,
                                gotoIndex,
                                currentProxyWaypoint.Label(),
                                currentProxyWaypoint.GetWaypointType());
         break;
      }
      case WsfPM_Waypoint::cOFFSET:
      {
         // Prevent routes from mixing offset and position waypoints, besides the initial waypoint
         if (status == Route::BuildStatus::SecondPosition)
         {
            return Route::BuildStatus::MixedWaypointError;
         }

         auto                                proxyOffset = currentProxyWaypoint.Offset();
         const double                        x           = proxyOffset.mX;
         const double                        y           = proxyOffset.mY;
         const double                        range       = std::hypot(x, y);
         const double                        heading     = std::atan2(y, x);
         const double                        headingDeg  = heading * UtMath::cDEG_PER_RAD;
         double                              lat, lon;
         wkf::AttachmentRoute::WaypointData& prefwpt = waypoints.at(i - 1);
         UtSphericalEarth::ExtrapolateGreatCirclePosition(prefwpt.mLat, prefwpt.mLon, headingDeg, range, lat, lon);
         waypoints.emplace_back(lat,
                                lon,
                                waypointAltitude,
                                gotoIndex,
                                currentProxyWaypoint.Label(),
                                currentProxyWaypoint.GetWaypointType());
         break;
      }
      default:
         // Add a null waypoint
         waypoints.emplace_back(0.0, 0.0, 0.0, gotoIndex, "");
         break;
      }
   }
   aRoutePtr->AddWaypoints(waypoints);
   return status;
}

MapRoute::Route::BuildStatus MapRoute::Plugin::GlobalRouteAddFirstWaypoint(const WsfPM_Waypoint& aProxyWaypoint,
                                                                           wkf::AttachmentRoute::WaypointDataList& aWaypoints)
{
   Route::BuildStatus status{Route::BuildStatus::NoPosition};

   auto waypointType  = aProxyWaypoint.GetWaypointType();
   auto proxyAltitude = aProxyWaypoint.Altitude();
   switch (waypointType)
   {
   case WsfPM_Waypoint::cPOSITION:
   {
      status             = Route::BuildStatus::InitialPosition;
      auto proxyPosition = aProxyWaypoint.Position();
      aWaypoints.emplace_back(proxyPosition.GetLatitude(),
                              proxyPosition.GetLongitude(),
                              proxyAltitude.GetValue(),
                              -1,
                              aProxyWaypoint.Label(),
                              waypointType);
      break;
   }
   case WsfPM_Waypoint::cOFFSET:
   {
      auto         proxyOffset = aProxyWaypoint.Offset();
      const double x           = proxyOffset.mX;
      const double y           = proxyOffset.mY;
      const double range       = sqrt(x * x + y * y);
      const double relBearing  = atan2(y, x);
      const double headingDeg  = relBearing * UtMath::cDEG_PER_RAD;
      double       lat, lon;
      UtSphericalEarth::ExtrapolateGreatCirclePosition(0.0, 0.0, headingDeg, range, lat, lon);
      aWaypoints.emplace_back(lat, lon, proxyAltitude.GetValue(), -1, aProxyWaypoint.Label(), waypointType);
      break;
   }
   default:
      // Add a null waypoint
      aWaypoints.emplace_back(0.0, 0.0, 0.0, -1, "");
      break;
   }
   return status;
}

int MapRoute::Plugin::GetWaypointGotoIndex(const WsfPM_Route& aRoute, const std::string& aLabel)
{
   if (aLabel.empty())
   {
      return -1;
   }

   size_t waypointCount = aRoute.WaypointCount();
   for (size_t i = 0; i < waypointCount; ++i)
   {
      WsfPM_Waypoint wypt = aRoute.GetWaypoint(i);
      if (wypt.Label() == aLabel)
      {
         return static_cast<int>(i);
      }
   }
   return -1;
}

void MapRoute::Plugin::BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId)
{
   wkf::AttachmentRoute* routePtr = dynamic_cast<wkf::AttachmentRoute*>(aAttachmentPtr);
   if (routePtr && routePtr->GetContextMenuEnabled())
   {
      if (aSubPartId != 0)
      {
         int waypointIndex = 0;
         // loop through the route and find the waypoint that was hit
         for (size_t i = 0; i < routePtr->GetWaypoints().size(); ++i)
         {
            wkf::Waypoint* curWaypt = routePtr->GetWaypoint(i);
            if (aSubPartId == curWaypt->GetId())
            {
               QMenu* routeSub = aMenu->findChild<QMenu*>("Route", Qt::FindDirectChildrenOnly);
               if (!routeSub)
               {
                  routeSub = aMenu->addMenu(QIcon::fromTheme("route"), "Route");
                  routeSub->setObjectName("Route");
               }
               waypointIndex     = i;
               QString      name = "Waypoint " + QString::number(waypointIndex);
               wkf::Action* deleteWaypointAction =
                  new wkf::Action(QIcon::fromTheme("delete"), QString("Delete %1").arg(name), routeSub);
               routeSub->addAction(deleteWaypointAction);
               connect(deleteWaypointAction,
                       &QAction::triggered,
                       this,
                       [=]() { DeleteWaypointActionHandler(routePtr, waypointIndex); });

               wkf::Action* createWaypointAction =
                  new wkf::Action(QIcon::fromTheme("add"), QString("Add Waypoint after"), routeSub);
               routeSub->addAction(createWaypointAction);
               connect(createWaypointAction,
                       &QAction::triggered,
                       this,
                       [=]() { CreateWaypointHandler(routePtr, waypointIndex); });
               createWaypointAction->setObjectName("Add Waypoint after");

               break;
            }
         }
      }
   }
}

void MapRoute::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr && aEntityPtr->IsA_TypeOf<wizard::Platform>())
   {
      auto platformPtr = dynamic_cast<wizard::Platform*>(aEntityPtr);
      if (platformPtr)
      {
         QString name = aEntityPtr->GetName().c_str();
         auto    rte  = aEntityPtr->FindFirstAttachmentOfType<wkf::AttachmentRoute>();
         if ((!rte) || (rte->GetWaypoints().empty()))
         {
            if (platformPtr->IsRouteable())
            {
               QMenu* routeSub = aMenu->findChild<QMenu*>("Route", Qt::FindDirectChildrenOnly);
               if (!routeSub)
               {
                  routeSub = aMenu->addMenu(QIcon::fromTheme("route"), "Route");
                  routeSub->setObjectName("Route");
               }
               wkf::Action* addRoute =
                  new wkf::Action(QIcon::fromTheme("add"), QString("Create Route on %1").arg(name), routeSub);
               routeSub->addAction(addRoute);
               connect(addRoute, &QAction::triggered, this, [=]() { AddRouteToPlatformHandler(platformPtr); });
            }
         }
      }
   }
}

void MapRoute::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   if (aViewerPtr && aViewerPtr == vaEnv.GetStandardViewer())
   {
      auto AddLocationMenu = [aMenu]()
      {
         auto addMenu = aMenu->findChild<QMenu*>("Add at Location", Qt::FindDirectChildrenOnly);
         if (!addMenu)
         {
            addMenu = aMenu->addMenu("Add at Location");
            addMenu->setObjectName("Add at Location");
         }
         return addMenu;
      };

      vespa::VaHitSet hs = aViewerPtr->GetSelection();
      if (hs.size() == 1)
      {
         wkf::AttachmentRoute* rte = dynamic_cast<wkf::AttachmentRoute*>(hs.begin()->FindAttachment());
         if (rte)
         {
            int waypointIndex = -1;
            for (size_t i = 0; i < rte->GetWaypoints().size(); ++i)
            {
               wkf::Waypoint* wp = rte->GetWaypoint(i);
               if (wp->IsSelected())
               {
                  if (waypointIndex == -1)
                  {
                     waypointIndex = static_cast<int>(i);
                  }
                  else
                  {
                     break;
                  }
               }
            }
            if (waypointIndex >= 0)
            {
               auto routeSub = aMenu->findChild<QMenu*>("Route", Qt::FindDirectChildrenOnly);
               if (!routeSub) // if we aren't already showing the cursor-over-route-context-menu
               {
                  auto addMenu              = AddLocationMenu();
                  auto createWaypointAction = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("route"),
                                                                               QString("Waypoint after selection"),
                                                                               addMenu);
                  addMenu->addAction(createWaypointAction);
                  connect(createWaypointAction,
                          &QAction::triggered,
                          this,
                          [=]() { CreateWaypointAtLocationHandler(rte, waypointIndex); });
                  createWaypointAction->setObjectName("Waypoint after selection");
               }
            }
         }
      }
   }
}

void MapRoute::Plugin::DeleteWaypointActionHandler(wkf::AttachmentRoute* aRoutePtr, int aWaypointIndex)
{
   auto platformPtr = dynamic_cast<wizard::Platform*>(&aRoutePtr->GetParent());
   if (platformPtr)
   {
      platformPtr->RemoveWaypointFromEditor(aRoutePtr, aWaypointIndex);
   }
   else
   {
      // this route is not attached to a platform, treat it as a global route
      aRoutePtr->DeleteWaypoint(aWaypointIndex);
      EditGlobalRoute(aRoutePtr);
   }
}

void MapRoute::Plugin::EditGlobalRoute(wkf::AttachmentRoute* aRoutePtr)
{
   WsfPM_Root proxyRoot(wizard::Project::Instance()->Proxy());
   auto       curRoute = WsfPM_Route(proxyRoot + "routes" + aRoutePtr->GetName());
   if (curRoute.IsValid() && (curRoute.WaypointCount() > 0))
   {
      std::string routeDef = "route ";
      routeDef += aRoutePtr->GetName();
      routeDef += "\n";
      for (auto curWayPt : aRoutePtr->GetWaypoints())
      {
         std::stringstream ss;
         vespa::VaPosition wayPtPos;
         wayPtPos.SetLat(curWayPt->GetLat());
         wayPtPos.SetLon(curWayPt->GetLon());
         wayPtPos.SetAlt(curWayPt->GetAlt());
         ss << wayPtPos.GetLat() << " " << wayPtPos.GetLon() << " altitude " << wayPtPos.GetAlt();
         unsigned int tabSpace = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
         for (unsigned int i = 0; i < tabSpace; i++)
         {
            routeDef += " ";
         }
         routeDef += "position " + ss.str() + "\n";
      }
      routeDef += "end_route\n";

      auto routeTextRange = curRoute.GetSourceDocumentRange();
      auto fileName       = curRoute.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
      auto editorPtr      = wizard::Project::Instance()->GotoFile(fileName);
      if (editorPtr)
      {
         auto cursor = editorPtr->textCursor();
         cursor.setPosition(routeTextRange.GetBegin());
         cursor.setPosition(routeTextRange.GetEnd() + 1, QTextCursor::KeepAnchor);
         editorPtr->setTextCursor(cursor);
         wizEnv.BeginUndoCapture();
         cursor.insertText(QString::fromStdString(routeDef));
         wizEnv.EndUndoCapture();
      }
   }
}

void MapRoute::Plugin::AddRouteToPlatformHandler(wizard::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      auto viewerPtr = vaEnv.GetStandardViewer();
      if (viewerPtr)
      {
         viewerPtr->ClearSelection();
      }
      wkf::RouteBrowserInterface::WaypointInfo info;
      info.mLat = aPlatformPtr->GetPosition().GetLat();
      info.mLon = aPlatformPtr->GetPosition().GetLon();

      // This is to catch garbage values due to uninitialized altitude variable.
      double alt = static_cast<double>(aPlatformPtr->GetPosition().GetAlt());
      if (UtMath::NearlyZero(alt, 1e-6))
      {
         info.mAlt = 0.0;
      }
      else
      {
         info.mAlt = alt;
      }

      // check to see if the platform's alt value is in agl
      wsf::proxy::AltitudeReferenceEnum altRefVal = aPlatformPtr->GetPlatform().AltitudeReferenceValue();
      switch (altRefVal)
      {
      case wsf::proxy::AltitudeReferenceEnum::MSL:
         info.mAltRef = static_cast<int>(wkf::RouteBrowserInterface::WaypointInfo::AltitudeReference::AGL);
         break;
      case wsf::proxy::AltitudeReferenceEnum::AGL:
         info.mAltRef = static_cast<int>(wkf::RouteBrowserInterface::WaypointInfo::AltitudeReference::MSL);
         break;
      case wsf::proxy::AltitudeReferenceEnum::Default:
      default:
         break;
      }
      info.mAlt = aPlatformPtr->GetPlatform().Altitude();

      info.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;
      aPlatformPtr->AddWaypointToEditor(info, 0);
      mWaypointSelectionRequest = std::make_pair(aPlatformPtr->GetName(), 0);
   }
}

void MapRoute::Plugin::CreateWaypointHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx)
{
   if (aRoutePtr != nullptr && aWaypointIdx < aRoutePtr->GetWaypoints().size())
   {
      auto viewerPtr = vaEnv.GetStandardViewer();
      if (viewerPtr)
      {
         int mouseX, mouseY;
         viewerPtr->GetMousePosition(mouseX, mouseY);
         double lat, lon, alt;
         viewerPtr->GetCamera()->ConstConvertScreenToLLA(mouseX, mouseY, lat, lon, alt);
         alt = aRoutePtr->GetWaypoint(aWaypointIdx)->GetAlt();

         auto platPtr = dynamic_cast<wizard::Platform*>(&aRoutePtr->GetParent());
         if (platPtr)
         {
            viewerPtr->ClearSelection();
            wkf::RouteBrowserInterface::WaypointInfo info;
            info.mLat       = lat;
            info.mLon       = lon;
            info.mAlt       = alt;
            info.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;

            platPtr->AddWaypointToEditor(info, aWaypointIdx);
            mWaypointSelectionRequest = std::make_pair(platPtr->GetName(), aWaypointIdx + 1);
            // 1. we push a change to the proxy via the above.
            // 2. PlatformUpdated is called due to the proxy change and rebuilds the route
            // 3. mWaypointSelectionRequest informs that we would like the new waypoint selected
         }
         else
         {
            aRoutePtr->GetViewer()->ClearSelection();
            aRoutePtr->InsertWaypoint(lat, lon, alt, -1, "", aWaypointIdx + 1);
            aRoutePtr->SelectWaypoint(aWaypointIdx + 1);
            EditGlobalRoute(aRoutePtr);
         }
      }
   }
}

void MapRoute::Plugin::CreateWaypointAtLocationHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx)
{
   if (aRoutePtr != nullptr && aWaypointIdx < aRoutePtr->GetWaypoints().size())
   {
      vespa::VaViewer* viewerPtr = vaEnv.GetStandardViewer();
      if (viewerPtr)
      {
         int mouseX, mouseY;
         viewerPtr->GetMousePosition(mouseX, mouseY);
         double lat, lon, alt;
         viewerPtr->GetCamera()->ConstConvertScreenToLLA(mouseX, mouseY, lat, lon, alt);
         alt = aRoutePtr->GetWaypoint(aWaypointIdx)->GetAlt();

         auto platPtr = dynamic_cast<wizard::Platform*>(&aRoutePtr->GetParent());
         if (platPtr)
         {
            viewerPtr->ClearSelection();
            wkf::RouteBrowserInterface::WaypointInfo info;
            info.mLat       = lat;
            info.mLon       = lon;
            info.mAlt       = alt;
            info.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;
            platPtr->AddWaypointToEditor(info, aWaypointIdx);
            mWaypointSelectionRequest = std::make_pair(platPtr->GetName(), aWaypointIdx + 1);
         }
         else
         {
            aRoutePtr->GetViewer()->ClearSelection();
            aRoutePtr->InsertWaypoint(lat, lon, alt, aWaypointIdx, "", aWaypointIdx + 1);
            EditGlobalRoute(aRoutePtr);
         }
      }
   }
}

void MapRoute::Plugin::LineWidthChanged(int aWidth)
{
   for (const auto& r : mRouteMap)
   {
      r.second->SetOrbitLineWidth(aWidth);
   }
}

void MapRoute::Plugin::PeriodsChanged(unsigned int aPeriods)
{
   for (const auto& r : mRouteMap)
   {
      r.second->SetOrbitPeriods(aPeriods);
   }
}
