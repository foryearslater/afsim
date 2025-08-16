// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WkPluginNetworkBrowser_HPP
#define WkPluginNetworkBrowser_HPP

#include "NetworkBrowserDataContainer.hpp"
#include "NetworkBrowserDockWidget.hpp"
#include "NetworkBrowserSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkNetworkBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

private:
   void GuiUpdate() override;

   DataContainer mNetworkData;
   DockWidget*   mDockWidget;
};
} // namespace WkNetworkBrowser

#endif
