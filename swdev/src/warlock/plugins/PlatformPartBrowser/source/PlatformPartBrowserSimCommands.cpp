// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserSimCommands.hpp"

#include "WkObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformPart.hpp"
#include "WsfSimulation.hpp"

namespace
{
using namespace WkPlatformPartBrowser;

void SetPlatformPartProperty(WsfSimulation& aSimulation, WsfPlatformPart* aPlatformPart, PropertyName aPropertyName, bool aState)
{
   double simTime = aSimulation.GetSimTime();

   switch (aPropertyName)
   {
   case eON:
      aState ? aSimulation.TurnPartOn(simTime, aPlatformPart) : aSimulation.TurnPartOff(simTime, aPlatformPart);
      WkObserver::SimulationUserAction(aSimulation)("Part Turned " + std::string(aState ? "On" : "Off"),
                                                    aPlatformPart->GetPlatform());
      break;
   case eOPERATIONAL:
      aSimulation.SetPartOperational(simTime, aPlatformPart, aState);
      WkObserver::SimulationUserAction(aSimulation)("Part Set Operational " + std::string(aState ? "True" : "False"),
                                                    aPlatformPart->GetPlatform());
      break;
   case eDEBUG:
      aPlatformPart->SetDebugEnabled(aState);
      break;
   default:
      break;
   }
}
} // namespace

void WkPlatformPartBrowser::ChangePartCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform)
   {
      auto* platformPartPtr = platform->GetComponent<WsfPlatformPart>(mPartName);
      if (platformPartPtr)
      {
         SetPlatformPartProperty(aSimulation, platformPartPtr, mPropertyName, mState);
      }
   }
}
