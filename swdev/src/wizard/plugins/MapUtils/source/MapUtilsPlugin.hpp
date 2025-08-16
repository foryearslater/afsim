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

#ifndef MAPUTILSPLUGIN_HPP
#define MAPUTILSPLUGIN_HPP

#include <QMenu>

#include "Plugin.hpp"
#include "RotateScenarioDialog.hpp"
#include "TranslateScenarioDialog.hpp"
#include "VaCallbackHolder.hpp"
#include "WkfAction.hpp"

namespace wizard
{
class Editor;
class Project;
} // namespace wizard

namespace wkf
{
class AttachmentRoute;
}

namespace MapUtils
{
namespace ScenarioTransformation
{
namespace RotateScenario
{
class Dialog;
}
namespace TranslateScenario
{
class Dialog;
}
} // namespace ScenarioTransformation

//! @brief class definition of MapUtils plug-in
class Plugin : public wizard::Plugin
{
   Q_OBJECT
   using AvailableActions        = MapUtils::PluginUtil::WhatEntities;
   using Message                 = MapUtils::Message;
   using RotateMenuItem          = MapUtils::ScenarioTransformation::RotateScenario::Menu;
   using RotateScenarioDialog    = MapUtils::ScenarioTransformation::RotateScenario::Dialog;
   using TranslateMenuItem       = MapUtils::ScenarioTransformation::TranslateScenario::Menu;
   using TranslateScenarioDialog = MapUtils::ScenarioTransformation::TranslateScenario::Dialog;

public:
   static MapUtils::Plugin* Instance();

   //! @brief constructs the MapUtils plug-in
   //! @param aPluginName is the name of the plug-in
   //! @param aUniqueId is the unique ID of the plug-in
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   Plugin(Plugin&& /* aSrc */) = default;

   //! @brief destructs the MapUtils plug-in
   ~Plugin() override;

   Plugin& operator=(Plugin&& /* aSrc */) = default;

   //! @brief gets the list of actions
   //! @return the list of actions
   QList<wkf::Action*> GetActions() const override;

signals:
   //! @brief signal to force close the dialogs
   //! @param aDialogPtr is the particular dialog to force close
   void ForceClose(ScenarioTransformation::DialogInterface* aDialogPtr);

private slots:
   //! @brief function that runs when a project is opened
   //! @param aProjectPtr is the wizard::Project that was opened (unused)
   void OnProjectOpened(wizard::Project* aProjectPtr);

   //! @brief function that runs when a project is closed
   //! @param aProjectPtr is the wizard::Project that was closed (unused)
   void OnProjectClosed(wizard::Project* aProjectPtr);

   //! @brief function that runs when the project start-up file(s) is changed
   //! @param aMainFiles is the updated start-up file list
   void OnProjectStartupFilesChanged(const std::vector<UtPath>& aMainFiles);

   //! @brief starts the RotateScenario::Dialog
   //! @param aMode is the mode with which to start up the RotateScenario::Dialog
   void StartRotateScenario(const RotateMenuItem aMode);

   //! @brief starts the TranslateScenario::Dialog
   //! @param aMode is the mode with which to start up the TranslateScenario::Dialog
   void StartTranslateScenario(const TranslateMenuItem aMode);

   //! @brief connects the QMenu::aboutToShow signal to the UpdateAvailableActions slot
   void ConnectMapUtilitiesMenuActionAvailabilityUpdate();

   //! @brief disconnects the QMenu::aboutToShow signal from the UpdateAvailableActions slot
   void DisconnectMapUtilitiesMenuActionAvailabilityUpdate();

   //! @brief disables the actions of the Map Utilities menu
   void DisableMapUtilitiesMenuActions();

   //! @brief enables the actions of the Map Utilities menu
   //! @param aWhichOnes determines which actions are available
   void EnableMapUtilitiesMenuActions(const AvailableActions aWhichOnes);

   //! @brief slot for the "Delete" action in the entity context menu
   //! @param aEntityPtr is the entity in question
   void DeletePlatformActionHandler(wkf::Entity* aEntityPtr);

   //! @brief slot for the "Clone" action in the entity context menu
   //! @param aPlatformPtr is the platform (entity) in question
   void ClonePlatformActionHandler(wizard::Platform* aPlatformPtr);

   //! @brief slot for the "add Platform" action in the viewer context menu
   void CreatePlatformActionHandler();

   //! @brief slot for the editor context menu
   //! @param aEditorPtr is the current text editor
   //! @param aMenu is the editor context menu in question
   void OnEditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);

   //! @brief slot for requesting name suggestions (for text editor auto-complete)
   //! @param aNameType is the type of property for which the request was made
   //! @param aNameList is the list of suggestions for the property
   void OnRequestNameSuggestions(const std::string& aNameType, std::vector<std::string>& aNameList);

   //! @brief slot for when the epoch is updated
   //! @param aCalendar is the calendar in question
   void EpochUpdated(const UtCalendar& aCalendar);

protected:
   Plugin(const Plugin& /* aSrc */) = delete;

   Plugin& operator=(const Plugin& /* aSrc */) = delete;

private:
   //! @brief force closes any ScenarioTransformation dialogs that are open
   //! @note this is to prevent unintentional crashes due to updating the wizard::ProjectWorkspace
   void ForceCloseScenarioTransformationDialogs();

   //! @brief builds the "Map Utilities" menu
   //! @param aMenuPtr is the menu to build
   void BuildMapUtilitiesMenu(QMenu* aMenuPtr);

   //! @brief updates the available actions
   //! @note needed for EnableMapUtilitiesMenuActions
   void UpdateAvailableActions();

   //! @brief builds the entity context menu
   //! @param aMenu is the context menu
   //! @param aEntityPtr is the entity in question
   //! @note the entity context menu appears when the user right-clicks an entity or a selection of entities
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   //! @brief builds the viewer context menu
   //! @param aMenu is the context menu
   //! @param aViewerPtr is the viewer in question
   //! @note the viewer context menu appears when the user right-clicks anywhere on the map where there is no entity
   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;

   //! @brief populates the model menu
   void PopulateModelMenu();
   void DeleteCurrentlySelectedEntities();
   void CreateWaypointHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx);
   void CreateWaypointAtLocationHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx);

   //! @brief instance pointer
   //! @note needed for signal/slot connections in ScenarioTransformation dialogs
   static Plugin* mInstancePtr;

   //! @name member variables for the map entity context menu
   //@{

   //! @brief denotes the model menu
   QMenu mModelMenu;

   //! @brief denotes actions for the entity context menu
   QList<wkf::Action*> mActions;

   //@}

   //! @name list variables for the platform tools
   //@{

   //! @brief denotes the rotate menu
   QPointer<QMenu> mRotateMenuPtr;

   //! @brief denotes the translate menu
   QPointer<QMenu> mTranslateMenuPtr;

   //! @brief list containing the different modes for the RotateScenario::Dialog
   QList<QPair<RotateMenuItem, QPointer<wkf::Action>>> mRotateActionList;

   //! @brief list containing the different modes for the TranslateScenario::Dialog
   QList<QPair<TranslateMenuItem, QPointer<wkf::Action>>> mTranslateActionList;

   //! @brief denotes the RotateScenario::Dialog (only one at a time)
   QPointer<RotateScenarioDialog> mRotateDialogPtr;

   //! @brief denotes the TranslateScenario::Dialog (only one at a time)
   QPointer<TranslateScenarioDialog> mTranslateDialogPtr;

   //@}
};

} // namespace MapUtils
#endif
