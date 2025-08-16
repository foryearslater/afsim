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

#include "OrbitPlugin.hpp"

#include <memory>

#include "OrbitSimCommands.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkOrbit::Plugin,
                          "Orbit",
                          "Displays orbit visualization of space-domain platforms in the Map Display.",
                          "warlock")

WkOrbit::Plugin::Plugin(const QString& aPluginName, const size_t& aUniqueId)
   : warlock::PluginT<SimInterface, wkf::OrbitPluginBase>(aPluginName, aUniqueId)
{
}

void WkOrbit::Plugin::SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr)
{
   OrbitPluginBase::SetPlatformOptionState(aOptionId, aState, aPlatformPtr);
   GetInterface()->AddSimCommand(ut::make_unique<RequestOrbitUpdateCommand>(GetInterface(), aPlatformPtr->GetName()));
}

void WkOrbit::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mOrbitInterfacePtr);
}
