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
#ifndef WSFSCRIPTLOOKUP_HPP
#define WSFSCRIPTLOOKUP_HPP

#include <map>

#include "Us/UsSymbolTable.hpp"
#include "UtMemory.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"
#include "WsfScriptData.hpp"

namespace wizard
{
class Project;

// Wraps lookup of script symbols given a location.
class VI_EXPORT WsfScriptLookup
{
public:
   explicit WsfScriptLookup(Project* aProjectPtr);
   WsfScriptLookup(const WsfScriptLookup&) = delete;
   WsfScriptLookup(WsfScriptLookup&&)      = default;
   WsfScriptLookup& operator=(const WsfScriptLookup&) = delete;
   WsfScriptLookup& operator=(WsfScriptLookup&&) = default;

   ~WsfScriptLookup();
   void SetScenario(Project* aProjectPtr);
   enum StopLocation
   {
      // Parse the script up to the given location.  This allows lookup of symbols available at the current scope.
      // This is usually the desired functionality for auto-complete purposes.
      cSTOP_AT_LOCATION,
      // Parse the entire function.
      // Doing this makes it hard to determine the symbols available at some location in the source.
      cSTOP_AT_END_OF_FUNCTION,
      // Same as cSTOP_AT_LOCATION, except parse until the end of the statement.
      // This allows lookup of a variable created in that statement, but will not be able to identify method calls by name.
      cSTOP_AT_END_OF_STATEMENT
   };
   void SetLocation(UtTextDocumentLocation aLocation, StopLocation aParseStopLocation = cSTOP_AT_LOCATION);
   void SetStoreErrors(bool aStoreErrors);


   enum UpdateResult
   {
      cNO_SCRIPT,
      cFAILURE,
      cSUCCESS,
      cSUCCESS_NO_CHANGES
   };

   UpdateResult Update();

   // A symbol map maps the identifier string to the symbol that matches it.
   typedef std::map<std::string, UsValRef> SymbolMap;
   UsSymbolTable*                          GetLocalScope();
   UsSymbol*                               Search(const std::string& aSymbolName);
   UsType*                                 FindType(const std::string& aName);
   UsType*                                 GetType(int aTypeId);

   bool AddSymbolsInScope(SymbolMap& aSymbols);
   bool AddSymbolsFromThis(SymbolMap& aSymbols);
   bool AddSymbolsFromBuiltins(SymbolMap& aSymbols);
   bool AddSymbolsFromType(SymbolMap& aSymbols, UsType* aTypePtr);
   bool HasMethodSymbols();
   bool AddMethodSymbols(SymbolMap& aSymbols);

   size_t      GetInsertPosition() const { return mInsertPosition; }
   size_t      GetReplaceCharacters() const { return mReplaceCharacters; }
   std::string GetIdentifierLeftOfCursor() const { return mIdentifierLeftOfCursor; }

   WsfScriptData* GetScriptData() const { return mSharedScriptDataPtr; }
   WsfScriptInfo* GetScriptInfo() const { return mScriptInfoPtr.get(); }

   UtTextDocumentRange GetDefinitionLocation(UsSymbol* aSymPtr);

   static bool
   IdentifierToken(const char* aBufferBegin, const char* aBufferEnd, size_t aCharOffset, size_t& aBegin, size_t& aEnd);

   static UtTextDocumentRange                 IdentifierRange(UtTextDocumentLocation aLocation);
   std::vector<WsfScriptParser::ErrorRecord>* GetErrors();
   std::string                                GetFunctionDescription(const std::string& aFunctionName,
                                                                     const UsFunction&  aFunc,
                                                                     const std::string& aClassName = std::string());
   std::string                                GetScriptValDescription(UsValRef aVal);

   UtTextDocumentLocation GetLocation() const { return mLocation; }

   bool GetCurrentCallInfo(int&                     aArgNumber,
                           size_t&                  aOpenParenPos,
                           size_t&                  aCloseParenPos,
                           std::string&             aFunctionName,
                           std::vector<UsFunction>& aFunctionOptions);

private:
   void FreeParseData();


   UpdateResult Parse(UtTextDocumentLocation aLocation);

   void GetText(UtTextDocumentLocation& aScriptStart,
                size_t                  aCursorPos,
                long long               aSequenceNumber,
                std::string&            aText,
                size_t&                 aFinalScriptStartPos);

   Project* mProjectPtr;

   UtTextDocumentLocation mLocation;
   StopLocation           mParseStopLocation;

   std::string mScriptText;
   // Sequence number from parse results the last time we computed the script data
   long long mComputedDataSequenceNumber;

   // Location in document where an insertion is to be made when doing
   // autocomplete
   size_t mInsertPosition;
   // The number of characters to delete when doing autocomplete insertion
   size_t mReplaceCharacters;
   // The symbol characters the user has already typed left of the cursor
   std::string mIdentifierLeftOfCursor;
   int         mSymbolSequenceNumber;

   bool                                     mStoreErrors;
   WsfScriptData*                           mSharedScriptDataPtr;
   std::unique_ptr<WsfScriptInfo>           mScriptInfoPtr;
   std::unique_ptr<WsfScriptDetailObserver> mScriptDetailObserverPtr;
};
} // namespace wizard

#endif
