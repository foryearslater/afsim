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

#include "CreatePlatformSimInterface.hpp"

#include <UtMemory.hpp>

#include "WsfPlatformTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

WkCreatePlatform::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<CreatePlatformEvent>(aPluginName)
{
}

void WkCreatePlatform::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   std::vector<WsfPlatform*> currentTypes;
   std::vector<std::string>  types;
   WsfPlatformTypes*         platformTypes = &aSimulation.GetScenario().GetPlatformTypes();
   platformTypes->GetCurrentTypes(currentTypes);
   for (const auto& base : currentTypes)
   {
      types.emplace_back(base->GetType());
      std::sort(types.begin(), types.end());
   }

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>(types));
}
