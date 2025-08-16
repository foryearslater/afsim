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
#ifndef WKDEMOMODEINTERFACE_HPP
#define WKDEMOMODEINTERFACE_HPP

#include "DemoModeSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkDemoMode
{
class SimInterface : public warlock::SimInterfaceT<DemoModeEvent>
{
   Q_OBJECT

public:
   SimInterface(const QString& aPluginName);
   ~SimInterface() override = default;

   void SetRunSpeedMultiplier(double aRunSpeedMultiplier);

protected:
   void ProcessEnableFlagChanged(bool aEnabled) override;

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;

   double mRunSpeedMultiplier;
};
} // namespace WkDemoMode
#endif // WKDEMOMODEINTERFACE_HPP
