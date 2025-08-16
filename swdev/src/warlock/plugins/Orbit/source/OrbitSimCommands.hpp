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

#ifndef ORBITSIMCOMMANDS_HPP
#define ORBITSIMCOMMANDS_HPP

#include <string>

class WsfSimulation;

#include "OrbitSimInterface.hpp"

namespace WkOrbit
{

class RequestOrbitUpdateCommand : public SimInterface::Command
{
public:
   RequestOrbitUpdateCommand(SimInterface* aSimInterfacePtr, const std::string& aPlatformName, bool aUseWallClock = false)
      : Command(aSimInterfacePtr, aUseWallClock)
      , mPlatformName{aPlatformName}
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mPlatformName;
};

} // namespace WkOrbit

#endif // ORBITSIMCOMMANDS_HPP
