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

#ifndef WSFDRAWSIMINTERFACE_HPP
#define WSFDRAWSIMINTERFACE_HPP

#include <QObject>

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfDraw.hpp"
#include "WsfDrawSimEvents.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

namespace WkWsfDraw
{
class SimInterface : public warlock::SimInterfaceT<WsfDrawEvent>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);

   // Executed on the simulation thread to read and write data from/to the simulation
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   void ReceiveNetworkDraws(bool aEnabled) { mReceiveDraws = aEnabled; }

   void HandleDrawPkt(WsfXIO_DrawPkt& aPkt);

private:
   UtCallbackHolder mCallbacks;
   // WARNING: Only use this mSimulation object on the simulation thread (not GUI).
   //  Use of this should be minimal, it is the work around for not receiving the
   //  simulation when processing XIO packets such as WsfXIO_DrawPkt.
   const WsfSimulation* mSimulation;
   bool                 mReceiveDraws;
};
} // namespace WkWsfDraw

#endif // !WSFDRAWSIMINTERFACE_HPP
