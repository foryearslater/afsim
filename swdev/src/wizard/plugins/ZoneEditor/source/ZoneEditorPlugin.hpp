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

#ifndef ZONEEDITORPLUGIN_HPP
#define ZONEEDITORPLUGIN_HPP

#include <memory>

#include <QObject>

#include "CreateZoneDialog.hpp"
#include "DockWidget.hpp"
#include "Plugin.hpp"
#include "UtCallbackHolder.hpp"
#include "WsfPM_Zone.hpp"
#include "WsfPProxy.hpp"
#include "zone_browser/WkfZoneBrowserDataContainer.hpp"
#include "zone_browser/WkfZoneBrowserPrefWidget.hpp"

namespace ZoneEditor
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aName, const size_t aUniqueId);
   ~Plugin() override;

   void BuildViewerContextMenu(QMenu* aMenuPtr, vespa::VaViewer* aViewerPtr) override;
   void BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId) override;
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   void ApplyColors(wkf::ColorOption aColorChoice);
   void CreateDialog(vespa::VaPosition aInitialPosition);
   void PopulateDataContainer(WsfPProxy* aProxyPtr);

   void             OnProxyAvailable(WsfPProxy* aProxyPtr);
   void             HandleZoneProxyChange(const wizard::ProxyChange& aChange);
   void             HandlePlatformProxyChange(const wizard::ProxyChange& aChange);
   wkf::ZoneSetData CreateZoneSet(const WsfPM_Zone& aZone);
   wkf::ZoneSetData CreateZoneSet(const WsfPM_Zone& aZone, wkf::ZoneSetData& aZoneSetData);
   void             AddZoneSet(const WsfPM_Zone& aZone, bool aShow = false);
   void             AddZone(const WsfPM_ZoneDefinition& aZone, wkf::ZoneSetData& aZoneSet);
   void             UpdateZone(const WsfPM_Zone& aZone);
   void             UpdatePlatformZoneSet(const WsfPM_Zone& aZone, const WsfPM_Platform& aPlatform);
   void             RemoveZone(const WsfPM_Zone& aZone);
   void             RemovePlatformReferenceZones(const std::string& aPlatformName);
   void             RemovePlatformReferenceZoneSetZone(const WsfPM_Zone& aZone, const std::string& aPlatformName);

   wkf::CommonZoneVariables     ExtractCommonVars(const WsfPM_Zone& aZone);
   wkf::CircularZoneVariables   ExtractCircularVars(const WsfPM_ZoneDefinition& aZone);
   wkf::EllipticalZoneVariables ExtractEllipticalVars(const WsfPM_ZoneDefinition& aZone);
   wkf::SphericalZoneVariables  ExtractSphericalVars(const WsfPM_ZoneDefinition& aZone);
   wkf::PolygonalZoneVariables  ExtractPolygonalVars(const WsfPM_ZoneDefinition& aZone);

   wkf::ZoneBrowserDataContainer               mDataContainer;
   DockWidget*                                 mDockWidgetPtr;
   CreateZoneDialog*                           mDialogPtr;
   PluginUiPointer<wkf::ZoneBrowserPrefWidget> mPrefWidgetPtr;

   UtCallbackHolder mCallbacks;
   UtCallbackHolder mProxyCallbacks;
};
} // namespace ZoneEditor

#endif // !ZONEEDITORPLUGIN_HPP
