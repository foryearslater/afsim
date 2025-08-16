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

#include "SpaceToolsScenario.hpp"

#include "Environment.hpp"
#include "UtMemory.hpp"
#include "WsfScenario.hpp"
#include "WsfStandardApplication.hpp"

SpaceTools::Scenario::Scenario()
   : mScenarioPtr{nullptr}
{
   // Note that if this class is used somewhere where it is in scope beyond a method/function call than
   // it would be necessary to be aware of the the WsfStandardApplication status in the wizard application
   // this can be done be connecting to wizard::Environment::WsfStandardApplicationPendingDestruction
   WsfStandardApplication& application = wizEnv.GetWsfStandardApplication();

   mScenarioPtr = ut::make_unique<WsfScenario>(application);
}
