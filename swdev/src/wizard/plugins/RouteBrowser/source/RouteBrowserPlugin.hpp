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

#ifndef ROUTE_BROWSER_PLUGIN_HPP
#define ROUTE_BROWSER_PLUGIN_HPP

#include <QString>

#include "Plugin.hpp"
#include "RouteBrowserInterface.hpp"
#include "RouteBrowserPrefWidget.hpp"

namespace RouteBrowser
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId) override;

   void EditWaypointActionHandler(wkf::AttachmentRoute* aAttachmentPtr, unsigned int aWaypointIndex);

   void    RouteChangesApplied(const RouteBrowserInterface::RouteInfo& aRouteInfo);
   void    RouteWaypointAdded(const wkf::RouteBrowserInterface::WaypointInfo& aWaypoint,
                              int                                             aIndex,
                              const QString&                                  aPlatformName,
                              const QString&                                  aRouteName);
   void    RouteWaypointRemoved(int aIndex, const QString& aPlatformName, const QString& aRouteName);
   void    CreateRoute(const QString& aPlatformName, const RouteBrowserInterface::RouteInfo& aRouteInfo);
   void    CreateGlobalRoute(const RouteBrowserInterface::RouteInfo& aGlobalRoute);
   void    EditGlobalRoute(const RouteBrowserInterface::RouteInfo& aGlobalRoute);
   void    PlatformDeleted(const QString& aPlatformName) override;
   QString GetGlobalRouteText(const RouteBrowserInterface::RouteInfo& aGlobalRoute) const;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   void                        PreferencesChanged();
   void                        GlobalRouteDeleted(const QString& aRouteName);
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;
   std::string                 mNewRouteSaveFileName = "";
   // managed by Qt
   RouteBrowserInterface* mRouteBrowserInterfacePtr;
};
} // namespace RouteBrowser
#endif
