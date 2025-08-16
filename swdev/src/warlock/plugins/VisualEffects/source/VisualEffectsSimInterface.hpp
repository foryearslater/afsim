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

#ifndef VISUALEFFECTSSIMINTERFACE_HPP
#define VISUALEFFECTSSIMINTERFACE_HPP

#include "DisAppearance.hpp"
#include "UtCallbackHolder.hpp"
#include "VisualEffectsSimEvents.hpp"
#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkVisualEffects
{
class SimInterface : public warlock::SimInterfaceT<VisualEffectsSimEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   std::map<std::string, DisEntityAppearance> mAppearanceMap;
   UtCallbackHolder                           mCallbacks;
};
} // namespace WkVisualEffects

#endif
