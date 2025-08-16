// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SENSORCONTROLLERPLUGIN_HPP
#define SENSORCONTROLLERPLUGIN_HPP

#include "SensorControllerDataContainer.hpp"
#include "SensorControllerDockWidget.hpp"
#include "SensorControllerSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkSensorController
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void GuiUpdate() override;

private:
   DockWidget*   mDockWidget{nullptr};
   SimInterface* mSimInterfacePtr{nullptr};
   DataContainer mDataContainer;
};
} // namespace WkSensorController
#endif
