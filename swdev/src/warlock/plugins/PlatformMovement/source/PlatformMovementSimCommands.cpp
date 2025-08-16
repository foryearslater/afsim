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
#include "PlatformMovementSimCommands.hpp"

#include "PlatformMovementSimEvents.hpp"
#include "PlatformMovementSimInterface.hpp"
#include "PlatformMovementUtils.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtMemory.hpp"
#include "WkObserver.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfEvent.hpp"
#include "WsfMover.hpp"
#include "WsfPlatform.hpp"
#include "WsfRoute.hpp"
#include "WsfRouteMover.hpp"
#include "xio/WsfXIO_ObjectInfo.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"


namespace
{
void ReturnToRouteHelper(WsfSimulation& aSimulation, const std::string& aPlatformName)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(aPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex = info->GetIndex();
            pkt.mType          = WsfXIO_ReRoutePlatformPkt::cRETURN_TO_ROUTE;
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            mover->ReturnToRoute(aSimulation.GetSimTime());
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Return to Route", platform);
   }
}

void GoToHeadingHelper(WsfSimulation&     aSimulation,
                       const std::string& aPlatformName,
                       double             aHeading,
                       double             aRadialAcceleration,
                       double             aIsAbsolute)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(aPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex   = info->GetIndex();
            pkt.mType            = WsfXIO_ReRoutePlatformPkt::cGO_TO_HEADING;
            pkt.mHeading         = aHeading;
            pkt.mRateOfChange    = aRadialAcceleration;
            pkt.mRelativeHeading = !aIsAbsolute;
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            if (aIsAbsolute)
            {
               mover->TurnToHeading(aSimulation.GetSimTime(), aHeading, aRadialAcceleration, WsfPath::cTURN_DIR_SHORTEST);
            }
            else
            {
               mover->TurnToRelativeHeading(aSimulation.GetSimTime(), aHeading, aRadialAcceleration, WsfPath::cTURN_DIR_SHORTEST);
            }
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Change Heading", platform);
   }
}

// WsfEvents for Queued Operations
class ReturnToRouteWsfEvent : public WsfEvent
{
public:
   ReturnToRouteWsfEvent(const std::string& aPlatformName, double aSimTime)
      : WsfEvent(aSimTime)
      , mPlatformName(aPlatformName)
   {
   }

   EventDisposition Execute() override
   {
      if (GetSimulation())
      {
         ReturnToRouteHelper(*GetSimulation(), mPlatformName);
      }
      return WsfEvent::cDELETE;
   }

private:
   std::string mPlatformName;
};
} // namespace


void WkPlatformMovement::AltitudeCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex = info->GetIndex();
            pkt.mType          = WsfXIO_ReRoutePlatformPkt::cGO_TO_ALTITUDE;
            pkt.mAltitude      = mAltitude;
            pkt.mRateOfChange  = mRateOfChange;
            pkt.mKeepRoute     = mKeepRoute;
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            mover->GoToAltitude(aSimulation.GetSimTime(), mAltitude, mRateOfChange, mKeepRoute);
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Change Altitude", platform);
   }
}

void WkPlatformMovement::HeadingCommand::Process(WsfSimulation& aSimulation)
{
   GoToHeadingHelper(aSimulation, mPlatformName, mHeading, mRadialAcceleration, mIsAbsolute);
}

void WkPlatformMovement::LocationCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            double altitude = (mGoToAltitude ? mAltitude : platform->GetAltitude());

            double locationWCS[3];
            UtEllipsoidalEarth::ConvertLLAToECEF(mLatitude, mLongitude, altitude, locationWCS);

            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex  = info->GetIndex();
            pkt.mLocationWCS[0] = locationWCS[0];
            pkt.mLocationWCS[1] = locationWCS[1];
            pkt.mLocationWCS[2] = locationWCS[2];
            if (mInstant)
            {
               pkt.mType = WsfXIO_ReRoutePlatformPkt::cSET_LOCATION;
            }
            else
            {
               pkt.mType = WsfXIO_ReRoutePlatformPkt::cGO_TO_LOCATION;
            }
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         double    altitude = (mGoToAltitude ? mAltitude : platform->GetAltitude());
         WsfMover* moverPtr = platform->GetMover();
         if (mInstant)
         {
            platform->SetLocationLLA(mLatitude, mLongitude, altitude);

            // If the platform has a mover, it may need to be told that the position has changed.
            // Note: This does not support all movers
            // Currently, this supports WsfRouteMover and WsfP6DOF_Mover
            if (moverPtr)
            {
               moverPtr->ResetPosition(aSimulation.GetSimTime());
            }
         }
         else if (moverPtr)
         {
            moverPtr->GoToLocation(aSimulation.GetSimTime(), mLatitude, mLongitude, altitude);
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Change Location", platform);
   }
}

void WkPlatformMovement::SpeedCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex = info->GetIndex();
            pkt.mType          = WsfXIO_ReRoutePlatformPkt::cGO_TO_SPEED;
            pkt.mSpeed         = mSpeed;
            pkt.mRateOfChange  = mLinearAcceleration;
            pkt.mKeepRoute     = mKeepRoute;
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            mover->GoToSpeed(aSimulation.GetSimTime(), mSpeed, mLinearAcceleration, mKeepRoute);
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Change Speed", platform);
   }
}

void WkPlatformMovement::FollowRouteCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex = info->GetIndex();
            pkt.mType          = WsfXIO_ReRoutePlatformPkt::cFOLLOW_ROUTE;
            pkt.mRoutePtr      = &mRoute;
            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            mover->UpdateRoute(aSimulation.GetSimTime(), mRoute);
            mover->GoToWaypoint(aSimulation.GetSimTime(), mGoToWaypoint);
         }
      }
      WkObserver::SimulationUserAction(aSimulation)("Follow Route", platform);
   }
}

void WkPlatformMovement::DelayRouteCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   GoToHeadingHelper(aSimulation, mPlatformName, UtUnitAngle::ConvertToStandard(180, UtUnitAngle::cDEGREES), 0, false);
   double queuedTime = aSimulation.GetSimTime() + (mDelayTime / 2);
   aSimulation.AddEvent(ut::make_unique<ReturnToRouteWsfEvent>(mPlatformName, queuedTime));
   WkObserver::SimulationUserAction(aSimulation)("Delay Route", platform);
}

void WkPlatformMovement::ReturnToRouteCommand::Process(WsfSimulation& aSimulation)
{
   ReturnToRouteHelper(aSimulation, mPlatformName);
}

void WkPlatformMovement::AssignNewRouteCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      if (!platform->IsExternallyControlled())
      {
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            WsfRoute route;
            for (auto& w : mRouteInfo.mWaypoints)
            {
               WsfWaypoint waypoint;
               UpdateWaypoint(waypoint, w, static_cast<WsfPath::EndPath>(mRouteInfo.mEndPathType));
               route.Append(waypoint);
            }
            mover->UpdateRoute(aSimulation.GetSimTime(), route);
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(
               QMessageBox::Warning,
               "Route Error",
               QString("Cannot assign route to %1 without a route mover").arg(QString::fromStdString(mPlatformName)));
         }
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(
            QMessageBox::Warning,
            "Route Error",
            QString("Cannot assign route to the externally controlled platform %1").arg(QString::fromStdString(mPlatformName)));
      }
      WkObserver::SimulationUserAction(aSimulation)("New Route Assigned", platform);
   }
}

void WkPlatformMovement::ModifyRouteCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform != nullptr)
   {
      WsfMover* mover = platform->GetMover();
      if (mover != nullptr && mover->GetRoute() != nullptr)
      {
         WsfRoute route(*mover->GetRoute());

         UpdateRoute(route, mRouteInfo);

         int            targetWaypoint = 0;
         WsfRouteMover* routeMover     = dynamic_cast<WsfRouteMover*>(mover);
         if (routeMover != nullptr)
         {
            targetWaypoint = routeMover->GetTargetIndex();
         }

         mover->UpdateRoute(aSimulation.GetSimTime(), route);

         // GoToWaypoint is required because UpdateRoute causes mover to go to first waypoint
         mover->GoToWaypoint(aSimulation.GetSimTime(), targetWaypoint);
      }
      WkObserver::SimulationUserAction(aSimulation)("Route Modified", platform);
   }
}

void WkPlatformMovement::LocalRouteRequestCommand::Process(WsfSimulation& aSimulation)
{
   if (mResponseEventType == ROUTE)
   {
      const WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
      if (platform)
      {
         const WsfMover* mover = platform->GetMover();
         if (mover)
         {
            const WsfRoute* route = mover->GetRoute();
            if (route != nullptr)
            {
               AddSimEvent(ut::make_unique<RouteEvent>(platform->GetIndex(), mPlatformName, route));
            }
         }
      }
   }
   else if (!mPlatformName.empty())
   {
      wkf::RouteBrowserInterface::RouteInfo info = FindLocalRouteInfo(aSimulation, mPlatformName);
      if (mResponseEventType == ROUTE_SELECT)
      {
         AddSimEvent(ut::make_unique<RouteSelectEvent>(info));
      }
      else if (mResponseEventType == ROUTE_DIALOG)
      {
         AddSimEvent(ut::make_unique<RouteDialogEvent>(info));
      }
   }
}
