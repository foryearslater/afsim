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


#include "TrackDetailsDisplayPlugin.hpp"

#include <QMenu>

#include "TrackDetailsDisplayDialog.hpp"
#include "UtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatform.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkTrackDetailsDisplay::Plugin,
                          "Track Details Display...",
                          "The Track Details Display plugin creates a dialog, under the Tools menu, that displays "
                          "tracks within the simulation.",
                          "warlock")

WkTrackDetailsDisplay::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mDialog(new Dialog(*mInterfacePtr, mDataContainer, wkfEnv.GetMainWindow()))
{
   mDialog->setWindowFlags(mDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenu     = mainWindowPtr->GetMenuByName("Tools");
   if (toolsMenu)
   {
      wkf::Action* dlgActionPtr =
         new wkf::Action(QIcon::fromTheme("track_details"), "Track Details Display...", mainWindowPtr);
      connect(dlgActionPtr, &QAction::triggered, mDialog.data(), &QDialog::show);
      toolsMenu->addAction(dlgActionPtr);
   }
}

void WkTrackDetailsDisplay::Plugin::GuiUpdate()
{
   // Don't bother updating dialog if it is not visible
   if (mDialog->isVisible())
   {
      mInterfacePtr->ProcessEvents(mDataContainer);
   }
}
