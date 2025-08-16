// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PluginScriptBrowser_HPP
#define PluginScriptBrowser_HPP

#include "ScriptBrowserDockWidget.hpp"
#include "ScriptBrowserSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkScriptBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

private:
   void GuiUpdate() override;

   DockWidget* mDockWidget;
};
} // namespace WkScriptBrowser
#endif
