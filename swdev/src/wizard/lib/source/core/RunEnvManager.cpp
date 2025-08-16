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
#include "RunEnvManager.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

// Qt
#include <QApplication>
#include <QMessageBox>

// Utilities
#include "UtDynamicLibrary.hpp"
#include "UtRunEnvManager.hpp"

namespace
{
UtPath cINSTALL_ROOT_DIR;
UtPath cBIN_DIR;
UtPath cDEMOS_DIR;
UtPath cTRAINING_DIR;
UtPath cSCENARIOS_DIR;
UtPath cDOC_DIR;
UtPath cDATA_DIR;
UtPath cGRAMMAR_DIR;

bool cHAS_DEVELOPER_PATHS = false;

int cMAJOR_VERSION;
int cMINOR_VERSION;
int cPATCH_VERSION;
} // namespace

// ============================================================================
bool wizard::RunEnvManager::Initialize(const UtPath& aBinPath, std::string& aErrorString)
{
   bool ok = true;

   // If cBIN_DIR is not empty, directories have already been initialized.
   if (cBIN_DIR.GetFileName().empty())
   {
      // Path to executable
      cBIN_DIR = aBinPath;
      ok &= InitDir(cBIN_DIR, "Bin", aErrorString);

      // Don't continue if bin directory is not valid
      if (ok)
      {
         // Path to installation root
         ok &= InitRootDir(aErrorString);
         if (ok)
         {
            // Path to demos/training/scenarios - they may not exist
            cDEMOS_DIR     = cINSTALL_ROOT_DIR + "demos";
            cTRAINING_DIR  = cINSTALL_ROOT_DIR + "training";
            cSCENARIOS_DIR = cINSTALL_ROOT_DIR + "scenarios";
         }
         else
         {
            return ok;
         }

         // Path to documentation
         cDOC_DIR = FindPath(cBIN_DIR, "documentation/html", 1);

         // Path to default grammar
         cGRAMMAR_DIR = FindPath(cBIN_DIR, "grammar", 1);

         // Path to resources
         ok &= InitDataDir(aErrorString);
      }
   }
   return ok;
}

// ============================================================================
std::string wizard::RunEnvManager::GetVersionString(bool aDate)
{
   std::stringstream ss;
   ss << GetSimpleVersionString();
   if (aDate)
   {
      std::stringstream dateParse;
      dateParse << __DATE__;
      std::string month, date, year;
      dateParse >> month >> date >> year;
      ss << " " << month << " " << date << " " << year;
   }
   return ss.str();
}

void wizard::RunEnvManager::SetVersion(int aMajor, int aMinor, int aPatch)
{
   cMAJOR_VERSION = aMajor;
   cMINOR_VERSION = aMinor;
   cPATCH_VERSION = aPatch;
}

// ============================================================================
const UtPath& wizard::RunEnvManager::GetInstallRootDir()
{
   return cINSTALL_ROOT_DIR;
}

const UtPath& wizard::RunEnvManager::GetBinDir()
{
   return cBIN_DIR;
}

const UtPath& wizard::RunEnvManager::GetDemosDir()
{
   return cDEMOS_DIR;
}

const UtPath& wizard::RunEnvManager::GetDocumentationDir()
{
   return cDOC_DIR;
}

const UtPath& wizard::RunEnvManager::GetDataDir()
{
   return cDATA_DIR;
}

const UtPath& wizard::RunEnvManager::GetGrammarDir()
{
   return cGRAMMAR_DIR;
}

const UtPath& wizard::RunEnvManager::GetTrainingDir()
{
   return cTRAINING_DIR;
}

const UtPath& wizard::RunEnvManager::GetScenariosDir()
{
   return cSCENARIOS_DIR;
}

bool wizard::RunEnvManager::HasDeveloperPaths()
{
   return cHAS_DEVELOPER_PATHS;
}

// ============================================================================
std::vector<UtPath> wizard::RunEnvManager::GetDemoSearchPaths()
{
   std::vector<UtPath> paths;

   paths.push_back(cDEMOS_DIR);
   paths.push_back(cSCENARIOS_DIR);
   paths.push_back(cTRAINING_DIR);

   return paths;
}

// ============================================================================
namespace
{
std::string MakeDocURL_Static(const std::string& aPageName, const std::string& aAnchor, const UtPath& aBasePath)
{
   std::string url;
   UtPath      pagePath = aBasePath + (aPageName);
   url                  = pagePath.GetNormalizedPath();
   if (url.length() > 1 && url.substr(0, 2) == "//")
   {
      url = url.substr(1);
   }
   url = "file:///" + url;
   if (!aAnchor.empty())
   {
      url += "#" + aAnchor;
   }
   return url;
}
} // namespace

std::string wizard::RunEnvManager::DocumentationURL_Static(const std::string& aPageName, const std::string& aAnchor)
{
   return MakeDocURL_Static(aPageName, aAnchor, GetDocumentationDir());
}

std::string wizard::RunEnvManager::DocumentationURL(const std::string& aPageName, const std::string& aAnchor)
{
   return DocumentationURL_Static(aPageName, aAnchor);
}

// ============================================================================
UtPath wizard::RunEnvManager::Search(const std::vector<UtPath>& aPathsToSearch,
                                     const UtPath&              aRelativePath,
                                     bool                       aTargetIsDirectory)
{
   for (auto&& pts : aPathsToSearch)
   {
      UtPath           path   = pts + aRelativePath;
      UtPath::StatType result = path.Stat();
      if (aTargetIsDirectory && result == UtPath::cDIRECTORY)
      {
         return path;
      }
      else if (!aTargetIsDirectory && result == UtPath::cFILE)
      {
         return path;
      }
   }
   return UtPath();
}

// ============================================================================
UtPath wizard::RunEnvManager::RelativePath(const std::vector<UtPath>& aPathsToSearch, const UtPath& aFullPath)
{
   // Rate each option looking for the shortest relative path.
   UtPath bestPath;
   int    bestScore = 1000000;
   for (auto&& pts : aPathsToSearch)
   {
      UtPath                   relPath = pts.GetRelativePath(aFullPath);
      std::string              prefix;
      std::vector<std::string> tokens;
      relPath.Tokenize(prefix, tokens);
      // if the prefix is non-empty, we can't get a relative path
      int score = (prefix.empty() ? 0 : 1000) + (int)tokens.size();
      if (score < bestScore)
      {
         bestPath  = relPath;
         bestScore = score;
      }
   }
   return bestPath;
}

// ============================================================================
UtPath wizard::RunEnvManager::FindCriticalResource(const std::string& aFileRelativePath)
{
   UtPath path = GetDataDir() + aFileRelativePath;
   if (path.Stat() != UtPath::cFILE)
   {
      std::stringstream ss;
      ss << "Critical resource file missing: " << aFileRelativePath << ".  Application cannot continue.";
      QMessageBox::critical(qApp->activeWindow(), "Missing resource", ss.str().c_str());
      exit(1);
   }
   return path;
}

// ============================================================================
std::string wizard::RunEnvManager::GetSimpleVersionString()
{
   std::stringstream ss;
   ss << cMAJOR_VERSION << '.' << cMINOR_VERSION << '.' << cPATCH_VERSION;
   return ss.str();
}

// ============================================================================
UtPath wizard::RunEnvManager::InitSubDir(const UtPath& aParentDir, const std::string& aSubDirName)
{
   UtPath dir = aParentDir + aSubDirName;
   assert(dir.Stat() == UtPath::cDIRECTORY);
   dir.MakeFullPath();
   return dir;
}

// ============================================================================
bool wizard::RunEnvManager::InitDataDir(std::string& aErrorString)
{
   // Assume an installed application
   // First look inside the bin directory
   UtPath dataDir = cBIN_DIR + "resources/data";
   if (dataDir.IsDirectory())
   {
      cDATA_DIR = dataDir;
   }
   else // Next look for peer directory to bin directory
   {
      dataDir = cBIN_DIR + "../resources/data";
      if (dataDir.IsDirectory())
      {
         cDATA_DIR = dataDir;
      }
   }

   // Still have not found the directory must be running
   // a non-installed or development application
   cHAS_DEVELOPER_PATHS = (cDATA_DIR.Stat() != UtPath::cDIRECTORY);
   if (cHAS_DEVELOPER_PATHS)
   {
      // Get the path from environment variables
      // Defaults assumption is the source directory
      const char* devDataPath = getenv("SOURCE_ROOT");
      if (devDataPath && (*devDataPath != '\0'))
      {
         std::string pathstr = std::string(devDataPath) + "/wizard/data";
         cDATA_DIR           = UtPath(pathstr);
      }
   }

   return InitDir(cDATA_DIR, "Data", aErrorString);
}

// ============================================================================
bool wizard::RunEnvManager::InitRootDir(std::string& aErrorString)
{
   // Root install directory always one up from executable
   cINSTALL_ROOT_DIR = cBIN_DIR + "../";
   return InitDir(cINSTALL_ROOT_DIR, "Root", aErrorString);
}

// ============================================================================
UtPath wizard::RunEnvManager::FindPath(const UtPath& aPath, const std::string& aDir, int aIter)
{
   UtPath currPath = aPath;
   while (aIter >= 0)
   {
      UtPath path = currPath + aDir;
      if (path.IsDirectory())
      {
         return path;
      }

      currPath.Up();
      aIter--;
   }

   return UtPath();
}

// ============================================================================
bool wizard::RunEnvManager::InitDir(UtPath& aDirPath, const std::string& aDirName, std::string& aErrorString)
{
   // Check to ensure the directory exists
   // Fill the error string if necessary
   bool initialized = false;
   if (aDirPath.Exists() && aDirPath.IsDirectory())
   {
      aDirPath.MakeFullPath();
      initialized = true;
   }
   else
   {
      aErrorString = "Invalid directory [" + aDirName + "] [" + aDirPath.GetSystemPath() + "]";
   }

   return initialized;
}
