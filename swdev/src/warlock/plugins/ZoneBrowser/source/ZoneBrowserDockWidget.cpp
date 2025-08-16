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

#include "ZoneBrowserDockWidget.hpp"

#include "UtColor.hpp"
#include "UtOptional.hpp"
#include "UtQtMemory.hpp"
#include "UtRandom.hpp"
#include "ZoneBrowserSimCommands.hpp"
#include "ZoneBrowserSimInterface.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"

WkZoneBrowser::DockWidget::DockWidget(wkf::ZoneBrowserDataContainer& aDataContainer,
                                      SimInterface*                  aInterfacePtr,
                                      QWidget*                       aParentPtr)
   : ZoneBrowserDockWidget(aDataContainer, aParentPtr)
   , mInterfacePtr(aInterfacePtr)
{
   mCallbacks.Add(wkf::Observer::PlatformsAdded.Connect(&DockWidget::PlatformAddedCB, this));
}

void WkZoneBrowser::DockWidget::ApplyRandomColorPreferences()
{
   mButtonClicked  = false;
   auto widgetList = mUI.mTreeWidget->findChildren<wkf::ZoneSelectionWidget*>();
   mRandom.SetSeed(15);
   QColor      lineColor(mRandom.Uniform(0, 255), mRandom.Uniform(0, 255), mRandom.Uniform(0, 255), 255);
   QColor      fillColor(mRandom.Uniform(0, 255), mRandom.Uniform(0, 255), mRandom.Uniform(0, 255), 63);
   std::string platformName;
   std::string zoneName;
   for (const auto& widgetPtr : widgetList)
   {
      platformName = widgetPtr->mOwningPlatformName;
      zoneName     = widgetPtr->GetName();
      mInterfacePtr->AddSimCommand(
         ut::make_unique<UpdateZoneColorCommand>(mInterfacePtr, platformName, zoneName, fillColor, lineColor));
   }
}

void WkZoneBrowser::DockWidget::ApplySetColorPreferences(const QColor& aLineColor, const QColor& aFillColor)
{
   mButtonClicked = false;
   mRandom.SetSeed(0);
   mDefaultLineColor = aLineColor;
   mDefaultFillColor = aFillColor;
   std::string platformName;
   std::string zoneName;
   auto        widgetList = mUI.mTreeWidget->findChildren<wkf::ZoneSelectionWidget*>();
   for (const auto& widgetPtr : widgetList)
   {
      platformName = widgetPtr->mOwningPlatformName;
      zoneName     = widgetPtr->GetName();
      mInterfacePtr->AddSimCommand(
         ut::make_unique<UpdateZoneColorCommand>(mInterfacePtr, platformName, zoneName, mDefaultFillColor, mDefaultLineColor));
   }
}

void WkZoneBrowser::DockWidget::SetZoneLineColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor)
{
   if (mButtonClicked)
   {
      mInterfacePtr->AddSimCommand(ut::make_unique<SetLineColorCommand>(mInterfacePtr, aPlatform, aName, aColor));
   }
}

void WkZoneBrowser::DockWidget::SetZoneFillColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor)
{
   if (mButtonClicked)
   {
      mInterfacePtr->AddSimCommand(ut::make_unique<SetFillColorCommand>(mInterfacePtr, aPlatform, aName, aColor));
   }
}

void WkZoneBrowser::DockWidget::PlatformAddedCB(const wkf::PlatformList& aPlatforms)
{
   for (const auto& it : aPlatforms)
   {
      PlatformAdded(it->GetName());
   }
}
