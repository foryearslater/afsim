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

#include "ScriptBrowserSimInterface.hpp"

#include <UtMemory.hpp>

WkScriptBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<ScriptBrowserEvent>(aPluginName)
{
}

void WkScriptBrowser::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimulationStartingEvent>());
}
