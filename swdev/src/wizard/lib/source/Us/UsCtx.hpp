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
#ifndef UTCTX_HPP
#define UTCTX_HPP

#include <UtReferenceCount.hpp>

#include "Scanner.h"
#include "UsSymbolTable.hpp"
#include "UtMemory.hpp"

////////////////////////////////////////////////////////////////////
// WARNING: Large portions of this file are kept in sync with
//          util/source/UtScriptLanguageCtx.hpp
////////////////////////////////////////////////////////////////////
// Script Parser:
// The script parser is divided into these parts:
//  * script.atg - The grammar file.  An input into the COCO/R parser generator.
//                 Results in the Us::Parser and Us::Scanner classes
//  * UsCtx - Maintains a syntactic context while parsing a script.  Used almost exclusively by script.atg
//  * UsVal - A script object, function, value, etc... to be used while parsing.
//  * UtScriptParser - A front end to the script parser

class UsSymbolTablePool;

namespace Us
{
class Parser;
class UsCtx;
class UsInitializerList;
struct UsInitializerListData;
class UsVal;

using Cx  = UsCtx;
using Ty  = UsType;
using Fn  = UsFunction;
using Val = UsVal;

// Initializer lists are the result of parsing the '{' ... '}' syntax for representing containers
// This object stores information about the values as they were parsed without knowledge of any left-hand
// side type information.  At a later time, initializer lists are 'realized' as a container value.
class UsInitializerList
{
public:
   UsInitializerList();
   ~UsInitializerList();

   UtReferenceCount* GetReferenceCount() const { return mRefCount; }

   void AddRef() { mRefCount->AddStrongRef(); }
   void RemoveRef()
   {
      if (mRefCount->RemoveStrongRef())
         delete this;
   }

   UsInitializerListData* mDataPtr;

private:
   UtReferenceCount* mRefCount;
};

// Represents a script value during the compilation process
class UsVal
{
public:
   enum Flags
   {
      cLOCAL_VARIABLE     = 1,
      cSTATIC_VARIABLE    = 2,
      cGLOBAL_VARIABLE    = 4,
      cEXTERN_VARIABLE    = 8,
      cTEMPORARY_VARIABLE = 16
      // Only for Wizard:
      ,
      cPARAMETER_VARIABLE = 32
   };

   UsVal()
      : mSym()
      , mType()
      , mMethodNamePtr(nullptr)
   {
   }
   UsVal(const UsVal& aRhs)
      : mSym(aRhs.mSym)
      , mType(aRhs.mType)
      , mInitList(aRhs.mInitList)
      , mMethodNamePtr(aRhs.mMethodNamePtr)
   {
   }
   UsVal&      operator=(const UsVal&) = default;
   bool        IsEmpty() const { return mSym == nullptr && mType == nullptr && !mInitList.IsValid(); }
   Ty*         Type() { return mType; }
   std::string TypeName();

   bool IsAppMethod() const
   {
      // Class method using an implied class reference
      return (mMethodNamePtr != nullptr) && (mType != nullptr);
   }

   bool IsScript() const
   {
      // User-defined script
      return (mMethodNamePtr == nullptr) && (mType == nullptr) && (mSym != nullptr);
   }

   // The symbol if this is a symbol, null if not a symbol
   UsSymbol* mSym;
   // The type of value.  May be null.
   // In the case of an App method, this is the type of the class containing the method
   Ty* mType;
   // If this value is an initializer list, this points to it
   UtReferencePtr<UsInitializerList> mInitList;
   // To represent a method, the name is pointed to here,
   // and the class type is pointed to by mType
   const char* mMethodNamePtr;
};

// The data inside an initializer list.  Required to be separate due to order of definition
struct UsInitializerListData
{
   struct Entry
   {
      UsVal key;
      UsVal val;
   };
   std::vector<Entry> mEntries;
};

using UsValList = std::vector<UsVal>;

// May be subclassed to handle information resulting from script parsing
class UsObserver
{
public:
   enum SpecialTokenType
   {
      cSTATIC_METHOD,
      cMETHOD,
      cLOCAL_VARIABLE,
      cPARAMETER
   };
   virtual ~UsObserver() {}
   virtual void ScriptDefinition(UsSymbolTable* aFuncSymTable, UsSymbol* aSymbolPtr) = 0;
   virtual void VarDecl(UsSymbol* aSymbolPtr, Token* ident)                          = 0;
   virtual void PushScope(UsSymbolTable* aSymTablePtr)                               = 0;
   virtual void PopScope(UsSymbolTable* aSymTablePtr)                                = 0;
   virtual void AtStat() {}
   virtual void AtDot(Ty* aTypePtr, int aPos) {}
   virtual void AtDot(UsVal aVal, int aPos) {}
   virtual void HitEOF() = 0;
   virtual void SpecialToken(SpecialTokenType aType, Token* tok) {}
   virtual void BeginCall(const UsVal&, int aPos) {}
   virtual void EndCall(const UsVal&, int aPos) {}
   virtual void CallArgNext(int aPos) {}
};

// Contains script information that is shared by all contexts
class UsConstCtx
{
public:
   Us::Parser* mParser;
   UsTypeList* mTypes;
   int         mNextSequenceNumber;

   int NextSequenceNumber() { return ++mNextSequenceNumber; }
   UsConstCtx();
   ~UsConstCtx();

   UsVal ValFromType(Ty* aType)
   {
      UsVal v;
      v.mType = aType;
      return v;
   }
   Ty* tyNull;
   Ty* tyVoid;
   Ty* tyVar;
   Ty* tyBool;
   Ty* tyInt;
   Ty* tyDouble;
   Ty* tyString;
   Ty* tyChar;
   Ty* tyObject;
   Ty* tyBuiltin;

   UsObserver* mObserverPtr;

   UsSymbolTablePool* mSymbolTablePoolPtr;
};

// Represents a script context.  This is the primary interface used by the parser.
class UsCtx
{
public:
   enum ContextFlags
   {
      cTEMPORARY_SYMBOL_TABLE = 1
   };

   enum BlockType
   {
      cSCRIPT_BLOCK,
      cNORMAL_BLOCK,
      cWHILE_BLOCK,
      cDO_BLOCK,
      cFOR_BLOCK,
      cFOREACH_BLOCK
   };
   // Parent context.  This context is syntactically nested in the parent.
   UsCtx* mParentCtx;
   // Pointer to data shared by all contexts
   UsConstCtx*    mConst;
   UsSymbolTable* mSym;
   // UsCtx*         p;
   UsSymbol* mFuncDefSym;
   bool      mGlobalVarMode;
   bool mAllowGlobals; // globals may be disabled, allowing a single block to be processed without affecting the global state.
   int              mContextFlags;
   int              mSequenceNumberLimit;
   static const int cLARGE_INT = 0x7fffffff;
   BlockType        mBlockType;

   explicit UsCtx(UsConstCtx* c);
   UsCtx(UsCtx& p, UsSymbolTable::TableLinkage aLinkage = UsSymbolTable::cNEW_SCOPE);
   void EndContext();
   // Create a new context for the purpose of reparsing a single script.
   static std::unique_ptr<UsCtx> NewMirror(UsConstCtx* c, UsSymbolTable* aParentSymbolTable, int aSequenceNumberLimit)
   {
      auto ctx                  = ut::make_unique<UsCtx>(c);
      ctx->mSequenceNumberLimit = aSequenceNumberLimit;
      // compile this new context inside of the context we want to mirror
      // Add as a local scope linkage to keep the same symbol lookup depth
      ctx->mSym->AddOuter(UsSymbolTable::cLOCAL_SCOPE, aParentSymbolTable);
      ctx->mContextFlags |= cTEMPORARY_SYMBOL_TABLE;
      return ctx;
   }

   ~UsCtx();
   UsObserver* GetObserver() const { return mConst->mObserverPtr; }
   Ty*         qType(Token* t);
   Ty*         Type(Token* t);
   Ty*         qType(Ty* aTypePtr, std::pair<Ty*, Ty*> aArgs);

   void AddParam(Fn& fn, Ty* type, Token* aIdent, bool aIsFormalParam = false);
   void SetRType(Fn& fn, Ty* type);
   void FuncDefStart(UsVal& aScriptVal);

   UsVal     AddEntry(Token* aIdent, Fn& aFunc, bool aExtern, UsCtx* aFnCtx = nullptr);
   UsSymbol* Search(const std::string& aName, int aDepth = 100)
   {
      return mSym->Search(aName, aDepth, mSequenceNumberLimit);
   }

   // Returns 'true' if the token is the name of a method on the 'this' type.
   bool IsThisMethod(Token* t);

   UsVal     ThisValue();
   UsSymbol* AddAppVariable(Ty* type, const std::string& aName);

   UsSymbol*   AddEntry(Ty* type, Token* aIdent, int aVarFlags = 0);
   UsVal       SymVal(UsSymbol* aSym);
   UsVal       Identifier(Token* aIdent);
   UsVal       Search(Token* aIdent);
   Ty*         RType(UsSymbol* s);
   Ty*         FuncRType();
   std::string Text(Token* t);
   enum OperationId
   {
      cASSIGN,
      cASSIGN_INITIAL,
      cASSIGN_PLUS,
      cASSIGN_MINUS,
      cASSIGN_TIMES,
      cASSIGN_DIV,
      cADD,
      cSUBTRACT,
      cMULTIPLY,
      cDIVIDE,
      cOR,
      cAND,
      cEQ,
      cNE,
      cLT,
      cGT,
      cLE,
      cGE,
      cUNARY_MINUS,
      cUNARY_PLUS,
      cINDEX,
      cNOT
   };
   UsVal Op(UsVal& lhs, int op);
   UsVal Ctor(Ty* aType, const UsValList& aArgs);
   UsVal Op(UsVal& lhs, UsVal& idx, UsVal& rhs, int op);
   UsVal Op(UsVal& lhs, UsVal& rhs, int op);
   // Returns a map's key type if the type is a map
   Ty* GetContainerKey(Ty* aType);
   // Returns the type of value a container holds
   Ty* GetContainerValue(Ty* aType);
   // { ...    p1 : p2 , ... }
   void  AddToInitializerList(UsVal& initList, UsVal& p1, UsVal* p2); // UsVal& val, UsVal* key);
   UsVal Cast(UsVal& v, Ty* ty);
   UsVal StringLiteral(Token* t);
   UsVal DoubleLiteral(Token* t);
   UsVal IntLiteral(Token* t);
   UsVal BoolLiteral(Token* t);
   UsVal NullLiteral(Token* t);
   UsVal NewInitializerList();
   UsVal RealizeContainer(UsInitializerList& aInitializer, Ty* aLhsType);
   UsVal ImplicitCast(UsVal v, Ty* ty);
   UsVal ExplicitCast(UsVal& v, Ty* ty);
   void  ExpectedType(UsVal& v, Ty* t);
   UsVal GetAttribute(UsVal& v, Token* t);
   UsVal GetAttribute(Ty* ty, Token* t);
   UsVal GetDynAttribute(UsVal& v, Token* t);
   UsVal GetClassMethod(UsVal& v, Token* t);
   UsVal GetClassVariable(UsVal& v, Token* t);
   UsVal VarDecl(Ty* ty, Token* t, int flags = 0);
   UsVal Call(UsVal& fn, UsValList& args, UsVal* aReturnVal = nullptr);
   void  VarDeclInit(UsVal& v);
   void  VarDeclPreAssign(UsVal& v);
   Ty*   BoolTy() { return mConst->tyBool; }
   Ty*   VoidTy() { return mConst->tyVoid; }
   Ty*   IntTy() { return mConst->tyInt; }
   void  AtDot(Ty* aTypePtr, int pos);
   void  AtDot(const UsVal& aVal, int pos);
   void  HitEOF();
   UsVal AndComplete(UsVal& orResult, UsVal& rhs, int aJumpOffset);
   void  WhileCond(UsVal& v);
   void  WhileEnd(int aEnd);
   int   OrCheck(UsVal& aLhs, UsVal& aOrResult);
   UsVal OrComplete(UsVal& orResult, UsVal& rhs, int aJumpOffset);

   int    AndCheck(UsVal& aLhs, UsVal& aOrResult);
   int    IfCond(UsVal& v);
   int    IfTrueEnd(int falseTarget, bool aHasElse);
   void   IfElseEnd(int aElseEnd);
   void   SetBlockType(BlockType aBlockType);
   void   Break();
   void   Continue();
   void   DoWhileStart();
   void   DoCond(UsVal& v);
   void   ForCond(UsVal& v);
   void   ForInc();
   void   ForEnd();
   void   ForEachBegin(UsVal& key, UsVal& val, UsVal& container);
   void   Return(UsVal& v);
   void   FuncComplete(UsVal& aFnVal, UsCtx& aFnCtx);
   int    CodeOffset();
   UsCtx* ScriptContext();
   int    BeginStat();
   void   EndStat(int aStatNum);

   void SemErr(const char* aMsg);
   void BeginCall(UsVal& fn, int aPos);
   void EndCall(UsVal& fn, int aPos);
   void CallArgNext(int aPos);


   UsVal NewVal(Ty* ty);
};


inline void UsCtx::AtDot(Ty* aTypePtr, int pos)
{
   if (mConst->mObserverPtr)
   {
      mConst->mObserverPtr->AtDot(aTypePtr, pos);
   }
}
inline void UsCtx::AtDot(const UsVal& aVal, int pos)
{
   if (mConst->mObserverPtr)
   {
      mConst->mObserverPtr->AtDot(aVal, pos);
   }
}
inline void UsCtx::HitEOF()
{
   if (GetObserver())
   {
      GetObserver()->HitEOF();
   }
}
// Inline unused functions here
inline void  UsCtx::EndContext() {}
inline void  UsCtx::VarDeclInit(UsVal& v) {}
inline void  UsCtx::VarDeclPreAssign(UsVal& v) {}
inline UsVal UsCtx::AndComplete(UsVal& orResult, UsVal& rhs, int aJumpOffset)
{
   return mConst->ValFromType(mConst->tyBool);
}
inline void UsCtx::WhileCond(UsVal& v) {}
inline void UsCtx::WhileEnd(int aEnd) {}
inline int  UsCtx::OrCheck(UsVal& aLhs, UsVal& aOrResult)
{
   return -1;
}
inline UsVal UsCtx::OrComplete(UsVal& orResult, UsVal& rhs, int aJumpOffset)
{
   return mConst->ValFromType(mConst->tyBool);
}

inline int UsCtx::AndCheck(UsVal& aLhs, UsVal& aOrResult)
{
   return -1;
}
inline int UsCtx::IfCond(UsVal& v)
{
   return -1;
}
inline int UsCtx::IfTrueEnd(int falseTarget, bool aHasElse)
{
   return -1;
}
inline void UsCtx::IfElseEnd(int aElseEnd) {}
inline void UsCtx::SetBlockType(BlockType aBlockType) {}
inline void UsCtx::Break() {}
inline void UsCtx::Continue() {}
inline void UsCtx::DoWhileStart() {}
inline void UsCtx::DoCond(UsVal& v) {}
inline void UsCtx::ForCond(UsVal& v) {}
inline void UsCtx::ForInc() {}
inline void UsCtx::ForEnd() {}
inline void UsCtx::ForEachBegin(UsVal& key, UsVal& val, UsVal& container) {}
inline void UsCtx::Return(UsVal& v) {}
inline void UsCtx::FuncComplete(UsVal& aFnVal, UsCtx& aFnCtx) {}
inline int  UsCtx::CodeOffset()
{
   return -1;
}
inline void UsCtx::EndStat(int aStatNum) {}
} // namespace Us

#endif // UTCTX_HPP
