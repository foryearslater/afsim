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

#include "WkFactory.hpp"

#include <QMainWindow>
#include <QMessageBox>

#include "WkAboutDialog.hpp"
#include "WkDocumentationDialog.hpp"
#include "WkPlatformDataDockWidget.hpp"
#include "WkPlottingWidget.hpp"
#include "WkPreferencesDialog.hpp"
#include "WkRunManager.hpp"
#include "WkScenario.hpp"
#include "WkfDialog.hpp"
#include "WkfEnvironment.hpp"
#include "annotation/WkfPointOfInterestPropertiesWidget.hpp"
#include "annotation/WkfRingPropertiesWidget.hpp"
#include "visibility/WkfVisibilityDockWidget.hpp"
#include "wsf_draw/WkfOverlayWsfDraw.hpp"
#include "wsf_draw/WkfWsfDrawDockWidget.hpp"

wkf::Dialog* warlock::Factory::CreateDialog(const std::string& aDialogType, wkf::MainWindow* aParent /*= nullptr */) const
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

QDockWidget* warlock::Factory::CreateDockWidget(const std::string& aDockWidgetType, QMainWindow* aParent /*= 0*/) const
{
   if (aDockWidgetType == "PlottingWidget")
   {
      return new PlottingWidget(aParent);
   }
   else if (aDockWidgetType == "PlatformData")
   {
      return new PlatformDataDockWidget(aParent);
   }
   else if (aDockWidgetType == "VisibilityWidget")
   {
      return new wkf::VisibilityDockWidget(aParent);
   }
   return wkf::Factory::CreateDockWidget(aDockWidgetType, aParent);
}

QWidget* warlock::Factory::CreateWidget(const std::string& aWidgetType, QMainWindow* aParent) const
{
   if (aWidgetType == "PointOfInterestProperties")
   {
      return new Annotation::PointOfInterestPropertiesWidget(aParent);
   }
   else if (aWidgetType == "RangeRingProperties")
   {
      return new Annotation::RingPropertiesWidget(aParent);
   }
   return wkf::Factory::CreateWidget(aWidgetType, aParent);
}

void warlock::Factory::UserOpenFile(const QStringList& aFileNames) const
{
   // In code this is referred to as locking File Browsing, but the Permission's Dialog
   // and documentation present it to the user as locking File Loading. Thus use this
   // option to prevent a file from being opened when it shouldn't be.
   if (!wkfEnv.IsFileBrowsingLocked())
   {
      auto& runManager = warlock::RunManager::GetInstance();
      if (runManager.IsSimulationActive())
      {
         if (QMessageBox::Yes == QMessageBox::question(nullptr,
                                                       "Terminate Current Simulation",
                                                       "Do you want to terminate the current simulation?",
                                                       QMessageBox::Yes | QMessageBox::No))
         {
            warlock::RunManager::GetInstance().LoadScenario(aFileNames);
         }
      }
      else
      {
         warlock::RunManager::GetInstance().LoadScenario(aFileNames);
      }
   }
}

vespa::VaScenario* warlock::Factory::UserCreateScenario(const std::string& aScenarioType) const
{
   if (aScenarioType == "wk")
   {
      return new Scenario();
   }
   else
   {
      return wkf::Factory::UserCreateScenario(aScenarioType);
   }
}

vespa::VaOverlay* warlock::Factory::UserCreateOverlay(const std::string& aOverlayType,
                                                      const std::string& aOverlayName /*= ""*/) const
{
   if (aOverlayType == "wsfdraw")
   {
      if (!aOverlayName.empty())
      {
         return new wkf::OverlayWsfDraw(aOverlayName);
      }
      else
      {
         return new wkf::OverlayWsfDraw();
      }
   }
   else
   {
      return wkf::Factory::UserCreateOverlay(aOverlayType, aOverlayName);
   }
}
