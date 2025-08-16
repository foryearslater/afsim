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
#ifndef NetworkBrowserInterface_HPP
#define NetworkBrowserInterface_HPP

#include <QObject>

#include "UtCallbackHolder.hpp"

class WsfXIO_Connection;

#include "NetworkBrowserSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkNetworkBrowser
{
class SimInterface : public warlock::SimInterfaceT<NetworkEvent>
{
   Q_OBJECT

public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationStarting(const WsfSimulation& aSimulation) override;
};
} // namespace WkNetworkBrowser

#endif // NetworkBrowserInterface_HPP
