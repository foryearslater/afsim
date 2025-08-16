//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#include "OrbitalDataSimInterface.hpp"

#include "UtCalendar.hpp"
#include "WsfDateTime.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"
#include "dis/WsfDisMover.hpp"

WkOrbitalData::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<OrbitalDataEvent>(aPluginName)
   , mPlatformOfInterest("")
{
}

void WkOrbitalData::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

UtOrbitalElements WkOrbitalData::SimInterface::GenerateOrbitalElements(WsfPlatform* aPlatformPtr, const UtCalendar& aCal)
{
   ut::OrbitalState orbitalState(ut::EarthEGM96(),
                                 ut::OrbitalState::CoordinateSystem::cEQUATORIAL,
                                 ut::OrbitalState::ReferenceFrame::cECI);
   orbitalState.SetInertial(aCal,
                            ut::OrbitalStateVector(UtVec3d(aPlatformPtr->GetLocationECI()),
                                                   UtVec3d(aPlatformPtr->GetVelocityECI())));
   return orbitalState.GetOrbitalElementsTOD();
}

void WkOrbitalData::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   auto createUpdateEvent = [this](const UtOrbitalElements& aElements, const std::string& aPlatName)
   {
      DataContainer::OrbitalData data;
      data.mName           = aPlatName;
      data.mEccentricity   = aElements.GetEccentricity();
      data.mSemiMajorAxis  = aElements.GetSemiMajorAxis();
      data.mInclination    = aElements.GetInclination();
      data.mRAAN           = aElements.GetRAAN();
      data.mArgOfPeriapsis = aElements.GetArgumentOfPeriapsis();
      data.mTrueAnomaly    = aElements.GetTrueAnomaly();

      AddSimEvent(ut::make_unique<UpdateEvent>(data));
   };

   std::string platformName = "";
   if (mMutex.tryLock())
   {
      platformName = mPlatformOfInterest;
      mMutex.unlock();
   }

   WsfPlatform* platform = aSimulation.GetPlatformByName(platformName);
   if (platform)
   {
      WsfSpaceMoverBase* spaceMover = dynamic_cast<WsfSpaceMoverBase*>(platform->GetMover());
      WsfDisMover*       disMover   = dynamic_cast<WsfDisMover*>(platform->GetMover());

      if (spaceMover)
      {
         UtOrbitalElements orbitalElements = spaceMover->GetOrbitalState().GetOrbitalElementsTOD();
         createUpdateEvent(orbitalElements, platform->GetName());
      }
      else if (disMover && disMover->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
      {
         auto              cal             = aSimulation.GetDateTime().GetCurrentTime(aSimulation.GetSimTime());
         UtOrbitalElements orbitalElements = std::move(GenerateOrbitalElements(platform, cal));
         createUpdateEvent(orbitalElements, platform->GetName());
      }
   }
}

void WkOrbitalData::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<PlatformClearEvent>());
}

void WkOrbitalData::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   WsfSpaceMoverBase* spaceMover = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
   WsfDisMover*       disMover   = dynamic_cast<WsfDisMover*>(aPlatform.GetMover());
   if (spaceMover || (disMover && disMover->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE))
   {
      AddSimEvent(ut::make_unique<PlatformAddedEvent>(aPlatform.GetName()));
   }
}

void WkOrbitalData::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   WsfSpaceMoverBase* spaceMover = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
   WsfDisMover*       disMover   = dynamic_cast<WsfDisMover*>(aPlatform.GetMover());
   if (spaceMover || (disMover && disMover->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE))
   {
      AddSimEvent(ut::make_unique<PlatformRemovedEvent>(aPlatform.GetName()));
   }
}
