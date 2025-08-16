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
#ifndef WSFSCRIPTPARSE_HPP
#define WSFSCRIPTPARSE_HPP
#include <map>
#include <vector>

#include <QString>

#include "Us/UsCtx.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"
#include "WsfParseType.hpp"


namespace WsfGrammar
{
namespace M
{
class Expr;
}
} // namespace WsfGrammar

using Us::UsConstCtx;
using Us::UsCtx;
using Us::UsObserver;
using Us::UsVal;

class UsSymbol;
class UsSymbolTable;
class WsfParseDefinitions;
class WsfParseIndex;
class WsfParseNode;

namespace wizard
{
class GrammarDocumentation;
class WsfExe;
class WsfScriptContainer;
class WsfScriptData;
class WsfScriptTypes;

class WsfScriptToken
{
public:
   enum Type
   {
      cIDENTIFIER,
      cKEYWORD,
      cOPERATOR,
      cSTRING,
      cNUMBER,
      cCOMMENT,

      // 'special' tokens (not known by lexer)
      cSTATIC_METHOD,
      cMETHOD,
      cLOCAL_VARIABLE,
      cPARAMETER,
      cLOCAL_METHOD
   };
   bool        operator<(const WsfScriptToken& aTok) const { return mRange.GetBegin() < aTok.mRange.GetBegin(); }
   UtTextRange mRange;
   Type        mTokenType;
};

// Holds basic information about a script's parse results.
class WsfScriptInfo
{
public:
   WsfScriptInfo()
      : mParent(nullptr)
      , mBlockType(cFUNCTION)
      , mSymbolPtr(nullptr)
      , mSymbolTablePtr(nullptr)
   {
   }
   enum BlockType
   {
      cFUNCTION,
      cIMPLICIT_SCRIPT,
      cSCRIPT_VARIABLES
   };
   void SetRange(UtTextDocumentRange aRange);

   WsfScriptContainer*    mParent;
   UtTextDocumentLocation mLocation;
   // The range of the script without the begin/end keywords
   UtTextDocumentRange mRange;
   // The range of the script text, including the begin/end keywords
   UtTextRange                      mBlockRange;
   std::vector<WsfScriptToken>      mTokens;
   std::vector<std::pair<int, int>> mErrors;
   BlockType                        mBlockType;
   // Symbol table containing the function's local symbols.
   UsSymbol*      mSymbolPtr;
   UsSymbolTable* mSymbolTablePtr;
};

class WsfScriptObserver;

// Represents a container that may hold scripts, such as a platform
class WsfScriptContainer
{
public:
   WsfScriptContainer();
   ~WsfScriptContainer();
   bool HasContext() const { return mContextPtr != nullptr; }
   void CreateContext(WsfScriptContainer*     aParentContainer,
                      WsfScriptContainer*     aCopyFromContainer,
                      const WsfParseTypePath& aLoadKey);

   UsCtx* GetContext();
   void   AddScript(WsfScriptInfo* aScriptPtr)
   {
      mScopeToScript[aScriptPtr->mSymbolTablePtr] = aScriptPtr;
      mScripts.push_back(aScriptPtr);
   }
   const std::vector<WsfScriptInfo*>& GetScripts() const { return mScripts; }
   WsfScriptContainer*                GetRoot()
   {
      WsfScriptContainer* containerPtr = this;
      while (containerPtr->mParent)
      {
         containerPtr = containerPtr->mParent;
      }
      return containerPtr;
   }

   UsSymbolTable*      GetScope() const { return mContextPtr->mSym; }
   WsfScriptContainer* GetChildContainer(const WsfParseTypePath& aSaveKey,
                                         const WsfParseTypePath& aLoadKey,
                                         WsfParseType&           aSymbolTable);
   WsfScriptContainer* FindChildContainer(const WsfParseTypePath& aKey);
   WsfScriptContainer* SearchContainer(const WsfParseTypePath& aKey);
   WsfScriptContainer* FindNearestAncestor(const WsfParseTypePath& aTypeKey, WsfParseType& aSymbolTable);
   WsfScriptContainer* GetContainerByScope(UsSymbolTable* aTablePtr) const
   {
      std::map<UsSymbolTable*, WsfScriptContainer*>::const_iterator iter = mScopeToContainer.find(aTablePtr);
      if (iter != mScopeToContainer.end())
         return iter->second;
      return nullptr;
   }
   WsfScriptInfo* GetScriptByScope(UsSymbolTable* aTablePtr) const
   {
      std::map<UsSymbolTable*, WsfScriptInfo*>::const_iterator iter = mScopeToScript.find(aTablePtr);
      if (iter != mScopeToScript.end())
         return iter->second;
      return nullptr;
   }

   WsfScriptData*      mScriptDataPtr;
   WsfScriptContainer* mParent;
   UsConstCtx*         mConstCtx;

protected:
   typedef std::map<WsfParseTypePath, WsfScriptContainer, WsfParseTypePathCompare> ChildContainerMap;
   ChildContainerMap                                                               mChildContainers;
   std::vector<WsfScriptInfo*>                                                     mScripts;
   std::map<UsSymbolTable*, WsfScriptInfo*>                                        mScopeToScript;
   std::map<UsSymbolTable*, WsfScriptContainer*>                                   mScopeToContainer;
   WsfParseTypePath                                                                mContainerTypePath;
   UsCtx*                                                                          mContextPtr;
   UT_MEMORY_DEBUG_MARKER(cMDB_ViWsfScriptContainer);
};

// Contains data about all scripts from a scenario
class WsfScriptData
{
public:
   typedef std::map<UtTextDocumentLocation, WsfScriptInfo*> LocationMap;
   const LocationMap&                                       GetLocationMap() const { return mLocationToScript; }

   WsfScriptData(WsfScriptTypes* aTypesPtr, WsfParseIndex* aParseIndex, WsfParseDefinitions* aDefinitions);
   ~WsfScriptData();

   UtTextDocumentRange FindSymbolDefinition(UsSymbol* aSymbolPtr);
   WsfScriptContainer* FindContainerFromScope(UsSymbolTable* aTablePtr);
   WsfScriptInfo*      FindScriptFromScope(UsSymbolTable* aTablePtr);

   QString PeekScriptDocumentation(GrammarDocumentation& aDocumentationData,
                                   UsType*               aClassPtr,
                                   const std::string&    aMethodName);

   bool FindScriptMethodDocumentation(GrammarDocumentation& aDocumentationData,
                                      UsType*               aClassPtr,
                                      const std::string&    aMethodName,
                                      std::string&          aPageName,
                                      std::string&          aClassName);

   bool FindScriptTypeDocumentation(GrammarDocumentation& aDocumentationData, UsType* aClassPtr, std::string& aPageName);

   WsfParseType* FindParseType(const WsfParseTypePath& aLoadKey);

   WsfScriptTypes*    mTypesPtr;
   WsfScriptContainer mRoot;
   // Maps script file position to script info
   LocationMap          mLocationToScript;
   WsfParseIndex*       mParseIndex;
   WsfParseDefinitions* mParseDefinitions;
   UT_MEMORY_DEBUG_MARKER(cMDB_WsfScriptData);
};

class WsfScriptTypes
{
public:
   explicit WsfScriptTypes(const WsfGrammar::M::Expr& aGrammar);
   WsfScriptTypes(const WsfScriptTypes&) = delete; // not allowed
   ~WsfScriptTypes();

   UsConstCtx* GetConstCtx() { return mConstCtx; }

   struct AppVar
   {
      AppVar()
         : mIsThis(false)
         , mTypePtr(nullptr)
      {
      }
      bool        mIsThis;
      std::string mName;
      UsType*     mTypePtr;
   };

protected:
   void                          InitType(const std::string&                                              aName,
                                          std::map<std::string, std::vector<const WsfGrammar::M::Expr*>>& aTypeExpressions);
   UsType*                       FindTypeOrMakeTemplate(const std::string&                                              aName,
                                                        std::map<std::string, std::vector<const WsfGrammar::M::Expr*>>& aTypeExpressions);
   void                          CopyInheritedMethods(UsType* aTypePtr);
   std::vector<WsfParseTypePath> mTypeStack;
   UsConstCtx*                   mConstCtx;
   UT_MEMORY_DEBUG_MARKER(cMDB_WsfScriptTypes);
};


class WsfScriptParser
{
public:
   struct ErrorRecord
   {
      std::string mText;
      int         mPosition;
      int         mLength;
   };


   WsfScriptParser(WsfScriptTypes* aTypesPtr, WsfScriptData* aDataPtr);

   void Rebuild(WsfParseNode* aParseTree, WsfParseType& aSymbolTable);


   enum ProcessScriptFlags
   {
      cRECORD_TOKENS         = 1,
      cRECORD_ERRORS         = 2,
      cRECORD_ERROR_DETAILS  = 4,
      cSAVE_RESULTS          = 8,
      cNO_SCOPE_CHANGE       = 0x10,
      cDEFAULT_PROCESS_FLAGS = cRECORD_TOKENS | cRECORD_ERRORS | cSAVE_RESULTS
   };

   void ProcessScript(WsfScriptInfo*     aScriptPtr,
                      const char*        aTextPtr,
                      size_t             aTextLength,
                      UsCtx&             aStartCtx,
                      int                aProcessFlags,
                      WsfScriptObserver* aObserverPtr = nullptr);

   struct TypeEntry
   {
      WsfParseTypePath mLoadKey;
      WsfParseTypePath mSaveKey;
   };
   std::vector<TypeEntry> mTypeStack;

   UsConstCtx*     mConstCtx;
   WsfScriptTypes* mTypesPtr;
   WsfScriptData*  mDataPtr;

private:
   void Walk(WsfParseNode* aParseTree, WsfParseType& aSymbolTable);
   void RecordScript(WsfParseNode* aScriptBlockNode, WsfParseType& aSymbolTable);

   void DeferredProcessScript(const std::vector<TypeEntry>& aTypeStack,
                              WsfParseNode*                 aScriptBlockNode,
                              WsfParseType&                 aSymbolTable);

   // Stores script information to be processed later in order defined
   // by the deferred input processing
   struct RecordedScript
   {
      WsfParseNode*          mNode;
      WsfParseType*          mSymbolTable;
      std::vector<TypeEntry> mTypeStack;
   };
   std::map<size_t, std::vector<RecordedScript>> mRecordedScripts;
};


class WsfScriptObserver : public UsObserver
{
public:
   virtual void AcceptErrors(std::vector<WsfScriptParser::ErrorRecord>& aErrors) {}
};

class WsfScriptNormalObserver : public WsfScriptObserver
{
public:
   WsfScriptInfo* mScriptPtr;
   struct SpecialTok
   {
      UtTextRange                  range;
      UsObserver::SpecialTokenType tokType;
   };
   std::vector<SpecialTok> mSpecialTokens;
   explicit WsfScriptNormalObserver(WsfScriptInfo* aScriptPtr)
      : mScriptPtr(aScriptPtr)
   {
   }

   void ScriptDefinition(UsSymbolTable* aFuncSymTable, UsSymbol* aSymbolPtr) override
   {
      mScriptPtr->mSymbolTablePtr = aFuncSymTable;
      mScriptPtr->mSymbolPtr      = aSymbolPtr;
   }

   void PushScope(UsSymbolTable* aSymTablePtr) override {}
   void PopScope(UsSymbolTable* aTablePtr) override {}
   void VarDecl(UsSymbol* aSymbolPtr, Us::Token* ident) override {}
   void HitEOF() override {}
   void SpecialToken(SpecialTokenType aType, Us::Token* t) override
   {
      SpecialTok tok;
      size_t     startPos = t->pos + mScriptPtr->mRange.GetBegin();
      tok.range.SetRange(startPos, startPos + t->len);
      tok.tokType = aType;
      mSpecialTokens.push_back(tok);
   }
};

class WsfScriptDetailObserver : public WsfScriptNormalObserver
{
public:
   enum LHS_Kind
   {
      cTYPE,
      cVALUE
   };
   struct ErrorRecord
   {
      std::string mText;
      int         mPosition;
      int         mLength;
   };
   struct CallInfo
   {
      CallInfo()
         : mMethodBaseType(nullptr)
         , mPrototypeId(0)
         , mCurrentArg(0)
         , mValPtr(nullptr)
         , mOpenParenPos(ut::npos)
         , mCloseParenPos(ut::npos)
      {
      }
      UsType*      mMethodBaseType;
      std::string  mMethodName;
      int          mPrototypeId;
      int          mCurrentArg;
      const UsVal* mValPtr;
      size_t       mOpenParenPos;
      size_t       mCloseParenPos;
   };
   std::vector<WsfScriptParser::ErrorRecord> mErrorRecords;
   std::vector<UsSymbolTable*>               mScopeStack;
   std::map<UsSymbol*, int>                  mVarDeclLocations;
   std::vector<CallInfo>                     mCurrentCalls;
   UsVal                                     mLHS_Val;
   UsType*                                   mLHS_Type;
   LHS_Kind                                  mLHS_Kind;

   bool mEOF_Hit;
   bool mRecordErrors;
   int  mDotLHS_Pos;
   int  mCursorPos;

   explicit WsfScriptDetailObserver(WsfScriptInfo* aScriptPtr)
      : WsfScriptNormalObserver(aScriptPtr)
      , mLHS_Type(nullptr)
      , mLHS_Kind(cTYPE)
      , mEOF_Hit(false)
      , mRecordErrors(false)
   {
      mCursorPos  = -1;
      mDotLHS_Pos = -1;
   }

   void ScriptDefinition(UsSymbolTable* aFuncSymTable, UsSymbol* aSymbolPtr) override;
   void AtStat() override;
   void AtDot(UsType* aTypePtr, int aPos) override;
   void AtDot(UsVal aVal, int aPos) override;

   void PushScope(UsSymbolTable* aSymTablePtr) override;
   void PopScope(UsSymbolTable* aTablePtr) override;
   void VarDecl(UsSymbol* aSymbolPtr, Us::Token* ident) override;
   void HitEOF() override;
   void AcceptErrors(std::vector<WsfScriptParser::ErrorRecord>& aErrors) override { std::swap(mErrorRecords, aErrors); }
   void BeginCall(const UsVal& v, int aPos) override;
   void EndCall(const UsVal& v, int aPos) override;
   void CallArgNext(int aPos) override;
};
} // namespace wizard
#endif
