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

#include "ChatSimInterface.hpp"

#include "ChatSimCommands.hpp"
#include "UtMemory.hpp"
#include "WsfSimulation.hpp"

Chat::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceBase(aPluginName)
{
}

void Chat::SimInterface::MessageSent(const QString& aName, const QString& aChannel, const QString& aText)
{
   AddSimCommand(ut::make_unique<ChatCommand>(aName.toStdString(), aChannel.toStdString(), aText.toStdString(), false));
}

void Chat::SimInterface::MessageReceived(const QString& aName, const QString& aChannel, const QString& aText)
{
   AddSimCommand(ut::make_unique<ChatCommand>(aName.toStdString(), aChannel.toStdString(), aText.toStdString(), true));
}

double Chat::SimInterface::GetSimTime() const noexcept
{
   std::lock_guard<std::mutex> l{mSimTimeMutex};
   return mSimTime;
}

void Chat::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   std::lock_guard<std::mutex> l{mSimTimeMutex};
   mSimTime = aSimulation.GetSimTime();
}
