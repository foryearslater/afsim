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

#ifndef EVENT_OUTPUT_PLUGIN_HPP
#define EVENT_OUTPUT_PLUGIN_HPP

#include "Dialog.hpp"
#include "UtQtUiPointer.hpp"
#include "core/Plugin.hpp"

namespace wizard
{

class Editor;
class Project;

//! Namespace of the EventOutput plug-in
namespace EventOutput
{

//! Provides facilities for editing csv_event_output and event_output blocks in AFSIM Wizard
class Plugin : public wizard::Plugin
{
public:
   //! Constructs a Plugin
   //! @param aPluginName is the name of the plug-in
   //! @param aUniqueId is the unique identifier of the plug-in
   Plugin(QString aPluginName, size_t aUniqueId);
   //! Destructs a Plugin
   ~Plugin() override = default;

private slots:
   //! Function that runs when a project is opened
   //! @param aProjectPtr is the wizard::Project that was opened (unused)
   void OnProjectOpened(wizard::Project* aProjectPtr);

   //! Function that runs when a project is closed
   //! @param aProjectPtr is the wizard::Project that was closed (unused)
   void OnProjectClosed(wizard::Project* aProjectPtr);

   void OnEditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);

   //! Creates and starts the EventOutput::Dialog
   void StartEventOutputDialog();
   //! Destroys the EventOutput::Dialog, causing a force close
   void ForceCloseEventOutputDialog();

   //! Disable the EventOutput::Dialog actions
   void DisableEventOutputDialogActions();
   //! Enable the EventOutput::Dialog actions
   void EnableEventOutputDialogActions();

private:
   //! The Tools menu action for the EventOutput::Dialog
   ut::qt::UiPointer<wkf::Action> mToolsActionPtr;
   //! The context menu action for the EventOutput::Dialog
   ut::qt::UiPointer<wkf::Action> mContextActionPtr;
   //! The EventOutput::Dialog
   ut::qt::UiPointer<Dialog> mDialogPtr;
};

} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_PLUGIN_HPP
