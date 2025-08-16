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

#include "CyberEngagementControllerSimCommand.hpp"

#include <UtMemory.hpp>

#include "WkfQueueableMessageObject.hpp"
#include "WsfCyberAttackTypes.hpp"
#include "WsfCyberEngagement.hpp"
#include "WsfCyberEngagementManager.hpp"
#include "WsfCyberSimulationExtension.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"


void WkCyberEngagementController::CyberEngagementControllerCommand::Process(WsfSimulation& aSimulation)
{
   auto& manager = wsf::cyber::SimulationExtension::Get(aSimulation);
   manager.GetCyberEngagementManager().CyberAttack(mAttackType, mAttacker, mVictim, aSimulation);
}

void WkCyberEngagementController::CyberScanCommand::Process(WsfSimulation& aSimulation)
{
   auto& manager = wsf::cyber::SimulationExtension::Get(aSimulation);
   manager.GetCyberEngagementManager().CyberScan(mAttackType, mAttacker, mVictim, aSimulation);
}
