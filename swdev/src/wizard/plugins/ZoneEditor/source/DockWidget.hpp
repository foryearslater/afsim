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

#ifndef ZONEBROWSERDOCKWIDGET_HPP
#define ZONEBROWSERDOCKWIDGET_HPP

#include "WsfPM_Zone.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"

namespace ZoneEditor
{
class DockWidget : public wkf::ZoneBrowserDockWidget
{
   Q_OBJECT
public:
   DockWidget(wkf::ZoneBrowserDataContainer& aDataContainer, QWidget* aParentPtr = nullptr);

   void AddZoneProxy(const WsfPM_Zone& aZoneProxy) { mZoneProxies.emplace_back(aZoneProxy); }
   void ClearZoneProxies() { mZoneProxies.clear(); }
   void RemoveZoneProxy(const WsfPM_Zone& aZoneProxy);

   bool ZoneProxyFound(const WsfPM_Zone& aZone);

   void CreateZoneContextMenu(const QPoint& aPoint);

   void ApplyRandomColorPreferences();
   void ApplySetColorPreferences(const QColor& aLineColor, const QColor& aFillColor);

protected:
   void DrawZone(const std::string& aPlatform,
                 const std::string& aName,
                 const QColor&      aLineColor,
                 const QColor&      aFillColor,
                 bool               aPreview = false) override;
   void AddZone(wkf::ZoneSetData* aDataPtr, bool aShow = false) override;
   void HideZone(const std::string& aPlatform, const std::string& aName) override;

   void GetAnchorEntityData(const std::string&                       aPlatform,
                            const std::string&                       aName,
                            wkf::ZoneSetData*&                       aDataPtr,
                            std::map<std::string, vespa::VaEntity*>& aAnchors,
                            bool                                     aPreview = false) override;

   void DeleteZone(wkf::ZoneSetData* aDataPtr) override;
   void RemovePlatform(const std::string& aPlatformName) override;
   void PreviewPendingDeletionCB() override;
   void AttachmentDeletedCB(vespa::VaAttachment* aAttachPtr, const vespa::VaAttachmentSubId& aId);

private:
   void AttachmentDeletedHandler(vespa::VaAttachment* aAttachmentPtr, const vespa::VaAttachmentSubId& aId) override {}
   void SetZoneLineColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor) override;
   void SetZoneFillColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor) override;
   void ZoneUpdatedCB(const std::string& aZoneName, const std::string& aPlatformName) override;

   void SetAllColors(bool aRandom = false);

   std::vector<WsfPM_Zone> mZoneProxies;
   vespa::VaCallbackHolder mCallbacks;
};
} // namespace ZoneEditor

#endif // !ZONEBROWSERDOCKWIDGET_HPP
