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

#include "SatelliteTetherSimCommands.hpp"

#include "SatelliteTetherSimEvents.hpp"
#include "UtQt.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"

void SatelliteTether::AddTrackCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr(aSimulation.GetPlatformByName(mPlatformName));
   if (platformPtr && platformPtr->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
   {
      auto moverPtr(dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover()));
      if (moverPtr && moverPtr->GetOrbitColor().has_value())
      {
         mColor = UtQtColor(moverPtr->GetOrbitColor().value());
      }
      AddSimEvent(
         ut::make_unique<AddTrackEvent>(platformPtr->GetIndex(), mTetherId, mColor, ut::clone(moverPtr->GetPropagator())));
   }
}

void SatelliteTether::AddOwnshipCommand::Process(WsfSimulation& aSimulation)
{
   auto platformPtr = aSimulation.GetPlatformByName(mPlatformName);
   if (platformPtr && platformPtr->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
   {
      auto moverPtr = dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover());
      if (moverPtr)
      {
         AddSimEvent(ut::make_unique<ReportPropagatorEvent>(platformPtr->GetIndex(),
                                                            ut::clone(moverPtr->GetPropagator()),
                                                            aSimulation.GetSimTime()));
      }
   }
}
