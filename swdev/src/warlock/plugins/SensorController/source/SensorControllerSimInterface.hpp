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

#ifndef SENSORCONTROLLERSIMINTERFACE_HPP
#define SENSORCONTROLLERSIMINTERFACE_HPP

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
#include "WsfSensor.hpp"
class WsfSimulation;

#include "SensorControllerSimEvents.hpp"

namespace WkSensorController
{
class SimInterface : public warlock::SimInterfaceT<SensorControllerEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

protected:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

private:
   void UpdateSensor(WsfSensor* aSensorPtr);

   void AddSlewingCompleteCallback(WsfPlatform* aPlatformPtr);

   UtCallbackHolder mCallbacks;
};
} // namespace WkSensorController

#endif
