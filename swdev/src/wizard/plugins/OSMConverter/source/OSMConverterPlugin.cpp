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

#include "OSMConverterPlugin.hpp"

#include <QMenu>

#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(OSMConverter::Plugin, "OSMConverter", "Converts OSM ways into AFSIM routes with auxiliary data", "wizard")

OSMConverter::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDialog(new OSMConverterDialog(wkfEnv.GetMainWindow()))
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenuPtr  = mainWindowPtr->GetMenuByName("Tools");

   // TODO: Get an icon to represent the plugin on the tools menu
   wkf::Action* startAction = new wkf::Action(QIcon::fromTheme("osm_converter"), "OSM Converter...", mainWindowPtr);
   connect(startAction, &QAction::triggered, [this]() { mDialog->show(); });
   toolsMenuPtr->addAction(startAction);
}
