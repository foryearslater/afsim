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

#ifndef SCOREBOARDSIMINTERFACE_HPP
#define SCOREBOARDSIMINTERFACE_HPP

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
class WsfSimulation;

#include "ScoreboardSimEvents.hpp"

namespace WkScoreboard
{
class SimInterface : public warlock::SimInterfaceT<ScoreboardEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   UtCallbackHolder mCallbacks;
};
} // namespace WkScoreboard

#endif