// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "DialogBuilderPlugin.hpp"

#include "WkSimEnvironment.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkDialogBuilder::Plugin,
                          "Dialog Builder",
                          "The Dialog Builder plugin is responsible for managing the creation and display of custom "
                          "dialogs used for executing AFSIM scripts.",
                          "warlock")

WkDialogBuilder::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::Plugin(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new PrefWidget())
   , mSimInterfacePtr(simEnv.GetScriptSimInterface())
{
   connect(mPrefWidgetPtr->GetPreferenceObject(), &PrefObject::DialogDefinitionChanged, this, &Plugin::DialogChangedCB);

   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   wkf::Action*     scriptInputs  = new wkf::Action("Script Inputs", mainWindowPtr);
   mainWindowPtr->addAction(scriptInputs);

   mScriptInputsDockWidget = new ScriptInputsDockWidget(mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, mScriptInputsDockWidget);
   mScriptInputsDockWidget->hide();
}

QList<wkf::PrefWidget*> WkDialogBuilder::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void WkDialogBuilder::Plugin::DialogChangedCB()
{
   const auto& prefs = mPrefWidgetPtr->GetPreferenceObject()->GetPreferences();

   QStringList dialogsToRemove;
   // Check to see if any DockWidgets exist with a name that no longer exist in preferences.
   // If so, add DockWidget to the list of dialogsToRemove
   for (auto& widget : mDockWidgets)
   {
      bool    dialogDefinitionExists = false;
      QString name                   = widget.first;
      for (const auto& def : prefs.dialogs)
      {
         if (def.name == name)
         {
            dialogDefinitionExists = true;
            break;
         }
      }
      if (!dialogDefinitionExists)
      {
         dialogsToRemove.push_back(name);
      }
   }

   // Loop over dialogsToRemove and remove each one
   for (const auto& name : dialogsToRemove)
   {
      for (auto& widget : mDockWidgets)
      {
         if (widget.first == name)
         {
            delete widget.second;
            mDockWidgets.erase(name);
            break;
         }
      }
   }

   // Update and Create dialogs based on current preferences.
   for (const auto& def : prefs.dialogs)
   {
      bool dockWidgetExists = false;
      for (auto& widget : mDockWidgets)
      {
         if (widget.first == def.name)
         {
            dockWidgetExists = true;
            widget.second->SetDefinition(def);
            break;
         }
      }
      if (!dockWidgetExists)
      {
         // Create a dock widget and add it to the main window
         wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
         auto             dock          = new DockWidget(def, mSimInterfacePtr, mScriptInputsDockWidget, mainWindowPtr);
         mDockWidgets[def.name]         = dock;
         mainWindowPtr->AddDockWidget(Qt::RightDockWidgetArea, dock, "Dialog Builder");
         dock->hide();
      }
   }
}

void WkDialogBuilder::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   QMap<QString, DialogDef> dialogMap = mPrefWidgetPtr->GetPreferenceObject()->GetPreferences().dialogs;
   if (!dialogMap.isEmpty())
   {
      auto plat = dynamic_cast<wkf::Platform*>(aEntityPtr);
      if (plat)
      {
         QMenu* scriptsMenu = new QMenu(QString("Scripts"), aMenu);
         scriptsMenu->setIcon(QIcon::fromTheme("script"));

         bool        platformScriptsFound = false;
         QStringList scriptNames;
         for (auto&& dialog : dialogMap)
         {
            for (auto&& script : dialog.scripts)
            {
               auto typeList = plat->GetTypeList();
               if ((script.context == warlock::script::Context::PLATFORM) &&
                   (script.platformType.isEmpty() ||
                    std::find(typeList.begin(), typeList.end(), script.platformType.toStdString()) != typeList.end()))
               {
                  scriptNames.push_back(dialog.name.remove(' ') + '.' + script.displayName.remove(' '));
                  platformScriptsFound = true;
               }
            }
         }

         if (platformScriptsFound)
         {
            for (auto& dockWidget : mDockWidgets)
            {
               for (auto&& action : dockWidget.second->GetActions())
               {
                  if (std::find(scriptNames.begin(), scriptNames.end(), action->GetUniqueName()) != scriptNames.end())
                  {
                     scriptsMenu->addAction(action);
                  }
               }
            }
            aMenu->addMenu(scriptsMenu);
         }
      }
   }
}
