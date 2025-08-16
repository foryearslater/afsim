// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RouteBrowserInterface.hpp"

#include "Editor.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyUtil.hpp"
#include "ProxyWatcher.hpp"
#include "RouteBrowserCreateDialog.hpp"
#include "RouteBrowserEditDialog.hpp"
#include "UtCompilerAttributes.hpp"
#include "UtSphericalEarth.hpp"
#include "VaUtils.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPath.hpp"
#include "route/WkfRouteBrowserDockWidget.hpp"

RouteBrowser::RouteBrowserInterface::RouteBrowserInterface()
   : wkf::RouteBrowserInterface(wkfEnv.GetMainWindow(), vaEnv.GetStandardViewer())
   , mProxyRoot(nullptr)
{
   auto                                    curCreateDialog = mDockWidget->GetCreateDialog();
   RouteBrowser::RouteBrowserCreateDialog* wizCreateDialog =
      new RouteBrowser::RouteBrowserCreateDialog(curCreateDialog->parentWidget(), curCreateDialog->windowFlags());
   mDockWidget->SetCreateDialog(wizCreateDialog);

   auto                                  curEditDialog = mDockWidget->GetEditDialog();
   RouteBrowser::RouteBrowserEditDialog* wizEditDialog =
      new RouteBrowser::RouteBrowserEditDialog(curEditDialog->parentWidget(), curEditDialog->windowFlags());
   mDockWidget->SetEditDialog(wizEditDialog);

   ConnectP();
   GetDockWidget()->SetNewRouteButtonEnabled(false);
}

void RouteBrowser::RouteBrowserInterface::SelectRoute(const std::string& aPlatformName)
{
   QString platformName = QString::fromStdString(aPlatformName);
   if (platformName.endsWith("_anchor"))
   {
      // this is a global route
      wkf::RouteBrowserInterface::RouteInfo globalRouteInfo = GetGlobalRouteInfo(aPlatformName);
      GetDockWidget()->AddGlobal(globalRouteInfo);
      mDockWidget->CollapseAllGlobalRoutes();
      mDockWidget->ExpandGlobalRoute(globalRouteInfo.mName);
   }
   else
   {
      mDockWidget->CollapseAllGlobalRoutes();
      wkf::RouteBrowserInterface::RouteInfo routeInfo = GetRouteInfo(aPlatformName);
      if (routeInfo.mWaypoints.isEmpty())
      {
         // this is a platform with no route. See if it is a platform with a mover that can support a route, and if so
         // enable the New Route button
         mDockWidget->SetEditRouteButtonEnabled(false);

         if (!PlatformHasRouteMover(aPlatformName))
         {
            mDockWidget->SetNewRouteButtonEnabled(false);
            mDockWidget->ClearSelection();
         }
         else
         {
            mDockWidget->SetNewRouteButtonEnabled(true);
         }
      }
      PlatformOfInterestAddedHandler(routeInfo);
   }
}

void RouteBrowser::RouteBrowserInterface::GetRouteInfo(WsfPM_Route& aRoute, RouteInfo& aRouteInfo)
{
   mDockWidget->SetNewRouteButtonEnabled(true);
   int         endOfPath     = 0;
   size_t      waypointCount = aRoute.WaypointCount();
   BuildStatus status{BuildStatus::NoPosition};
   for (size_t i = 0; i < waypointCount; ++i)
   {
      WsfPM_Waypoint                           wypt = aRoute.GetWaypoint(i);
      wkf::RouteBrowserInterface::WaypointInfo waypointInfo;

      // label
      waypointInfo.mLabel = QString::fromStdString(wypt.Label());

      status = GetPositionInfo(status, waypointInfo, aRouteInfo, wypt);
      if (status == BuildStatus::MixedWaypointError)
      {
         wizard::ProxyUtil::InvalidateInput(aRoute);
      }
      GetAltitudeInfo(waypointInfo, aRouteInfo, wypt);
      GetAltitudeReferenceInfo(waypointInfo, aRouteInfo, wypt);

      // goto
      waypointInfo.mGoTo = QString::fromStdString(wypt.GotoLabel());

      // heading
      waypointInfo.mHeading = GetUnitaryInfo(aRouteInfo, wypt.Heading());
      // speed
      waypointInfo.mSpeed = GetUnitaryInfo(aRouteInfo, wypt.Speed());
      // linear acceleration
      waypointInfo.mLinAccel = GetUnitaryInfo(aRouteInfo, wypt.LinearAcceleration());
      // radial acceleration
      waypointInfo.mRadAccel = GetUnitaryInfo(aRouteInfo, wypt.RadialAcceleration());
      // climb rate
      waypointInfo.mClimbRate = GetUnitaryInfo(aRouteInfo, wypt.ClimbRate());
      GetTurnDirectionInfo(waypointInfo, aRouteInfo, wypt);
      GetSwitchTypeInfo(waypointInfo, aRouteInfo, wypt);

      endOfPath = static_cast<int>(wypt.EndOfPath());

      aRouteInfo.mWaypoints.push_back(waypointInfo);
   }

   aRouteInfo.mEndPathType = endOfPath;
}

RouteBrowser::BuildStatus RouteBrowser::RouteBrowserInterface::GetPositionInfo(const BuildStatus aPreviousStatus,
                                                                               WaypointInfo&     aInfo,
                                                                               const RouteInfo&  aRoute,
                                                                               const WsfPM_Waypoint& aProxyWaypoint) const noexcept
{
   // position
   auto waypointType = aProxyWaypoint.GetWaypointType();
   if (aRoute.mWaypoints.isEmpty())
   {
      switch (waypointType)
      {
      case WsfPM_Waypoint::cPOSITION:
      {
         aInfo.mPointType   = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;
         auto proxyPosition = aProxyWaypoint.Position();
         aInfo.mLat         = proxyPosition.GetLatitude();
         aInfo.mLon         = proxyPosition.GetLongitude();
         return BuildStatus::InitialPosition;
      }
      case WsfPM_Waypoint::cOFFSET:
      {
         aInfo.mPointType         = wkf::RouteBrowserInterface::WaypointInfo::cRELATIVE;
         auto         proxyOffset = aProxyWaypoint.Offset();
         const double x           = proxyOffset.mX;
         const double y           = proxyOffset.mY;
         aInfo.mXOffset           = x;
         aInfo.mYOffset           = y;
         const double range       = std::hypot(x, y);
         const double heading     = std::atan2(y, x);
         const double headingDeg  = heading * UtMath::cDEG_PER_RAD;
         // There is no conversion from relative to absolute in WKF, so do it here
         double lat, lon;
         UtSphericalEarth::ExtrapolateGreatCirclePosition(0.0, 0.0, headingDeg, range, lat, lon);
         aInfo.mLat = lat;
         aInfo.mLon = lon;
         return BuildStatus::NoPosition;
      }
      case WsfPM_Waypoint::cTURN:
      {
         aInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cOTHER;
         // TODO for AFSIM-1205
         return BuildStatus::NoPosition;
      }
      case WsfPM_Waypoint::cOTHER:
         aInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cOTHER;
         CPP17_FALLTHROUGH;
      default:
         // Add a null waypoint
         aInfo.mLat = 0.0;
         aInfo.mLon = 0.0;
         return BuildStatus::NoPosition;
      }
   }
   else
   {
      switch (waypointType)
      {
      case WsfPM_Waypoint::cPOSITION:
      {
         aInfo.mPointType   = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;
         auto proxyPosition = aProxyWaypoint.Position();
         aInfo.mLat         = proxyPosition.GetLatitude();
         aInfo.mLon         = proxyPosition.GetLongitude();
         return BuildStatus::SecondPosition;
      }
      case WsfPM_Waypoint::cOFFSET:
      {
         aInfo.mPointType          = wkf::RouteBrowserInterface::WaypointInfo::cRELATIVE;
         auto         proxyOffset  = aProxyWaypoint.Offset();
         const double x            = proxyOffset.mX;
         const double y            = proxyOffset.mY;
         aInfo.mXOffset            = x;
         aInfo.mYOffset            = y;
         const double range        = std::hypot(x, y);
         const double heading      = std::atan2(y, x);
         const double headingDeg   = heading * UtMath::cDEG_PER_RAD;
         const auto&  previousInfo = aRoute.mWaypoints.back();
         // There is no conversion from relative to absolute in WKF, so do it here
         double lat, lon;
         UtSphericalEarth::ExtrapolateGreatCirclePosition(previousInfo.mLat, previousInfo.mLon, headingDeg, range, lat, lon);
         aInfo.mLat = lat;
         aInfo.mLon = lon;
         if (aPreviousStatus == BuildStatus::SecondPosition)
         {
            return BuildStatus::MixedWaypointError;
         }
         else
         {
            return BuildStatus::InitialPosition;
         }
      }
      case WsfPM_Waypoint::cTURN:
      {
         aInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cOTHER;
         // TODO for AFSIM-1205
         return aPreviousStatus;
      }
      case WsfPM_Waypoint::cOTHER:
         aInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cOTHER;
         CPP17_FALLTHROUGH;
      default:
         // Add a null waypoint
         aInfo.mLat = 0.0;
         aInfo.mLon = 0.0;
         return aPreviousStatus;
      }
   }
}

void RouteBrowser::RouteBrowserInterface::GetAltitudeInfo(WaypointInfo&         aInfo,
                                                          const RouteInfo&      aRoute,
                                                          const WsfPM_Waypoint& aProxyWaypoint) const noexcept
{
   // altitude
   WsfProxy::Length alt = aProxyWaypoint.Altitude();
   if (alt.IsUnset())
   {
      // Note:  Wizard does not support default altitude reference, which would be the correct decision for the altitude
      // reference when unset.
      //        For now, just use the same terminology for altitude reference as for altitude.
      if (aRoute.mWaypoints.isEmpty())
      {
         aInfo.mAlt    = wkf::WaypointValueType::cNOT_SET;
         aInfo.mAltRef = static_cast<int>(WaypointInfo::AltitudeReference::Unset);
      }
      else
      {
         aInfo.mAlt    = wkf::WaypointValueType::cUSE_PREVIOUS;
         aInfo.mAltRef = static_cast<int>(WaypointInfo::AltitudeReference::UsePrevious);
      }
   }
   else
   {
      aInfo.mAlt = alt.GetValue();
   }
}

void RouteBrowser::RouteBrowserInterface::GetAltitudeReferenceInfo(WaypointInfo&         aInfo,
                                                                   const RouteInfo&      aRoute,
                                                                   const WsfPM_Waypoint& aProxyWaypoint) const noexcept
{
   // agl/msl
   if (aProxyWaypoint.GetAltitudeReference().IsUnset())
   {
      aInfo.mAltRef = static_cast<int>(WaypointInfo::AltitudeReference::MoverDefault);
   }
   else if (aProxyWaypoint.AltitudeReferenceValue() == wsf::proxy::AltitudeReferenceEnum::MSL)
   {
      aInfo.mAltRef = static_cast<int>(WaypointInfo::AltitudeReference::MSL);
   }
   else if (aProxyWaypoint.AltitudeReferenceValue() == wsf::proxy::AltitudeReferenceEnum::AGL)
   {
      aInfo.mAltRef = static_cast<int>(WaypointInfo::AltitudeReference::AGL);
   }
}

void RouteBrowser::RouteBrowserInterface::GetTurnDirectionInfo(WaypointInfo&         aInfo,
                                                               const RouteInfo&      aRoute,
                                                               const WsfPM_Waypoint& aProxyWaypoint) const noexcept
{
   // turn direction
   switch (aProxyWaypoint.GetTurnDirection())
   {
   case WsfPM_Waypoint::TurnDirection::cTD_LEFT:
   {
      aInfo.mTurnDir = static_cast<int>(WsfPath::cTURN_DIR_LEFT);
      break;
   }
   case WsfPM_Waypoint::TurnDirection::cTD_SHORTEST:
   {
      aInfo.mTurnDir = static_cast<int>(WsfPath::cTURN_DIR_SHORTEST);
      break;
   }
   case WsfPM_Waypoint::TurnDirection::cTD_RIGHT:
   {
      aInfo.mTurnDir = static_cast<int>(WsfPath::cTURN_DIR_RIGHT);
      break;
   }
   default:
      break;
   }
}

void RouteBrowser::RouteBrowserInterface::GetSwitchTypeInfo(WaypointInfo&         aInfo,
                                                            const RouteInfo&      aRoute,
                                                            const WsfPM_Waypoint& aProxyWaypoint) const noexcept
{
   // switch
   if (aProxyWaypoint.SwitchOnApproach().IsUnset())
   {
      aInfo.mSwitchType = 0; // mover default
   }
   else if (aProxyWaypoint.SwitchOnApproach())
   {
      aInfo.mSwitchType = 2; // switch on approach
   }
   else
   {
      aInfo.mSwitchType = 1; // switch on passing
   }
}

void RouteBrowser::RouteBrowserInterface::ConnectP()
{
   Connect(); // reestablish connections for the create and edit dialogs

   connect(mDockWidget,
           &wkf::RouteBrowserDockWidget::LocalRouteEditDialogRequested,
           this,
           &RouteBrowserInterface::LocalRouteEditDialogRequestedHandler);
   connect(mDockWidget,
           &wkf::RouteBrowserDockWidget::DockRouteSelectionChanged,
           this,
           &RouteBrowserInterface::DockRouteSelectionChangedHandler);

   connect(mDockWidget->GetEditDialog(), &wkf::RouteBrowserEditDialog::RouteEdited, this, &RouteBrowserInterface::EditRoute);
   connect(mDockWidget->GetEditDialog(),
           &wkf::RouteBrowserEditDialog::Apply,
           this,
           [&]() { mDockWidget->GetEditDialog()->DisableButtons(); });
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::FullParseComplete,
           this,
           [&]() { mDockWidget->GetEditDialog()->EnableButtons(); });

   RouteBrowser::RouteBrowserCreateDialog* createDialog =
      dynamic_cast<RouteBrowser::RouteBrowserCreateDialog*>(mDockWidget->GetCreateDialog());
   if (createDialog != nullptr)
   {
      connect(createDialog, &RouteBrowserCreateDialog::TempRouteEdited, this, &RouteBrowserInterface::TempRouteEdited);
   }

   mCallbacks.Add(
      wkf::Observer::PlatformPropertiesChanged.Connect(&RouteBrowserInterface::PlatformPropertiesChangedHandler, this));

   mProxyCallbacks.Add(wizard::ProxyWatcher::ProxyAvailable.Connect(&RouteBrowserInterface::OnProxyAvailable, this));
}

wkf::RouteBrowserInterface::RouteInfo RouteBrowser::RouteBrowserInterface::GetRouteInfo(const std::string& aPlatformName)
{
   wkf::RouteBrowserInterface::RouteInfo routeInfo;
   QString                               platformName = QString::fromStdString(aPlatformName);
   if (platformName.endsWith("_anchor"))
   {
      return GetGlobalRouteInfo(aPlatformName);
   }

   routeInfo.mPlatformName = QString::fromStdString(aPlatformName);
   if (vaEnv.GetStandardScenario())
   {
      wizard::Platform* platform =
         dynamic_cast<wizard::Platform*>(vaEnv.GetStandardScenario()->FindPlatformByName(aPlatformName));
      if (platform)
      {
         WsfPM_Mover mover = platform->GetPlatform().Mover();
         if (!mover.IsValid())
         {
            mDockWidget->SetNewRouteButtonEnabled(false);
         }
         else if (WsfPM_WaypointMover::IsWaypointMover(mover))
         {
            mDockWidget->SetNewRouteButtonEnabled(true);
            WsfPM_WaypointMover waypoint_mover = static_cast<WsfPM_WaypointMover>(mover);

            WsfPM_Route route = waypoint_mover.InitialRoute(); // TODO: use default route?
            if ((route.IsValid()) && (route.WaypointCount() > 0))
            {
               GetRouteInfo(route, routeInfo);
            }
         }
         else if (mover.IsOfType("WSF_P6DOF_MOVER"))
         {
            mDockWidget->SetNewRouteButtonEnabled(true);
            WsfPM_Route route = mover.InitialRoute();
            if ((route.IsValid()) && (route.WaypointCount() > 0))
            {
               GetRouteInfo(route, routeInfo);
            }
         }
         else
         {
            mDockWidget->SetNewRouteButtonEnabled(true);
         }
         if (mDockWidget->GetEditDialog()->isVisible())
         {
            mDockWidget->GetEditDialog()->PopulateDialog(routeInfo);
         }
      }
   }
   return routeInfo;
}

wkf::RouteBrowserInterface::RouteInfo RouteBrowser::RouteBrowserInterface::GetGlobalRouteInfo(const std::string& aPlatformName)
{
   wkf::RouteBrowserInterface::RouteInfo routeInfo;
   QString                               routePlatformName = QString::fromStdString(aPlatformName);
   routeInfo.mPlatformName                                 = routePlatformName;
   QString routeName                                       = routePlatformName;
   routeName.chop(7); // chop off "_anchor" from the global route platform name to get the route name
   routeInfo.mName      = routeName;
   WsfPM_Route curRoute = WsfPM_Route(mProxyRoot + "routes" + routeName.toStdString());
   if (curRoute.IsValid() && (curRoute.WaypointCount() > 0))
   {
      // this is the route that needs to be updated...
      GetRouteInfo(curRoute, routeInfo);
   }
   return routeInfo;
}

void RouteBrowser::RouteBrowserInterface::ConstructRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo,
                                                         const std::string&                           aEntityName)
{
   vespa::VaScenario* scenario = vaEnv.GetStandardScenario();
   if (scenario != nullptr)
   {
      vespa::VaEntity* tempAnchor         = new vespa::VaEntity(aEntityName);
      auto             routeAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*tempAnchor, mViewer);
      routeAttachmentPtr->SetName("route");
      scenario->AddEntity(tempAnchor);
      DrawRoute(aRouteInfo, routeAttachmentPtr);
      if (routeAttachmentPtr->GetWaypoints().size() > cMAX_DISPLAYED_INDICES)
      {
         routeAttachmentPtr->ShowIndices(false);
      }

      routeAttachmentPtr->SelectWaypoint(aRouteInfo.mWaypoints.size() - 1, false);
      routeAttachmentPtr->SetTeamColor(UtColor(1, 1, 0, 1));
      routeAttachmentPtr->UpdateTeamColor();
      routeAttachmentPtr->SetSelectionEnabled(true);
      routeAttachmentPtr->SetDraggable(true);
      routeAttachmentPtr->SetContextMenuEnabled(false);
   }
}


void RouteBrowser::RouteBrowserInterface::EditRoute(const RouteInfo& aRouteInfo)
{
   // This function gets triggered when fields are edited in the edit dialog
}

void RouteBrowser::RouteBrowserInterface::LocalRouteEditDialogRequestedHandler(const QString& aPlatformName)
{
   mDockWidget->GetEditDialog()->PopulateDialog(GetRouteInfo(aPlatformName.toStdString()));
   mDockWidget->GetEditDialog()->show();
}

void RouteBrowser::RouteBrowserInterface::PlatformPropertiesChangedHandler(wkf::Platform* aPlatformPtr)
{
   // Only update the Route Browser if the edited platform is being displayed.
   if (mDockWidget->IsPlatformDisplayed(QString::fromStdString(aPlatformPtr->GetName())))
   {
      mDockWidget->Clear();
      RouteInfo info = GetRouteInfo(aPlatformPtr->GetName());
      if (!info.mWaypoints.empty())
      {
         mDockWidget->PopulateLocal(info);
      }
   }
}

void RouteBrowser::RouteBrowserInterface::UpdateAllGlobalRoutes()
{
   if (wizard::Project::Instance() == nullptr)
   {
      return;
   }

   WsfPM_RouteMap         routeMap = WsfPM_RouteMap(mProxyRoot + "routes");
   std::vector<RouteInfo> globalRouteList;
   if (routeMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      routeMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            WsfPM_Route curRoute(*typeit);
            UpdateGlobalRoute(curRoute);
         }
      }
   }
}

void RouteBrowser::RouteBrowserInterface::UpdateGlobalRoute(const WsfPProxyNode& aNode)
{
   std::string routeName = "";
   WsfPM_Route route     = WsfPM_Route(aNode);
   if (aNode.IsOfType("Route"))
   {
      routeName = aNode.GetName();
   }
   else if (aNode.GetParent().IsOfType("Route"))
   {
      routeName = aNode.GetParent().GetName();
      route     = WsfPM_Route(aNode.GetParent());
   }

   if (!routeName.empty())
   {
      vespa::VaAttachment*      foundAttachment = vaEnv.FindAttachment(routeName);
      std::vector<unsigned int> selectedWaypoints;
      // if the attachment is found, delete it and recreate it
      if (foundAttachment)
      {
         // if any of this route's waypoints are currently selected, save off the selected indices and reselect them
         // after the route is rebuilt
         if (foundAttachment->GetSelected(vaEnv.GetFirstViewer()))
         {
            wkf::AttachmentRoute* attachmentRoutePtr = dynamic_cast<wkf::AttachmentRoute*>(foundAttachment);
            if (attachmentRoutePtr != nullptr)
            {
               for (unsigned int i = 0; i < attachmentRoutePtr->GetWaypoints().size(); ++i)
               {
                  if (attachmentRoutePtr->GetWaypoint(i)->IsSelected())
                  {
                     selectedWaypoints.emplace_back(i);
                  }
               }
            }
         }
         mDockWidget->GetCreateDialog()->RemoveRouteName(QString::fromStdString(routeName));
         DeleteRoute(QString::fromStdString(routeName));
         foundAttachment = nullptr;
      }
      if (foundAttachment == nullptr)
      {
         vespa::VaScenario* scenario = vaEnv.GetStandardScenario();
         if (scenario != nullptr)
         {
            std::string      anchorName         = routeName + "_anchor";
            vespa::VaEntity* tempAnchor         = new vespa::VaEntity(anchorName);
            auto             routeAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*tempAnchor, mViewer);
            routeAttachmentPtr->ShowIndices(false);
            routeAttachmentPtr->SetName(routeName);
            // Set entity position to the first waypoint position - otherwise route will be drawn starting at 0n 0e
            RouteInfo curRouteInfo;
            GetRouteInfo(route, curRouteInfo);
            curRouteInfo.mName = QString::fromStdString(routeName);
            if (!curRouteInfo.mWaypoints.empty())
            {
               WaypointInfo waypointInfo = curRouteInfo.mWaypoints[0];
               tempAnchor->SetPosition(vespa::VaPosition(waypointInfo.mLat, waypointInfo.mLon, waypointInfo.mAlt));
            }
            scenario->AddEntity(tempAnchor);
            DrawRoute(curRouteInfo, routeAttachmentPtr);
            if (wkfEnv.GetPreferenceObject<RouteBrowser::PrefObject>()->GetShowGlobalRoutes())
            {
               routeAttachmentPtr->SetTeamColor(UtColor(0.75, 0.75, 0.75, 1));
               routeAttachmentPtr->UpdateTeamColor();
               routeAttachmentPtr->SetSelectionEnabled(true);
               routeAttachmentPtr->SetDraggable(true);
               routeAttachmentPtr->SetContextMenuEnabled(true);
            }
            else
            {
               routeAttachmentPtr->SetStateVisibility(false);
            }

            mDockWidget->AddGlobal(curRouteInfo);
            routeAttachmentPtr->ClearSelection(
               mViewer); // Needed to ensure other waypoints aren't selected from DrawRoute above
            for (size_t i = 0; i < selectedWaypoints.size(); ++i)
            {
               auto curSelectedIdx = selectedWaypoints[i];
               if (curSelectedIdx < routeAttachmentPtr->GetWaypoints().size())
               {
                  routeAttachmentPtr->SetSelected(mViewer, true, routeAttachmentPtr->GetWaypoint(curSelectedIdx)->GetId());
               }
               if (i == selectedWaypoints.size() - 1)
               {
                  SelectRoute(anchorName);
                  mDockWidget->ExpandGlobalRoute(QString::fromStdString(routeName), i == 0 ? curSelectedIdx : -1);
               }
            }
            mDockWidget->GetCreateDialog()->AddSavedRoute(curRouteInfo);

            if (mDockWidget->GetEditDialog()->isVisible() &&
                mDockWidget->GetEditDialog()->GetRouteInfo().mName == curRouteInfo.mName)
            {
               mDockWidget->GetEditDialog()->PopulateDialog(curRouteInfo);
            }
         }
      }
   }
}

void RouteBrowser::RouteBrowserInterface::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (mProxyRoot.GetProxy() == aProxyPtr)
   {
      return;
   }
   mProxyRoot = aProxyPtr;
   mDockWidget->Clear();
   mDockWidget->GetCreateDialog()->ClearSavedRoutes();
   ToggleGlobalRouteVisibility();

   // add global routes to the route create dialog validator so no duplicates can be made
   WsfPM_RouteMap routeMap = WsfPM_RouteMap(mProxyRoot + "routes");
   if (routeMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      routeMap.GetAllValues(types);
      for (const auto& typeIter : types)
      {
         if (typeIter.IsValid())
         {
            WsfPM_Route curRoute(typeIter);
            mDockWidget->GetCreateDialog()->AddRouteName(QString::fromStdString(curRoute.GetName()));

            // Add this global route to the dock tree widget
            GetDockWidget()->AddGlobal(GetGlobalRouteInfo(curRoute.GetName() + "_anchor"));
            GetDockWidget()->GetCreateDialog()->AddSavedRoute(GetGlobalRouteInfo(curRoute.GetName() + "_anchor"));
         }
      }
   }

   mProxyCallbacks.Add(wizard::WatchProxy(mProxyRoot.routes().GetPath())
                          .AnyUpdate.Connect(&RouteBrowserInterface::OnGlobalRouteListChange, this));
}

void RouteBrowser::RouteBrowserInterface::GlobalRouteCreated(const RouteInfo& aInfo)
{
   if (wkfEnv.GetPreferenceObject<RouteBrowser::PrefObject>()->GetShowGlobalRoutes() && mViewer != nullptr)
   {
      vespa::VaScenario* scenario = vaEnv.GetStandardScenario();
      if (scenario != nullptr)
      {
         std::string      anchorName         = aInfo.mName.toStdString() + "_anchor";
         vespa::VaEntity* tempAnchor         = new vespa::VaEntity(anchorName);
         auto             routeAttachmentPtr = vespa::make_attachment<wkf::AttachmentRoute>(*tempAnchor, mViewer);
         routeAttachmentPtr->ShowIndices(false);
         routeAttachmentPtr->SetName(aInfo.mName.toStdString());
         // Set entity position to the first waypoint position - otherwise route will be drawn starting at 0n 0e
         if (!aInfo.mWaypoints.empty())
         {
            WaypointInfo waypointInfo = aInfo.mWaypoints[0];
            tempAnchor->SetPosition(vespa::VaPosition(waypointInfo.mLat, waypointInfo.mLon, waypointInfo.mAlt));
         }
         scenario->AddEntity(tempAnchor);
         DrawRoute(aInfo, routeAttachmentPtr);
         routeAttachmentPtr->SetTeamColor(UtColor(1, 1, 0, 1));
         routeAttachmentPtr->UpdateTeamColor();
         routeAttachmentPtr->SetSelectionEnabled(true);
         routeAttachmentPtr->SetDraggable(true);
         routeAttachmentPtr->SetContextMenuEnabled(false);

         mDockWidget->GetCreateDialog()->AddSavedRoute(aInfo);
      }
   }
}

void RouteBrowser::RouteBrowserInterface::OnGlobalRouteListChange(const wizard::ProxyChange& aProxyChange)
{
   switch (aProxyChange.reason())
   {
   case wizard::ProxyChange::cADDED:
   case wizard::ProxyChange::cUPDATED:
      UpdateGlobalRoute(aProxyChange.changed());
      break;
   case wizard::ProxyChange::cREMOVED:
      DeleteRoute(QString::fromStdString(WsfPM_Route(aProxyChange.changed()).GetName()));
      break;
   default:
      break;
   }
}

void RouteBrowser::RouteBrowserInterface::DeleteRoute(const QString& aRouteName)
{
   if (mViewer != nullptr)
   {
      vespa::VaEntity* entity = vaEnv.FindEntity(aRouteName.toStdString() + "_anchor");
      if (entity != nullptr)
      {
         wkf::Scenario* scenario = vaEnv.GetStandardScenario();
         if (scenario != nullptr)
         {
            vespa::VaAttachment* attachment = entity->FindAttachment(aRouteName.toStdString());
            if (attachment != nullptr)
            {
               entity->RemoveAttachment(attachment->GetUniqueId());
            }
            scenario->RemoveEntity(entity);
            mDockWidget->RemoveGlobal(aRouteName);
         }
      }
   }
}


void RouteBrowser::RouteBrowserInterface::DockRouteSelectionChangedHandler(const QString& aPrevPlatformName,
                                                                           const QString& aPrevRouteName,
                                                                           int            aPrevWaypoint,
                                                                           const QString& aCurrPlatformName,
                                                                           const QString& aCurrRouteName,
                                                                           int            aCurrWaypoint)
{
   wkf::RouteBrowserInterface::DockRouteSelectionChangedHandler(aPrevPlatformName,
                                                                aPrevRouteName,
                                                                aPrevWaypoint,
                                                                aCurrPlatformName,
                                                                aCurrRouteName,
                                                                aCurrWaypoint);
   mDockWidget->SetEditRouteButtonEnabled(false);
   mDockWidget->SetNewRouteButtonEnabled(false);
   vespa::VaEntity*      currEntity = nullptr;
   wkf::AttachmentRoute* currRoute  = nullptr;
   if (aCurrPlatformName.isEmpty()) // global route
   {
      if (!aCurrRouteName.isEmpty())
      {
         currEntity = vaEnv.FindEntity(aCurrRouteName.toStdString() + "_anchor");
         if (currEntity != nullptr)
         {
            currRoute = dynamic_cast<wkf::AttachmentRoute*>(currEntity->FindAttachment(aCurrRouteName.toStdString()));
            mDockWidget->SetEditRouteButtonEnabled(true);
         }
      }
      mDockWidget->SetNewRouteButtonEnabled(true);
   }
   else
   {
      currEntity = vaEnv.FindEntity(aCurrPlatformName.toStdString());
      if (currEntity != nullptr)
      {
         currRoute = dynamic_cast<wkf::AttachmentRoute*>(currEntity->FindAttachment("route"));

         if (currRoute != nullptr)
         {
            mDockWidget->SetEditRouteButtonEnabled(true);
         }
         else
         {
            mDockWidget->SetNewRouteButtonEnabled(true);
         }

         if (!PlatformHasRouteMover(aCurrPlatformName.toStdString()))
         {
            mDockWidget->SetEditRouteButtonEnabled(false);
            mDockWidget->SetNewRouteButtonEnabled(false);
         }
      }
   }
}

bool RouteBrowser::RouteBrowserInterface::PlatformHasRouteMover(const std::string& aPlatformName)
{
   vespa::VaEntity*  currEntity  = vaEnv.FindEntity(aPlatformName);
   wizard::Platform* platformPtr = dynamic_cast<wizard::Platform*>(currEntity);
   if (platformPtr == nullptr)
   {
      return false;
   }
   WsfPM_Mover aMover = platformPtr->GetPlatform().Mover();
   return (aMover.IsValid() && platformPtr->IsRouteable());
}

void RouteBrowser::RouteBrowserInterface::PlatformDeselectedHandler(wkf::Platform* aPlatform)
{
   wkf::RouteBrowserInterface::PlatformDeselectedHandler(aPlatform);
   if (mDockWidget->GetSelectedRouteName().isEmpty())
   {
      mDockWidget->SetEditRouteButtonEnabled(false);
      mDockWidget->SetNewRouteButtonEnabled(true);
   }
   else
   {
      if (mDockWidget->GlobalRouteSelected())
      {
         mDockWidget->SetEditRouteButtonEnabled(true);
         mDockWidget->SetNewRouteButtonEnabled(true);
      }
   }
}

void RouteBrowser::RouteBrowserInterface::ToggleGlobalRouteVisibility()
{
   if (wizard::Project::Instance() == nullptr)
   {
      return;
   }

   WsfPM_RouteMap         routeMap = WsfPM_RouteMap(mProxyRoot + "routes");
   std::vector<RouteInfo> globalRouteList;
   if (routeMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      routeMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            WsfPM_Route curRoute(*typeit);

            std::string routeName = "";
            if (curRoute.IsOfType("Route"))
            {
               routeName = curRoute.GetName();
            }
            else if (curRoute.GetParent().IsOfType("Route"))
            {
               routeName = curRoute.GetParent().GetName();
               curRoute  = WsfPM_Route(curRoute.GetParent());
            }

            if (!routeName.empty())
            {
               vespa::VaAttachment* foundAttachment = vaEnv.FindAttachment(routeName);
               if (foundAttachment)
               {
                  wkf::AttachmentRoute* attachmentRoutePtr = dynamic_cast<wkf::AttachmentRoute*>(foundAttachment);
                  if (attachmentRoutePtr != nullptr)
                  {
                     if (wkfEnv.GetPreferenceObject<RouteBrowser::PrefObject>()->GetShowGlobalRoutes())
                     {
                        attachmentRoutePtr->SetStateVisibility(true);
                     }
                     else
                     {
                        attachmentRoutePtr->SetStateVisibility(false);
                     }
                  }
               }
            }
         }
      }
   }
}

void RouteBrowser::RouteBrowserInterface::PlatformSelectionChanged(const wkf::PlatformList& aSelected,
                                                                   const wkf::PlatformList& aUnselected)
{
   for (const auto& platform : aSelected)
   {
      if (mDockWidget && platform)
      {
         if (mDockWidget->GetEditDialog()->isVisible() &&
             mDockWidget->GetEditDialog()->GetCurrentPlatform().toStdString() == platform->GetName())
         {
            // this platform's info is already the one loaded up in the edit dialog, no need to redo all that work.
            return;
         }
      }
   }

   wkf::RouteBrowserInterface::PlatformSelectionChanged(aSelected, aUnselected);
}

void RouteBrowser::RouteBrowserInterface::TempRouteEdited(const RouteInfo&      aRouteInfo,
                                                          wkf::AttachmentRoute* aRouteAttachmentPtr)
{
   aRouteAttachmentPtr->DeleteWaypoints();
   DrawRoute(aRouteInfo, aRouteAttachmentPtr);
   aRouteAttachmentPtr->SetDraggable(true);
}
