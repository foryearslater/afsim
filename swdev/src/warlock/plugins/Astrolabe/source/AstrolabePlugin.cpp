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

#include "AstrolabePlugin.hpp"

#include <QMenu>

#include "AstrolabeConfig.hpp"
#include "AstrolabeEventInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkAstrolabe::Plugin,
   "Astrolabe",
   "The Astrolabe plugin provides a means to create and modify orbital mission sequeunces during a simulation.",
   "warlock")

WkAstrolabe::Plugin::Plugin(const QString& aPluginName, size_t aUniqueId)
   : warlock::PluginT<SimInterface>{aPluginName, aUniqueId}
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   wsfg::spaceg::AstrolabeConfig config{true};

   mDockWidgetPtr = new DockWidget{config, mInterfacePtr.get(), mainWindowPtr};
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   mEventInterfacePtr = new EventInterface{*mDockWidgetPtr, this};
}

void WkAstrolabe::Plugin::BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      auto platformName = QString::fromStdString(aEntityPtr->GetName());
      if (mDockWidgetPtr->IsRelevantPlatform(platformName))
      {
         QAction* actionPtr = new QAction(QString("Open with Astrolabe"), aMenuPtr);
         actionPtr->setIcon(QIcon::fromTheme("satellite"));
         aMenuPtr->addAction(actionPtr);
         connect(actionPtr,
                 &QAction::triggered,
                 [this, platformName]() { mDockWidgetPtr->HandlePlatformSelectionRequest(platformName); });
      }
   }
}

void WkAstrolabe::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*mEventInterfacePtr);
}
