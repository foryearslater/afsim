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

#include "PlatformBrowserPlugin.hpp"

#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(PlatformBrowser::Plugin, "Platform Browser", "Provides a platform list with components", "wizard")

PlatformBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDockWidget(nullptr)
//   , mPrefWidgetPtr(new PrefWidget)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);


   //   mPlatformBrowserInterface = new RvPlatformBrowser::Interface();
   //   connect(mPrefWidgetPtr->GetPreferenceObject(), &PrefObject::ShowInactiveChanged, mPlatformBrowserInterface,
   //   &Interface::ShowInactiveChanged);
}

void PlatformBrowser::Plugin::FocusedCopy()
{
   if (mDockWidget->isVisible())
   {
      mDockWidget->TryCopy();
   }
}

// QList<wkf::PrefWidget*> ProjectBrowser::Plugin::GetPreferencesWidgets() const
//{
//    QList<wkf::PrefWidget*> prefs;
//    prefs << mPrefWidgetPtr;
//    return prefs;
// }
