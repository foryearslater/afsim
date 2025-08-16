// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKEVENTMARKERSIMINTERFACE_HPP
#define WKEVENTMARKERSIMINTERFACE_HPP

#include "EventMarkerSimEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"

namespace WkEventMarker
{
class SimInterface : public warlock::SimInterfaceT<EventMarkSimEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   UtCallbackHolder mCallbacks;
};
} // namespace WkEventMarker

#endif
