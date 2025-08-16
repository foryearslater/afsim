// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformMovementUtils.hpp"

wkf::RouteBrowserInterface::RouteInfo WkPlatformMovement::GetRouteInfo(const WsfRoute* aRoute)
{
   wkf::RouteBrowserInterface::RouteInfo routeInfo;
   if (aRoute != nullptr)
   {
      int endOfPath = 0;
      for (auto& wypt : aRoute->GetWaypoints())
      {
         wkf::RouteBrowserInterface::WaypointInfo waypointInfo;

         waypointInfo.mLabel = QString::fromStdString(wypt.GetLabelId());

         if (wypt.GetPointType() & WsfWaypoint::cLATITUDE_AND_LONGITUDE || wypt.GetPointType() & WsfWaypoint::cHEADING_MASK)
         {
            waypointInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE;

            waypointInfo.mLat = UtLatPos(wypt.GetLat());
            waypointInfo.mLon = UtLonPos(wypt.GetLon());

            if (wypt.GetAlt() == WsfPath::cDOUBLE_NOT_SET)
            {
               if (wypt.GetPositionInRoute() == 0)
               {
                  waypointInfo.mAlt = wkf::WaypointValueType::cNOT_SET;
               }
               else
               {
                  waypointInfo.mAlt = wkf::WaypointValueType::cUSE_PREVIOUS;
               }
            }
            else
            {
               waypointInfo.mAlt = wypt.GetAlt();
            }
         }
         else if (wypt.GetPointType() & WsfWaypoint::cRELATIVE_OFFSET)
         {
            waypointInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cRELATIVE;

            waypointInfo.mXOffset = wypt.GetLat();
            waypointInfo.mYOffset = wypt.GetLat();
            waypointInfo.mAlt     = wkf::WaypointValueType::cUSE_PREVIOUS;
         }
         else
         {
            waypointInfo.mPointType = wkf::RouteBrowserInterface::WaypointInfo::cOTHER;
         }

         // agl/msl
         waypointInfo.mAltRef = wypt.GetAltRef();

         // goTo
         waypointInfo.mGoTo = QString::fromStdString(wypt.GetGotoId());

         // heading
         if (wypt.GetHeading() == WsfPath::cDOUBLE_NOT_SET)
         {
            waypointInfo.mHeading = wkf::WaypointValueType::cNOT_SET;
         }
         else
         {
            waypointInfo.mHeading = wypt.GetHeading();
         }

         // speed
         if (wypt.GetSpeed() == WsfPath::cDOUBLE_NOT_SET)
         {
            if (wypt.GetPositionInRoute() == 0)
            {
               waypointInfo.mSpeed = wkf::WaypointValueType::cNOT_SET;
            }
            else
            {
               waypointInfo.mSpeed = wkf::WaypointValueType::cUSE_PREVIOUS;
            }
         }
         else
         {
            waypointInfo.mSpeed = wypt.GetSpeed();
         }

         // climb rate
         if (wypt.GetClimbRate() == WsfPath::cUSE_DEFAULT)
         {
            waypointInfo.mClimbRate = wkf::WaypointValueType::cUSE_DEFAULT;
         }
         else if (wypt.GetClimbRate() == WsfPath::cDOUBLE_NOT_SET)
         {
            if (wypt.GetPositionInRoute() == 0)
            {
               waypointInfo.mClimbRate = wkf::WaypointValueType::cNOT_SET;
            }
            else
            {
               waypointInfo.mClimbRate = wkf::WaypointValueType::cUSE_PREVIOUS;
            }
         }
         else
         {
            waypointInfo.mClimbRate = wypt.GetClimbRate();
         }

         // linear acceleration
         if (wypt.GetLinearAccel() == WsfPath::cUSE_DEFAULT)
         {
            waypointInfo.mLinAccel = wkf::WaypointValueType::cUSE_DEFAULT;
         }
         else if (wypt.GetLinearAccel() == WsfPath::cDOUBLE_NOT_SET)
         {
            if (wypt.GetPositionInRoute() == 0)
            {
               waypointInfo.mLinAccel = wkf::WaypointValueType::cNOT_SET;
            }
            else
            {
               waypointInfo.mLinAccel = wkf::WaypointValueType::cUSE_PREVIOUS;
            }
         }
         else
         {
            waypointInfo.mLinAccel = wypt.GetLinearAccel();
         }

         // radial acceleration
         if (wypt.GetRadialAccel() == WsfPath::cUSE_DEFAULT)
         {
            waypointInfo.mRadAccel = wkf::WaypointValueType::cUSE_DEFAULT;
         }
         else if (wypt.GetRadialAccel() == WsfPath::cDOUBLE_NOT_SET)
         {
            if (wypt.GetPositionInRoute() == 0)
            {
               waypointInfo.mRadAccel = wkf::WaypointValueType::cNOT_SET;
            }
            else
            {
               waypointInfo.mRadAccel = wkf::WaypointValueType::cUSE_PREVIOUS;
            }
         }
         else
         {
            waypointInfo.mRadAccel = wypt.GetRadialAccel();
         }

         // turn direction
         waypointInfo.mTurnDir = wypt.GetTurnDirection();

         // switch
         waypointInfo.mSwitchType = 0; // mover default
         if (wypt.GetSwitch() == WsfPath::Switch::cSWITCH_ON_PASSING)
         {
            waypointInfo.mSwitchType = 1;
         }
         else if (wypt.GetSwitch() == WsfPath::Switch::cSWITCH_ON_APPROACH)
         {
            waypointInfo.mSwitchType = 2;
         }

         // end of path
         endOfPath = 0; // mover default
         if (wypt.GetEndOfPathOption() == WsfPath::EndPath::cEP_EXTRAPOLATE)
         {
            endOfPath = 1;
         }
         else if (wypt.GetEndOfPathOption() == WsfPath::EndPath::cEP_STOP)
         {
            endOfPath = 2;
         }
         else if (wypt.GetEndOfPathOption() == WsfPath::EndPath::cEP_REMOVE)
         {
            endOfPath = 3;
         }
         routeInfo.mWaypoints.push_back(waypointInfo);
      }

      routeInfo.mEndPathType = endOfPath;
      routeInfo.mName        = QString::fromStdString(aRoute->GetName());
   }
   return routeInfo;
}

const WsfRoute* WkPlatformMovement::FindLocalRoute(const WsfSimulation& aSimulation, const std::string& aPlatformName)
{
   WsfPlatform*                          platform = aSimulation.GetPlatformByName(aPlatformName);
   wkf::RouteBrowserInterface::RouteInfo routeInfo;
   if (platform != nullptr)
   {
      if (platform->GetMover() != nullptr)
      {
         return platform->GetMover()->GetRoute();
      }
   }
   return nullptr;
}

wkf::RouteBrowserInterface::RouteInfo WkPlatformMovement::FindLocalRouteInfo(const WsfSimulation& aSimulation,
                                                                             const std::string&   aPlatformName)
{
   wkf::RouteBrowserInterface::RouteInfo routeInfo = GetRouteInfo(FindLocalRoute(aSimulation, aPlatformName));
   routeInfo.mPlatformName                         = QString::fromStdString(aPlatformName);
   return routeInfo;
}

void WkPlatformMovement::UpdateWaypoint(WsfWaypoint&                                    aWaypoint,
                                        const wkf::RouteBrowserInterface::WaypointInfo& aWaypointInfo,
                                        WsfPath::EndPath                                aEndOfPathType)
{
   // label
   aWaypoint.SetLabelId(aWaypointInfo.mLabel.toStdString());

   // latitude
   aWaypoint.SetLat(aWaypointInfo.mLat);

   // longitude
   aWaypoint.SetLon(aWaypointInfo.mLon);

   // altitude
   if (aWaypointInfo.mAlt == wkf::WaypointValueType::cUSE_PREVIOUS)
   {
      aWaypoint.SetAlt(WsfPath::cUSE_PREVIOUS);
   }
   else if (aWaypointInfo.mAlt == wkf::WaypointValueType::cNOT_SET)
   {
      aWaypoint.SetAlt(WsfPath::cDOUBLE_NOT_SET);
   }
   else if (aWaypointInfo.mAlt == wkf::WaypointValueType::cUSE_DEFAULT)
   {
      aWaypoint.SetAlt(WsfPath::cUSE_DEFAULT);
   }
   else
   {
      aWaypoint.SetAlt(aWaypointInfo.mAlt);
   }

   // agl
   aWaypoint.SetAltRef(static_cast<WsfPath::AltRef>(aWaypointInfo.mAltRef));

   // mGoTo
   aWaypoint.SetGotoId(aWaypointInfo.mGoTo.toStdString());

   // Speed
   if (aWaypointInfo.mSpeed == wkf::WaypointValueType::cUSE_PREVIOUS)
   {
      aWaypoint.SetSpeed(WsfPath::cUSE_PREVIOUS);
   }
   else if (aWaypointInfo.mSpeed == wkf::WaypointValueType::cNOT_SET)
   {
      aWaypoint.SetSpeed(WsfPath::cDOUBLE_NOT_SET);
   }
   else if (aWaypointInfo.mSpeed == wkf::WaypointValueType::cUSE_DEFAULT)
   {
      aWaypoint.SetSpeed(WsfPath::cUSE_DEFAULT);
   }
   else
   {
      aWaypoint.SetSpeed(aWaypointInfo.mSpeed);
   }

   // Climb Rate
   if (aWaypointInfo.mClimbRate == wkf::WaypointValueType::cUSE_PREVIOUS)
   {
      aWaypoint.SetClimbRate(WsfPath::cUSE_PREVIOUS);
   }
   else if (aWaypointInfo.mClimbRate == wkf::WaypointValueType::cNOT_SET)
   {
      aWaypoint.SetClimbRate(WsfPath::cDOUBLE_NOT_SET);
   }
   else if (aWaypointInfo.mClimbRate == wkf::WaypointValueType::cUSE_DEFAULT)
   {
      aWaypoint.SetClimbRate(WsfPath::cUSE_DEFAULT);
   }
   else
   {
      aWaypoint.SetClimbRate(aWaypointInfo.mClimbRate);
   }

   // Linear Acceleration
   if (aWaypointInfo.mLinAccel == wkf::WaypointValueType::cUSE_PREVIOUS)
   {
      aWaypoint.SetLinearAccel(WsfPath::cUSE_PREVIOUS);
   }
   else if (aWaypointInfo.mLinAccel == wkf::WaypointValueType::cNOT_SET)
   {
      aWaypoint.SetLinearAccel(WsfPath::cDOUBLE_NOT_SET);
   }
   else if (aWaypointInfo.mLinAccel == wkf::WaypointValueType::cUSE_DEFAULT)
   {
      aWaypoint.SetLinearAccel(WsfPath::cUSE_DEFAULT);
   }
   else
   {
      aWaypoint.SetLinearAccel(aWaypointInfo.mLinAccel);
   }

   // Radial Acceleration
   if (aWaypointInfo.mRadAccel == wkf::WaypointValueType::cUSE_PREVIOUS)
   {
      aWaypoint.SetRadialAccel(WsfPath::cUSE_PREVIOUS);
   }
   else if (aWaypointInfo.mRadAccel == wkf::WaypointValueType::cNOT_SET)
   {
      aWaypoint.SetRadialAccel(WsfPath::cDOUBLE_NOT_SET);
   }
   else if (aWaypointInfo.mRadAccel == wkf::WaypointValueType::cUSE_DEFAULT)
   {
      aWaypoint.SetRadialAccel(WsfPath::cUSE_DEFAULT);
   }
   else
   {
      aWaypoint.SetRadialAccel(aWaypointInfo.mRadAccel);
   }

   // Turn Dir
   aWaypoint.SetTurnDirection(static_cast<WsfPath::TurnDirection>(aWaypointInfo.mTurnDir));

   // Switch Type
   if (aWaypointInfo.mSwitchType == 1)
   {
      aWaypoint.SetSwitch(WsfPath::Switch::cSWITCH_ON_PASSING);
   }
   else if (aWaypointInfo.mSwitchType == 2)
   {
      aWaypoint.SetSwitch(WsfPath::Switch::cSWITCH_ON_APPROACH);
   }
   else
   {
      aWaypoint.SetSwitch(WsfPath::Switch::cSWITCH_MOVER_DEFAULT);
   }

   // End Path Option
   aWaypoint.SetEndOfPathOption(aEndOfPathType);
}

void WkPlatformMovement::UpdateRoute(WsfRoute& aRoute, const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   aRoute.Clear();
   for (auto& w : aRouteInfo.mWaypoints)
   {
      WsfWaypoint waypoint;
      UpdateWaypoint(waypoint, w, static_cast<WsfPath::EndPath>(aRouteInfo.mEndPathType));
      aRoute.Append(waypoint);
   }
}
