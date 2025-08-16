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

// WIZARD
#include "RunEnvManager.hpp"
#include "Runner.hpp"
#include "UtQtException.hpp"
#include "ViExport.hpp"

extern void RedirectIOToConsole();

int VI_EXPORT wizard_main(int         argc,
                          char*       argv[],
                          int         aVersionMajor,
                          int         aVersionMinor,
                          int         aVersionPatch,
                          const char* aCompanyStr,
                          const char* aCompanyDomainStr,
                          const char* aProductNameStr,
                          const char* aProductVersionStr)
try
{
   ut::SetApplicationErrorHandling(&ut::qt::TerminateHandler);

   // Set the version data in the run environment manager
   wizard::RunEnvManager::SetVersion(aVersionMajor, aVersionMinor, aVersionPatch);

   // Check for an environment variable; can be used with release version
   const char* consoleEnv = getenv("WIZARD_CONSOLE"); // WIZARD ENV VAR

   // Decide whether to show the console window
   bool showConsole = consoleEnv && *consoleEnv != '0';

   std::vector<char*> argv2;

   // Check for a command line option
   for (int i = 1; i < argc; ++i)
   {
      std::string cmd(argv[i]);
      if (cmd == "-console")
      {
         showConsole = true;
      }
      else
      {
         argv2.push_back(argv[i]);
      }
   }

   // If running in debug mode then always show the console window
#if !NDEBUG
   showConsole = true;
#endif

   if (showConsole)
   {
      RedirectIOToConsole();
   }

   // Create the wizard environment
   int            argc2 = static_cast<int>(argv2.size());
   wizard::Runner runner(argc2, argv2.data(), aCompanyStr, aCompanyDomainStr, aProductNameStr, aProductVersionStr);
   // Inner try-catch to create exception report popup before QApplication close
   try
   {
      runner.run();
      return 0;
   }
   catch (...)
   {
      ut::qt::ReportException("Wizard", std::current_exception());
      return 1;
   }
}
catch (...)
{
   ut::qt::ReportException("Wizard", std::current_exception());
   return 1;
}
