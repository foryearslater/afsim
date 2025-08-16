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
#ifndef PLATFORMMOVEMENTCOMMAND_HPP
#define PLATFORMMOVEMENTCOMMAND_HPP

#include "PlatformMovementSimInterface.hpp"
#include "WkSimInterface.hpp"
#include "WsfRoute.hpp"
#include "WsfSimulation.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace WkPlatformMovement
{
class Command : public warlock::SimCommand
{
public:
   Command(const std::string& aPlatformName, bool aUseWallClock = false)
      : warlock::SimCommand(aUseWallClock)
      , mPlatformName(aPlatformName)
   {
   }

protected:
   std::string mPlatformName;
};

class AltitudeCommand : public Command
{
public:
   AltitudeCommand(const std::string& aPlatformName, double aAltitude, double aRateOfChange, bool aKeepRoute)
      : Command(aPlatformName)
      , mAltitude(aAltitude)
      , mRateOfChange(aRateOfChange)
      , mKeepRoute(aKeepRoute)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mAltitude;
   double mRateOfChange;
   bool   mKeepRoute;
};

class HeadingCommand : public Command
{
public:
   HeadingCommand(const std::string& aPlatformName, double aHeading, double aRadialAcceleration, bool aIsAbsolute)
      : Command(aPlatformName)
      , mHeading(aHeading)
      , mRadialAcceleration(aRadialAcceleration)
      , mIsAbsolute(aIsAbsolute)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mHeading;
   double mRadialAcceleration;
   bool   mIsAbsolute;
};

class LocationCommand : public Command
{
public:
   LocationCommand(const std::string& aPlatformName,
                   double             aLatitude,
                   double             aLongitude,
                   bool               aGoToAltitude,
                   double             aAltitude,
                   bool               aInstant)
      : Command(aPlatformName)
      , mLatitude(aLatitude)
      , mLongitude(aLongitude)
      , mGoToAltitude(aGoToAltitude)
      , mAltitude(aAltitude)
      , mInstant(aInstant)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mLatitude;
   double mLongitude;
   bool   mGoToAltitude;
   double mAltitude;
   bool   mInstant;
};

class SpeedCommand : public Command
{
public:
   SpeedCommand(const std::string& aPlatformName, double aSpeed, double aLinearAcceleration, bool aKeepRoute)
      : Command(aPlatformName)
      , mSpeed(aSpeed)
      , mLinearAcceleration(aLinearAcceleration)
      , mKeepRoute(aKeepRoute)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mSpeed;
   double mLinearAcceleration;
   bool   mKeepRoute;
};

class FollowRouteCommand : public Command
{
public:
   FollowRouteCommand(const std::string& aPlatformName, const WsfRoute& aRoute, int aGoToWaypoint)
      : Command(aPlatformName)
      , mRoute(aRoute)
      , mGoToWaypoint(aGoToWaypoint)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   WsfRoute mRoute;
   int      mGoToWaypoint;
};

class DelayRouteCommand : public Command
{
public:
   DelayRouteCommand(const std::string& aPlatformName, double aDelayTime)
      : Command(aPlatformName)
      , mDelayTime(aDelayTime)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mDelayTime;
};

class ReturnToRouteCommand : public Command
{
public:
   ReturnToRouteCommand(const std::string& aPlatformName)
      : Command(aPlatformName)
   {
   }

   void Process(WsfSimulation& aSimulation) override;
};

class AssignNewRouteCommand : public Command
{
public:
   AssignNewRouteCommand(const std::string& aPlatformName, const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
      : Command(aPlatformName, true)
      , mRouteInfo(aRouteInfo)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   wkf::RouteBrowserInterface::RouteInfo mRouteInfo;
};

class ModifyRouteCommand : public Command
{
public:
   ModifyRouteCommand(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
      : Command(aRouteInfo.mPlatformName.toStdString(), true)
      , mRouteInfo(aRouteInfo)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   wkf::RouteBrowserInterface::RouteInfo mRouteInfo;
};

class LocalRouteRequestCommand : public SimInterface::Command
{
public:
   enum ReturnEventType
   {
      ROUTE,        // RouteEvent
      ROUTE_SELECT, // RouteSelectEvent
      ROUTE_DIALOG, // RouteDialogEvent
   };

   LocalRouteRequestCommand(SimInterface* aSimInterface, const std::string& aPlatformName, ReturnEventType aEventType)
      : Command(aSimInterface, true)
      , mPlatformName(aPlatformName)
      , mResponseEventType(aEventType)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string     mPlatformName;
   ReturnEventType mResponseEventType;
};
} // namespace WkPlatformMovement
#endif // PLATFORMMOVEMENTCOMMAND_HPP
