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

#ifndef ORBITALDATASIMINTERFACE_HPP
#define ORBITALDATASIMINTERFACE_HPP

#include "OrbitalDataSimEvents.hpp"
#include "WkSimInterface.hpp"

class UtCalendar;
class UtOrbitalElements;
class WsfPlatform;
class WsfSimulation;

namespace WkOrbitalData
{

class SimInterface : public warlock::SimInterfaceT<OrbitalDataEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

   void                     SetPlatformOfInterest(const std::string& aPlatformName);
   static UtOrbitalElements GenerateOrbitalElements(WsfPlatform* aPlatformPtr, const UtCalendar& aCal);

private:
   // Executed on the simulation thread to read and write data from/to the simulation
   void WallClockRead(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;

   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   std::string mPlatformOfInterest;
};

} // namespace WkOrbitalData
#endif
