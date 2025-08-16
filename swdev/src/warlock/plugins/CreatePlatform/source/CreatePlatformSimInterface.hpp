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

#ifndef CREATEPLATFORMSIMINTERFACE_HPP
#define CREATEPLATFORMSIMINTERFACE_HPP

#include "CreatePlatformSimEvents.hpp"
#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkCreatePlatform
{
class SimInterface : public warlock::SimInterfaceT<CreatePlatformEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
};
} // namespace WkCreatePlatform

#endif