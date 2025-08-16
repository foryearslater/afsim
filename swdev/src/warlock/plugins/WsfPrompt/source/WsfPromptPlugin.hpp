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

#ifndef WSFPROMPTPLUGIN_HPP
#define WSFPROMPTPLUGIN_HPP

#include "WkPlugin.hpp"
#include "WkScriptSimInterface.hpp"
#include "WsfPromptSimInterface.hpp"

namespace WkWsfPrompt
{
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void GuiUpdate() override;

private:
   Dialogs               mDialogs;
   ScriptSimInterfacePtr mScriptSimInterfacePtr;
};
} // namespace WkWsfPrompt

#endif