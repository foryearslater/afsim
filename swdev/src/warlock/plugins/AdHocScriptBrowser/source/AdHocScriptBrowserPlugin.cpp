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
#include "AdHocScriptBrowserPlugin.hpp"

#include <QMessageBox>

#include "WkfDebugPrefObject.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkAdHocScriptBrowser::Plugin,
   "Ad Hoc Script Browser",
   "Provides the ability for a user to type an AFSIM script and execute it at the global or platform context.",
   "warlock")

WkAdHocScriptBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wkf::Plugin(aPluginName, aUniqueId)
{
   auto* wkfMainWindowPtr = wkfEnv.GetMainWindow();
   // addDockWidget is not polymorphic, the wkf::MainWindow::addDockWidget() will add the dock widget to the View menu.
   // We want this to appear in the developer menu, so use QMainWndow::addDockWidget, and add in the Developer explicitly.
   QMainWindow* qMainWindowPtr = wkfMainWindowPtr;
   mDockWidgetPtr              = new DockWidget(qMainWindowPtr);
   qMainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   QMenu* devMenuPtr = wkfMainWindowPtr->GetMenuByName("Developer");
   if (devMenuPtr)
   {
      wkf::Action* dlgActionPtr = new wkf::Action("Ad Hoc Scripting...", wkfMainWindowPtr);
      connect(dlgActionPtr, &QAction::triggered, this, &Plugin::DialogActionTriggered);
      devMenuPtr->addAction(dlgActionPtr);
   }

   // If the user disables the developer menu, hide this dock widget
   connect(wkfEnv.GetPreferenceObject<wkf::DebugPrefObject>(),
           &wkf::DebugPrefObject::DeveloperMenuVisibilityChanged,
           [this](bool aState)
           {
              if (!aState)
              {
                 mDockWidgetPtr->setVisible(false);
              }
           });
}

void WkAdHocScriptBrowser::Plugin::DialogActionTriggered()
{
   if (wkfEnv.IsFileBrowsingLocked())
   {
      QMessageBox::information(nullptr,
                               "Dialog Disabled",
                               "Ad Hoc Scripting interface is disabled due to use of the -lock_fileload command");
   }
   else
   {
      mDockWidgetPtr->show();
   }
}
