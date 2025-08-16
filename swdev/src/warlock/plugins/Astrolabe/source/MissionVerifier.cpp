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

#include "MissionVerifier.hpp"

#include <tuple>
#include <utility>

#include "OrbitalModelToSequence.hpp"
#include "WsfDateTime.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSpaceMoverBase.hpp"

namespace WkAstrolabe
{

MissionVerifier::MissionVerifier(const WsfSimulation& aSimulation)
   : mSimulation(aSimulation)
{
}

WsfSpaceMoverBase* MissionVerifier::GetSpaceMover(const std::string& aPlatformName)
{
   WsfSpaceMoverBase* retvalPtr{nullptr};
   const WsfPlatform* simPlatPtr = mSimulation.GetPlatformByName(aPlatformName);
   if (simPlatPtr)
   {
      WsfSpaceMoverBase* simMoverPtr = dynamic_cast<WsfSpaceMoverBase*>(simPlatPtr->GetMover());
      if (simMoverPtr)
      {
         retvalPtr = simMoverPtr;
      }
   }
   return retvalPtr;
}

std::pair<UtCalendar, UtCalendar> MissionVerifier::GetStartEpoch(const std::string& aPlatformName)
{
   UtCalendar   simStartTime{};
   UtCalendar   creationTime{};
   WsfPlatform* platPtr = mSimulation.GetPlatformByName(aPlatformName);
   if (platPtr)
   {
      simStartTime             = mSimulation.GetDateTime().GetStartDateAndTime();
      double simTimeOfCreation = mSimulation.GetSimTime();
      creationTime             = simStartTime;
      creationTime.AdvanceTimeBy(simTimeOfCreation);
   }
   return std::make_pair(simStartTime, creationTime);
}

} // namespace WkAstrolabe
