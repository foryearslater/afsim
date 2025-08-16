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

#ifndef CYBERENGAGEMENTBROWSERSIMINTERFACE_HPP
#define CYBERENGAGEMENTBROWSERSIMINTERFACE_HPP

#include "CyberEngagementBrowserSimEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkCyberEngagementBrowser
{
class SimInterface : public warlock::SimInterfaceT<CyberEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   UtCallbackHolder mCallbacks;
};
} // namespace WkCyberEngagementBrowser

#endif