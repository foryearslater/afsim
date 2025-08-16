// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "MissionVerifier.hpp"

#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "InputReader.hpp"
#include "OrbitalModelToSequence.hpp"
#include "WsfOrbitalManeuvering.hpp"
#include "WsfSpaceMoverBase.hpp"

SpaceTools::MissionVerifier::MissionVerifier()
   : mScenario{}
   , mMoverPtr{nullptr}
{
}

WsfSpaceMoverBase* SpaceTools::MissionVerifier::GetSpaceMover(const std::string& aPlatformName)
{
   InputReader reader{GetScenario()};
   mMoverPtr = reader.GetPlatformSpaceMover(aPlatformName);
   UtCalendar start;
   UtCalendar creation;
   std::tie(start, creation) = GetStartEpoch(aPlatformName);
   // We initialize the maneuvering here to deal with an edge case. If the space mover has a rocket
   // maneuvering model, we need to initialize it so that we can read out the mass.
   mMoverPtr->InitializeOrbitalManeuvering(creation);

   if (!mMoverPtr->GetPropagator().InitializeOrbitalState(creation) ||
       !mMoverPtr->GetPropagator().GetInitialOrbitalState().IsValid())
   {
      std::stringstream msg{};
      msg << "Unable to initialize orbital state for platform '"
          << aPlatformName << "'. Astrolabe can only verify sequences for initial orbital states that can be initialized outside of a simulation.";
      throw std::runtime_error{msg.str()};
   }

   return mMoverPtr.get();
}

std::pair<UtCalendar, UtCalendar> SpaceTools::MissionVerifier::GetStartEpoch(const std::string& aPlatformName)
{
   InputReader reader{GetScenario()};
   UtCalendar  simStartTime = reader.GetStartDateAndTime();
   UtCalendar  creationTime = reader.GetPlatformCreationTime(aPlatformName, simStartTime);
   return std::make_pair(simStartTime, creationTime);
}
