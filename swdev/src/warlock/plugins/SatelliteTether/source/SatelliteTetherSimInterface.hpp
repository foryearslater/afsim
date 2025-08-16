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

#ifndef SATELLITETETHERSIMINTERFACE_HPP
#define SATELLITETETHERSIMINTERFACE_HPP

#include <map>
#include <string>

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
class WsfOrbitalEvent;
class WsfSpaceMoverBase;

#include "SatelliteTetherSimEvents.hpp"

namespace SatelliteTether
{

class SimInterface : public warlock::SimInterfaceT<SatelliteTetherEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);
   ~SimInterface() override = default;

   void AddPlatformOfInterest(const std::string& aName);
   void RemovePlatformOfInterest(const std::string& aName);

protected:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

private:
   void OnOrbitColorChanged(double aSimTime, WsfSpaceMoverBase* aMoverPtr);
   void OnOrbitUpdated(double aSimTime, WsfSpaceMoverBase* aSpaceMoverPtr, const WsfOrbitalEvent& aEvent);

   void AddColorUpdateEvent(WsfSpaceMoverBase* aMoverPtr);

   UtCallbackHolder                    mCallbacks;
   std::map<std::string, unsigned int> mPlatformsOfInterest;
};

} // namespace SatelliteTether

#endif // SATELLITETETHERSIMINTERFACE_HPP
