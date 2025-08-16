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

#include "WsfScriptLookup.hpp"

#include <QTextDocument> // for Qt::escape()

#include "Editor.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "TextSource.hpp"
#include "TextSourceView.hpp"
#include "UtTextDocument.hpp"
#include "WsfParseUtil.hpp"

wizard::WsfScriptLookup::WsfScriptLookup(Project* aProjectPtr)
   : mProjectPtr(aProjectPtr)
   , mParseStopLocation(cSTOP_AT_LOCATION)
   , mComputedDataSequenceNumber(-1)
   , mInsertPosition(ut::npos)
   , mReplaceCharacters(ut::npos)
   , mSymbolSequenceNumber(-1)
   , mStoreErrors(false)
   , mSharedScriptDataPtr(nullptr)
   , mScriptInfoPtr(nullptr)
   , mScriptDetailObserverPtr(nullptr)
{
}

wizard::WsfScriptLookup::~WsfScriptLookup()
{
   FreeParseData();
}

void wizard::WsfScriptLookup::FreeParseData()
{
   mSharedScriptDataPtr        = nullptr;
   mScriptInfoPtr              = nullptr;
   mScriptDetailObserverPtr    = nullptr;
   mComputedDataSequenceNumber = -1;
   mScriptText.clear();
}

wizard::WsfScriptLookup::UpdateResult wizard::WsfScriptLookup::Parse(UtTextDocumentLocation aLocation)
{
   ParseResultsTiny* resultsPtr = mProjectPtr->TinyParseResults();
   if (!resultsPtr || !resultsPtr->mScriptDataPtr)
   {
      return cNO_SCRIPT;
   }
   bool                                       parseSuccess    = false;
   UtTextDocumentLocation                     currentPos      = aLocation;
   bool                                       isInScript      = false;
   TextSource*                                sourcePtr       = static_cast<TextSource*>(aLocation.mSource);
   const WsfScriptData::LocationMap&          scriptLocations = resultsPtr->mScriptDataPtr->GetLocationMap();
   WsfScriptData::LocationMap::const_iterator iter            = scriptLocations.lower_bound(currentPos);
   if (iter != scriptLocations.begin())
   {
      --iter;
   }
   if (iter != scriptLocations.end())
   {
      if (iter->first.mSource == sourcePtr)
      {
         TextSource* srcPtr         = static_cast<TextSource*>(currentPos.mSource);
         UtTextRange scriptNewRange = iter->second->mBlockRange;
         srcPtr->ApplyModifications(scriptNewRange, resultsPtr->mSequenceNumber);
         if (scriptNewRange.Contains(currentPos.mPosition - 1))
         {
            isInScript = true;

            UtTextDocumentRange scriptRangeToCursor(sourcePtr, scriptNewRange);
            scriptRangeToCursor.SetEnd(currentPos.mPosition - 1);
            if (scriptRangeToCursor.Length() > 0)
            {
               WsfScriptInfo*         scriptPtr = iter->second;
               std::string            scriptText;
               UtTextDocumentLocation startLoc  = scriptRangeToCursor.GetPosition();
               size_t                 stopAtPos = aLocation.mPosition - 1;
               switch (mParseStopLocation)
               {
               case cSTOP_AT_END_OF_FUNCTION:
               case cSTOP_AT_END_OF_STATEMENT:
                  stopAtPos = scriptNewRange.GetEnd();
                  break;
               default:
                  break;
               }
               size_t finalScriptStartPos;
               GetText(startLoc, stopAtPos, resultsPtr->mSequenceNumber, scriptText, finalScriptStartPos);
               // GetText(scriptRangeToCursor, resultsPtr->mSequenceNumber, scriptText);
               // std::cerr << "----------\n" << scriptText << "|\n---------\n";
               const char* beginScript = scriptText.c_str();
               const char* endScript   = beginScript + scriptText.size();
               while (!WsfParseUtil::IsWhitespace(*beginScript) && beginScript < endScript) // skip past begin block keyword
               {
                  ++beginScript;
               }
               while (WsfParseUtil::IsWhitespace(*beginScript) &&
                      beginScript < endScript) // skip past whitespace after begin block
               {
                  ++beginScript;
               }
               mReplaceCharacters = 0;
               // If the cursor is on an identifier, go left
               const char* oldEndScript = endScript;
               while ((endScript > beginScript) && WsfParseUtil::IsAlphaNumeric(*(endScript - 1)))
               {
                  --endScript;
                  ++mReplaceCharacters;
               }

               // If we are parsing to the end of the statement, find that location now
               if (mParseStopLocation == cSTOP_AT_END_OF_STATEMENT)
               {
                  endScript = std::min(beginScript + aLocation.mPosition - scriptNewRange.GetBegin(), oldEndScript);
                  while ((endScript < oldEndScript) && *endScript != ';')
                  {
                     ++endScript;
                  }
                  mReplaceCharacters = 0;
               }

               mIdentifierLeftOfCursor.assign(endScript, oldEndScript);
               mInsertPosition = scriptNewRange.GetBegin() + (endScript - scriptText.c_str());

               // Re-Parsing notes:
               // In order to only parse a single function, we are re-using the existing parse
               // data from the parse results.  The new script is parsed in a new context along-side
               // its old context.  To make sure the new script does not use any symbols that are defined
               // later, we mark each symbol with a sequence number, and limit lookups to sequence numbers less than
               // some value. New symbols created by this script are never put in the global scope, as that would change
               // the existing script data And new symbols are given negative sequence number so that they are found
               // upon lookup.
               //
               // Finally, as a result, we have a hybrid of new script information and old script information
               // The new script information is only valid while the old script information is kept.  Therefore,
               // we need to recompute this data if the old script information is updated.

               assert(!mScriptInfoPtr);
               mScriptInfoPtr              = ut::make_unique<WsfScriptInfo>();
               mScriptInfoPtr->mBlockRange = scriptNewRange;
               mScriptInfoPtr->mRange.SetRange(scriptNewRange.GetBegin() + (beginScript - scriptText.c_str()),
                                               scriptNewRange.GetBegin() + (endScript - scriptText.c_str()));
               mScriptInfoPtr->mRange.mSource = sourcePtr;
               mScriptInfoPtr->mBlockType     = scriptPtr->mBlockType;
               mScriptInfoPtr->mLocation      = scriptPtr->mRange.GetPosition();

               // Get the sequence number at the start of the old script definition
               int sequenceNumber = 0x7fffffff;
               if (scriptPtr->mSymbolPtr)
               {
                  sequenceNumber = scriptPtr->mSymbolPtr->mSequenceNumber;
               }
               mSymbolSequenceNumber = sequenceNumber;

               // begin using a negative sequence number for new symbols
               int previousNextSequenceNumber = -1000000;
               std::swap(previousNextSequenceNumber, scriptPtr->mParent->mConstCtx->mNextSequenceNumber);

               // Create a new context as a mirror of the old one.  This new context will not change any existing script data
               auto ctx =
                  UsCtx::NewMirror(scriptPtr->mParent->mConstCtx, scriptPtr->mParent->GetContext()->mSym, sequenceNumber);

               WsfScriptParser parser(resultsPtr->mScriptTypesPtr, resultsPtr->mScriptDataPtr);
               mScriptDetailObserverPtr = ut::make_unique<WsfScriptDetailObserver>(mScriptInfoPtr.get());
               mScriptDetailObserverPtr->mCursorPos =
                  ut::cast_to_int(mLocation.mPosition) - ut::cast_to_int(mScriptInfoPtr->mRange.GetBegin());

               // we already have the appropriate scope set up, don't let ProcessScript change it
               int flags = WsfScriptParser::cNO_SCOPE_CHANGE;
               if (mStoreErrors)
               {
                  flags |= WsfScriptParser::cRECORD_ERROR_DETAILS;
               }
               parser.ProcessScript(mScriptInfoPtr.get(),
                                    beginScript,
                                    endScript - beginScript,
                                    *ctx,
                                    flags,
                                    mScriptDetailObserverPtr.get());

               // Restore the next sequence number to its previous value
               std::swap(previousNextSequenceNumber, scriptPtr->mParent->mConstCtx->mNextSequenceNumber);
               mSharedScriptDataPtr = resultsPtr->mScriptDataPtr;
               mScriptText.assign(beginScript, endScript);
               parseSuccess = true;
            }
         }
      }
   }
   if (!isInScript)
   {
      return cNO_SCRIPT;
   }
   else if (parseSuccess)
   {
      return cSUCCESS;
   }
   return cFAILURE;
}

void wizard::WsfScriptLookup::GetText(UtTextDocumentLocation& aScriptStart,
                                      size_t                  aCursorPos,
                                      long long               aSequenceNumber,
                                      std::string&            aText,
                                      size_t&                 aFinalScriptStartPos)
{
   TextSource* sourcePtr = static_cast<TextSource*>(aScriptStart.mSource);
   if (sourcePtr->HasAppliedChanges(aSequenceNumber))
   {
      // This branch should only be taken for a recently updated document
      // This gets new text corresponding to an old text range

      aScriptStart.mPosition = sourcePtr->ApplyModifications(aScriptStart.mPosition, aSequenceNumber);

      if (sourcePtr->HasPendingChanges() && !sourcePtr->GetViews().empty())
      {
         Editor* editPtr = sourcePtr->GetView(mProjectPtr)->mEditorPtr;
         size_t  endPos  = aCursorPos + 1;
         aText           = editPtr->RangeText(aScriptStart.mPosition, endPos);
      }
      else
      {
         UtTextDocumentRange range(aScriptStart.mSource, aScriptStart.mPosition, aCursorPos);
         aFinalScriptStartPos = aScriptStart.mPosition;
         range.Text(aText);
      }
   }
   else
   {
      UtTextDocumentRange range(aScriptStart.mSource, aScriptStart.mPosition, aCursorPos);
      aFinalScriptStartPos = aScriptStart.mPosition;
      range.Text(aText);
   }
}

void wizard::WsfScriptLookup::SetLocation(UtTextDocumentLocation aLocation, StopLocation aParseStopLocation)
{
   if (!(mLocation == aLocation && mParseStopLocation == aParseStopLocation))
   {
      mLocation          = aLocation;
      mParseStopLocation = aParseStopLocation;
      FreeParseData();
   }
}
void wizard::WsfScriptLookup::SetScenario(Project* aProjectPtr)
{
   if (mProjectPtr != aProjectPtr)
   {
      mProjectPtr = aProjectPtr;
      FreeParseData();
   }
}


// Updated the parsed data if necessary
wizard::WsfScriptLookup::UpdateResult wizard::WsfScriptLookup::Update()
{
   ParseResultsTiny* resultsPtr = mProjectPtr->TinyParseResults();
   if (resultsPtr)
   {
      if (mComputedDataSequenceNumber == resultsPtr->mSequenceNumber && mSharedScriptDataPtr == resultsPtr->mScriptDataPtr)
      {
         return cSUCCESS_NO_CHANGES;
      }
      else
      {
         FreeParseData();
         UpdateResult result = Parse(mLocation);
         if (result == cSUCCESS)
         {
            mSharedScriptDataPtr        = resultsPtr->mScriptDataPtr;
            mComputedDataSequenceNumber = resultsPtr->mSequenceNumber;
         }
         else
         {
            mSharedScriptDataPtr        = nullptr;
            mComputedDataSequenceNumber = -1;
         }
         return result;
      }
   }
   return cFAILURE;
}
namespace
{

void GetAllSymbols(UsSymbolTable* aTablePtr, wizard::WsfScriptLookup::SymbolMap& aSymbolMap, int aSequenceNumberLimit)
{
   for (auto&& i : aTablePtr->mEntries)
   {
      if (i.second.mSequenceNumber < aSequenceNumberLimit)
      {
         UsValRef& sym = aSymbolMap[i.first];
         if (sym.IsNull())
         {
            sym = &i.second;
         }
      }
   }
   for (auto&& link : aTablePtr->mOuterScopes)
   {
      GetAllSymbols(link.mTablePtr, aSymbolMap, aSequenceNumberLimit);
   }
}
} // namespace

bool wizard::WsfScriptLookup::AddSymbolsInScope(SymbolMap& aSymbols)
{
   if (!mScriptDetailObserverPtr)
   {
      return false;
   }
   if (mScriptDetailObserverPtr->mScopeStack.empty())
   {
      return false;
   }
   UsSymbolTable* tablePtr = mScriptDetailObserverPtr->mScopeStack.back();
   GetAllSymbols(tablePtr, aSymbols, mSymbolSequenceNumber);
   UsType* builtinPtr = mSharedScriptDataPtr->mTypesPtr->GetConstCtx()->tyBuiltin;
   if (builtinPtr)
   {
      AddSymbolsFromType(aSymbols, builtinPtr);
   }
   return true;
}

UsSymbolTable* wizard::WsfScriptLookup::GetLocalScope()
{
   if (!mScriptDetailObserverPtr)
   {
      return nullptr;
   }
   if (mScriptDetailObserverPtr->mScopeStack.empty())
   {
      return nullptr;
   }
   return mScriptDetailObserverPtr->mScopeStack.back();
}

bool wizard::WsfScriptLookup::AddSymbolsFromThis(SymbolMap& aSymbols)
{
   if (!mScriptDetailObserverPtr)
   {
      return false;
   }
   if (mScriptDetailObserverPtr->mScopeStack.empty())
   {
      return false;
   }
   UsSymbolTable* tablePtr = mScriptDetailObserverPtr->mScopeStack.back();
   if (tablePtr->mThisTypePtr)
   {
      AddSymbolsFromType(aSymbols, tablePtr->mThisTypePtr);
   }
   return true;
}

bool wizard::WsfScriptLookup::AddSymbolsFromType(SymbolMap& aSymbols, UsType* aTypePtr)
{
   for (UsType::MethodMap::iterator iter = aTypePtr->mMethods.begin(); iter != aTypePtr->mMethods.end(); ++iter)
   {
      aSymbols[iter->first] = UsValRef(aTypePtr, iter->first);
   }
   return true;
}

bool wizard::WsfScriptLookup::HasMethodSymbols()
{
   if (mScriptText.empty())
   {
      return false;
   }

   bool processedDotAtEnd = mScriptDetailObserverPtr->mDotLHS_Pos == ((int)mScriptText.size() - 1);
   if (mScriptText[mScriptText.size() - 1] == '.' && processedDotAtEnd)
   {
      UsType* typePtr = nullptr;
      if (mScriptDetailObserverPtr->mLHS_Kind == WsfScriptDetailObserver::cTYPE)
      {
         typePtr = mScriptDetailObserverPtr->mLHS_Type;
      }
      else
      {
         typePtr = mScriptDetailObserverPtr->mLHS_Val.mType;
      }
      return typePtr != nullptr && !typePtr->mMethods.empty();
   }
   return false;
}
bool wizard::WsfScriptLookup::AddMethodSymbols(SymbolMap& aSymbols)
{
   if (HasMethodSymbols())
   {
      UsType* typePtr    = nullptr;
      bool    onlyStatic = false;
      if (mScriptDetailObserverPtr->mLHS_Kind == WsfScriptDetailObserver::cTYPE)
      {
         typePtr    = mScriptDetailObserverPtr->mLHS_Type;
         onlyStatic = true;
      }
      else
      {
         typePtr = mScriptDetailObserverPtr->mLHS_Val.mType;
      }

      if (typePtr)
      {
         for (UsType::MethodMap::iterator iter = typePtr->mMethods.begin(); iter != typePtr->mMethods.end(); ++iter)
         {
            if (onlyStatic && (0 == (iter->second.mFlags & UsFunction::cSTATIC)))
            {
               continue;
            }
            aSymbols[iter->first] = UsValRef(typePtr, iter->first);
         }
      }
      return true;
   }
   return false;
}

UtTextDocumentRange wizard::WsfScriptLookup::GetDefinitionLocation(UsSymbol* aSymPtr)
{
   if (!mScriptInfoPtr)
   {
      return UtTextDocumentRange();
   }
   UtTextDocumentRange range;
   if (aSymPtr->mStorageType != UsSymbol::cSE_EXTERN && ((mScriptInfoPtr->mSymbolTablePtr == aSymPtr->mTablePtr) ||
                                                         aSymPtr->mTablePtr->IsInheriting(mScriptInfoPtr->mSymbolTablePtr)))
   {
      range = UtTextDocumentRange(mScriptInfoPtr->mLocation, (int)aSymPtr->mName.length());
      range.Move(aSymPtr->mPosition);
   }
   else
   {
      range = mSharedScriptDataPtr->FindSymbolDefinition(aSymPtr);
   }
   return range;
}

// Given a character in a buffer, expands left and right until a full identifier is found.
bool wizard::WsfScriptLookup::IdentifierToken(const char* aBufferBegin,
                                              const char* aBufferEnd,
                                              size_t      aCharOffset,
                                              size_t&     aBegin,
                                              size_t&     aEnd)
{
   const char* start = aBufferBegin + aCharOffset;
   const char* end   = aBufferBegin + aCharOffset;
   if (!WsfParseUtil::IsAlphaNumeric(aBufferBegin[aCharOffset]))
   {
      return false;
   }
   while (start - 1 >= aBufferBegin && WsfParseUtil::IsAlphaNumeric(*(start - 1)))
   {
      --start;
   }
   while (end < aBufferEnd && WsfParseUtil::IsAlphaNumeric(*(end)))
   {
      ++end;
   }
   aBegin = start - aBufferBegin;
   aEnd   = end - aBufferBegin;
   return true;
}

UtTextDocumentRange wizard::WsfScriptLookup::IdentifierRange(UtTextDocumentLocation aLocation)
{
   size_t      line    = aLocation.mSource->PositionToLineNumber(aLocation.mPosition);
   std::string lineStr = aLocation.mSource->GetLineString(line);
   size_t      linePos = aLocation.mSource->GetLinePosition(line);
   size_t      tokBeg, tokEnd;

   if (WsfScriptLookup::IdentifierToken(lineStr.c_str(),
                                        lineStr.c_str() + lineStr.length(),
                                        aLocation.mPosition - linePos,
                                        tokBeg,
                                        tokEnd))
   {
      return UtTextDocumentRange(aLocation.mSource, linePos + tokBeg, linePos + tokEnd - 1);
   }
   return UtTextDocumentRange();
}

UsSymbol* wizard::WsfScriptLookup::Search(const std::string& aSymbolName)
{
   UsSymbolTable* tablePtr = GetLocalScope();
   if (tablePtr)
   {
      UsSymbol* sym = tablePtr->Search(aSymbolName, 1, mSymbolSequenceNumber);
      if (sym)
      {
         return sym;
      }
      sym = tablePtr->Search(aSymbolName, 10);
      return sym;
   }
   return nullptr;
}

UsType* wizard::WsfScriptLookup::FindType(const std::string& aName)
{
   if (mSharedScriptDataPtr)
   {
      return mSharedScriptDataPtr->mRoot.mConstCtx->mTypes->FindTypeOrMakeTemplate(aName);
   }
   return nullptr;
}

UsType* wizard::WsfScriptLookup::GetType(int aTypeId)
{
   if (mSharedScriptDataPtr)
   {
      return mSharedScriptDataPtr->mRoot.mConstCtx->mTypes->GetType(aTypeId);
   }
   return nullptr;
}

void wizard::WsfScriptLookup::SetStoreErrors(bool aStoreErrors)
{
   if (mStoreErrors != aStoreErrors)
   {
      mStoreErrors = aStoreErrors;
      FreeParseData();
   }
}

std::vector<wizard::WsfScriptParser::ErrorRecord>* wizard::WsfScriptLookup::GetErrors()
{
   return mScriptDetailObserverPtr ? &mScriptDetailObserverPtr->mErrorRecords : nullptr;
}


std::string wizard::WsfScriptLookup::GetFunctionDescription(const std::string& aFunctionName,
                                                            const UsFunction&  aFunc,
                                                            const std::string& aClassName)
{
   WsfScriptData* scriptDataPtr = GetScriptData();
   UsConstCtx*    constCtx      = scriptDataPtr->mTypesPtr->GetConstCtx();

   std::stringstream ss;
   ss << constCtx->mTypes->GetTypeName(aFunc.mReturnType) << ' ';
   if (!aClassName.empty() && aClassName != "__BUILTIN__")
   {
      ss << aClassName << '.';
   }
   ss << aFunctionName;
   ss << '(';
   for (size_t i = 0; i < (size_t)aFunc.mArgs.size(); ++i)
   {
      if (i != 0)
      {
         ss << ", ";
      }
      ss << constCtx->mTypes->GetTypeName(aFunc.mArgs[i]);
   }
   ss << ')';
   return ss.str();
}

std::string wizard::WsfScriptLookup::GetScriptValDescription(UsValRef aVal)
{
   WsfScriptData* scriptDataPtr = GetScriptData();
   UsConstCtx*    constCtx      = scriptDataPtr->mTypesPtr->GetConstCtx();
   if (aVal.mFunctionNamePtr == nullptr)
   {
      if (aVal.mSymbolPtr->mRTypeId >= UsTypeList::cPROTOTYPE_ID_START)
      {
         const UsFunction* fnPtr = constCtx->mTypes->GetPrototype(aVal.mSymbolPtr->mRTypeId);
         return GetFunctionDescription(aVal.mSymbolPtr->mName, *fnPtr);
      }
      UsType* typePtr = constCtx->mTypes->GetType(aVal.mSymbolPtr->mRTypeId);
      if (typePtr)
      {
         return QString::fromStdString(typePtr->mName).toHtmlEscaped().toStdString() + " <i>" + aVal.mSymbolPtr->mName +
                "</i>";
      }
   }
   else
   {
      // this is a built-in method
      UsType* typePtr = aVal.mFunctionTypePtr;
      if (typePtr)
      {
         UsType::MethodMap::iterator iter = typePtr->mMethods.lower_bound(aVal.mFunctionNamePtr);
         std::string                 methodDefinitions;
         for (; iter != typePtr->mMethods.end(); ++iter)
         {
            if (iter->first == aVal.mFunctionNamePtr)
            {
               const UsFunction& func = iter->second;
               if (!methodDefinitions.empty())
               {
                  methodDefinitions += "<BR/>";
               }
               methodDefinitions +=
                  QString::fromStdString(GetFunctionDescription(aVal.mFunctionNamePtr, func, typePtr->mName))
                     .toHtmlEscaped()
                     .toStdString();
            }
         }
         return methodDefinitions;
      }
   }
   return "";
}

bool wizard::WsfScriptLookup::GetCurrentCallInfo(int&                     aArgNumber,
                                                 size_t&                  aOpenParenPos,
                                                 size_t&                  aCloseParenPos,
                                                 std::string&             aFunctionName,
                                                 std::vector<UsFunction>& aFunctionOptions)
{
   if (!mScriptDetailObserverPtr || !mScriptInfoPtr)
   {
      return false;
   }
   if (mScriptDetailObserverPtr->mCurrentCalls.empty())
   {
      return false;
   }
   aFunctionOptions.clear();
   WsfScriptDetailObserver::CallInfo ci = mScriptDetailObserverPtr->mCurrentCalls.back();
   if (ci.mMethodBaseType)
   {
      UsType::MethodMap::iterator iter = ci.mMethodBaseType->mMethods.lower_bound(ci.mMethodName.c_str());
      for (; iter != ci.mMethodBaseType->mMethods.end(); ++iter)
      {
         if (iter->first == ci.mMethodName)
         {
            aFunctionOptions.push_back(iter->second);
            aFunctionName = ci.mMethodName;
         }
         else
         {
            break;
         }
      }
   }
   else if (ci.mPrototypeId)
   {
      const UsFunction* fn = mSharedScriptDataPtr->mTypesPtr->GetConstCtx()->mTypes->GetPrototype(ci.mPrototypeId);
      if (fn)
      {
         aFunctionOptions.push_back(*fn);
         aFunctionName = ci.mMethodName;
      }
   }
   aOpenParenPos = ci.mOpenParenPos + mScriptInfoPtr->mRange.GetBegin();

   aCloseParenPos = ci.mCloseParenPos;
   if (aCloseParenPos != ut::npos)
   {
      aCloseParenPos += mScriptInfoPtr->mRange.GetBegin();
   }
   aArgNumber = ci.mCurrentArg;
   return !aFunctionOptions.empty();
}

bool wizard::WsfScriptLookup::AddSymbolsFromBuiltins(SymbolMap& aSymbols)
{
   UsType* builtinPtr = mSharedScriptDataPtr ? mSharedScriptDataPtr->mTypesPtr->GetConstCtx()->tyBuiltin : nullptr;
   if (builtinPtr)
   {
      return AddSymbolsFromType(aSymbols, builtinPtr);
   }
   return false;
}
