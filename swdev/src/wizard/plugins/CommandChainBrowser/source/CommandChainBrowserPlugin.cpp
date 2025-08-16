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

#include "CommandChainBrowserPlugin.hpp"

#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(CommandChain::Plugin, "Command Chain Browser", "Provides a command chain tree", "wizard")

CommandChain::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDockWidget(nullptr)
//   , mPrefWidgetPtr(new PrefWidget)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();

   //   mPlatformBrowserInterface = new RvPlatformBrowser::Interface();
   //   connect(mPrefWidgetPtr->GetPreferenceObject(), &PrefObject::ShowInactiveChanged, mPlatformBrowserInterface,
   //   &Interface::ShowInactiveChanged);
}

void CommandChain::Plugin::FocusedCopy()
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
