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

#include "WsfPromptPlugin.hpp"

#include "WkSimEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkWsfPrompt::Plugin,
                          "WsfPrompt",
                          "The WsfPrompt plugin receives WsfPrompt events from the simulation and will display message "
                          "boxes to the user in which the user can choose different responses.",
                          "warlock")

WkWsfPrompt::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mDialogs{}
   , mScriptSimInterfacePtr(simEnv.GetScriptSimInterface())
{
}

void WkWsfPrompt::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDialogs, mScriptSimInterfacePtr);
}
