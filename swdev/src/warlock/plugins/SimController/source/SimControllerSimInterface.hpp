// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SIMCONTROLLERINTERFACE_HPP
#define SIMCONTROLLERINTERFACE_HPP

#include "WkSimInterface.hpp"
class WsfSimulation;
#include <atomic>

#include "SimControllerSimEvents.hpp"
#include "UtCallbackHolder.hpp"

namespace WkSimController
{
class SimInterface : public warlock::SimInterfaceT<SimControllerEvent>
{
public:
   SimInterface(const QString& aPluginName);

   void OnFallingBehindThresholdChanged(double aThreshold);

protected:
   // Executed on the simulation thread to read data from the simulation
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   //! Written to on the GUI thread. Read from on the sim thread during SimulationInitializing().
   std::atomic<double> mFallingBehindThreshold;

   UtCalendar       mWallTime;
   UtCallbackHolder mCallbacks;
};
} // namespace WkSimController
#endif // SIMCONTROLLERINTERFACE_HPP
