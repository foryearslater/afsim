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

#include "SimCommands.hpp"

#include "UtMath.hpp"
#include "WsfMover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

WarlockTraining::TurnCommand::TurnCommand(const std::string& aPlatformName, double aHeading)
// Pass false to the SimCommand to inform the simulation to process this Command on the SimClock
   : warlock::SimCommand(false)
   , mHeading(aHeading)
   , mPlatformName(aPlatformName)
{
}

void WarlockTraining::TurnCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   if (platform)
   {
      WsfMover* mover = platform->GetMover();
      if (mover)
      {
         // EXERCISE 1 TASK 4
         // Command the mover to turn to the specified Heading
         // PLACE YOUR CODE HERE
      }
   }
}

