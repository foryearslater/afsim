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
#ifndef PLUGINPLATFORMBROWSER_HPP
#define PLUGINPLATFORMBROWSER_HPP

class PlatformBrowserDockWidget;
#include "PlatformBrowserSimInterface.hpp"
class WsfPlatform;

#include "PlatformBrowserInterface.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class Action;
}

namespace WkPlatformBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

private:
   void GuiUpdate() override;

   // managed by Qt (by way of base class)
   Interface* mPlatformBrowserInterface;
};
} // namespace WkPlatformBrowser
#endif
