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

#include "CyberEngagementControllerSimInterface.hpp"

#include <vector>

#include <UtMemory.hpp>

#include "WsfCyberAttackTypes.hpp"
#include "WsfCyberScenarioExtension.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfStringId.hpp"

WkCyberEngagementController::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<CyberEngagementControllerEvent>(aPluginName)
{
}

void WkCyberEngagementController::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   auto&                    cyberExtension = wsf::cyber::ScenarioExtension::Get(aSimulation.GetScenario());
   wsf::cyber::AttackTypes& attackTypes    = cyberExtension.GetAttackTypes();
   std::vector<WsfStringId> attackNames;
   attackTypes.GetTypeIds(attackNames);
   std::vector<std::string> attackStrVec;
   for (const auto& attack : attackNames)
   {
      // Get only attack types that do not require additional input
      if (!cyberExtension.GetAttackTypeInputRequirement(attack, wsf::cyber::Effect::InputRequirement::cREQUIRED) &&
          attack != "WSF_CYBER_ATTACK")
      {
         attackStrVec.push_back(attack); // changes WsfStringID to std::string to keep thread safety.
      }
   }
   AddSimEvent(ut::make_unique<SimulationInitializedEvent>(attackStrVec));
}
