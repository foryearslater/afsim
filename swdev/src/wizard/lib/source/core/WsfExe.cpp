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

#include "WsfExe.hpp"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QtConcurrent>

// Utilities
#include "UtProcess.hpp"

// Application specific
#include "RunEnvManager.hpp"
#include "Util.hpp"
#include "WsfExeManager.hpp"
#include "WsfScriptData.hpp"

UtCallbackListN<void(wizard::WsfExe*)> wizard::WsfExe::ExeUpdated;

wizard::WsfExe::WsfExe(const UtPath& aExePath)
   : mExePath(aExePath)
   , mGrammarDocumentationFutureStarted(false)
   , mWSF_StylerPtr(wizExeMgr.GetStyler())
{
}

wizard::WsfExe::WsfExe(std::shared_ptr<WsfParseDefinitions>& aParseDefinition, const std::string& aGrammarText)
   : mApplication("No application")
   , mAlias("No application")
   , mGrammarText(aGrammarText)
   , mParseDefinitions(aParseDefinition)
   , mGrammarDocumentationFutureStarted(false)
   , mWSF_StylerPtr(wizExeMgr.GetStyler())
   , mGrammarVersion(aParseDefinition->GetVersionString())
{
   BuildProxyRegistry();
   BeginBuildDoc();
}

wizard::WsfExe::~WsfExe()
{
   Cleanup();
   FreeReference();
}

void wizard::WsfExe::Cleanup()
{
   for (size_t i = 0; i < mScriptTypesObjects.size(); ++i)
   {
      delete mScriptTypesObjects[i];
   }
   mScriptTypesObjects.clear();
   mGivenScriptTypes.clear();
}

bool wizard::WsfExe::LoadFromDisk()
{
   if (Util::IsExecutable(mExePath))
   {
      std::string tempFileName = Util::MakeTempFileName();
      // std::vector<std::string> args;
      // args.push_back("--ide-output");
      // args.push_back(tempFileName);
      // UtProcess::ExecOptions execOptions;
      // UtProcess* processPtr = UtProcess::Execute(mExePath.GetSystemPath(), args, execOptions);
      auto processPtr = UtProcess::Execute(mExePath.GetSystemPath(), "--ide-output " + tempFileName);
      if (processPtr)
      {
         processPtr->Wait();
         std::ifstream exeDataFile(tempFileName.c_str());
         if (exeDataFile)
         {
            Cleanup();

            char        line[4000];
            std::string currentBlock;
            std::string currentName;
            while (exeDataFile.getline(line, 4000))
            {
               if (line[0] == '^' && line[1] == '^' && line[2] == '^')
               {
                  char* endP = line + 3;
                  while (*endP && *endP != '\n')
                  {
                     ++endP;
                  }
                  currentName.assign(line + 3, endP);
                  continue;
               }
               else if (line[0] == '$' && line[1] == '$' && line[2] == '$')
               {
                  if (!currentName.empty())
                  {
                     ProcessData(currentName, currentBlock);
                  }
                  currentName.clear();
                  currentBlock.clear();
               }
               else if (!currentName.empty())
               {
                  currentBlock += line;
                  currentBlock += '\n';
               }
            }
            mParseDefinitions = std::shared_ptr<WsfParseDefinitions>();
            GetParseDefinitions();
            return true;
         }
         else
         {
            WsfExe* noExePtr = wizExeMgr.GetNoExe();
            if (noExePtr)
            {
               mVersion              = "Unknown";
               mApplication          = mExePath.GetFileName(false);
               mGrammarText          = noExePtr->mGrammarText;
               mParseDefinitions     = noExePtr->mParseDefinitions;
               mGrammarDocumentation = noExePtr->mGrammarDocumentation;
               BuildProxyRegistry();
               return true;
            }
         }
      }
   }
   return false;
}

bool wizard::WsfExe::IsLoaded() const
{
   return !mGrammarText.empty();
}

wizard::WsfExe* wizard::WsfExe::FromDisk(const UtPath& aExePath)
{
   WsfExe* exePtr = nullptr;
   if (Util::IsExecutable(aExePath))
   {
      exePtr = new WsfExe(aExePath);
      if (!exePtr->LoadFromDisk())
      {
         delete exePtr;
         exePtr = nullptr;
      }
   }
   return exePtr;
}

void wizard::WsfExe::ProcessData(const std::string& aBlockName, const std::string& aText)
{
   if (aBlockName == "version")
   {
      mVersion = aText;
      UtStringUtil::TrimWhiteSpace(mVersion);
   }
   else if (aBlockName == "application")
   {
      mApplication = aText;
      UtStringUtil::TrimWhiteSpace(mApplication);
   }
   else if (aBlockName == "grammar")
   {
      mGrammarText = aText;
   }
}

const std::string& wizard::WsfExe::GetGrammar()
{
   return mGrammarText;
}

std::shared_ptr<wizard::GrammarDocumentation> BuildDocumentationAsync(std::shared_ptr<WsfParseDefinitions> aDefinitions,
                                                                      std::string                          aIndexFile)
{
   auto docPtr = std::make_shared<wizard::GrammarDocumentation>();
   docPtr->Process(*aDefinitions, aIndexFile);
   return docPtr;
}

void wizard::WsfExe::BeginBuildDoc()
{
   UtPath docPath                     = RunEnvManager::GetDocumentationDir();
   mGrammarDocumentationFutureStarted = true;
   mGrammarDocumentationFuture =
      QtConcurrent::run(BuildDocumentationAsync, mParseDefinitions, (docPath + "command_index.idx").GetSystemPath());
}

void wizard::WsfExe::BuildProxyRegistry()
{
   WsfParseDefinitions* parseDefs = mParseDefinitions.get();
   auto                 regPtr    = std::make_shared<WsfPProxyRegistry>();
   regPtr->mApplicationPointer    = this;
   mProxyRegistry                 = regPtr;
   regPtr->Setup(parseDefs->GetRootStruct(), parseDefs->mBasicTypes);
}

std::shared_ptr<WsfParseDefinitions> wizard::WsfExe::GetParseDefinitions()
{
   // Check to see if pointer is empty
   if (!mParseDefinitions)
   {
      // Create a new parse definition object
      mParseDefinitions = std::make_shared<WsfParseDefinitions>();

      // Add the currently loaded grammar text and add to the definitions
      std::stringstream ss(mGrammarText);
      mParseDefinitions->AddGrammar("", ss);

      // Have to initialize before we can check the version
      bool ok = mParseDefinitions->Initialize();
      if (!ok)
      {
         // Show an error message and use default grammar
         QString message;
         message += "WARNING! Using default grammar.\n\n";
         message += "WSF grammar for ";
         // NOTE:
         // QByteArray is implicitly convertible to QString, so this is functionally equivalent to
         // QString::fromStdString. However, using QByteArray as an intermediate masks a string
         // termination vulnerability identified by static code analysis, with sink in QString's
         // use of 'strlen'. The source is std::string input from a file stream in WsfExe, which
         // should be null-terminated, so this is apparently a false positive, but this workaround
         // might be unnecessary in a future version of Qt.
         message += QByteArray::fromStdString(GetDescriptiveName());
         message += " is malformed.\n";
         message += "Errors include: ";
         const std::list<std::string>& errors = mParseDefinitions->GetDefinitionErrors();
         for (auto&& err : errors)
         {
            message += '\n';
            message += QString::fromStdString(err);
         }
         QMessageBox::warning(nullptr, "Invalid Grammar", message, QMessageBox::Ok);
      }
      else
      {
         // Now do a version check
         mGrammarVersion = GrammarVersion(mParseDefinitions->GetVersionString());
         ok = ((GrammarVersion(1, 1000000, 1000000) < mGrammarVersion) && (mGrammarVersion < GrammarVersion(3, 0, 0)));

         if (!ok)
         {
            // Throw a version warning and use default grammar
            QString message;
            message += "WSF grammar for ";
            // See NOTE above about use of QByteArray
            message += QByteArray::fromStdString(GetDescriptiveName());
            message += " has invalid version (";
            message += QString::fromStdString(mParseDefinitions->GetVersionString());
            message += ").  Using default grammar.\n";

            QMessageBox::warning(nullptr, "Invalid Grammar", message, QMessageBox::Ok);
         }
      }

      // Use default grammar if error encountered from any of the checks above
      if (!ok)
      {
         mParseDefinitions = wizExeMgr.GetDefaultParseDefinitions();
      }

      BuildProxyRegistry();
      BeginBuildDoc();
   }
   return mParseDefinitions;
}

std::shared_ptr<wizard::GrammarDocumentation> wizard::WsfExe::GetGrammarDocumentation()
{
   if (mGrammarDocumentation)
   {
      return mGrammarDocumentation;
   }
   GetParseDefinitions();
   if (mGrammarDocumentationFutureStarted)
   {
      mGrammarDocumentationFuture.waitForFinished();
      mGrammarDocumentation = mGrammarDocumentationFuture.result();
   }
   return mGrammarDocumentation;
}

void wizard::WsfExe::ExeTypeToXML(XML_Context& aInput)
{
   UtQtXmlWriter& writer = aInput.GetWriter();
   writer.WriteStartElement("wsf-exe-info");
   {
      writer.WriteAttribute("application-name", mApplication);
      writer.WriteAttribute("application-version", mVersion);
      writer.WriteAttribute("application-alias", mAlias);
      writer.WriteAttribute("application-path", mExePath.GetSystemPath());
   }
   writer.WriteEndElement();
}

wizard::WsfScriptTypes* wizard::WsfExe::TakeScriptTypes()
{
   WsfScriptTypes* typesPtr;
   if (mScriptTypesObjects.empty() && mParseDefinitions)
   {
      typesPtr = new WsfScriptTypes(mParseDefinitions->GetInputGrammar());
   }
   else
   {
      typesPtr = mScriptTypesObjects.back();
      mScriptTypesObjects.pop_back();
   }
   mGivenScriptTypes.insert(typesPtr);
   return typesPtr;
}

void wizard::WsfExe::GiveScriptTypes(WsfScriptTypes* aScriptTypesPtr)
{
   if (mGivenScriptTypes.find(aScriptTypesPtr) != mGivenScriptTypes.end())
   {
      mScriptTypesObjects.push_back(aScriptTypesPtr);
      mGivenScriptTypes.erase(aScriptTypesPtr);
   }
   else
   {
      delete aScriptTypesPtr;
   }
}

std::string wizard::WsfExe::GetDescriptiveNameFor(const std::string& aApplicationName, const std::string& aVersion)
{
   std::string descName = aApplicationName;
   if (!aVersion.empty())
   {
      descName += " " + aVersion;
   }
   return descName;
}

std::string wizard::WsfExe::GetDescriptiveName() const
{
   return GetDescriptiveNameFor(mApplication, mVersion);
}

wizard::WsfExe::GrammarVersion::GrammarVersion(const std::string& aVersion)
{
   std::vector<std::string> parts;
   UtStringUtil::Parse(aVersion, parts, '.');
   majorNo = minorNo = patchNo = 0;
   if (aVersion.size() >= 1)
   {
      majorNo = UtStringUtil::ToInt(parts[0]);
   }
   if (aVersion.size() >= 2)
   {
      minorNo = UtStringUtil::ToInt(parts[1]);
   }
   if (aVersion.size() >= 3)
   {
      patchNo = UtStringUtil::ToInt(parts[2]);
   }
}

bool wizard::WsfExe::GrammarVersion::operator<(const GrammarVersion& aRhs) const
{
   if (majorNo < aRhs.majorNo)
   {
      return true;
   }
   if (majorNo > aRhs.majorNo)
   {
      return false;
   }
   if (minorNo < aRhs.minorNo)
   {
      return true;
   }
   if (minorNo > aRhs.minorNo)
   {
      return false;
   }
   return patchNo < aRhs.patchNo;
}
