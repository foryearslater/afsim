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

#include "CrdImporterPlugin.hpp"

#include <QMenu>

#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   CrdImporter::Plugin,
   "CRD Importer",
   "Allows the user to read a Common Route Definition (CRD) file and convert it into AFSIM input.",
   "wizard")

CrdImporter::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDialogPtr(new Dialog(wkfEnv.GetMainWindow()))
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenu     = mainWindowPtr->GetMenuByName("Tools");
   QMenu*           importersMenu = new QMenu("Importers", toolsMenu);
   if (toolsMenu)
   {
      QMenu* iMenu = toolsMenu->findChild<QMenu*>("Importers");
      if (!iMenu)
      {
         importersMenu->setObjectName("Importers");
         toolsMenu->addMenu(importersMenu);
      }
      else
      {
         importersMenu = iMenu;
      }
   }
   // TODO: Get an icon to represent the plugin on the tools menu
   if (importersMenu)
   {
      wkf::Action* startAction = new wkf::Action(QIcon::fromTheme("crd_importer"), "CRD Importer ...", mainWindowPtr);
      connect(startAction, &QAction::triggered, [this]() { mDialogPtr->show(); });

      importersMenu->addAction(startAction);
   }
}
