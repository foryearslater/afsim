// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SatelliteTetherPropagationManager.hpp"

namespace SatelliteTether
{

bool PropagationManager::IsOfInterest(const std::string& aPlatformName) const
{
   return mPlatformsOfInterest.find(aPlatformName) != mPlatformsOfInterest.end();
}

UtOrbitalPropagatorBase* PropagationManager::GetPropagator(const std::string& aPlatformName) const
{
   UtOrbitalPropagatorBase* retvalPtr{nullptr};
   auto                     iter = mPropagators.find(aPlatformName);
   if (iter != mPropagators.end())
   {
      retvalPtr = iter->second.get();
   }
   return retvalPtr;
}

void PropagationManager::AddPlatformOfInterest(const std::string& aPlatformName)
{
   mPlatformsOfInterest[aPlatformName]++;
}

void PropagationManager::RemovePlatformOfInterest(const std::string& aPlatformName)
{
   auto iter = mPlatformsOfInterest.find(aPlatformName);
   if (iter != mPlatformsOfInterest.end())
   {
      iter->second--;
      if (iter->second == 0)
      {
         mPropagators.erase(aPlatformName);
         mPlatformsOfInterest.erase(iter);
      }
   }
}

void PropagationManager::SetPropagator(const std::string&                       aPlatformName,
                                       std::unique_ptr<UtOrbitalPropagatorBase> aPropagatorPtr)
{
   mPropagators[aPlatformName] = std::move(aPropagatorPtr);
}

void PropagationManager::SetSimulationStartEpoch(const UtCalendar& aEpoch)
{
   mSimulationStartEpoch = aEpoch;
}

void PropagationManager::Clear()
{
   mPlatformsOfInterest.clear();
   mPropagators.clear();
}

} // namespace SatelliteTether
