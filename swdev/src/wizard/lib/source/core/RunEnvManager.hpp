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
#ifndef RUNENVMANAGER_HPP
#define RUNENVMANAGER_HPP

#include <string>
#include <vector>

#include "ViExport.hpp"

// Utilities
#include "UtPath.hpp"

// Forward Declaration
class UtDynamicLibrary;

namespace wizard
{
class VI_EXPORT RunEnvManager
{
public:
   static bool Initialize(const UtPath& aBinPath, std::string& aErrorString);

   static std::string GetVersionString(bool aDate = true);
   static void        SetVersion(int aMajor, int aMinor, int aPatch);

   static const UtPath& GetInstallRootDir();

   static const UtPath& GetBinDir();
   static const UtPath& GetDemosDir();
   static const UtPath& GetTrainingDir();
   static const UtPath& GetScenariosDir();
   static const UtPath& GetDocumentationDir();
   static const UtPath& GetDataDir();
   static const UtPath& GetGrammarDir();

   static bool HasDeveloperPaths();

   static std::vector<UtPath> GetDemoSearchPaths();

   static std::string DocumentationURL_Static(const std::string& aPageName, const std::string& aAnchor = std::string());

   static std::string DocumentationURL(const std::string& aPageName, const std::string& aAnchor = std::string());

   //! Search for a file in a list of directories. returns an empty path if the file is not found
   static UtPath Search(const std::vector<UtPath>& aPathsToSearch, const UtPath& aRelativePath, bool aTargetIsDirectory);

   //! Complimentary to Search(), this yields the best relative path given a search path
   static UtPath RelativePath(const std::vector<UtPath>& aPathsToSearch, const UtPath& aFullPath);

   // Search for a data file under data/, if the file is not found, exit with error message
   static UtPath FindCriticalResource(const std::string& aFileRelativePath);

   static std::string GetSimpleVersionString();

private:
   static UtPath InitSubDir(const UtPath& aParentDir, const std::string& aSubDirName);
   static bool   InitDataDir(std::string& aErrorString);
   static bool   InitRootDir(std::string& aErrorString);

   static UtPath FindPath(const UtPath& aPath, const std::string& aDir, int aIter);

   static bool InitDir(UtPath& aDirPath, const std::string& aDirName, std::string& aErrorString);
};
} // namespace wizard

#endif
