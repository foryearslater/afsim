// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: ScenarioAnalyzerPlugin.hpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/

#ifndef SCENARIO_ANALYZER_IDE_PLUGIN_HPP
#define SCENARIO_ANALYZER_IDE_PLUGIN_HPP

//#include <vld.h>

#include <vector>

#include "Plugin.hpp"
#include "Gui.hpp"
#include "WkfAction.hpp"

namespace ScenarioAnalyzer
{
struct ScenarioAnalyzerGui;

struct ScenarioAnalyzerPlugin
{
   AbsoluteDir suiteDir;
   AbsoluteDir sessionNoteDir;
   AbsoluteDir debugSuiteDir;
   AbsoluteDir debugSessionNoteDir;
   AbsoluteFile wsfExecExe;
   wkf::Action* viewChecksAction;
   wkf::Action* viewResultsAction;
   QMetaObject::Connection connections[4];
};

class Plugin : public wizard::Plugin
{
   public:
      Plugin(const QString& aPluginName,
             const size_t   aUniqueId);
      ~Plugin() override = default;

      ScenarioAnalyzerGui gui;
      ScenarioAnalyzerPlugin plugin;
};

AbsoluteFile utPathToAbsoluteFile(UtPath path)
{
   path.MakeFullPath();
   return createAbsoluteFile(QString::fromStdString(path.GetNormalizedPath()));
}

AbsoluteDir utPathToAbsoluteDir(UtPath path)
{
   path.MakeFullPath();
   return createAbsoluteDir(QString::fromStdString(path.GetNormalizedPath()));
}

class ScenarioAnalyzerPluginRegistration : public wizard::Plugin
{
   public:
      ScenarioAnalyzerPluginRegistration(const QString& aPluginName,
                                         const size_t   aUniqueId);

      virtual ~ScenarioAnalyzerPluginRegistration();

      ScenarioAnalyzerGui gui;
      ScenarioAnalyzerPlugin plugin;
};
}

#endif
