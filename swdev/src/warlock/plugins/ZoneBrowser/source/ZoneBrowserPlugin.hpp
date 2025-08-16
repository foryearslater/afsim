// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WkPluginZoneBrowser_HPP
#define WkPluginZoneBrowser_HPP

#include "WkPlugin.hpp"
#include "ZoneBrowserDockWidget.hpp"
#include "ZoneBrowserSimInterface.hpp"
#include "zone_browser/WkfZoneBrowserDataContainer.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"
#include "zone_browser/WkfZoneBrowserPrefWidget.hpp"

namespace WkZoneBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   void                                        GuiUpdate() override;
   WkZoneBrowser::DockWidget*                  mDockWidgetPtr;
   wkf::ZoneBrowserDataContainer               mDataContainer;
   PluginUiPointer<wkf::ZoneBrowserPrefWidget> mPrefWidgetPtr;
};
} // namespace WkZoneBrowser

#endif
