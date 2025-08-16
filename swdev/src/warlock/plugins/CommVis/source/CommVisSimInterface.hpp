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

#ifndef COMMVISSIMINTERFACE_HPP
#define COMMVISSIMINTERFACE_HPP

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfCommNetworkManager.hpp"
#include "WsfEvent.hpp"
class WsfSimulation;

#include "CommVisSimEvents.hpp"

namespace WkCommVis
{
class SimInterface : public warlock::SimInterfaceT<CommVisEvent>
{
public:
   SimInterface(const QString& aPluginName);

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   void AddNetworks(const wsf::comm::NetworkManager* aNetworkManagerPtr);
   void AddComms(const wsf::comm::NetworkManager* aNetworkManagerPtr);
   void TurnOffComm(double aSimTime, wsf::comm::Comm* aCommPtr);
   void TurnOnComm(double aSimTime, wsf::comm::Comm* aCommPtr);

   void RemoveComm(double aSimTime, wsf::comm::Comm* aCommPtr);

   void MessageTransmitted(double aSimTime, wsf::comm::Comm* aCommPtr, const WsfMessage& aMessage);

   void MessageHop(double            aSimTime,
                   wsf::comm::Comm*  aReceiverCommPtr,
                   wsf::comm::Comm*  aDestinationCommPtr,
                   const WsfMessage& aMessage);

   UtCallbackHolder mCallbacks;
};
} // namespace WkCommVis

#endif
