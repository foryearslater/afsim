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

#include "WsfPromptSimCommands.hpp"

#include "WsfSimulation.hpp"

void WkWsfPrompt::PauseCommand::Process(WsfSimulation& aSimulation)
{
   aSimulation.Pause();
}

void WkWsfPrompt::ResumeCommand::Process(WsfSimulation& aSimulation)
{
   aSimulation.Resume();
}