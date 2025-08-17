// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

//! This is a main() application routine for one or more processors that
//! accomplish some weapon utility function.  Usually, it will be used to
//! launch an iterative series of weapons to calculate Firing Tables,
//! Error Tables, Launch Acceptable Regions (LARs), or other tabularized
//! data.

#include <cstring>
#include <iostream>

// Includes all of the optional projects which can be compiled with WSF
#include "TargetMover.hpp"
#include "ToolManager.hpp"
#include "UtException.hpp"
#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "UtProcessorTime.hpp"
#include "UtWallClock.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfEventStepSimulation.hpp"
#include "WsfMoverTypes.hpp"
#include "WsfPluginManager.hpp"
#include "WsfScenario.hpp"
#include "WsfStandardApplication.hpp"
#include "script/WsfScriptManager.hpp"
#include "weapon_tools_version_defines.hpp"
#include "wsf_extensions.hpp"

namespace
{
void ShowHelp()
{
   auto logInfo = ut::log::info() << "Usage: weapon_tools file1 [file2 ...]";
   logInfo.AddNote() << "weapon_tools -? | -help";
   logInfo.AddNote() << "file1 [file2 ...]";
   logInfo.AddNote() << "Specify the name(s) of the simulation input files";
}


class WeaponToolsExtension : public WsfApplicationExtension
{
public:
   // =================================================================================================
   WeaponToolsExtension()
      : WsfApplicationExtension()
   {
   }

   // =================================================================================================
   void ScenarioCreated(WsfScenario& aScenario) override
   {
      aScenario.RegisterExtension("weapon_tools", ut::make_unique<ToolManager>(aScenario));

      // Used when a target must flee away from its shooter...
      WsfMoverTypes::Get(aScenario).Add("TARGET_MOVER", ut::make_unique<TargetMover>(aScenario));
   }

   // virtual
   int ProcessCommandLine(WsfStandardApplication::Options& aOptions, int aArgc, char* aArgv[]) override
   {
      if (strcmp(aArgv[0], "--") == 0)
      {
         return 1;
      }
      else if ((strcmp(aArgv[0], "-?") == 0) || (strcmp(aArgv[0], "-h") == 0))
      {
         ShowHelp();
         return 0;
      }
      return 0;
   }
};
} // namespace

int weapon_tools(WsfStandardApplication& app)
{
   // Register our application extension...
   app.RegisterExtension("weapon_tools_ext", ut::make_unique<WeaponToolsExtension>());
   app.ExtensionDepends("weapon_tools_ext", "wsf_mil", true);

   // Load built-in extensions (defined in wsf_extensions.hpp)
   RegisterBuiltinExtensions(app);

   // Load optional extensions (defined in wsf_extensions.hpp)
   RegisterOptionalExtensions(app);

   WsfScenario scenario(app);

   // Process command-line arguments
   WsfStandardApplication::Options options;

   try
   {
      app.ProcessCommandLine(options);
      // Read WSF input files into the scenario
      app.ProcessInputFiles(scenario, options.mInputFiles);
   }
   catch (WsfApplication::Exception& e)
   {
      ut::log::error() << e.what();
      return e.GetReturnCode();
   }

   app.ProcessCommandLineCommands(scenario, options);

   if (options.mRunMode == WsfStandardApplication::cRUN_SCENARIO)
   {
      ToolManager* toolManagerPtr = static_cast<ToolManager*>(scenario.FindExtension("weapon_tools"));

      // Initialize all of the objects in the simulation.
      WsfEventStepSimulation sim(scenario, 0);
      try
      {
         ut::log::info() << "Initializing simulation";
         sim.Initialize();
         toolManagerPtr->Initialize(sim);
      }
      catch (...)
      {
         ut::log::error() << "Initialization of simulation failed!";
         return 1;
      }

      // Run the simulation

      ut::log::info() << "Starting simulation";
      UtWallClock     wallClock;
      UtProcessorTime procTime;

      double messageInterval = 5000.0;
      double lastMessageTime = 0.0;
      double simTime         = 0.0;

      // Do not want the sim to end early.  The ToolManager handles this...
      sim.SetEndTime(1.0E10);
      sim.Start();

      while (!toolManagerPtr->AllToolsDone())
      {
         if ((simTime - lastMessageTime) >= messageInterval)
         {
            // cout << "T=" << simTime << '\n';
            lastMessageTime += messageInterval;
         }

         simTime = sim.AdvanceTime();
      }

      sim.Complete(simTime);

      auto logInfo = ut::log::info() << "Simulation complete";
      logInfo.AddNote() << "Elapsed Wall Clock Time: " << wallClock.GetClock();
      logInfo.AddNote() << "Elapsed Processor Time : " << procTime.GetTime();
   }
   return 0;
}

#ifndef WEAPON_TOOLS_BUILD_PLUGIN

int main(int argc, char* argv[])
try
{
   ut::SetApplicationErrorHandling(&ut::TerminateHandler);
   ut::SetupApplicationLog("Weapon Tools", WEAPON_TOOLS_VERSION, "weapon-tools-exception.log");

   WsfStandardApplication app("weapon_tools", argc, argv);

   return weapon_tools(app);
}
catch (...)
{
   ut::ReportException("Weapon Tools", std::current_exception());
   return 1;
}


#else
//////////////////////////////////////////////////////////////////////////////////
// Load weapon tools as a plugin
// Usage:
//         mission_x.exe --weapon-tools my_weapon_tool_inputs.txt

#include "UtPlugin.hpp"

WSF_PLUGIN_DEFINE_VERSION

extern "C"
{
   UT_PLUGIN_EXPORT void WSF_PluginSetup(WsfApplication* aApplicationPtr)
   {
      WsfStandardApplication* applicationPtr = dynamic_cast<WsfStandardApplication*>(aApplicationPtr);
      if (applicationPtr == 0)
      {
         ut::log::fatal() << "weapon_tools plugin must only be used with a WsfStandardApplication";
         exit(1);
      }
      int    argc;
      char** argv;
      applicationPtr->GetCommandLineArguments(argc, argv);
      if (argc > 1 && argv[1] == std::string("--weapon-tools"))
      {
         // At this point, the WsfApplication has been constructed, but no extensions should be added yet.
         // Here we just load the application with weapon-tool's extensions.
         //
         // A better solution would be to defer execution until later, letting the application choose which
         // extensions are used.
         applicationPtr->RemoveCommandLineOptions(1, 1);
         int returnCode = weapon_tools(*applicationPtr);
         // exit() isn't the nicest way to quit, but WSF has no way to delegate all execution to a plugin.
         exit(returnCode);
      }
   }
}

#endif
