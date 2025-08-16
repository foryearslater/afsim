// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef P6DOF_DATAINTERFACE_HPP
#define P6DOF_DATAINTERFACE_HPP

#include <QObject>

#include "P6DOF_DataSimEvents.hpp"
#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkP6DOF_Data
{

class SimInterface : public warlock::SimInterfaceT<P6DOF_DataEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

   void SetPlatformOfInterest(const std::string& aPlatformName);

private:
   // Executed on the simulation thread to read and write data from/to the simulation
   void WallClockRead(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;

   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   std::string mPlatformOfInterest;
};

} // namespace WkP6DOF_Data
#endif
