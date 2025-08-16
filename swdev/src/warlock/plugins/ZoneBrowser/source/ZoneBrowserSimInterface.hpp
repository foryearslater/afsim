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

#ifndef WkZoneBrowserSimInterface_HPP
#define WkZoneBrowserSimInterface_HPP

#include <map>

#include <QMutex>

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfZone.hpp"
#include "ZoneBrowserSimEvents.hpp"

class UtAttributeContainer;

namespace WkZoneBrowser
{
class SimInterface : public warlock::SimInterfaceT<ZoneEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void InterpretZoneSet(WsfPlatformComponent* aZone);

   void ZoneFillColorChanged(const WsfZone& aZone);
   void ZoneLineColorChanged(const WsfZone& aZone);

   UtCallbackHolder mCallbacks;
};
} // namespace WkZoneBrowser

#endif
