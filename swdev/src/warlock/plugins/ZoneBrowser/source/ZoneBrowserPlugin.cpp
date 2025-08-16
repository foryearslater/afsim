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

#include "ZoneBrowserPlugin.hpp"

#include "UtQtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkZoneBrowser::Plugin,
                          "Zone Browser",
                          "Provides a selection tool to visualize zones on the Map Display",
                          "warlock")

WkZoneBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(ut::qt::make_qt_ptr<wkf::ZoneBrowserPrefWidget>())
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidgetPtr = ut::qt::make_qt_ptr<WkZoneBrowser::DockWidget>(mDataContainer, mInterfacePtr.get(), mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   auto applyColorLambda = [&](wkf::ColorOption aColorChoice)
   {
      if (aColorChoice == wkf::ColorOption::cRANDOM)
      {
         mDockWidgetPtr->ApplyRandomColorPreferences();
      }
      else if (aColorChoice == wkf::ColorOption::cSINGLESELECT)
      {
         mDockWidgetPtr->ApplySetColorPreferences(mPrefWidgetPtr->GetPreferenceObject()->GetDefaultLineColor(),
                                                  mPrefWidgetPtr->GetPreferenceObject()->GetDefaultFillColor());
      }
   };

   connect(&mDataContainer,
           &wkf::ZoneBrowserDataContainer::DataCopyComplete,
           [&, applyColorLambda]() { applyColorLambda(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice()); });

   // Overwrite existing colors when preferences change
   connect(mPrefWidgetPtr->GetPreferenceObject(), &wkf::ZoneBrowserPrefObject::ColorChoiceChanged, this, applyColorLambda);
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::ZoneBrowserPrefObject::DefaultLineColorChanged,
           [&, applyColorLambda](const QColor& aColor)
           { applyColorLambda(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice()); });
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::ZoneBrowserPrefObject::DefaultFillColorChanged,
           [&, applyColorLambda](const QColor& aColor)
           { applyColorLambda(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice()); });
}

QList<wkf::PrefWidget*> WkZoneBrowser::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void WkZoneBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDataContainer);
}
