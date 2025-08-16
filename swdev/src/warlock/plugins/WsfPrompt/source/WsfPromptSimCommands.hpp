// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PROMPTSIMCOMMANDS_HPP
#define PROMPTSIMCOMMANDS_HPP

#include "WkSimInterface.hpp"

namespace WkWsfPrompt
{
class PauseCommand : public warlock::SimCommand
{
public:
   PauseCommand()
      : SimCommand()
   {
   }

   void Process(WsfSimulation& aSimulation) override;
};

class ResumeCommand : public warlock::SimCommand
{
public:
   ResumeCommand()
      : SimCommand(true)
   {
   }

   void Process(WsfSimulation& aSimulation) override;
};
} // namespace WkWsfPrompt

#endif