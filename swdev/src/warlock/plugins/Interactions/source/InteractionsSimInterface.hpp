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
#ifndef WKINTERACTIONSINTERFACE_HPP
#define WKINTERACTIONSINTERFACE_HPP

#include "UtCallbackHolder.hpp"
class WsfSimulation;
#include "WkSimInterface.hpp"

namespace wkf
{
class InteractionPrefObject;
}

#include "InteractionsSimEvents.hpp"

namespace WkInteractions
{
class SimInterface : public warlock::SimInterfaceT<InteractionEvent>
{
public:
   SimInterface(const QString& aPluginName);

   void SetTimeout(double aTimeout);

protected:
   void ProcessEnableFlagChanged(bool aEnabled) override;

private:
   // Executed on the simulation thread to read and write data from/to the simulation
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   UtCallbackHolder mCallbacks;
   double           mTimeout; // (sec), for comm and kill events

   // This is needed to provide a unique id for messages.
   class MessageId
   {
   public:
      MessageId(size_t aSerialNumber, std::string aSrcAddr, std::string aDstAddr)
         : mSerialNumber(aSerialNumber)
         , mSrcAddr(aSrcAddr)
         , mDstAddr(aDstAddr)
      {
      }
      bool operator<(const MessageId& aRHS) const;

   private:
      size_t      mSerialNumber;
      std::string mSrcAddr;
      std::string mDstAddr;
   };
   // maps message serial number to previous hop data
   std::map<MessageId, std::pair<size_t, std::string>> mMessageHopTracker;
};
} // namespace WkInteractions
#endif // WKINTERACTIONSINTERFACE_HPP
