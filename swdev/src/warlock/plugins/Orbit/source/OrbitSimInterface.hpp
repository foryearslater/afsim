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

#ifndef ORBITSIMINTERFACE_HPP
#define ORBITSIMINTERFACE_HPP

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
class WsfSpaceMoverBase;
class WsfOrbitalEvent;

#include "OrbitSimEvents.hpp"

namespace WkOrbit
{

class SimInterface : public warlock::SimInterfaceT<OrbitEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);
   ~SimInterface() override = default;

protected:
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationClockRead(const WsfSimulation& aSimulation) override;

private:
   class OrbitUpdateEvent : public WsfEvent
   {
   public:
      OrbitUpdateEvent(WkOrbit::SimInterface& aInterface, size_t aPlatformIndex)
         : mInterface(aInterface)
         , mPlatformIndex{aPlatformIndex}
      {
      }
      ~OrbitUpdateEvent() override = default;

      WsfEvent::EventDisposition Execute() override;

   private:
      WkOrbit::SimInterface& mInterface;
      size_t                 mPlatformIndex;
   };

   void OnSpaceMoverUpdate(double aSimTime, WsfSpaceMoverBase* aMoverPtr);

   void OnOrbitalManeuverCanceled(double aSimTime, WsfSpaceMoverBase* aMoverPtr, const WsfOrbitalEvent& aOrbitalEvent);
   void OnOrbitalManeuverCompleted(double aSimTime, WsfSpaceMoverBase* aMoverPtr, const WsfOrbitalEvent& aOrbitalEvent);

   UtCallbackHolder mCallbacks;
   double           mLastOrbitRedrawTime{0.0};
   double           mLastMoonRedrawTime{0.0};
};

} // namespace WkOrbit

#endif // ORBITSIMINTERFACE_HPP
