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

#include "WsfPromptSimInterface.hpp"

#include <QPointer>

#include "UtMemory.hpp"
#include "WkSimEnvironment.hpp"
#include "WsfPromptDialog.hpp"
#include "WsfPromptObserver.hpp"

WkWsfPrompt::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<PromptEvent>(aPluginName)
{
}

WkWsfPrompt::SimInterface::~SimInterface()
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
}

void WkWsfPrompt::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   // When the sim restarts, we want to close any dialogs that are open,
   // then clear out the list we are maintaining.
   AddSimEvent(ut::make_unique<WkWsfPrompt::CloseEvent>());
}

void WkWsfPrompt::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
   mCallbacks.Add(WsfObserver::PromptDisplayed(&aSimulation).Connect(&SimInterface::DisplayMessageHandler, this));
}

void WkWsfPrompt::SimInterface::DisplayMessageHandler(const WsfScriptPromptClass::PromptData& aPromptData)
{
   AddSimEvent(ut::make_unique<WkWsfPrompt::DisplayEvent>(aPromptData));
}