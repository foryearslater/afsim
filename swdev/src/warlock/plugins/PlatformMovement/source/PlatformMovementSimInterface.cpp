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

#include "PlatformMovementSimInterface.hpp"

#include "PlatformMovementSimCommands.hpp"
#include "PlatformMovementUtils.hpp"
#include "UtMemory.hpp"
#include "WsfMover.hpp"
#include "WsfMoverObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfRoute.hpp"
#include "WsfRouteMover.hpp"
#include "WsfRouteTypes.hpp"
#include "WsfSimulation.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

WkPlatformMovement::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<PlatformMovementEvent>(aPluginName)
{
}

WkPlatformMovement::SimInterface::~SimInterface()
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
}

void WkPlatformMovement::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();

   mCallbacks.Add(WsfObserver::MoverChanged(&aSimulation)
                     .Connect([this](WsfPlatform* aPlatformPtr) { RouteChangedHandler(aPlatformPtr->GetMover()); }));
   mCallbacks.Add(
      WsfObserver::RouteChanged(&aSimulation).Connect(&WkPlatformMovement::SimInterface::RouteChangedHandler, this));
}

void WkPlatformMovement::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   std::vector<WsfRoute*> allRoutes;
   WsfRouteTypes::Get(aSimulation.GetScenario()).GetCurrentTypes(allRoutes);

   std::vector<WsfRoute*> globalRoutes;
   for (auto& route : allRoutes)
   {
      if (route->IsGlobalRoute())
      {
         globalRoutes.emplace_back(route);
      }
   }
   AddSimEvent(ut::make_unique<GlobalRouteListEvent>(globalRoutes));
}

void WkPlatformMovement::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
}

void WkPlatformMovement::SimInterface::RouteChangedHandler(WsfMover* aMover)
{
   if (aMover != nullptr)
   {
      const WsfRoute* route = aMover->GetRoute();
      if (route != nullptr)
      {
         AddSimEvent(
            ut::make_unique<RouteEvent>(aMover->GetPlatform()->GetIndex(), aMover->GetPlatform()->GetName(), route));
      }
   }
}
