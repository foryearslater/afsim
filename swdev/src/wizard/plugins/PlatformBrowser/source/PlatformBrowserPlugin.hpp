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
#ifndef PLATFORMBROWSERPLUGIN_HPP
#define PLATFORMBROWSERPLUGIN_HPP

#include "DockWidget.hpp"
#include "Plugin.hpp"

// Platform Browser namespace.
namespace PlatformBrowser
{

//! Platform Browser plugin.
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
private:
   void        FocusedCopy() override;
   DockWidget* mDockWidget;
};

} // namespace PlatformBrowser

#endif
