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

#ifndef SATELLITETETHERPROPAGATIONMANAGER_HPP
#define SATELLITETETHERPROPAGATIONMANAGER_HPP

#include <map>
#include <string>

#include "UtCalendar.hpp"
#include "UtOrbitalPropagatorBase.hpp"

namespace SatelliteTether
{

class PropagationManager
{
public:
   bool                     IsOfInterest(const std::string& aPlatformName) const;
   UtOrbitalPropagatorBase* GetPropagator(const std::string& aPlatformName) const;
   const UtCalendar&        GetSimulationStartEpoch() const { return mSimulationStartEpoch; }

   void AddPlatformOfInterest(const std::string& aPlatformName);
   void RemovePlatformOfInterest(const std::string& aPlatformName);
   void SetPropagator(const std::string& aPlatformName, std::unique_ptr<UtOrbitalPropagatorBase> aPropagatorPtr);
   void SetSimulationStartEpoch(const UtCalendar& aEpoch);

   void Clear();

private:
   std::map<std::string, unsigned int>                             mPlatformsOfInterest{};
   std::map<std::string, std::unique_ptr<UtOrbitalPropagatorBase>> mPropagators{};
   UtCalendar                                                      mSimulationStartEpoch;
};

} // namespace SatelliteTether

#endif // SATELLITETETHERPROPAGATIONMANAGER_HPP
