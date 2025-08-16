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

#include "AcoImporterPlugin.hpp"

#include <QMenu>

#include "ImporterDialog.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   AcoImporter::Plugin,
   "ACO/ATO Importer",
   "The ACO/ATO Importer plugin creates a dialog, under the Tools menu, that allows the import of ACO/ATO Messages",
   "wizard")

AcoImporter::Plugin::Plugin(const QString& aPluginName, const size_t aId)
   : wizard::Plugin(aPluginName, aId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDialog                        = new Dialog(mainWindowPtr);

   // toolbar code
   QMenu* toolsMenu     = mainWindowPtr->GetMenuByName("Tools");
   QMenu* importersMenu = new QMenu("Importers", toolsMenu);
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

   if (importersMenu)
   {
      wkf::Action* dlgActionAcoPtr = new wkf::Action("ACO Importer ...", mainWindowPtr);
      wkf::Action* dlgActionAtoPtr = new wkf::Action("ATO Importer ...", mainWindowPtr);

      connect(dlgActionAcoPtr,
              &QAction::triggered,
              mDialog.data(),
              [this](void) -> void
              {
                 this->mDialog.data()->UnregisterAtoRequirements();
                 this->mDialog.data()->RegisterAcoRequirements();
                 this->mDialog.data()->show();
              });
      connect(dlgActionAtoPtr,
              &QAction::triggered,
              mDialog.data(),
              [this](void) -> void
              {
                 this->mDialog.data()->UnregisterAcoRequirements();
                 this->mDialog.data()->RegisterAtoRequirements();
                 this->mDialog.data()->show();
              });
      importersMenu->addAction(dlgActionAcoPtr);
      importersMenu->addAction(dlgActionAtoPtr);
   }
}
