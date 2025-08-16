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

#include "Factory.hpp"

#include <QMainWindow>

#include "AboutDialog.hpp"
#include "DocumentationDialog.hpp"
#include "PlatformDataDockWidget.hpp"
#include "PointOfInterestPropertiesWidget.hpp"
#include "PreferencesDialog.hpp"
#include "RangeRingPropertiesWidget.hpp"
#include "VisibilityDockWidget.hpp"
#include "WizMapLayerDockWidget.hpp"

wkf::Dialog* wizard::Factory::CreateDialog(const std::string& aDialogType, wkf::MainWindow* aParent /*= nullptr */) const
{
   if (aDialogType == "About")
   {
      return new AboutDialog(aParent);
   }
   else if (aDialogType == "Preferences")
   {
      return new PreferencesDialog(aParent);
   }
   else if (aDialogType == "Documentation")
   {
      return new DocumentationDialog(aParent);
   }

   return wkf::Factory::CreateDialog(aDialogType, aParent);
}


QDockWidget* wizard::Factory::CreateDockWidget(const std::string& aDockWidgetType, QMainWindow* aParent /*= 0*/) const
{
   if (aDockWidgetType == "PlatformData")
   {
      return new PlatformDataDockWidget(aParent);
   }
   else if (aDockWidgetType == "MapLayer")
   {
      return new MapLayerDockWidget(aParent);
   }
   else if (aDockWidgetType == "VisibilityWidget")
   {
      return new VisibilityDockWidget(aParent);
   }

   return wkf::Factory::CreateDockWidget(aDockWidgetType, aParent);
}

QWidget* wizard::Factory::CreateWidget(const std::string& aWidgetType, QMainWindow* aParent) const
{
   if (aWidgetType == "PointOfInterestProperties")
   {
      return new wizard::PointOfInterestPropertiesWidget(aParent);
   }
   else if (aWidgetType == "RangeRingProperties")
   {
      return new wizard::RingPropertiesWidget(aParent);
   }
   return wkf::Factory::CreateWidget(aWidgetType, aParent);
}
