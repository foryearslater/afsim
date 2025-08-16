// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMMOVEMENTSIMINTERFACE_HPP
#define PLATFORMMOVEMENTSIMINTERFACE_HPP

#include <QObject>

class WsfSimulation;

#include "PlatformMovementSimEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfMover.hpp"

namespace WkPlatformMovement
{
class SimInterface : public warlock::SimInterfaceT<PlatformMovementEvent>
{
   Q_OBJECT

public:
   SimInterface(const QString& aPluginName);
   ~SimInterface() override;

protected:
   void RouteChangedHandler(WsfMover* aMover);

   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;

private:
   UtCallbackHolder mCallbacks;
};
} // namespace WkPlatformMovement
#endif // PLATFORMMOVEMENTSIMINTERFACE_HPP
