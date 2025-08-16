// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "PlatformBrowserPlugin.hpp"

#include <functional>

#include <QIcon>
#include <QLabel>
#include <QMenu>

#include "PlatformBrowserInterface.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "platform_browser/WkfPlatformBrowserDockWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkPlatformBrowser::Plugin,
                          "Platform Browser",
                          "The Platform Browser plugin creates a dialog that displays an interactive list of platforms "
                          "in an AFSIM simulation.",
                          "warlock")

WkPlatformBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   mPlatformBrowserInterface = new Interface(GetInterface());
   connect(&wkfEnv,
           &wkf::Environment::DeleteShortcutTriggered,
           mPlatformBrowserInterface,
           &Interface::DeleteCurrentlySelectedPlatforms);
}

void WkPlatformBrowser::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      QString      name = aEntityPtr->GetName().c_str();
      wkf::Action* deleteAction =
         new wkf::Action(QIcon::fromTheme("delete"), QString("Delete Platform %1").arg(name), aMenu);
      connect(deleteAction, &QAction::triggered, std::bind(&Interface::DeletePlatform, mPlatformBrowserInterface, name));
   }
}

void WkPlatformBrowser::Plugin::GuiUpdate()
{
   // Don't bother updating dialog if it is not visible
   if (mPlatformBrowserInterface->GetDockWidget()->isVisible())
   {
      mPlatformBrowserInterface->Update();
   }
}
