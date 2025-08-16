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

#include "ScoreboardPlugin.hpp"

#include <QMenu>

#include "ScoreboardDialog.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkScoreboard::Plugin,
                          "Scoreboard",
                          "The Scoreboard plugin creates a dialog, under the Tools menu, that shows statistics for "
                          "local weapon performance within the simulation.",
                          "warlock")

WkScoreboard::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDialog                        = new Dialog(mScoreboardData, mainWindowPtr);

   // toolbar code
   QMenu* toolsMenuPtr = mainWindowPtr->GetMenuByName("Tools");
   if (toolsMenuPtr)
   {
      wkf::Action* dlgActionPtr = new wkf::Action(QIcon::fromTheme("scoreboard"), "Scoreboard...", mainWindowPtr);
      connect(dlgActionPtr, &QAction::triggered, mDialog.data(), &QDialog::show);
      toolsMenuPtr->addAction(dlgActionPtr);
   }
}

QList<wkf::Tip> WkScoreboard::Plugin::GetTips() const
{
   QList<wkf::Tip> tips;
   tips.append(wkf::Tip("Scoreboard",
                        "You can view weapon performance such as number of weapons fired and the number of kills using "
                        "the Scoreboard dialog located under the Tools menu.",
                        "wkf_plugin/wk_scoreboard.html",
                        "Scoreboard"));
   return tips;
}

void WkScoreboard::Plugin::GuiUpdate()
{
   // Process all the new SimEvents
   mInterfacePtr->ProcessEvents(mScoreboardData);
   mDialog->Update();
}
