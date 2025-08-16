// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SatelliteTetherSimInterface.hpp"

#include "UtMemory.hpp"
#include "UtQt.hpp"
#include "WsfOrbitalEvent.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"
#include "WsfSpaceObserver.hpp"

SatelliteTether::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<SatelliteTetherEvent>(aPluginName)
{
}

void SatelliteTether::SimInterface::AddPlatformOfInterest(const std::string& aName)
{
   QMutexLocker lock{&mMutex};
   mPlatformsOfInterest[aName]++;
}

void SatelliteTether::SimInterface::RemovePlatformOfInterest(const std::string& aName)
{
   QMutexLocker lock{&mMutex};
   auto         iter = mPlatformsOfInterest.find(aName);
   if (iter != mPlatformsOfInterest.end())
   {
      iter->second--;
      if (iter->second == 0)
      {
         mPlatformsOfInterest.erase(iter);
      }
   }
}

void SatelliteTether::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   mCallbacks.Clear();

   mCallbacks.Add(
      WsfObserver::OrbitColorChanged(&aSimulation).Connect(&SatelliteTether::SimInterface::OnOrbitColorChanged, this));
   mCallbacks.Add(
      WsfObserver::OrbitalManeuverCompleted(&aSimulation).Connect(&SatelliteTether::SimInterface::OnOrbitUpdated, this));
   mCallbacks.Add(
      WsfObserver::OrbitalManeuverCanceled(&aSimulation).Connect(&SatelliteTether::SimInterface::OnOrbitUpdated, this));

   {
      QMutexLocker lock{&mMutex};
      mPlatformsOfInterest.clear();
   }

   AddSimEvent(ut::make_unique<ReportInitialEpochEvent>(aSimulation.GetDateTime().GetStartDateAndTime()));
}

void SatelliteTether::SimInterface::OnOrbitColorChanged(double /*aSimTime*/, WsfSpaceMoverBase* aMoverPtr)
{
   AddColorUpdateEvent(aMoverPtr);
}

void SatelliteTether::SimInterface::OnOrbitUpdated(double             aSimTime,
                                                   WsfSpaceMoverBase* aSpaceMoverPtr,
                                                   const WsfOrbitalEvent& /*aEvent*/)
{
   if (aSpaceMoverPtr)
   {
      QMutexLocker lock{&mMutex};
      if (mPlatformsOfInterest.find(aSpaceMoverPtr->GetPlatform()->GetName()) != mPlatformsOfInterest.end())
      {
         AddSimEvent(ut::make_unique<ReportPropagatorEvent>(aSpaceMoverPtr->GetPlatform()->GetIndex(),
                                                            ut::clone(aSpaceMoverPtr->GetPropagator()),
                                                            aSimTime));
      }
   }
}

void SatelliteTether::SimInterface::AddColorUpdateEvent(WsfSpaceMoverBase* aMoverPtr)
{
   if (aMoverPtr)
   {
      auto orbitColor(aMoverPtr->GetOrbitColor());
      if (orbitColor.has_value())
      {
         AddSimEvent(ut::make_unique<OrbitColorUpdateEvent>(aMoverPtr->GetPlatform()->GetIndex(),
                                                            UtQtColor(orbitColor.value())));
      }
   }
}
