// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WarlockApplicationExtension.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstring>
#include <fstream>
#include <iostream>

#include <QMessageBox>

#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "WkEventPipe.hpp"
#include "WkObserver.hpp"
#include "WsfApplication.hpp"
#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"

namespace
{
class WarlockScenarioExtension : public WsfScenarioExtension
{
public:
   void AddedToScenario() override;
   void SimulationCreated(WsfSimulation& aSimulation) override;
};

void WarlockScenarioExtension::AddedToScenario()
{
   // If the "event_pipe" extension has been defined then hook things up so our events will also be logged.
   WsfEventPipeExtension* eventPipePtr = WsfEventPipeExtension::Find(GetScenario());
   if (eventPipePtr)
   {
      wk::EventPipe::RegisterEvents(*eventPipePtr);
   }
}

void WarlockScenarioExtension::SimulationCreated(WsfSimulation& aSimulation)
{
   aSimulation.RegisterExtension("wk_observer", ut::make_unique<WkSimulationObserver>());
   aSimulation.RegisterExtension("wk_eventpipe", ut::make_unique<wk::EventPipeInterface>());
}
} // namespace

static bool IsFileReadable(const char* aFilename) noexcept
{
   return std::ifstream(aFilename).is_open();
}

WarlockApplicationExtension::WarlockApplicationExtension()
   : WsfApplicationExtension()
   , mConfigFile("")
   , mImportConfigFile(true)
#ifdef _DEBUG
   , mShowConsole(true)
#else
   , mShowConsole(false)
#endif
   , mLoadPreviousScenario(false)
   , mStartMinimized(false)
{
}

WarlockApplicationExtension* WarlockApplicationExtension::Find(WsfApplication& aApplication)
{
   return static_cast<WarlockApplicationExtension*>(aApplication.FindExtension("warlock"));
}

int WarlockApplicationExtension::ProcessCommandLine(WsfStandardApplication::Options& aOptions, int aArgc, char* aArgv[])
{
   if (0 == strcmp(aArgv[0], "-cf"))
   {
      if (aArgc > 1)
      {
         if (!IsFileReadable(aArgv[1]))
         {
            QMessageBox::warning(nullptr,
                                 "Invalid Argument",
                                 QString("-cf file does not exist or cannot be opened.\nFile: %1").arg(aArgv[1]));
            auto out = ut::log::warning() << "-cf file does not exist or cannot be opened.";
            out.AddNote() << "File: " << aArgv[1];
         }

         mConfigFile       = aArgv[1];
         mImportConfigFile = false;
         return 2; // We processed two arguments from the command line
      }
      else
      {
         QMessageBox::warning(nullptr, "Invalid Argument", "-cf requires a filename to be provided.");
         ut::log::error() << "-cf requires a filename to be provided.";
      }
   }
   else if (0 == strcmp(aArgv[0], "-icf"))
   {
      if (aArgc > 1)
      {
         if (!IsFileReadable(aArgv[1]))
         {
            QMessageBox::warning(nullptr,
                                 "Invalid Argument",
                                 QString("-icf file does not exist or cannot be opened.\nFile: %1").arg(aArgv[1]));
            auto out = ut::log::warning() << "-icf file does not exist or cannot be opened.";
            out.AddNote() << "File: " << aArgv[1];
         }

         mConfigFile       = aArgv[1];
         mImportConfigFile = true;
         return 2; // We processed two arguments from the command line
      }
      else
      {
         QMessageBox::warning(nullptr, "Invalid Argument", "-icf requires a filename to be provided.");
         ut::log::error() << "-icf requires a filename to be provided.";
      }
   }
   else if (0 == strcmp(aArgv[0], "-console"))
   {
      mShowConsole = true;
   }
   else if (0 == strcmp(aArgv[0], "-permission_file"))
   {
      if (aArgc > 1)
      {
         if (!IsFileReadable(aArgv[1]))
         {
            QMessageBox::warning(nullptr,
                                 "File Not Found",
                                 QString("-permission_file file does not exist or cannot be opened.\nFile: %1").arg(aArgv[1]));
            auto out = ut::log::warning() << "-permission_file file does not exist or cannot be opened.";
            out.AddNote() << "File: " << aArgv[1];
         }
         else
         {
            mPermissionFile = aArgv[1];
         }
         return 2; // We processed two arguments from the command line
      }
      else
      {
         QMessageBox::warning(nullptr, "Invalid Argument", "-permission_file requires a filename to be provided.");
         ut::log::error() << "-permission_file requires a filename to be provided.";
      }
   }
   else if (0 == strcmp(aArgv[0], "-ups") || 0 == strcmp(aArgv[0], "-use_previous_scenario"))
   {
      mLoadPreviousScenario = true;
   }
   else if (0 == strcmp(aArgv[0], "-minimized"))
   {
      mStartMinimized = true;
   }
   else
   {
      return 0;
   }
   return 1;
}

void WarlockApplicationExtension::PrintCommandLineArguments() const
{
   std::cout << "\n-cf <filename>               Use the specified configuration file,"
                "\n                             modifications will be saved to specified file"
                "\n-icf <filename>              Imports the specified configuration file,"
                "\n                             modifications will not be saved to specified file"
                "\n-permission_file <filename>  Use the specified permission file,"
                "\n                             prevents the user from editing permissions"
                "\n-console                     Enables the console window"
                "\n-ups                         Uses the previous scenario, if no scenario was specified"
                "\n-minimized                   The application will start minimized";
}

void WarlockApplicationExtension::ProcessDisplayConsoleWindow() const
{
#ifdef _WIN32
   if (mShowConsole)
   {
      bool has_console = ::AttachConsole(ATTACH_PARENT_PROCESS) == TRUE;
      if (!has_console)
      {
         // We weren't launched from a console, so make one.
         has_console = AllocConsole() == TRUE;
      }
      if (has_console)
      {
         for (auto& file : {stdout, stderr})
         {
            if (freopen("CONOUT$", "w", file))
            {
               setvbuf(file, nullptr, _IONBF, 0);
            }
         }
      }
   }
#endif
}

void WarlockApplicationExtension::ScenarioCreated(WsfScenario& aScenario)
{
   aScenario.RegisterExtension(GetExtensionName(), ut::make_unique<WarlockScenarioExtension>());
}

void Register_warlock(WsfApplication& aApplication)
{
   aApplication.RegisterFeature("warlock");
   aApplication.RegisterExtension("warlock", ut::make_unique<WarlockApplicationExtension>());
}
