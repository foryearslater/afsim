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

#include "ProjectBrowserPlugin.hpp"

#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(ProjectBrowser::Plugin, "Project Browser", "Provides a file list", "wizard")

ProjectBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDockWidget(nullptr)
   , mPrefWidgetPtr(new PrefWidget)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::LeftDockWidgetArea, mDockWidget);
}

// QList<wkf::PrefWidget*> ProjectBrowser::Plugin::GetPreferencesWidgets() const
//{
//    QList<wkf::PrefWidget*> prefs;
//    prefs << mPrefWidgetPtr;
//    return prefs;
// }
