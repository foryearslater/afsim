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

#ifndef WSFEXE_HPP
#define WSFEXE_HPP

#include <string>

// Qt
#include <QFuture>

// Utilities
#include "UtCallback.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"

// Application specific
#include "GrammarDocumentation.hpp"
#include "ViExport.hpp"

// WSF
#include "WsfPProxyRegistry.hpp"
#include "WsfParseDefinitions.hpp"

// Base class declaration
#include "UtReferenceTracked.hpp"

// Forward declarations
class QSettings;

namespace wizard
{
class WsfScriptTypes;
class WsfStyler;
class XML_Context;

class VI_EXPORT WsfExe : public UtReferenceTracked
{
public:
   explicit WsfExe(const UtPath& aExePath);
   WsfExe(std::shared_ptr<WsfParseDefinitions>& aParseDefinition, const std::string& aGrammar);
   WsfExe(const WsfExe&) = delete;
   WsfExe(WsfExe&&)      = default;
   WsfExe& operator=(const WsfExe&) = delete;
   WsfExe& operator=(WsfExe&&) = default;
   virtual ~WsfExe();

   bool                                  LoadFromDisk();
   bool                                  IsLoaded() const;
   static WsfExe*                        FromDisk(const UtPath& aExePath);
   const UtPath&                         GetPath() const { return mExePath; }
   const std::string&                    GetVersion() const { return mVersion; }
   const std::string&                    GetApplication() const { return mApplication; }
   const std::string&                    GetGrammar();
   std::shared_ptr<WsfParseDefinitions>  GetParseDefinitions();
   std::shared_ptr<GrammarDocumentation> GetGrammarDocumentation();
   std::shared_ptr<WsfPProxyRegistry>    GetProxyRegistry() const { return mProxyRegistry; }
   std::string                           GetAlias() const { return mAlias; }
   void                                  SetAlias(const std::string& aAlias) { mAlias = aAlias; }
   std::string                           GetDescriptiveName() const;

   virtual void ExeTypeToXML(XML_Context& aInput);

   WsfScriptTypes* TakeScriptTypes();
   void            GiveScriptTypes(WsfScriptTypes* aScriptTypesPtr);
   WsfStyler*      GetStyler() { return mWSF_StylerPtr; }

   static UtCallbackListN<void(WsfExe*)> ExeUpdated;

   class GrammarVersion
   {
   public:
      GrammarVersion()
         : majorNo(0)
         , minorNo(0)
         , patchNo(0)
      {
      }
      GrammarVersion(int aMajorNo, int aMinorNo, int aPatchNo)
         : majorNo(aMajorNo)
         , minorNo(aMinorNo)
         , patchNo(aPatchNo)
      {
      }
      explicit GrammarVersion(const std::string& aVersion);

      VI_EXPORT bool operator<(const GrammarVersion& aRhs) const;

      // Note: It's tempting to call these "major" and "minor", but doing so
      //       makes it possible to cause name collisions on Linux with glibc
      //       functions of the same names.
      int majorNo;
      int minorNo;
      int patchNo;
   };

   GrammarVersion GetGrammarVersion() const { return mGrammarVersion; }

   static std::string GetDescriptiveNameFor(const std::string& aApplicationName, const std::string& aVersion);

protected:
   void BeginBuildDoc();
   void Cleanup();
   void ProcessData(const std::string& aBlockName, const std::string& aText);
   void BuildProxyRegistry();

   UtPath                               mExePath;
   std::string                          mVersion;
   std::string                          mApplication;
   std::string                          mAlias;
   std::string                          mGrammarText;
   std::shared_ptr<WsfParseDefinitions> mParseDefinitions;

   // Because the script types do not change, we recycle these objects.
   // This allows us to re-use the same 2 or 3 instances
   std::vector<WsfScriptTypes*>                   mScriptTypesObjects;
   std::set<WsfScriptTypes*>                      mGivenScriptTypes;
   std::shared_ptr<GrammarDocumentation>          mGrammarDocumentation;
   bool                                           mGrammarDocumentationFutureStarted;
   QFuture<std::shared_ptr<GrammarDocumentation>> mGrammarDocumentationFuture;
   WsfStyler*                                     mWSF_StylerPtr;
   std::shared_ptr<WsfPProxyRegistry>             mProxyRegistry;
   GrammarVersion                                 mGrammarVersion;
};
} // namespace wizard

#endif
