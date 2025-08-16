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

#include "OrbitSimInterface.hpp"

#include "UtMemory.hpp"
#include "WsfDateTime.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"
#include "WsfSpaceObserver.hpp"

WkOrbit::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<OrbitEvent>(aPluginName)
{
}

void WkOrbit::SimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
{
   if (aPlatform.GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
      if (moverPtr && moverPtr->GetElementOutputUpdateInterval() > 0.0)
      {
         auto eventPtr = ut::make_unique<OrbitUpdateEvent>(*this, aPlatform.GetIndex());
         eventPtr->SetTime(aSimTime + moverPtr->GetElementOutputUpdateInterval());
         moverPtr->GetSimulation()->AddEvent(std::move(eventPtr));
      }
   }
}

void WkOrbit::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (aPlatform.GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
      if (moverPtr)
      {
         AddSimEvent(ut::make_unique<RemoveOrbitEvent>(aPlatform.GetIndex()));
      }
   }
}

void WkOrbit::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker{&mMutex};
   mCallbacks.Clear();

   mCallbacks.Add(WsfObserver::OrbitColorChanged(&aSimulation).Connect(&WkOrbit::SimInterface::OnSpaceMoverUpdate, this));
   mCallbacks.Add(
      WsfObserver::OrbitalManeuverCanceled(&aSimulation).Connect(&WkOrbit::SimInterface::OnOrbitalManeuverCanceled, this));
   mCallbacks.Add(
      WsfObserver::OrbitalManeuverCompleted(&aSimulation).Connect(&WkOrbit::SimInterface::OnOrbitalManeuverCompleted, this));

   mLastOrbitRedrawTime = 0.0;
   mLastMoonRedrawTime  = 0.0;
}

void WkOrbit::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimStartingEvent>(aSimulation.GetDateTime().GetStartDateAndTime()));
}

void WkOrbit::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   QMutexLocker locker{&mMutex};
   mCallbacks.Clear();
}

void WkOrbit::SimInterface::OnSpaceMoverUpdate(double aSimTime, WsfSpaceMoverBase* aMoverPtr)
{
   AddSimEvent(ut::make_unique<OrbitalElementsUpdateEvent>(aMoverPtr->GetPlatform()->GetIndex(),
                                                           aSimTime,
                                                           aMoverPtr->GetOrbitalState().GetOrbitalElementsTOD(),
                                                           false,
                                                           aMoverPtr->GetOrbitColor()));
}

WsfEvent::EventDisposition WkOrbit::SimInterface::OrbitUpdateEvent::Execute()
{
   WsfEvent::EventDisposition retval{WsfEvent::EventDisposition::cDELETE};

   if (GetSimulation()->PlatformExists(mPlatformIndex))
   {
      WsfPlatform* platformPtr = GetSimulation()->GetPlatformByIndex(mPlatformIndex);
      if (platformPtr->GetSpatialDomain() == WsfSpatialDomain::WSF_SPATIAL_DOMAIN_SPACE)
      {
         double eventTime = GetTime();
         platformPtr->Update(eventTime);

         WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover());
         if (moverPtr)
         {
            mInterface.OnSpaceMoverUpdate(eventTime, moverPtr);
            SetTime(eventTime + moverPtr->GetElementOutputUpdateInterval());
            retval = WsfEvent::EventDisposition::cRESCHEDULE;
         }
      }
   }

   return retval;
}

void WkOrbit::SimInterface::OnOrbitalManeuverCanceled(double                 aSimTime,
                                                      WsfSpaceMoverBase*     aMoverPtr,
                                                      const WsfOrbitalEvent& aOrbitalEvent)
{
   AddSimEvent(ut::make_unique<OrbitalElementsUpdateEvent>(aMoverPtr->GetPlatform()->GetIndex(),
                                                           aSimTime,
                                                           aMoverPtr->GetOrbitalState().GetOrbitalElementsTOD(),
                                                           true,
                                                           aMoverPtr->GetOrbitColor()));
}

void WkOrbit::SimInterface::OnOrbitalManeuverCompleted(double                 aSimTime,
                                                       WsfSpaceMoverBase*     aMoverPtr,
                                                       const WsfOrbitalEvent& aOrbitalEvent)
{
   AddSimEvent(ut::make_unique<OrbitalElementsUpdateEvent>(aMoverPtr->GetPlatform()->GetIndex(),
                                                           aSimTime,
                                                           aMoverPtr->GetOrbitalState().GetOrbitalElementsTOD(),
                                                           true,
                                                           aMoverPtr->GetOrbitColor()));
}

void WkOrbit::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   constexpr double cORBIT_UPDATE_INTERVAL{1.0};
   constexpr double cMOON_UPDATE_INTERVAL{500.0};

   if (aSimulation.GetSimTime() - mLastOrbitRedrawTime > cORBIT_UPDATE_INTERVAL)
   {
      mLastOrbitRedrawTime = aSimulation.GetSimTime();
      AddSimEvent(ut::make_unique<UpdateOrbitAngles>(mLastOrbitRedrawTime));
   }
   if (aSimulation.GetSimTime() - mLastMoonRedrawTime > cMOON_UPDATE_INTERVAL)
   {
      mLastMoonRedrawTime = aSimulation.GetSimTime();
      AddSimEvent(ut::make_unique<MoonOrbitUpdateEvent>(mLastMoonRedrawTime));
   }
}
