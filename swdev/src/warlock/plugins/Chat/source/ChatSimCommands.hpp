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
#ifndef CHAT_SIM_COMMANDS_HPP
#define CHAT_SIM_COMMANDS_HPP

#include "WkSimInterface.hpp"

namespace Chat
{
//! ChatCommand is used to send a chat message to the event_pipe.
//! Eventually, we may add commands for join/leave channel or for user rename.
class ChatCommand : public warlock::SimCommand
{
public:
   ChatCommand(const std::string& aName, const std::string& aChannel, const std::string& aText, bool aReceived);

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mName;
   std::string mChannel;
   std::string mText;
   bool        mReceived;
};
} // namespace Chat

#endif
