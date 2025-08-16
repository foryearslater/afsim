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

#ifndef PROMPTSIMINTERFACE_HPP
#define PROMPTSIMINTERFACE_HPP

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfPromptObserver.hpp"
#include "WsfPromptSimEvents.hpp"

namespace WkWsfPrompt
{
class PromptDialog;
class SimInterface : public warlock::SimInterfaceT<PromptEvent>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);
   ~SimInterface() override;

protected:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

private:
   void DisplayMessageHandler(const WsfScriptPromptClass::PromptData& aPromptData);

   UtCallbackHolder mCallbacks;
};
} // namespace WkWsfPrompt


#endif