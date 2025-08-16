//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef SimInterfacePlatformHistory_HPP
#define SimInterfacePlatformHistory_HPP

#include "PlatformHistorySimEvents.hpp"
#include "UtCallbackHolder.hpp"

class WsfSimulation;

namespace WkPlatformHistory
{
class SimInterface : public warlock::SimInterfaceT<PlatformHistoryEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void             SimulationInitializing(const WsfSimulation& aSimulation) override;
   void             PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   UtCallbackHolder mCallbacks;
};
} // namespace WkPlatformHistory

#endif
