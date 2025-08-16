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
#include "UsCtx.hpp"

#include <iostream>
#include <sstream>

#include "Parser.h"
#include "UsSymbolTable.hpp"
#include "UtCast.hpp"
namespace Us
{

UsConstCtx::UsConstCtx()
   : mParser(nullptr)
   , tyNull(nullptr)
   , tyVoid(nullptr)
   , tyVar(nullptr)
   , tyBool(nullptr)
   , tyInt(nullptr)
   , tyDouble(nullptr)
   , tyString(nullptr)
   , tyChar(nullptr)
   , tyObject(nullptr)
   , tyBuiltin(nullptr)
{
   mObserverPtr        = nullptr;
   mNextSequenceNumber = 0;
   mTypes              = new UsTypeList;
   mSymbolTablePoolPtr = nullptr;
}

UsConstCtx::~UsConstCtx()
{
   delete mTypes;
}

void UsCtx::SemErr(const char* aMsg)
{
   mConst->mParser->SemErr(aMsg);
}

void UsCtx::ExpectedType(UsVal& v, UsType* t)
{
   if (v.Type() != t)
   {
      std::stringstream ss;
      ss << "Expected type " << t->mName << ", not " << v.TypeName();
      SemErr(ss.str().c_str());
   }
}

UsVal UsCtx::ImplicitCast(UsVal v, UsType* ty)
{
   if (v.mInitList)
   {
      return RealizeContainer(*v.mInitList, ty);
   }
   if (v.mType && ty)
   {
      if (v.mType == ty)
         return v;
      if (v.mType == mConst->tyVar)
         return v;
      if (ty == mConst->tyVar || v.mType->IsImplicitlyCastable(ty->Id()) || v.mType->IsBaseType(ty->Id()) ||
          v.mType == mConst->tyNull)
      {
         return mConst->ValFromType(ty);
      }
      else
      {
         std::stringstream ss;
         ss << "Cannot implicitly cast '" << v.mType->mName << "' to '" << ty->mName << "'.";
         SemErr(ss.str().c_str());
      }
   }
   //
   return UsVal();
}

UsVal UsCtx::ExplicitCast(UsVal& v, UsType* ty)
{
   if (v.mType && ty)
   {
      if (v.mType == ty)
         return v;

      if (v.mType->IsExplicitlyCastable(ty->Id()) || v.mType->IsImplicitlyCastable(ty->Id()) ||
          v.mType->IsBaseType(ty->Id())    // down cast
          || ty->IsBaseType(v.mType->Id()) // up cast
      )
      {
         return mConst->ValFromType(ty);
      }

      // else
      {
         SemErr("Invalid cast.");
      }
   }
   //
   return UsVal();
}
UsVal UsCtx::GetAttribute(UsType* ty, Token* t)
{
   if (ty)
   {
      if (ty->HasStaticMethod(t->val))
      {
         UsVal m          = mConst->ValFromType(ty);
         m.mMethodNamePtr = t->val;
         if (mConst->mObserverPtr)
            mConst->mObserverPtr->SpecialToken(UsObserver::cSTATIC_METHOD, t);
         return m;
      }
      else
      {
         std::stringstream ss;
         ss << "Invalid Method Name '" << t->val << "'.";
         SemErr(ss.str().c_str());
      }
   }
   return UsVal();
}
UsVal UsCtx::GetAttribute(UsVal& v, Token* t)
{
   UsType* ty = v.Type();
   if (ty)
   {
      UsType::MethodMap::iterator methodIter = ty->mMethods.find(t->val);
      if (methodIter == ty->mMethods.end())
      {
         std::stringstream ss;
         ss << "Invalid Method Name '" << t->val << "'.";
         SemErr(ss.str().c_str());
      }
      else
      {
         UsVal m          = v;
         m.mMethodNamePtr = t->val;
         if (mConst->mObserverPtr)
         {
            if (methodIter->second.mFlags & UsFunction::cSTATIC)
            {
               mConst->mObserverPtr->SpecialToken(UsObserver::cSTATIC_METHOD, t);
            }
            else
            {
               mConst->mObserverPtr->SpecialToken(UsObserver::cMETHOD, t);
            }
         }
         return m;
      }
   }
   else
   {
      SemErr("Can't call method on this.");
   }
   return UsVal();
}

UsVal UsCtx::GetDynAttribute(UsVal& v, Token* t)
{
   UsVal r;
   r.mType = mConst->tyVar;
   // Could be method or variable access.  Just mark as method for now.
   if (mConst->mObserverPtr)
      mConst->mObserverPtr->SpecialToken(UsObserver::cMETHOD, t);
   return r;
}

// Ensure that the method (in 'ident') is a valid within the given class/script_struct (in 'v')
UsVal UsCtx::GetClassMethod(UsVal& v, Token* t)
{
   UsType* ty = v.Type();
   if (ty != nullptr)
   {
      auto iter = ty->mMethods.find(t->val);
      if (iter == ty->mMethods.end())
      {
         std::stringstream ss;
         ss << "Invalid Method Name '" << t->val << "'.";
         SemErr(ss.str().c_str());
      }
      else
      {
         UsVal retVal          = v;
         retVal.mMethodNamePtr = t->val;
         if (mConst->mObserverPtr)
         {
            if (iter->second.mFlags & UsFunction::cSTATIC)
            {
               mConst->mObserverPtr->SpecialToken(UsObserver::cSTATIC_METHOD, t);
            }
            else
            {
               mConst->mObserverPtr->SpecialToken(UsObserver::cMETHOD, t);
            }
         }
         return retVal;
      }
   }
   else
   {
      SemErr("Cannot call a method on type-less object");
   }
   return UsVal();
}

// Ensure that the variable (in 'ident') is a valid within the given class/script_struct (in 'v')
UsVal UsCtx::GetClassVariable(UsVal& v, Token* t)
{
   UsType* ty = v.Type();
   if (ty != nullptr)
   {
      std::string varName = t->val;
      auto        iter    = ty->mVariables.find(varName);
      if (iter == ty->mVariables.end())
      {
         std::stringstream ss;
         ss << "Invalid Variable Name '" << t->val << "'.";
         SemErr(ss.str().c_str());
      }
      else
      {
         Ty* varTypePtr = iter->second;
         if (varTypePtr)
         {
            UsVal retVal;
            retVal.mType = varTypePtr;
            return retVal;
         }
      }
   }
   else
   {
      SemErr("Cannot access a variable on type-less object");
   }
   return UsVal();
}

UsVal UsCtx::Call(UsVal& fn, UsValList& args, UsVal* aReturnVal)
{
   if (fn.mMethodNamePtr != nullptr && fn.Type())
   {
      std::vector<int> argTypes(args.size());
      for (size_t i = 0; i < args.size(); ++i)
      {
         if (args[i].mInitList)
         {
            args[i] = RealizeContainer(*args[i].mInitList, nullptr);
         }
         if (args[i].Type())
         {
            argTypes[i] = args[i].Type()->Id();
         }
      }
      UsFunction* fnPtr = fn.Type()->FindMethod(fn.mMethodNamePtr, argTypes);
      if (fnPtr)
      {
         return mConst->ValFromType(mConst->mTypes->GetType(fnPtr->mReturnType));
      }
      else
      {
         std::stringstream ss;
         ss << "Invalid method call to '" << fn.mMethodNamePtr << "'";
         SemErr(ss.str().c_str());
      }
   }
   else if (fn.mSym && fn.mSym->IsPrototype())
   {
      const UsFunction* protoPtr = mConst->mTypes->GetPrototype(fn.mSym->mRTypeId);
      if (protoPtr)
      {
         if (protoPtr->mArgs.size() == (int)args.size())
         {
            bool ok = true;
            // std::vector<UsVal> convertedValues;
            for (size_t i = 0; i < args.size() && ok; ++i)
            {
               int     id           = protoPtr->mArgs[i];
               UsType* protoArgType = mConst->mTypes->GetType(id);
               UsVal   v            = ImplicitCast(args[i], protoArgType);
               if (!v.mType)
               {
                  SemErr("Cannot convert parameter.");
                  ok = false;
               }
            }
            if (ok)
            {
               return mConst->ValFromType(mConst->mTypes->GetType(protoPtr->mReturnType));
            }
         }
         else
         {
            SemErr("Invalid number of arguments");
         }
      }
   }
   else if (mConst->tyVar && mConst->tyVar == fn.mType)
   {
      UsVal r;
      r.mType = mConst->tyVar;
      return r;
   }
   else
   {
      SemErr("Invalid method call");
   }

   return UsVal();
}

UsType* UsCtx::qType(Token* t)
{
   switch (t->kind)
   {
   case Parser::_string:
      return mConst->tyString;
   case Parser::_int:
      return mConst->tyInt;
   case Parser::_double:
      return mConst->tyDouble;
   case Parser::_char:
      return mConst->tyChar;
   case Parser::_bool:
      return mConst->tyBool;
   default:
      return mConst->mTypes->FindType(t->val);
   }
}

Ty* UsCtx::Type(Token* t)
{
   Ty* ty = qType(t);
   if (ty == nullptr)
   {
      std::stringstream ss;
      ss << "Invalid Type: '" << t->val << "'";
      SemErr(ss.str().c_str());
   }
   return ty;
}

UsType* UsCtx::qType(UsType* aTypePtr, std::pair<UsType*, UsType*> aArgs)
{
   UsTypeList::TemplateType tt;
   if (aTypePtr) // should be valid
   {
      tt.first         = aTypePtr->mTypeId;
      tt.second.first  = aArgs.first ? aArgs.first->mTypeId : 0;
      tt.second.second = aArgs.second ? aArgs.second->mTypeId : 0;
      UsType* specPtr  = mConst->mTypes->GetType(tt);
      if (!specPtr)
      {
         SemErr("Invalid template specification");
      }
      return specPtr;
   }
   return nullptr;
}

UsVal UsCtx::RealizeContainer(UsInitializerList& aInitializer, UsType* aLhsType)
{
   // Tries to create the most specific container from an initializer list.
   std::vector<UsInitializerListData::Entry>& entries = aInitializer.mDataPtr->mEntries;

   if (aLhsType)
   {
      // If a LHS type is given, that container type is used
      UsType* keyTypePtr = GetContainerKey(aLhsType);
      UsType* valTypePtr = GetContainerValue(aLhsType);
      if (valTypePtr)
      {
         for (size_t i = 0; i < entries.size(); ++i)
         {
            ImplicitCast(entries[i].val, valTypePtr);
            if (keyTypePtr)
            {
               ImplicitCast(entries[i].key, keyTypePtr);
            }
         }
      }
      else if (!(aLhsType->mFlags & UsType::cCONSTRUCTIBLE))
      {
         SemErr("Type not constructible.");
      }
      return NewVal(aLhsType);
   }
   else
   {
      if (entries.empty())
      {
         return NewVal(mConst->mTypes->FindType("Array<Object>"));
      }
      else
      {
         UsType* specificVal = entries.front().val.mType;
         UsType* specificKey = entries.front().key.mType;
         if (specificVal)
         {
            for (size_t i = 1; i < entries.size(); ++i)
            {
               if (specificVal != entries[i].val.Type())
               {
                  specificVal = mConst->tyObject;
                  break;
               }
            }
            if (specificKey)
            {
               for (size_t i = 1; i < entries.size(); ++i)
               {
                  if (specificKey != entries[i].key.Type())
                  {
                     specificKey = mConst->tyObject;
                     break;
                  }
               }
            }
         }
         if (specificVal)
         {
            if (specificKey)
            {
               UsType* basePtr = mConst->mTypes->FindType("Map");
               if (basePtr)
               {
                  return NewVal(mConst->mTypes->GetType(
                     UsTypeList::TemplateType(basePtr->Id(),
                                              UsTypeList::TemplateArgs(specificKey->mTypeId, specificVal->mTypeId))));
               }
            }
            else
            {
               UsType* basePtr = mConst->mTypes->FindType("Array");
               if (basePtr)
               {
                  return NewVal(mConst->mTypes->GetType(
                     UsTypeList::TemplateType(basePtr->Id(), UsTypeList::TemplateArgs(specificVal->mTypeId, 0))));
               }
            }
         }
      }
   }
   return UsVal();
}

UsVal UsCtx::NewInitializerList()
{
   UsVal v;
   v.mInitList = new UsInitializerList;
   return v;
}

UsVal UsCtx::VarDecl(UsType* ty, Token* t, int flags /*= 0*/)
{
   if (ty)
   {
      if (mGlobalVarMode)
         flags |= UsVal::cGLOBAL_VARIABLE;
      UsSymbol* sym = AddEntry(ty, t, flags);
      if (mConst->mObserverPtr)
         mConst->mObserverPtr->SpecialToken(UsObserver::cLOCAL_VARIABLE, t);
      if (sym)
      {
         return SymVal(sym);
      }
      else if (flags & UsVal::cEXTERN_VARIABLE) // extern is allowed if varaible is already defined, but make sure it's
                                                // the type specified
      {
         std::string name   = t->val;
         UsSymbol*   symPtr = mSym->Search(name, 1, mSequenceNumberLimit);
         if (ty->Id() != symPtr->mRTypeId)
         {
            SemErr("extern variable type does not match");
         }
      }
      else if (0 == (flags & UsVal::cEXTERN_VARIABLE)) // extern may be done even if variable is already in scope
                                                       // probably should check that
      {
         SemErr("Cannot reuse variable name.");
      }
   }
   // ERR
   return UsVal();
}

void UsCtx::AddToInitializerList(UsVal& initList, UsVal& p1, UsVal* p2) /*UsVal& val, UsVal* key) */
{
   if (!initList.mInitList)
      return;
   UsInitializerList*           initListPtr = initList.mInitList.Pointer();
   UsInitializerListData::Entry e;
   if (p2)
   {
      e.key = p1;
      e.val = *p2;
   }
   else
   {
      e.val = p1;
   }
   if (!initListPtr->mDataPtr->mEntries.empty())
   {
      if (initListPtr->mDataPtr->mEntries[0].key.mType)
      {
         if (!e.key.mType)
            SemErr("No key specified");
      }
      else
      {
         if (e.key.mType)
            SemErr("Initializer entry has key, unlike ealier entries.");
      }
   }
   initListPtr->mDataPtr->mEntries.push_back(e);
}

UsType* UsCtx::GetContainerValue(UsType* aType)
{
   return aType->mTemplateArgs.second ? aType->mTemplateArgs.second : aType->mTemplateArgs.first;
}

UsType* UsCtx::GetContainerKey(UsType* aType)
{
   if (aType)
      return aType->mTemplateArgs.second ? aType->mTemplateArgs.first : nullptr;
   return nullptr;
}

UsVal UsCtx::Op(UsVal& lhs, UsVal& rhs, int op)
{
   switch (op)
   {
   case cASSIGN:
   case cASSIGN_INITIAL:
   {
      UsVal rhs2 = ImplicitCast(rhs, lhs.Type());
      if (rhs2.Type() == lhs.Type())
      {
         return mConst->ValFromType(lhs.Type());
      }
      return UsVal();
   }
   case cOR:
   case cAND:
   case cEQ:
   case cNE:
   case cLT:
   case cGT:
   case cLE:
   case cGE:
      return mConst->ValFromType(mConst->tyBool);
   case cADD:
   case cSUBTRACT:
   case cMULTIPLY:
   case cDIVIDE:
      if (lhs.mType)
      {
         return mConst->ValFromType(lhs.mType);
      }
      return UsVal();
   default:
      return UsVal();
   }
}

UsVal UsCtx::Op(UsVal& lhs, UsVal& idx, UsVal& rhs, int op)
{
   // Change index operator to a call to Set() or Get()
   if (op == cINDEX && lhs.mType)
   {
      if (rhs.mInitList)
      {
         rhs = RealizeContainer(*rhs.mInitList, GetContainerValue(lhs.mType));
      }
      if (rhs.mType)
      {
         UsVal getMethod          = lhs;
         getMethod.mMethodNamePtr = "Set";
         UsValList args;
         args.push_back(idx);
         args.push_back(rhs);
         return Call(getMethod, args);
      }
      else
      {
         UsVal getMethod          = lhs;
         getMethod.mMethodNamePtr = "Get";
         UsValList args;
         args.push_back(idx);
         return Call(getMethod, args);
      }
   }
   return UsVal();
}

UsVal UsCtx::Op(UsVal& lhs, int op)
{
   switch (op)
   {
   case cNOT:
   {
      return mConst->ValFromType(mConst->tyBool);
   }
   default:
      return lhs;
   }
}

UsVal UsCtx::Ctor(Ty* aType, const UsValList& aArgs)
{
   if (aType)
   {
      if (aType->mFlags & UsType::cCONSTRUCTIBLE)
      {
         return mConst->ValFromType(aType);
      }
      else
      {
         SemErr("Type not constructible.");
      }
   }
   return UsVal();
}

UsVal UsCtx::Cast(UsVal& v, UsType* ty)
{
   return ExplicitCast(v, ty);
}

UsVal UsCtx::StringLiteral(Token* t)
{
   return mConst->ValFromType(mConst->tyString);
}

UsVal UsCtx::DoubleLiteral(Token* t)
{
   return mConst->ValFromType(mConst->tyDouble);
}

UsVal UsCtx::IntLiteral(Token* t)
{
   return mConst->ValFromType(mConst->tyInt);
}

UsVal UsCtx::BoolLiteral(Token* t)
{
   return mConst->ValFromType(mConst->tyBool);
}

UsVal UsCtx::NullLiteral(Token* t)
{
   return mConst->ValFromType(mConst->tyNull);
}

UsVal UsCtx::NewVal(UsType* ty)
{
   UsVal v;
   v.mType = ty;
   return v;
}

std::string UsCtx::Text(Token* t)
{
   std::string s;
   s.assign(t->val, t->val + t->len);
   return s;
}

UsType* UsCtx::FuncRType()
{
   if (mFuncDefSym)
   {
      const UsFunction* fn = mConst->mTypes->GetPrototype(mFuncDefSym->mRTypeId);
      if (fn)
      {
         return mConst->mTypes->GetType(fn->mReturnType);
      }
   }
   return nullptr;
}

UsType* UsCtx::RType(UsSymbol* s)
{
   UsType* ty = nullptr;
   if (s)
   {
      ty = mConst->mTypes->GetType(s->mRTypeId);
   }
   return ty;
}

UsVal UsCtx::Search(Token* aIdent)
{
   std::string name = Text(aIdent);

   UsVal v;
   v.mSym = Search(name);
   if (!v.mSym)
   {
      if (IsThisMethod(aIdent))
      {
         UsVal thisVal = ThisValue();
         return GetAttribute(thisVal, aIdent);
      }
      if (mConst->tyBuiltin)
      {
         if (mConst->tyBuiltin->HasStaticMethod(aIdent->val))
         {
            return GetAttribute(mConst->tyBuiltin, aIdent);
         }
      }
   }

   if (v.mSym)
   {
      if (mConst->mObserverPtr)
      {
         if (v.mSym->mStorageType == UsSymbol::cSE_PARAMETER)
         {
            mConst->mObserverPtr->SpecialToken(UsObserver::cPARAMETER, aIdent);
         }
         else
         {
            mConst->mObserverPtr->SpecialToken(UsObserver::cLOCAL_VARIABLE, aIdent);
         }
      }
      v.mType = mConst->mTypes->GetType(v.mSym->mRTypeId);
   }
   return v;
}

UsVal UsCtx::SymVal(UsSymbol* aSym)
{
   UsVal v;
   v.mType = mConst->mTypes->GetType(aSym->mRTypeId);
   return v;
}


UsSymbol* UsCtx::AddEntry(UsType* type, Token* aIdent, int aVarFlags /*=0*/)
{
   if (!type)
      return nullptr;
   UsSymbol sym;
   if (aIdent->pos > std::numeric_limits<decltype(sym.mPosition)>::max())
      return nullptr;
   sym.mEntryType      = UsSymbol::cSE_VARIABLE;
   sym.mStorageType    = UsSymbol::cSE_AUTOMATIC;
   sym.mPosition       = ut::safe_cast<unsigned short, int>(aIdent->pos);
   sym.mName           = Text(aIdent);
   sym.mRTypeId        = type->Id();
   sym.mSequenceNumber = mConst->NextSequenceNumber();

   UsSymbolTable* tbl = mSym;
   if (aVarFlags & UsVal::cEXTERN_VARIABLE)
   {
      sym.mStorageType = UsSymbol::cSE_EXTERN;
      if (mAllowGlobals)
      {
         tbl = mSym->GetParentScope();
         if (!tbl)
            tbl = mSym;
      }
   }
   else if (aVarFlags & UsVal::cGLOBAL_VARIABLE)
   {
      if (mAllowGlobals)
      {
         tbl = mSym->GetParentScope();
         if (!tbl)
            tbl = mSym;
      }
      sym.mStorageType = UsSymbol::cSE_GLOBAL;
   }
   else if (aVarFlags & UsVal::cSTATIC_VARIABLE)
   {
      sym.mStorageType = UsSymbol::cSE_STATIC;
   }
   else if (aVarFlags & UsVal::cPARAMETER_VARIABLE)
   {
      sym.mStorageType = UsSymbol::cSE_PARAMETER;
   }
   UsSymbol* previousDecl = tbl->Search(sym.mName, 0, mSequenceNumberLimit);
   if (!previousDecl)
   {
      sym.mTablePtr        = tbl;
      sym.mLexicalScopePtr = mSym;
      UsSymbol& rsym = tbl->mEntries[sym.mName] = sym;
      if (GetObserver())
         GetObserver()->VarDecl(&rsym, aIdent);
      return &rsym;
   }
   else
   {
      return nullptr;
   }
}

UsVal UsCtx::AddEntry(Token* aIdent, Fn& aFunc, bool aExtern, UsCtx* aFnCtx)
{
   UsSymbol sym;
   if (aIdent->pos > std::numeric_limits<decltype(sym.mPosition)>::max())
      return UsVal();

   int protoid = mConst->mTypes->AddPrototype(aFunc);
   sym.mName   = Text(aIdent);
   if (Search(sym.mName) == nullptr)
   {
      sym.mLexicalScopePtr = sym.mTablePtr = mSym;
      if (aFnCtx && aFnCtx->mSym)
      {
         sym.mLexicalScopePtr = aFnCtx->mSym;
      }
      sym.mEntryType      = static_cast<unsigned char>(UsSymbol::cSE_SCRIPT);
      sym.mStorageType    = aExtern ? static_cast<unsigned char>(UsSymbol::cSE_EXTERN) :
                                      static_cast<unsigned char>(UsSymbol::cSE_NOT_APPLICABLE);
      sym.mPosition       = ut::safe_cast<unsigned short, int>(aIdent->pos);
      sym.mRTypeId        = protoid;
      sym.mSequenceNumber = mConst->NextSequenceNumber();
      UsSymbol& rsym      = mSym->mEntries[sym.mName];
      rsym                = sym;
      UsVal r;
      r.mSym = &rsym;
      return r;
   }
   else
   {
      return UsVal();
   }
}

UsSymbol* UsCtx::AddAppVariable(UsType* type, const std::string& aName)
{
   if (!type)
      return nullptr;
   UsSymbol sym;
   sym.mEntryType      = UsSymbol::cSE_VARIABLE;
   sym.mStorageType    = UsSymbol::cSE_GLOBAL;
   sym.mPosition       = 0;
   sym.mName           = aName;
   sym.mRTypeId        = type->Id();
   sym.mSequenceNumber = mConst->NextSequenceNumber();
   if (mSym->Search(aName, 0) == nullptr)
   {
      sym.mTablePtr        = mSym;
      sym.mLexicalScopePtr = mSym;
      UsSymbol& rsym = mSym->mEntries[sym.mName] = sym;
      return &rsym;
   }
   else
   {
      return nullptr;
   }
}

UsVal UsCtx::ThisValue()
{
   if (mSym->mThisTypePtr)
   {
      return mConst->ValFromType(mSym->mThisTypePtr);
   }
   return UsVal();
}

bool UsCtx::IsThisMethod(Token* t)
{
   if (!mSym->mThisTypePtr)
      return false;
   bool isMethod = mSym->mThisTypePtr->mMethods.find(t->val) != mSym->mThisTypePtr->mMethods.end();
   if (isMethod)
   {
      if (mConst->mObserverPtr)
         mConst->mObserverPtr->SpecialToken(UsObserver::cMETHOD, t);
   }
   return isMethod;
}

void UsCtx::FuncDefStart(UsVal& aScriptVal)
{
   mFuncDefSym = aScriptVal.mSym;
}

void UsCtx::SetRType(UsFunction& fn, UsType* type)
{
   if (type)
   {
      fn.mReturnType = type->Id();
   }
}

void UsCtx::AddParam(UsFunction& fn, UsType* type, Token* aIdent, bool aIsFormalParam)
{
   if (type)
   {
      if (aIdent && mConst->mObserverPtr)
         mConst->mObserverPtr->SpecialToken(UsObserver::cPARAMETER, aIdent);
      fn.mArgs.push_back(type->Id());
      if (aIsFormalParam && aIdent)
         AddEntry(type, aIdent, UsVal::cPARAMETER_VARIABLE);
   }
}

UsCtx* UsCtx::ScriptContext()
{
   UsCtx* c = this;
   while (c)
   {
      if (c->mBlockType == cSCRIPT_BLOCK)
         return c;
      c = c->mParentCtx;
   }
   return c;
}

UsCtx::~UsCtx()
{
   if (GetObserver() && mFuncDefSym)
   {
      GetObserver()->ScriptDefinition(mSym, mFuncDefSym);
   }
   if (mConst->mObserverPtr)
      mConst->mObserverPtr->PopScope(mSym);
}

UsCtx::UsCtx(UsCtx& p, UsSymbolTable::TableLinkage aLinkage)
   : mParentCtx(&p)
   , mFuncDefSym()
   , mContextFlags(0)
{
   mBlockType           = cNORMAL_BLOCK;
   mSequenceNumberLimit = p.mSequenceNumberLimit;
   mGlobalVarMode       = false;
   mConst               = p.mConst;
   mSym                 = p.mSym->AddInner(aLinkage);
   mAllowGlobals        = p.mAllowGlobals;
   if (mConst->mObserverPtr)
      mConst->mObserverPtr->PushScope(mSym);
}

UsCtx::UsCtx(UsConstCtx* c)
   : mParentCtx(nullptr)
   , mContextFlags(0)
   , mBlockType(cSCRIPT_BLOCK)
{
   mFuncDefSym          = nullptr;
   mSequenceNumberLimit = cLARGE_INT;
   mGlobalVarMode       = false;
   mAllowGlobals        = true;
   mConst               = c;
   mSym                 = new UsSymbolTable(c->mSymbolTablePoolPtr);
   if (mConst->mObserverPtr)
      mConst->mObserverPtr->PushScope(mSym);
}

UsVal UsCtx::Identifier(Token* aIdent)
{
   UsVal v = Search(aIdent);
   if (v.IsEmpty())
      SemErr(COCO_STR("Unknown Identifier."));
   return v;
}

void UsCtx::BeginCall(UsVal& fn, int aPos)
{
   UsObserver* obs = GetObserver();
   if (obs)
      obs->BeginCall(fn, aPos);
}
void UsCtx::EndCall(UsVal& fn, int aPos)
{
   UsObserver* obs = GetObserver();
   if (obs)
      obs->EndCall(fn, aPos);
}
void UsCtx::CallArgNext(int aPos)
{
   UsObserver* obs = GetObserver();
   if (obs)
      obs->CallArgNext(aPos);
}

int UsCtx::BeginStat()
{
   if (GetObserver())
      GetObserver()->AtStat();
   return -1;
}


std::string UsVal::TypeName()
{
   if (mType)
   {
      return mType->mName;
   }
   return "No Type";
}

UsInitializerList::UsInitializerList()
   : mRefCount(new UtReferenceCount(0))
{
   mDataPtr = new UsInitializerListData;
}

UsInitializerList::~UsInitializerList()
{
   delete mDataPtr;
}
} // namespace Us
