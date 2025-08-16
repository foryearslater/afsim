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

#ifndef WARLOCKAPPLICATIONEXTENSION_HPP
#define WARLOCKAPPLICATIONEXTENSION_HPP

#include "WsfApplicationExtension.hpp"

class WarlockApplicationExtension : public WsfApplicationExtension
{
public:
   WarlockApplicationExtension();

   static WarlockApplicationExtension* Find(WsfApplication& aApplication);

   // Notifications from WsfApplication to support 'standard application' methods.
   int  ProcessCommandLine(WsfStandardApplication::Options& aOptions, int aArgc, char* aArgv[]) override;
   void PrintCommandLineArguments() const override;

   void ProcessDisplayConsoleWindow() const;

   const std::string& GetConfigFile() const { return mConfigFile; }
   const std::string& GetPermissionFile() const { return mPermissionFile; }
   bool               GetImportConfigFile() const { return mImportConfigFile; }
   bool               GetLoadPreviousScenario() const { return mLoadPreviousScenario; }
   bool               GetStartMinimized() const { return mStartMinimized; }

   void ScenarioCreated(WsfScenario& aScenario) override;

private:
   std::string mConfigFile;
   std::string mPermissionFile;
   bool        mImportConfigFile;
   bool        mShowConsole;
   bool        mLoadPreviousScenario;
   bool        mStartMinimized;
};

#endif
