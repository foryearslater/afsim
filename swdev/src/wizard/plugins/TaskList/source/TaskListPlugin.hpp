// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2022 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TASK_LIST_PLUGIN_HPP
#define TASK_LIST_PLUGIN_HPP

// Utility Includes
#include "UtQtUiPointer.hpp"

// Wizard Includes
#include "core/Plugin.hpp"

// TaskList Includes
#include "TaskListPrefWidget.hpp"

//! Namespace of the TaskList:Plugin
namespace TaskList
{
//! Provides a task list in Wizard
//! @par details
//!   This plug-in provides a task list in Wizard.
//!   This allows the user to add tasks in their scenarios as a reminder to complete later.
class Plugin : public wizard::Plugin
{
public:
   //! Constructs a Plugin
   //! @param aPluginName is the name of the plug-in
   //! @param aUniqueId is the unique identifier of the plug-in
   Plugin(QString aPluginName, size_t aUniqueId) noexcept;
   //! Destructs a Plugin
   ~Plugin() override = default;

   //! Get the list of wkf::PrefWidgets
   //! @return the list of wkf::PrefWidgets
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   //! The TaskList::PrefWidget
   ut::qt::UiPointer<PrefWidget> mPreferences;
};

} // end namespace TaskList

#endif // TASK_LIST_PLUGIN_HPP
