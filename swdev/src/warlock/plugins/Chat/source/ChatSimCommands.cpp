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
#include "ChatSimCommands.hpp"

#include "WkEventPipeClasses.hpp"
#include "WkObserver.hpp"

Chat::ChatCommand::ChatCommand(const std::string& aName, const std::string& aChannel, const std::string& aText, bool aReceived)
   : mName(aName)
   , mChannel(aChannel)
   , mText(aText)
   , mReceived(aReceived)
{
}

void Chat::ChatCommand::Process(WsfSimulation& aSimulation)
{
   WkObserver::SimulationChatMessage(aSimulation)(mName, mChannel, mText, mReceived);
}
