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

#ifndef SCRIPTBROWSERSIMINTERFACE_HPP
#define SCRIPTBROWSERSIMINTERFACE_HPP

#include "ScriptBrowserSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkScriptBrowser
{
class SimInterface : public warlock::SimInterfaceT<ScriptBrowserEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationStarting(const WsfSimulation& aSimulation) override;
};
} // namespace WkScriptBrowser

#endif