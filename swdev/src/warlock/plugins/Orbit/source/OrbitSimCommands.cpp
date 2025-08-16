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

#include "OrbitSimCommands.hpp"

#include "OrbitSimEvents.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"

void WkOrbit::RequestOrbitUpdateCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platformPtr = aSimulation.GetPlatformByName(mPlatformName);
   if (platformPtr)
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover());
      if (moverPtr)
      {
         AddSimEvent(ut::make_unique<OrbitalElementsUpdateEvent>(platformPtr->GetIndex(),
                                                                 aSimulation.GetSimTime(),
                                                                 moverPtr->GetOrbitalState().GetOrbitalElementsTOD(),
                                                                 false,
                                                                 moverPtr->GetOrbitColor()));
      }
   }
}
