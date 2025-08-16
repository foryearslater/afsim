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
#ifndef CHAT_SIM_INTERFACE_HPP
#define CHAT_SIM_INTERFACE_HPP

#include <mutex>

#include "WkSimInterface.hpp"

namespace Chat
{
class SimInterface : public warlock::SimInterfaceBase
{
public:
   explicit SimInterface(const QString& aPluginName);

   void MessageSent(const QString& aName, const QString& aChannel, const QString& aText);

   void MessageReceived(const QString& aName, const QString& aChannel, const QString& aText);

   double GetSimTime() const noexcept;

private:
   void SimulationClockRead(const WsfSimulation& aSimulation) override;

   mutable std::mutex mSimTimeMutex;
   double             mSimTime = 0;
};
} // namespace Chat

#endif
