// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKZONEBROWSERDOCKWIDGET_HPP
#define WKZONEBROWSERDOCKWIDGET_HPP
#include <QColor>
#include <QWidget>

#include "WkfPlatform.hpp"
#include "ZoneBrowserSimInterface.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"

namespace WkZoneBrowser
{
class DockWidget : public wkf::ZoneBrowserDockWidget
{
   Q_OBJECT
public:
   DockWidget(wkf::ZoneBrowserDataContainer& aDataContainer, SimInterface* aInterfacePtr, QWidget* aParentPtr = nullptr);

   void ApplyRandomColorPreferences() override;
   void ApplySetColorPreferences(const QColor& aLineColor, const QColor& aFillColor) override;

private:
   void SetZoneLineColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor) override;
   void SetZoneFillColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor) override;
   void PlatformAddedCB(const wkf::PlatformList& aPlatforms);
   SimInterface* mInterfacePtr;
};
} // namespace WkZoneBrowser

#endif // !ZONEBROWSERDOCKWIDGET_HPP
