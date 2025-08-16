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

#ifndef DEMOMODESIMCOMMANDS_HPP
#define DEMOMODESIMCOMMANDS_HPP

#include "WkSimInterface.hpp"
#include "WsfSimulation.hpp"

namespace WkDemoMode
{
class RunSpeedCommand : public warlock::SimCommand
{
public:
   RunSpeedCommand(double aRunSpeedMultiplier)
      : warlock::SimCommand(true)
      , mRunSpeedMultiplier(aRunSpeedMultiplier)
   {
   }

   void Process(WsfSimulation& aSimulation) override { aSimulation.SetClockRate(mRunSpeedMultiplier); }

private:
   double mRunSpeedMultiplier;
};
} // namespace WkDemoMode
#endif
