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

#ifndef WSFPROMPTOBSERVER_HPP
#define WSFPROMPTOBSERVER_HPP

#include "wsf_prompt_export.h"

#include "UtCallback.hpp"
#include "WsfScriptPrompt.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationExtension.hpp"

namespace WsfObserver
{
using PromptDisplayedCallback = UtCallbackListN<void(const WsfScriptPromptClass::PromptData&)>;

WSF_PROMPT_EXPORT PromptDisplayedCallback& PromptDisplayed(const WsfSimulation* aSimulationPtr);
} // namespace WsfObserver

class WsfPromptObserver : public WsfSimulationExtension
{
public:
   WsfPromptObserver()           = default;
   ~WsfPromptObserver() override = default;

   static WsfPromptObserver& Get(const WsfSimulation* aSimulationPtr);

   WsfObserver::PromptDisplayedCallback PromptDisplayed;
};

#endif