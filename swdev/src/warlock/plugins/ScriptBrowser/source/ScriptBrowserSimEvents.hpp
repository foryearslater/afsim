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

#ifndef SCRIPTBROWSERSIMEVENTS_HPP
#define SCRIPTBROWSERSIMEVENTS_HPP

#include "ScriptBrowserDockWidget.hpp"
#include "WkSimInterface.hpp"

namespace WkScriptBrowser
{
class ScriptBrowserEvent : public warlock::SimEvent
{
public:
   virtual void Process(DockWidget& aDockWidget) = 0;
};

class SimulationStartingEvent : public ScriptBrowserEvent
{
public:
   void Process(DockWidget& aDockWidget) override { aDockWidget.SimulationStarting(); }
};
} // namespace WkScriptBrowser

#endif