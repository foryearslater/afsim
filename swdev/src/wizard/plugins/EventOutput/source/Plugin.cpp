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

#include "Plugin.hpp"

#include "ProjectWorkspace.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "editor/Editor.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   wizard::EventOutput::Plugin,
   "Event Output Editor Plug-in",
   "This plug-in aids in writing csv_event_output and event_output blocks in AFSIM scenario input files",
   "wizard");

namespace wizard
{
namespace EventOutput
{

Plugin::Plugin(QString aPluginName, size_t aUniqueId)
   : wizard::Plugin(std::move(aPluginName), aUniqueId)
{
   // provide access from Tools menu
   auto mainWindowPtr = wkfEnv.GetMainWindow();
   auto toolsMenuPtr  = mainWindowPtr->GetMenuByName("Tools");

   // create the "event_output Editor" menu action
   mToolsActionPtr = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("checklist"), "Event Output Editor...", nullptr);
   mContextActionPtr = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("checklist"), "Event Output Editor...", nullptr);

   // do the necessary connections
   connect(mToolsActionPtr.data(), &wkf::Action::triggered, this, &Plugin::StartEventOutputDialog);
   // enable the tools of this plug-in once the full parse is complete (after the syntax highlighting appears)
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::FullParseComplete, this, &Plugin::OnProjectOpened);
   // disable the tools of this plug-in when project is closed
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &Plugin::OnProjectClosed);
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &Plugin::OnEditorContextMenu);
   connect(wizSignals, &wizard::Signals::ProjectStartupFilesChanged, this, &Plugin::ForceCloseEventOutputDialog);

   // add the "Event Output Editor" action to the "Tools" menu
   toolsMenuPtr->addAction(mToolsActionPtr);

   // disable the "Event Output Editor" action on start-up
   mToolsActionPtr->setDisabled(true);
}

void Plugin::OnProjectOpened(wizard::Project* /* aSrc */)
{
   ForceCloseEventOutputDialog();
   EnableEventOutputDialogActions();
}

void Plugin::OnProjectClosed(wizard::Project* /* aSrc */)
{
   ForceCloseEventOutputDialog();
   DisableEventOutputDialogActions();
}

void Plugin::OnEditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   if (aEditorPtr)
   {
      aMenu.addAction(mContextActionPtr);
      connect(mContextActionPtr.data(), &QAction::triggered, this, &Plugin::StartEventOutputDialog);
   }
}

void Plugin::StartEventOutputDialog()
{
   if (!mDialogPtr)
   {
      DisableEventOutputDialogActions();
      mDialogPtr = ut::qt::make_qt_ptr<Dialog>(false, wkfEnv.GetMainWindow(), Qt::WindowFlags());
      mDialogPtr->show();
      // Note:  if it is decided not to disable text interaction (or other), then enable modality again
      // Note:  this is modal, as the user should not need to refer back to the text editor
      // dialog->exec();
      connect(mDialogPtr.data(),
              &Dialog::Closed,
              [this]()
              {
                 ForceCloseEventOutputDialog();
                 EnableEventOutputDialogActions();
              });
   }
}

void Plugin::ForceCloseEventOutputDialog()
{
   if (mDialogPtr)
   {
      mDialogPtr->deleteLater();
   }
}

void Plugin::DisableEventOutputDialogActions()
{
   mToolsActionPtr->setDisabled(true);
   mContextActionPtr->setDisabled(true);
}

void Plugin::EnableEventOutputDialogActions()
{
   mToolsActionPtr->setEnabled(true);
   mContextActionPtr->setEnabled(true);
}

} // namespace EventOutput
} // namespace wizard
