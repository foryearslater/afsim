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
#ifndef PROJECTBROWSERPLUGIN_HPP
#define PROJECTBROWSERPLUGIN_HPP

#include "DockWidget.hpp"
#include "Plugin.hpp"
#include "PrefWidget.hpp"

namespace ProjectBrowser
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
private:
   DockWidget*                 mDockWidget;
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;
};
} // namespace ProjectBrowser
#endif
