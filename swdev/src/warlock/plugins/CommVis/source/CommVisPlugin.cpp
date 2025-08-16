// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CommVisPlugin.hpp"

#include <QMenu>

#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "comm_vis/WkfCommVisDialog.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkCommVis::Plugin,
                          "CommVis",
                          "The Comms Visualization plugin creates a dialog, under the Tools menu, that shows activity "
                          "on a communications network.",
                          "warlock")

WkCommVis::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDialog                        = ut::qt::make_qt_ptr<wkf::CommVisDialog>(mCommData, mainWindowPtr);

   // Toolbar code
   QMenu* toolsMenuPtr = mainWindowPtr->GetMenuByName("Tools");
   if (toolsMenuPtr)
   {
      auto dlgActionPtr = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("star"), "CommVis...", mainWindowPtr);
      connect(dlgActionPtr, &QAction::triggered, mDialog.data(), &QDialog::show);
      toolsMenuPtr->addAction(dlgActionPtr);
   }
}

QList<wkf::Tip> WkCommVis::Plugin::GetTips() const
{
   QList<wkf::Tip> tips;
   tips.append(wkf::Tip("Comms Visualization",
                        "You can view communication network activity.",
                        "wkf_plugin/wk_comm_vis.html",
                        "Comms Visualization"));
   return tips;
}

void WkCommVis::Plugin::GuiUpdate()
{
   // Process all the new SimEvents
   mInterfacePtr->ProcessEvents(mCommData);
   if (mDialog->isVisible())
   {
      mDialog->Update();
   }
}
