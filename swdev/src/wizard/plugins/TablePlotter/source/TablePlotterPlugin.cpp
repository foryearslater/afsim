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

#include "TablePlotterPlugin.hpp"

#include <regex>

#include <QMenu>

#include "TablePlotterDialog.hpp"
#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(TablePlotter::Plugin,
                          "TablePlotter",
                          "Provides a graphical representation of tables in Wizard.",
                          "wizard")

TablePlotter::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   PopulateUtCurveList();
   // provide access from tools menu
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenuPtr  = mainWindowPtr->GetMenuByName("Tools");

   wkf::Action* mStartAction = new wkf::Action(QIcon::fromTheme("table_plotter"), "Table Plotter...", mainWindowPtr);
   connect(mStartAction, &QAction::triggered, [this]() { Start(); });
   toolsMenuPtr->addAction(mStartAction);

   // provide access via right clicking in the wizard editor
   connect(wizSignals, &wizard::Signals::ActionsForCommandContentsRequested, this, &Plugin::ActionsForCommandContentsRequested);
   // open csv from project browser
   connect(wizSignals, &wizard::Signals::ActionsForFileRequested, this, &Plugin::ActionsForFileRequested);
}

// a new dialog is created every time the user wants to display a plot, to allow for easier comparison of data
void TablePlotter::Plugin::Start()
{
   TablePlotterDialog* mainDialog = new TablePlotterDialog(wkfEnv.GetMainWindow());
   mainDialog->show();
}

void TablePlotter::Plugin::ActionsForCommandContentsRequested(const std::string& aContent,
                                                              const std::string& aPath,
                                                              QMenu*             aMenuPtr,
                                                              QWidget*           aParentWidgetPtr)
{
   TablePlotterDialog* mainDialog = new TablePlotterDialog(wkfEnv.GetMainWindow());
   std::string         command    = aContent.substr(0, aContent.find(' '));
   if (command == "irregular_table" || command == "regular_table" || command == "curve" ||
       std::find(mValidCurveStrings.begin(), mValidCurveStrings.end(), command) != mValidCurveStrings.end())
   {
      QAction* actionPtr = aMenuPtr->addAction(QIcon::fromTheme("table_plotter"), "Plot Table Data...");
      QObject::connect(actionPtr,
                       &QAction::triggered,
                       [aContent, mainDialog, this]() { mainDialog->LoadTextInput(aContent); });
   }
   // Currently external data files are used to store large tables, including the table commands and format
   // If this changes to only store the numbers (no format, commands, etc.), then this case will not need to be handled
   else if (command == "file")
   {
      // extract filename from the command, then format the path based on OS
      std::regex  target("file ");
      std::string replace           = "";
      std::string filepath          = aPath + std::regex_replace(aContent, target, replace);
      QString     separatorFilePath = QDir::fromNativeSeparators(QString::fromStdString(filepath));
      QAction*    actionPtr = aMenuPtr->addAction(QIcon::fromTheme("table_plotter"), "Send file to Table Plotter...");
      QObject::connect(actionPtr,
                       &QAction::triggered,
                       [separatorFilePath, mainDialog, this]() { mainDialog->LoadFile(separatorFilePath); });
   }
}

void TablePlotter::Plugin::ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
{
   QString type = aFileInfo.completeSuffix();
   if (aFileInfo.completeSuffix() == "csv")
   {
      TablePlotterDialog* mainDialog = new TablePlotterDialog(wkfEnv.GetMainWindow());
      connect(aMenu->addAction(QIcon::fromTheme("table_plotter"), "Send file to Table Plotter..."),
              &QAction::triggered,
              [this, aFileInfo, mainDialog]() { mainDialog->LoadFile(aFileInfo.absoluteFilePath()); });
   }
}

void TablePlotter::Plugin::PopulateUtCurveList()
{
   mValidCurveStrings = {
      // P6DofAeroCoreObject
      "cmq_mach_table",
      "cnr_mach_table",
      "cnp_mach_table",
      "clp_mach_table",
      "clr_mach_table",
      // P6DofAtmosphere
      "density_altitude_table",
      "pressure_altitude_table",
      "temperature_altitude_table",
      "sonic_speed_altitude_table",
      // P6DofCommonController
      "cl_max_mach_table",
      "cl_min_mach_table",
      "alpha_max_mach_table",
      "alpha_min_mach_table",
      // P6DofFlightControlSystem
      "simple_table",
      "angle_mapping_table",
      "angle_mapping_auto_table",
      "angle_mapping_manual_table",
      // P6DofJetEngineObject
      "spin_up_table_mil_per_sec",
      "spin_down_table_mil_per_sec",
      "spin_up_table_ab_per_sec",
      "spin_down_table_ab_per_sec",
      "thrust_table_idle",
      "thrust_table_mil",
      "thrust_table_ab",
      // P6DofManualPilotAugmentedControls
      "pitch_control_mapping_table",
      "roll_control_mapping_table",
      "yaw_control_mapping_table",
      // P6DofRocketLiquidPropellantObject
      "normalized_thrust_vs_alt",
      "isp_vs_alt",
      // P6DofRocketSolidPropellantObject
      "thrust_vs_time_sealevel",
      "thrust_vs_time_vacuum",
      // WsfBrawlerProcessor
      "clmax_vs_mach_table",
      "alpha_at_clmax_vs_mach_table"
      // P6DofManualPilotSimpleControls and P6DofSyntheticPilot (Same as P6DofManualPilotAugmentedControls)
   };
}