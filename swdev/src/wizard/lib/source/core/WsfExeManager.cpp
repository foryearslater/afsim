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

#include "WsfExeManager.hpp"

#include <cassert>
#include <fstream>

#include <QMessageBox>
#include <QtWidgets>

#include "Project.hpp"
#include "RunEnvManager.hpp"
#include "RunManager.hpp"
#include "UtPath.hpp"
#include "UtQt.hpp"
#include "UtQtXml.hpp"
#include "UtScanDir.hpp"
#include "Util.hpp"
#include "WsfExe.hpp"
#include "WsfParseUtil.hpp"

UtCallbackListN<void()> wizard::WsfExeManager::WsfExeListUpdated;

wizard::WsfExeManager& wizard::WsfExeManager::GetInstance()
{
   static WsfExeManager instance;
   return instance;
}

wizard::WsfExeManager::WsfExeManager()
   : mNoExePtr(nullptr)
   , mWsfStylerPtr(ut::make_unique<WsfStyler>())
   , mDefaultExeIndex(0)
   , mAutoLoad(true)
{
   WsfParseUtil::InitializeParseUtil();

   // Easier to save the file names to ignore
   mFileNamesToIgnore.insert("wizard");
   mFileNamesToIgnore.insert("pack_to_cpp");
}

wizard::WsfExeManager::~WsfExeManager()
{
   Clear();
   delete mNoExePtr;
}

wizard::WsfExe* wizard::WsfExeManager::Find(const UtPath& aPath) const
{
   for (size_t i = 1; i < mExes.size(); ++i)
   {
      if (mExes[i] && mExes[i]->GetPath() == aPath)
      {
         return mExes[i];
      }
   }
   return nullptr;
}

wizard::WsfExe* wizard::WsfExeManager::Add(const UtPath& aPath)
{
   // Set default return value
   WsfExe* exePtr = nullptr;

   // If exe and path have not already been added
   if (Find(aPath) == nullptr)
   {
      // Check to see if the executable path exists
      bool pathExists{aPath.Exists()};
      // Check to see if this executable should be ignored
      bool ignoreExe{IgnoreExecutable(aPath.GetFileName())};
      if (pathExists && !ignoreExe)
      {
         exePtr = WsfExe::FromDisk(aPath);
         if (exePtr != nullptr)
         {
            exePtr->SetAlias(exePtr->GetDescriptiveName());
            mExes.push_back(exePtr);
            // Notify that the exe list has been updated.
            WsfExeListUpdated();
         }
      }
      else
      {
         QString errorMsg = "Cannot add the application\n";
         errorMsg.append("Application:  " + QString::fromStdString(aPath.GetFileName()));
         if (!pathExists)
         {
            errorMsg.append("\nReason:  Executable does not exist.");
         }
         if (ignoreExe)
         {
            errorMsg.append("\nReason:  Executable is not valid for WsfSimulation execution.");
         }
         QMessageBox::warning(nullptr, "Error!", errorMsg);
      }
   }
   return exePtr;
}

void wizard::WsfExeManager::Clear()
{
   for (size_t i = 1; i < mExes.size(); ++i)
   {
      delete mExes[i];
   }
   mExes.clear();
   mExes.push_back(mNoExePtr);
}

namespace
{
void DefaultGrammarError(WsfParseDefinitions* aDefinitionsPtr)
{
   QString message;
   message += "FATAL ERROR!\n\n";
   message += "WSF grammar for default grammar is malformed.\n";
   message += "Errors include: ";
   const std::list<std::string>& errors = aDefinitionsPtr->GetDefinitionErrors();
   for (auto&& error : errors)
   {
      message += '\n';
      message += QString::fromStdString(error);
   }
   QMessageBox::critical(nullptr, "Invalid Grammar", message, QMessageBox::Ok);
   exit(1);
}
} // namespace

bool wizard::WsfExeManager::IgnoreExecutable(const std::string& aFileName)
{
   // Remove the file name extension
   std::vector<std::string> fileParts;
   UtStringUtil::Parse(aFileName, fileParts, '.');

   // Look for a simple match
   bool ignore = mFileNamesToIgnore.contains(QString(aFileName.c_str()));

   // Do a more detailed search - starts with ...
   // this will catch _d, _win, _lnx, etc.
   if (!ignore)
   {
      QSet<QString>::iterator iter = mFileNamesToIgnore.begin();
      while (iter != mFileNamesToIgnore.end())
      {
         ignore = UtStringUtil::StartsWith(aFileName, (*iter).toStdString());
         if (ignore)
         {
            iter = mFileNamesToIgnore.end();
         }
         else
         {
            ++iter;
         }
      }
   }

   return ignore;
}

void wizard::WsfExeManager::LoadDefaultGrammar()
{
   auto        parseDefinitions = std::make_shared<WsfParseDefinitions>();
   std::string fullGrammarText;

   GetDefaultGrammar(*parseDefinitions.get(), fullGrammarText);

   if (!parseDefinitions->GetDefinitionErrors().empty())
   {
      DefaultGrammarError(parseDefinitions.get());
   }

   if (!parseDefinitions->Initialize())
   {
      DefaultGrammarError(parseDefinitions.get());
   }

   mNoExePtr = new WsfExe(parseDefinitions, fullGrammarText);

   if (mExes.empty())
   {
      mExes.push_back(mNoExePtr);
   }
   else
   {
      delete mExes[0];
      mExes[0] = mNoExePtr;
   }
}

void wizard::WsfExeManager::Remove(WsfExe* aExePtr)
{
   std::vector<WsfExe*>::iterator iter = std::find(mExes.begin(), mExes.end(), aExePtr);
   // Don't remove the first exe
   if ((iter != mExes.end()) && (iter != mExes.begin()))
   {
      Project* proj = Project::Instance();
      if (proj)
      {
         if (proj->GetExe() == aExePtr)
         {
            proj->SetExe(mExes[0], true);
         }
      }
      delete *iter;
      mExes.erase(iter);
      WsfExeListUpdated();
   }
}

void wizard::WsfExeManager::SetDefault(int aIndex)
{
   assert((aIndex >= 0) && (aIndex < (int)mExes.size()));

   // Check the bounds
   if ((aIndex >= 0) || (aIndex < (int)mExes.size()))
   {
      // Set the default
      mDefaultExeIndex = aIndex;
   }
}

void wizard::WsfExeManager::Refresh()
{
   LoadDefaultGrammar();

   std::vector<WsfExe*> newExes;
   newExes.push_back(mNoExePtr);
   for (size_t i = 1; i < mExes.size(); ++i)
   {
      WsfExe* exePtr = mExes[i];
      WsfExe* exe    = WsfExe::FromDisk(exePtr->GetPath());
      if (exe)
      {
         newExes.push_back(exe);
      }
   }
   Clear();
   mExes = newExes;
   WsfExeListUpdated();
}

bool wizard::WsfExeManager::Refresh(WsfExe* aExePtr)
{
   if (Project::Instance())
   {
      Project::Instance()->InvalidateScenario(true);
   }

   bool                           refreshed = false;
   std::vector<WsfExe*>::iterator iter      = std::find(mExes.begin(), mExes.end(), aExePtr);
   if (iter != mExes.end())
   {
      if (iter == mExes.begin())
      {
         LoadDefaultGrammar();
         WsfExeListUpdated();
         refreshed = true;
      }
      else if (aExePtr->LoadFromDisk())
      {
         WsfExeListUpdated();
         refreshed = true;
      }
   }
   return refreshed;
}

wizard::WsfExe* wizard::WsfExeManager::FindByPath(const std::string& aPath)
{
   if (!aPath.empty())
   {
      for (auto& exe : mExes)
      {
         if (exe->GetPath() == aPath)
         {
            return exe;
         }
      }
   }

   return nullptr;
}

wizard::WsfExe* wizard::WsfExeManager::FindByDescriptiveName(const std::string& aName)
{
   if (!aName.empty())
   {
      for (auto&& exe : mExes)
      {
         if (exe->GetDescriptiveName() == aName)
         {
            return exe;
         }
      }
   }

   return nullptr;
}

wizard::WsfExe* wizard::WsfExeManager::FindByApplicationName(const std::string& aName)
{
   if (!aName.empty())
   {
      for (auto&& exe : mExes)
      {
         if (exe->GetApplication() == aName)
         {
            return exe;
         }
      }
   }

   return nullptr;
}

std::shared_ptr<WsfParseDefinitions> wizard::WsfExeManager::GetDefaultParseDefinitions()
{
   return mNoExePtr->GetParseDefinitions();
}

wizard::WsfExe* wizard::WsfExeManager::FindFromXML(XML_Context& aContext)
{
   UtQtXml::UtQtXmlAttributeList attrList = aContext.GetReader().GetAttributes();
   std::string                   name = "mission", ver = "", path = "";
   for (auto&& attr : attrList)
   {
      if (attr.mName == "application-name")
         name = attr.mValue;
      if (attr.mName == "application-version")
         ver = attr.mValue;
      if (attr.mName == "application-path")
         path = attr.mValue;
   }
   return FindBest(name, ver, path);
}

wizard::WsfExe* wizard::WsfExeManager::FindBest(const std::string& aApplicationName,
                                                const std::string& aVersion,
                                                const std::string& aPath)
{
   WsfExe* exePtr = FindByPath(aPath);

   /*   if (!exePtr)
      {
         exePtr = FindByDescriptiveName(WsfExe::GetDescriptiveNameFor(aApplicationName, aVersion));
      }

      if (!exePtr)
      {
         exePtr = FindByApplicationName(aApplicationName);
      }*/

   if (!exePtr)
   {
      exePtr = Add(aPath);
   }

   return exePtr ? exePtr : mNoExePtr;
}

bool wizard::WsfExeManager::MoveDown(WsfExe* aExePtr)
{
   // Set the default return value
   bool moveSucceeded = false;

   int currentExeIndex = GetIndex(aExePtr);
   int exeSize         = static_cast<int>(mExes.size());
   if (currentExeIndex < (exeSize - 1))
   {
      // Move the exe pointer data
      WsfExe* currentExe         = mExes[currentExeIndex];
      mExes[currentExeIndex]     = mExes[currentExeIndex + 1];
      mExes[currentExeIndex + 1] = currentExe;

      // Change the default index
      if (currentExeIndex == mDefaultExeIndex)
      {
         ++mDefaultExeIndex;
      }
      else if ((mDefaultExeIndex - currentExeIndex) == 1)
      {
         --mDefaultExeIndex;
      }

      // Move succeeded
      moveSucceeded = true;
   }

   return moveSucceeded;
}

bool wizard::WsfExeManager::MoveUp(WsfExe* aExePtr)
{
   // Set the default return value
   bool moveSucceeded = false;

   int currentExeIndex = GetIndex(aExePtr);
   if (currentExeIndex > 1)
   {
      // Move the exe pointer data
      WsfExe* currentExe         = mExes[currentExeIndex];
      mExes[currentExeIndex]     = mExes[currentExeIndex - 1];
      mExes[currentExeIndex - 1] = currentExe;

      // Change the default index
      if (currentExeIndex == mDefaultExeIndex)
      {
         --mDefaultExeIndex;
      }
      else if ((currentExeIndex - mDefaultExeIndex) == 1)
      {
         ++mDefaultExeIndex;
      }

      // Move succeeded
      moveSucceeded = true;
   }

   return moveSucceeded;
}

wizard::WsfExe* wizard::WsfExeManager::GetDefaultExe()
{
   if (mDefaultExeIndex < (int)mExes.size())
   {
      return mExes[mDefaultExeIndex];
   }
   return nullptr;
}

wizard::WsfExe* wizard::WsfExeManager::GetNoExe()
{
   if (!mNoExePtr)
   {
      LoadDefaultGrammar();
   }
   return mNoExePtr;
}

int wizard::WsfExeManager::GetIndex(wizard::WsfExe* aExePtr)
{
   return (int)(std::find(mExes.begin(), mExes.end(), aExePtr) - mExes.begin());
}

void wizard::WsfExeManager::GetDefaultGrammar(WsfParseDefinitions& parseDefinitions, std::string& fullGrammarText)
{
   // Combine all '.ag' files into one grammar string
   UtScanDir::DirEntries grammarFiles;
   UtScanDir::ScanDir(RunEnvManager::GetGrammarDir().GetSystemPath(), grammarFiles, "*.ag");

   for (auto&& gfile : grammarFiles)
   {
      std::string   grammarFilePath = (RunEnvManager::GetGrammarDir() + gfile.mFilename).GetSystemPath();
      std::ifstream grammarInput(grammarFilePath.c_str(), std::ios::binary);
      parseDefinitions.AddGrammar(grammarFilePath, grammarInput);
      std::string fileText;
      grammarInput.seekg(0);
      fileText.assign(std::istreambuf_iterator<char>(grammarInput), std::istreambuf_iterator<char>());
      fullGrammarText += "# Grammar from file ";
      fullGrammarText += grammarFilePath;
      fullGrammarText += "\n\n";
      fullGrammarText += fileText;
   }
}
