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

// Utility Includes
#include "UtQtMemory.hpp"

// WKF Includes
#include "WkfMainWindow.hpp"

// TaskList Includes
#include "TaskListDockWidget.hpp"
#include "TaskListPlugin.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(TaskList::Plugin,
                          "Task List Plug-in",
                          "This plug-in consolidates tasks in a table.\n"
                          "The scenario developer can refer to the table as a reminder to complete the tasks.",
                          "wizard");

namespace TaskList
{
Plugin::Plugin(QString aPluginName, size_t aUniqueId) noexcept
   : wizard::Plugin(std::move(aPluginName), aUniqueId)
   , mPreferences(ut::qt::make_qt_ptr<PrefWidget>())
{
   auto* taskList = ut::qt::make_qt_ptr<DockWidget>();
   wkfEnv.GetMainWindow()->AddDockWidgetToViewMenu(taskList);
}

QList<wkf::PrefWidget*> Plugin::GetPreferencesWidgets() const
{
   return QList<wkf::PrefWidget*>{mPreferences.data()};
}

} // end namespace TaskList
