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

#include "WsfScriptData.hpp"

#include <sstream>

#include "CommandDocumentation.hpp"
#include "GrammarDocumentation.hpp"
#include "Us/Parser.h"
#include "Us/Scanner.h"
#include "UtStringUtil.hpp"
#include "WsfParseDefinitions.hpp"
#include "WsfParseGrammar.hpp"
#include "WsfParseIndex.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseScriptRule.hpp"
#include "WsfParseType.hpp"
#include "WsfParseTypeLoadRule.hpp"
#include "WsfParseUtil.hpp"

void wizard::WsfScriptParser::Rebuild(WsfParseNode* aParseTree, WsfParseType& aSymbolTable)
{
   Walk(aParseTree, aSymbolTable);

   for (std::map<size_t, std::vector<RecordedScript>>::const_iterator i = mRecordedScripts.begin();
        i != mRecordedScripts.end();
        ++i)
   {
      const std::vector<RecordedScript>& recordedList = i->second;
      for (auto&& record : recordedList)
      {
         DeferredProcessScript(record.mTypeStack, record.mNode, *record.mSymbolTable);
      }
   }
}

void wizard::WsfScriptParser::Walk(WsfParseNode* aParseTree, WsfParseType& aSymbolTable)
{
   if (!aParseTree)
      return;

   WsfParseNode* end = aParseTree->Next(false);
   WsfParseNode* n   = aParseTree->Down();
   while (n != end && n)
   {
      // If we hit a type node, change type and recurse
      bool recursing = false;
      if (n->GetAuxiliaryValue())
      {
         if (n->GetAuxiliaryValue()->mDataType == WsfParseAuxData::cTYPE_DATA)
         {
            WsfParseTypeInfoData* typeDataPtr = (WsfParseTypeInfoData*)n->GetAuxiliaryValue();
            TypeEntry             newType;
            bool                  hasType = false;
            if (WsfParseTypeInfoData::cLOAD_TYPE == typeDataPtr->mOperation)
            {
               newType.mSaveKey = typeDataPtr->mLoadKey;
               newType.mLoadKey = typeDataPtr->mLoadKey;
               hasType          = true;
            }
            else if (WsfParseTypeInfoData::cCREATE_TYPE == typeDataPtr->mOperation)
            {
               newType.mSaveKey = typeDataPtr->mSaveKey;
               newType.mLoadKey = typeDataPtr->mLoadKey;
               hasType          = true;
            }
            if (hasType)
            {
               WsfParseNode* c = n->Right();
               while (c)
               {
                  if (strcmp(c->mType, "block") == 0)
                  {
                     // script block in a script_struct
                     WsfParseNode* ln = n->Left();
                     if (ln)
                        ln = ln->Left();
                     if (ln && ln->mRulePtr->mRuleType == WsfParseRule::cLITERAL)
                     {
                        std::string scriptStructName = ((WsfParseLiteral*)ln->mRulePtr)->mText;
                        if (scriptStructName == "script_struct")
                        {
                           scriptStructName = newType.mSaveKey.back().Get();
                           UsType* classPtr = mConstCtx->mTypes->FindType(scriptStructName);
                           if (classPtr == nullptr)
                           {
                              // Class
                              classPtr        = new UsType(mConstCtx->mTypes);
                              classPtr->mName = scriptStructName;
                              classPtr->mFlags |= UsType::cCONSTRUCTIBLE;
                              classPtr->mFlags |= UsType::cLESS_THAN_COMPARABLE;
                              classPtr->mFlags |= UsType::cEQUALITY_COMPARABLE;
                              UsType* basePtr = mConstCtx->mTypes->FindTypeOrMakeTemplate("Object");
                              if (basePtr)
                              {
                                 classPtr->AddInheritedType(basePtr->Id());
                              }
                              mConstCtx->mTypes->AddType(scriptStructName, classPtr);
                           }
                           else
                           {
                              // Clear methods and variables as these will get rebuilt
                              classPtr->mMethods.clear();
                              classPtr->mVariables.clear();
                           }
                        }
                     }

                     mTypeStack.push_back(newType);
                     recursing = true;
                     Walk(c, aSymbolTable);
                     mTypeStack.pop_back();
                  }
                  c = c->Right();
               }
            }
         }
      }
      if (!recursing)
      {
         if (n->GetFlags() & WsfParseNode::cSCRIPT_MASK)
         {
            if (!mTypeStack.empty() && (mTypeStack.at(0).mSaveKey.front().Get() == "script_struct"))
            {
               std::string scriptStructName = mTypeStack.at(0).mSaveKey.back().Get();
               UsType*     classPtr         = mConstCtx->mTypes->FindType(scriptStructName);
               if (classPtr != nullptr)
               {
                  if (n->GetFlags() & WsfParseNode::cSCRIPT_FUNCTION)
                  {
                     // Methods
                     std::string scriptFn      = n->mValue.Text();
                     size_t      leftParenPos  = scriptFn.find_first_of("(");
                     size_t      rightParenPos = scriptFn.find_first_of(")");
                     if ((leftParenPos != std::string::npos) && (rightParenPos != std::string::npos))
                     {
                        // Split the sub-string up to the left paren on white-space
                        std::string              scriptName = scriptFn.substr(0, leftParenPos);
                        std::vector<std::string> scriptNameArr;
                        UtStringUtil::ParseWhitespace(scriptName, scriptNameArr);
                        if (!scriptNameArr.empty() && (scriptNameArr.size() > 1))
                        {
                           // Get the script name; sub-string just before the left paren
                           std::string fnName = scriptNameArr.back();
                           scriptNameArr.pop_back();

                           // Rejoin with no spaces; this is the return type
                           std::string rTypeStr = UtStringUtil::Join("", scriptNameArr);
                           UsType*     rTypePtr = mConstCtx->mTypes->FindTypeOrMakeTemplate(rTypeStr);
                           if (rTypePtr)
                           {
                              // Return type is valid
                              UsFunction fn;
                              fn.mFlags      = UsFunction::cAPP_METHOD;
                              fn.mReturnType = rTypePtr->Id();

                              // Get the function arguments; could be empty but if not empty need to be valid types
                              std::vector<int> argTypes;
                              if ((leftParenPos + 1) < rightParenPos)
                              {
                                 // Split on commas
                                 std::string argStr = scriptFn.substr(leftParenPos + 1, rightParenPos - leftParenPos - 1);
                                 std::vector<std::string> argsArr;
                                 UtStringUtil::Parse(argStr, argsArr, ',');
                                 for (auto& arg : argsArr)
                                 {
                                    std::vector<std::string> argArr;
                                    UtStringUtil::ParseWhitespace(arg, argArr);
                                    if (argArr.size() > 1)
                                    {
                                       UsType* argTypePtr = mConstCtx->mTypes->FindTypeOrMakeTemplate(argArr.front());
                                       if (argTypePtr)
                                       {
                                          fn.mArgs.push_back(argTypePtr->Id());
                                          argTypes.push_back(argTypePtr->Id());
                                       }
                                    }
                                 }
                              }

                              // Add the method to the class
                              if (!classPtr->FindMethod(fnName.c_str(), argTypes))
                              {
                                 classPtr->mMethods.insert(std::make_pair(mConstCtx->mTypes->GetString(fnName.c_str()), fn));
                              }
                           }
                        }
                     }
                  }
                  else if (n->GetFlags() & WsfParseNode::cSCRIPT_VARIABLES)
                  {
                     // Variables
                     std::string              scriptVars = n->mValue.Text();
                     std::vector<std::string> scriptArr;
                     UtStringUtil::Parse(scriptVars, scriptArr, ";");
                     for (auto& v : scriptArr)
                     {
                        std::vector<std::string> scriptNameArr;
                        UtStringUtil::ParseWhitespace(v, scriptNameArr);
                        if (scriptNameArr.size() > 1)
                        {
                           std::string varType = scriptNameArr.at(0);
                           std::string varName = scriptNameArr.at(1);
                           auto        iter = classPtr->mVariables.find(mConstCtx->mTypes->GetString(varName.c_str()));
                           if (iter == classPtr->mVariables.end())
                           {
                              UsType* varTypePtr = mConstCtx->mTypes->FindType(varType);
                              if (varTypePtr)
                              {
                                 classPtr->mVariables.insert(
                                    std::make_pair(mConstCtx->mTypes->GetString(varName.c_str()), varTypePtr));
                              }
                           }
                        }
                     }
                  }
               }
            }

            RecordScript(n, aSymbolTable);
         }
         n = n->Next();
      }
      else
      {
         n = n->GetParent();
         if (n)
         {
            n = n->Next(false);
         }
      }
   }
}

namespace
{
// int                  sCurrentOffset;
void NewTok(Us::Token* t, void* aCurrentScriptPtr)
{
   wizard::WsfScriptInfo* currentScriptPtr = static_cast<wizard::WsfScriptInfo*>(aCurrentScriptPtr);
   wizard::WsfScriptToken tok;
   using namespace Us;
   switch (t->kind)
   {
   case Parser::_EOF:
      return;
   case Parser::_ident:
      tok.mTokenType = wizard::WsfScriptToken::cIDENTIFIER;
      break;
   case Parser::_number:
   case Parser::_real_number:
      tok.mTokenType = wizard::WsfScriptToken::cNUMBER;
      break;
   case Parser::_string_literal:
   case Parser::_char_literal:
      tok.mTokenType = wizard::WsfScriptToken::cSTRING;
      break;
   case Parser::_semicolon:
   case Parser::_lparen:
   case Parser::_rparen:
   case Parser::_assign:
   case Parser::_eq:
   case Parser::_ne:
   case Parser::_ge:
   case Parser::_le:
   case Parser::_and:
   case Parser::_or:
   case Parser::_plus:
   case Parser::_minus:
      tok.mTokenType = wizard::WsfScriptToken::cOPERATOR;
      break;
   case Parser::_do:
   case Parser::_if:
   case Parser::_for:
   case Parser::_foreach:
   case Parser::_else:
   case Parser::_while:
   case Parser::_break:
   case Parser::_continue:
   case Parser::_return:
   case Parser::_null:
   case Parser::_true:
   case Parser::_false:
      tok.mTokenType = wizard::WsfScriptToken::cKEYWORD;
      break;
   case Parser::_string:
   case Parser::_int:
   case Parser::_double:
   case Parser::_char:
   case Parser::_bool:
      tok.mTokenType = wizard::WsfScriptToken::cIDENTIFIER;
      break;
   case Parser::_global:
   case Parser::_static:
   case Parser::_extern:
      tok.mTokenType = wizard::WsfScriptToken::cKEYWORD;
      break;
   default:
      return;
   }
   size_t startPos = t->pos + currentScriptPtr->mRange.GetBegin();
   tok.mRange.SetRange(startPos, startPos + t->len);
   currentScriptPtr->mTokens.push_back(tok);
}
} // namespace
//
// wizard::WsfScriptTypes::WsfScriptTypes(WsfExe* aExePtr)
// {
//    std::shared_ptr<WsfParseDefinitions> definitions = aExePtr->GetParseDefinitions();
//    mConstCtx = new UsConstCtx;
//
//    UsTypeList* types = mConstCtx->mTypes;
//    UsType* voidT = new UsType(types);
//    voidT->mName = "void";
//    mConstCtx->mTypes->AddType("void", voidT);
//    UsType* intT = new UsType(types);
//    intT->mName = "int";
//    mConstCtx->mTypes->AddType("int", intT);
//    UsType* doubleT = new UsType(types);
//    doubleT->mName = "double";
//    mConstCtx->mTypes->AddType("double", doubleT);
//    UsType* boolT = new UsType(types);
//    boolT->mName = "bool";
//    mConstCtx->mTypes->AddType("bool", boolT);
//    UsType* charT = new UsType(types);
//    charT->mName = "char";
//    mConstCtx->mTypes->AddType("char", charT);
//    UsType* stringT = new UsType(types);
//    stringT->mName = "string";
//    mConstCtx->mTypes->AddType("string", stringT);
//
//    intT->mImplicitCastTypes.push_back(doubleT->Id());
//    intT->mImplicitCastTypes.push_back(boolT->Id());
//    doubleT->mImplicitCastTypes.push_back(intT->Id());
//    doubleT->mImplicitCastTypes.push_back(boolT->Id());
//    mConstCtx->tyBool = boolT;
//    mConstCtx->tyVoid = voidT;
//    mConstCtx->tyInt = intT;
//    mConstCtx->tyDouble = doubleT;
//    mConstCtx->tyString = stringT;
//    mConstCtx->tyChar = charT;
//
//
//    if (!definitions.IsNull())
//    {
//       for (std::set<std::string>::const_iterator i = definitions->mScriptClasses.begin(); i !=
//       definitions->mScriptClasses.end(); ++i)
//       {
//          if (mConstCtx->mTypes->FindType(*i) == 0)
//          {
//             UsType* typePtr = new UsType(mConstCtx->mTypes);
//             typePtr->mName = *i;
//             mConstCtx->mTypes->AddType(*i, typePtr);
//          }
//       }
//    }
//    mDataPtr = new WsfScriptData(mConstCtx);
// }

namespace
{
bool ToInt(std::string aStr, int& aVal)
{
   std::stringstream ss(aStr);
   if (ss >> aVal)
      return true;
   return false;
}
void SubstTemplateType(UsTypeList* aTypes, int& aTypeId, std::string& aNewType)
{
   int intVal;
   if (ToInt(aNewType, intVal))
   {
      aTypeId = -intVal;
   }
}
} // namespace

void setflag(int& flags, bool aValue, int aBit)
{
   if (aValue)
      flags |= aBit;
   else
      flags &= ~aBit;
}

void wizard::WsfScriptTypes::InitType(const std::string&                                              aName,
                                      std::map<std::string, std::vector<const WsfGrammar::M::Expr*>>& aTypeExpressions)
{
   if (aTypeExpressions.find(aName) == aTypeExpressions.end())
      return;
   using namespace WsfGrammar::M;

   UsTypeList* types = mConstCtx->mTypes;

   std::vector<const WsfGrammar::M::Expr*> exprList = aTypeExpressions[aName];
   aTypeExpressions.erase(aName);

   for (auto&& e : exprList)
   {
      if (e->mRuleType == cRT_EXPRESSION && e->mType == "aux-script-class")
      {
         std::string type     = e->GetWord(0).mText;
         UsType*     classPtr = types->FindType(type);
         if (classPtr == nullptr)
         {
            if (type.find('<') == std::string::npos) // don't add template types
            {
               classPtr        = new UsType(types);
               classPtr->mName = type;
               types->AddType(type, classPtr);
            }
         }
         if (!classPtr)
            continue;
         for (size_t pi = 1; pi < e->words().size(); ++pi)
         {
            const Expr& classParam = e->GetWord(pi);
            if (classParam.mRuleType == cRT_LITERAL && !classParam.mLabel.empty())
            {
               if (classParam.mLabel == "constructible")
                  setflag(classPtr->mFlags, classParam.mText == "1", UsType::cCONSTRUCTIBLE);
               else if (classParam.mLabel == "cloneable")
                  setflag(classPtr->mFlags, classParam.mText == "1", UsType::cCLONEABLE);
               else if (classParam.mLabel == "container")
                  setflag(classPtr->mFlags, classParam.mText == "1", UsType::cCONTAINER);
               else if (classParam.mLabel == "less_compare")
                  setflag(classPtr->mFlags, classParam.mText == "1", UsType::cLESS_THAN_COMPARABLE);
               else if (classParam.mLabel == "equal_compare")
                  setflag(classPtr->mFlags, classParam.mText == "1", UsType::cEQUALITY_COMPARABLE);
               if (classParam.mLabel == "base_class")
               {
                  UsType* base = FindTypeOrMakeTemplate(classParam.mText, aTypeExpressions);
                  if (base)
                  {
                     classPtr->AddInheritedType(base->Id());
                  }
               }
               else if (classParam.mLabel == "explicit_cast")
               {
                  UsType* cast = FindTypeOrMakeTemplate(classParam.mText, aTypeExpressions);
                  if (cast)
                  {
                     classPtr->AddExplicitCastType(cast->Id());
                  }
               }
               else if (classParam.mLabel == "implicit_cast")
               {
                  UsType* cast = FindTypeOrMakeTemplate(classParam.mText, aTypeExpressions);
                  if (cast)
                  {
                     classPtr->AddImplicitCastType(cast->Id());
                  }
               }
            }
            else if (classParam.mRuleType == cRT_EXPRESSION && classParam.mType == "aux-script-method")
            {
               UsFunction               fn;
               std::string              rtype, name;
               std::vector<std::string> args;
               std::string              fnName   = classParam.GetWord(0).mText;
               bool                     isStatic = false;
               bool                     isVarArg = false;
               for (size_t mi = 1; mi < classParam.words().size(); ++mi)
               {
                  const Expr& methodParam = classParam.GetWord(mi);
                  if (methodParam.mRuleType == cRT_LITERAL && !methodParam.mLabel.empty())
                  {
                     if (methodParam.mLabel == "rtype")
                     {
                        rtype = methodParam.mText;
                     }
                     else if (methodParam.mLabel == "static")
                     {
                        isStatic = methodParam.mText == "1";
                     }
                     else if (methodParam.mLabel == "varargs")
                     {
                        isVarArg = methodParam.mText == "1";
                     }
                     else if (methodParam.mLabel == "parameter")
                     {
                        UsType* argTypePtr = FindTypeOrMakeTemplate(methodParam.mText, aTypeExpressions);
                        if (argTypePtr)
                        {
                           fn.mArgs.push_back(argTypePtr->Id());
                        }
                     }
                  }
               }

               UsType* rtypePtr = FindTypeOrMakeTemplate(rtype, aTypeExpressions);
               if (rtypePtr)
               {
                  fn.mReturnType = rtypePtr->Id();
                  fn.mFlags      = UsFunction::cAPP_METHOD;
                  if (isStatic)
                  {
                     fn.mFlags |= UsFunction::cSTATIC;
                  }
                  if (isVarArg)
                  {
                     fn.mFlags |= UsFunction::cVARARGS;
                  }
                  classPtr->mMethods.insert(std::make_pair(mConstCtx->mTypes->GetString(fnName.c_str()), fn));
               }
            }
         }
      }
   }
}

UsType* wizard::WsfScriptTypes::FindTypeOrMakeTemplate(const std::string& aName,
                                                       std::map<std::string, std::vector<const WsfGrammar::M::Expr*>>& aTypeExpressions)
{
   std::string base, arg1, arg2;
   int         args = UsTypeList::SplitTemplateType(aName, base, arg1, arg2);
   InitType(base, aTypeExpressions);
   if (args >= 2)
   {
      FindTypeOrMakeTemplate(arg2, aTypeExpressions);
   }
   if (args >= 1)
   {
      FindTypeOrMakeTemplate(arg1, aTypeExpressions);
   }
   return mConstCtx->mTypes->FindTypeOrMakeTemplate(aName);
}

wizard::WsfScriptTypes::WsfScriptTypes(const WsfGrammar::M::Expr& aGrammar)
{
   using namespace WsfGrammar::M;
   mConstCtx = new UsConstCtx;

   UsTypeList* types = mConstCtx->mTypes;

   std::map<std::string, std::vector<const Expr*>> typeExpressions, uninitializedTypes;

   for (size_t i = 0; i < aGrammar.words().size(); ++i)
   {
      const Expr& e = aGrammar.GetWord(i);
      if (e.mRuleType == cRT_EXPRESSION && e.mType == "aux-script-class")
      {
         std::string type = e.GetWord(0).mText;
         typeExpressions[type].push_back(&e);
      }
   }
   uninitializedTypes = typeExpressions;

   InitType("Array", uninitializedTypes);
   InitType("Map", uninitializedTypes);
   InitType("Set", uninitializedTypes);

   for (auto&& it : typeExpressions)
   {
      FindTypeOrMakeTemplate(it.first, uninitializedTypes);
   }

   mConstCtx->tyBool = types->FindType("bool");
   mConstCtx->tyNull = types->FindType("null");
   if (!types->FindType("void"))
   {
      UsType* voidT = new UsType(types);
      voidT->mName  = "void";
      mConstCtx->mTypes->AddType("void", voidT);
   }
   mConstCtx->tyVoid    = types->FindType("void");
   mConstCtx->tyInt     = types->FindType("int");
   mConstCtx->tyDouble  = types->FindType("double");
   mConstCtx->tyString  = types->FindType("string");
   mConstCtx->tyChar    = types->FindType("char");
   mConstCtx->tyObject  = types->FindType("Object");
   mConstCtx->tyBuiltin = types->FindType("__BUILTIN__");
   mConstCtx->tyVar     = types->FindType("var");
   // expand 1 & 2 to 1st and 2nd template types
   // expand 3 to TheType<1[,2]>
   const char*       templateMethods = "Array 1 Get int\n"
                                       "Array void Set int 1\n"
                                       "Array void PushBack 1\n"
                                       "Array 1 Front\n"
                                       "Array 1 Back\n"
                                       "Map void Set 1 2\n"
                                       "Map 2 Get 1"
                                       "Map 1 ElementKeyAtIndex int\n"
                                       "Map void __Insert 1 2\n"
                                       "Set void Insert 1\n"
                                       "Set 3 Union 3\n"
                                       "Set 3 Difference 3\n"
                                       "Set 3 Intersection 3\n"
                                       "Set void __Insert 1";
   std::stringstream tempss(templateMethods);
   char              line[1000];
   while (tempss.getline(line, 1000))
   {
      std::stringstream lss(line);
      std::string       className, rType, method;
      lss >> className >> rType >> method;
      std::vector<std::string> args;
      std::string              arg;
      while (lss >> arg)
      {
         args.push_back(arg);
      }
      UsType* typePtr = mConstCtx->mTypes->FindType(className);
      if (typePtr)
      {
         UsType::MethodMap::iterator iter = typePtr->mMethods.find(method.c_str());
         if (iter != typePtr->mMethods.end())
         {
            UsFunction& fn = iter->second;
            SubstTemplateType(mConstCtx->mTypes, fn.mReturnType, rType);
            for (int i = 0; i < fn.mArgs.size() && i < (int)args.size(); ++i)
            {
               SubstTemplateType(mConstCtx->mTypes, fn.mArgs[i], args[i]);
            }
         }
      }
   }


   // Initialization order may have caused some specialized template types to not have all template methods.
   // Go through each specialized template type and re-specialize if necessary
   for (size_t i = 1; i < types->mTypes.size(); ++i)
   {
      UsType* typePtr = types->mTypes[i];
      if (typePtr->mTemplateArgs.first)
      {
         std::string templateBase, arg1, arg2;
         UsTypeList::SplitTemplateType(typePtr->mName, templateBase, arg1, arg2);
         UsType* templateBasePtr = types->FindType(templateBase);
         if (templateBasePtr)
         {
            typePtr->ReSpecialize(templateBasePtr);
         }
      }
   }
   types->mArrayBase = types->FindTypeOrMakeTemplate("Array<Object>");
   types->mMapBase   = types->FindTypeOrMakeTemplate("Map<Object,Object>");
   types->mSetBase   = types->FindTypeOrMakeTemplate("Set<Object>");

   for (size_t i = 1; i < types->mTypes.size(); ++i)
   {
      UsType* typePtr = types->mTypes[i];
      if (typePtr->mTemplateArgs.first)
      {
         std::string templateBase, arg1, arg2;
         UsTypeList::SplitTemplateType(typePtr->mName, templateBase, arg1, arg2);
         UsType* templateBasePtr = types->FindType(templateBase);
         if (templateBasePtr)
         {
            if (templateBase == "Array")
               typePtr->mImplicitCastTypes.push_back(types->mArrayBase->Id());
            if (templateBase == "Set")
               typePtr->mImplicitCastTypes.push_back(types->mSetBase->Id());
            if (templateBase == "Map")
               typePtr->mImplicitCastTypes.push_back(types->mMapBase->Id());
         }
      }
      CopyInheritedMethods(typePtr);
   }
}


class ErrorSink : public Us::Errors
{
public:
   wizard::WsfScriptInfo* mScriptPtr;
   void SynErr(Us::Token* tok, int n) override { mScriptPtr->mErrors.push_back(std::make_pair(tok->pos, tok->len)); }
   void Error(Us::Token* tok, const char* s) override
   {
      mScriptPtr->mErrors.push_back(std::make_pair(tok->pos, tok->len));
   }
   void Warning(Us::Token* tok, const char* s) override {}
   void Warning(const char* s) override {}
   void Exception(const char* s) override {}
};


class ErrorNullSink : public Us::Errors
{
public:
   void SynErr(Us::Token* tok, int n) override {}
   void Error(Us::Token* tok, const char* s) override {}
   void Warning(Us::Token* tok, const char* s) override {}
   void Warning(const char* s) override {}
   void Exception(const char* s) override {}
};

class ErrorLog : public Us::Errors
{
public:
   std::vector<wizard::WsfScriptParser::ErrorRecord> mErrors;
   void                                              SynErr(Us::Token* tok, int n) override
   {
      const char* errTxt = GetStaticError(n);
      if (errTxt)
      {
         Error(tok, errTxt);
      }
   }
   void Error(Us::Token* tok, const char* s) override
   {
      wizard::WsfScriptParser::ErrorRecord e;
      e.mText     = s;
      e.mPosition = tok->pos;
      e.mLength   = tok->len;
      mErrors.push_back(e);
   }
   void Warning(Us::Token* tok, const char* s) override { Error(tok, s); }
   void Warning(const char* s) override {}
   void Exception(const char* s) override {}
};


void wizard::WsfScriptParser::RecordScript(WsfParseNode* aScriptBlockNode, WsfParseType& aSymbolTable)
{
   size_t loadOrder = aScriptBlockNode->LoadOrder();
   if (loadOrder == ut::npos)
   {
      // script is not delay loaded, we can safely process now
      DeferredProcessScript(mTypeStack, aScriptBlockNode, aSymbolTable);
   }
   else
   {
      // script needs to be processed later
      RecordedScript record;
      record.mTypeStack   = mTypeStack;
      record.mNode        = aScriptBlockNode;
      record.mSymbolTable = &aSymbolTable;
      mRecordedScripts[loadOrder].push_back(record);
   }
}

void wizard::WsfScriptParser::ProcessScript(WsfScriptInfo*     aScriptPtr,
                                            const char*        aTextPtr,
                                            size_t             aTextLength,
                                            UsCtx&             aStartCtx,
                                            int                aProcessFlags,
                                            WsfScriptObserver* aObserverPtr)
{
   Us::Scanner scanner((unsigned char*)aTextPtr, ut::cast_to_int(aTextLength));
   Us::Parser  parser(&scanner);
   if (aProcessFlags & cRECORD_ERROR_DETAILS)
   {
      delete parser.mErrorsPtr;
      parser.mErrorsPtr = new ErrorLog;
   }
   else
   {
      if (aProcessFlags & cRECORD_ERRORS)
      {
         ErrorSink* errPtr = new ErrorSink;
         delete parser.mErrorsPtr;
         parser.mErrorsPtr  = errPtr;
         errPtr->mScriptPtr = aScriptPtr;
      }
      else
      {
         delete parser.mErrorsPtr;
         parser.mErrorsPtr = new ErrorNullSink;
      }
   }
   mConstCtx->mParser      = &parser;
   mConstCtx->mObserverPtr = aObserverPtr; // new ScriptObserver(aScriptPtr);
   if (aProcessFlags & cRECORD_TOKENS)
   {
      scanner.NewTokCb         = &NewTok;
      scanner.mNewTokenDataPtr = aScriptPtr;
   }
   parser.mRootCx = &aStartCtx;
   switch (aScriptPtr->mBlockType)
   {
   case WsfScriptInfo::cFUNCTION:
      parser.Parse();

      // Change the type of the second token which is the script's name
      if ((aScriptPtr->mTokens.size() > 1))
      {
         aScriptPtr->mTokens[1].mTokenType = WsfScriptToken::cLOCAL_METHOD;
      }
      break;
   case WsfScriptInfo::cIMPLICIT_SCRIPT:
   case WsfScriptInfo::cSCRIPT_VARIABLES:
   {
      UsCtx ctx(*parser.mRootCx,
                (aProcessFlags & cNO_SCOPE_CHANGE) ? UsSymbolTable::cLOCAL_SCOPE : UsSymbolTable::cNEW_SCOPE);
      if (aScriptPtr->mBlockType == WsfScriptInfo::cSCRIPT_VARIABLES)
      {
         ctx.mGlobalVarMode = true;
      }
      aScriptPtr->mSymbolTablePtr = ctx.mSym;
      parser.InitParse();
      parser.block_func(ctx);
   }
   break;
   }

   if (scanner.NewTokCb)
   {
      // If the parser failed early, force the scanner to process the rest of the stream
      while (scanner.Scan()->kind != 0)
      {
      }
   }

   if (aProcessFlags & cSAVE_RESULTS)
   {
      aScriptPtr->mParent->AddScript(aScriptPtr);
   }
   if (aProcessFlags & cRECORD_ERROR_DETAILS)
   {
      aObserverPtr->AcceptErrors((static_cast<ErrorLog*>(parser.mErrorsPtr))->mErrors);
   }
   if ((aProcessFlags & cRECORD_TOKENS) && dynamic_cast<WsfScriptNormalObserver*>(aObserverPtr))
   {
      WsfScriptNormalObserver* obs = static_cast<WsfScriptNormalObserver*>(aObserverPtr);
      for (auto&& st : obs->mSpecialTokens)
      {
         WsfScriptToken newTok;
         newTok.mRange     = st.range;
         newTok.mTokenType = (WsfScriptToken::Type)(WsfScriptToken::cSTATIC_METHOD + st.tokType);

         std::vector<WsfScriptToken>::iterator iter =
            std::lower_bound(aScriptPtr->mTokens.begin(), aScriptPtr->mTokens.end(), newTok);
         if (iter != aScriptPtr->mTokens.end() && iter->mRange == newTok.mRange)
         {
            *iter = newTok;
         }
      }
   }

   mConstCtx->mParser      = nullptr;
   mConstCtx->mObserverPtr = nullptr;
}

wizard::WsfScriptParser::WsfScriptParser(WsfScriptTypes* aTypesPtr, WsfScriptData* aDataPtr)
{
   mConstCtx = aTypesPtr->GetConstCtx();
   mTypesPtr = aTypesPtr;
   mDataPtr  = aDataPtr;
}

void wizard::WsfScriptParser::DeferredProcessScript(const std::vector<TypeEntry>& aTypeStack,
                                                    WsfParseNode*                 aScriptBlockNode,
                                                    WsfParseType&                 aSymbolTable)
{
   WsfScriptContainer* container = &mDataPtr->mRoot;
   for (auto&& type : aTypeStack)
   {
      WsfScriptContainer* childPtr = container->GetChildContainer(type.mSaveKey, type.mLoadKey, aSymbolTable);
      container                    = childPtr;
   }

   WsfScriptInfo* scriptPtr       = new WsfScriptInfo;
   scriptPtr->mSymbolPtr          = nullptr;
   scriptPtr->mParent             = container;
   scriptPtr->mLocation           = aScriptBlockNode->mValue.GetPosition();
   UtTextDocumentRange blockRange = aScriptBlockNode->mValue;
   if (aScriptBlockNode->PreviousLeaf())
   {
      blockRange.Extend(aScriptBlockNode->PreviousLeaf()->mValue);
   }
   if (aScriptBlockNode->NextLeaf())
   {
      blockRange.Extend(aScriptBlockNode->NextLeaf()->mValue);
   }
   scriptPtr->SetRange(blockRange);

   if (aScriptBlockNode->GetFlags() & WsfParseNode::cSCRIPT_VARIABLES)
   {
      scriptPtr->mBlockType = WsfScriptInfo::cSCRIPT_VARIABLES;
   }
   else if (aScriptBlockNode->GetFlags() & WsfParseNode::cSCRIPT_FUNCTION)
   {
      scriptPtr->mBlockType = WsfScriptInfo::cFUNCTION;
   }
   else
   {
      assert(aScriptBlockNode->GetFlags() & WsfParseNode::cSCRIPT_BLOCK);
      scriptPtr->mBlockType = WsfScriptInfo::cIMPLICIT_SCRIPT;
   }

   if (scriptPtr->mRange.Length() > 1)
   {
      char* scriptText =
         scriptPtr->mLocation.mSource->GetPointer(scriptPtr->mRange.GetBegin(), scriptPtr->mRange.Length());
      WsfScriptNormalObserver obs(scriptPtr);
      ProcessScript(scriptPtr,
                    scriptText,
                    scriptPtr->mRange.Length(),
                    *scriptPtr->mParent->GetContext(),
                    cDEFAULT_PROCESS_FLAGS,
                    &obs);

      mDataPtr->mLocationToScript[scriptPtr->mLocation] = scriptPtr;
   }
   else
   {
      delete scriptPtr;
   }
}


wizard::WsfScriptTypes::~WsfScriptTypes()
{
   delete mConstCtx;
}

void wizard::WsfScriptTypes::CopyInheritedMethods(UsType* aTypePtr)
{
   for (size_t i = 0; i < aTypePtr->mInheritedTypes.size(); ++i)
   {
      int typeId = aTypePtr->mInheritedTypes[i];
      if (typeId == aTypePtr->mTypeId)
         continue;
      UsType* basePtr = mConstCtx->mTypes->GetType(typeId);
      if (basePtr)
      {
         for (UsType::MethodMap::const_iterator j = basePtr->mMethods.begin(); j != basePtr->mMethods.end(); ++j)
         {
            if (!aTypePtr->HasMethod(j->first, j->second))
            {
               aTypePtr->mMethods.insert(UsType::MethodMap::value_type(j->first, j->second));
            }
         }
      }
   }
}


//
// wizard::WsfScriptTypes::StructScriptTypes* wizard::WsfScriptTypes::FindScriptTypes(const WsfParseTypePath& aKey)
// {
//    std::map<WsfParseTypePath, StructScriptTypes>::iterator i = mTypeScriptData.find(aKey);
//    if (i != mTypeScriptData.end())
//    {
//       return &i->second;
//    }
//    return 0;
// }

UsCtx* wizard::WsfScriptContainer::GetContext()
{
   return mContextPtr;
}

void wizard::WsfScriptContainer::CreateContext(WsfScriptContainer*     aParentContainer,
                                               WsfScriptContainer*     aCopyFromContainer,
                                               const WsfParseTypePath& aLoadKey)
{
   assert(mContextPtr == nullptr);

   if (aParentContainer != nullptr)
   {
      mContextPtr = new UsCtx(*aParentContainer->GetContext(), UsSymbolTable::cNEW_SCOPE);

      if (aCopyFromContainer)
      {
         UsCtx* copyFromCtx = aCopyFromContainer->GetContext();
         mContextPtr->mSym->AddOuter(UsSymbolTable::cTRANSPARENT_COPY, copyFromCtx->mSym);
      }
      else
      {
         WsfParseType* loadTypePtr = mScriptDataPtr->FindParseType(aLoadKey);
         if (loadTypePtr && loadTypePtr->GetRule())
         {
            if (loadTypePtr->GetRule()->Type() == WsfParseRule::cSTRUCT)
            {
               WsfParseStruct*                             structPtr = (WsfParseStruct*)(loadTypePtr->GetRule());
               std::vector<WsfParseStruct::ScriptVariable> appVars   = structPtr->GetAllScriptVariables();
               for (auto&& appVar : appVars)
               {
                  UsType* varType = mConstCtx->mTypes->FindType(appVar.mTypeName);
                  if (varType)
                  {
                     mContextPtr->AddAppVariable(varType, appVar.mVariableName);
                     if (appVar.mIsThisType)
                     {
                        mContextPtr->mSym->mThisTypePtr = varType;
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      mContextPtr = new UsCtx(mConstCtx);
   }
   mContextPtr->AddAppVariable(mConstCtx->mTypes->FindType("double"), "TIME_NOW");
   mContextPtr->AddAppVariable(mConstCtx->mTypes->FindType("WsfRandom"), "RANDOM");
   mContextPtr->AddAppVariable(mConstCtx->mTypes->FindType("Math"), "MATH");
}


wizard::WsfScriptContainer* wizard::WsfScriptContainer::FindChildContainer(const WsfParseTypePath& aKey)
{
   ChildContainerMap::iterator iter = mChildContainers.find(aKey);
   if (iter != mChildContainers.end())
   {
      return &iter->second;
   }

   return nullptr;
}
wizard::WsfScriptContainer* wizard::WsfScriptContainer::SearchContainer(const WsfParseTypePath& aKey)
{
   if (aKey.empty())
      return nullptr;

   WsfScriptContainer* base = this;
   WsfParseTypePath    sub;
   for (auto&& it : aKey)
   {
      sub.push_back(it);
      ChildContainerMap::iterator iter = base->mChildContainers.find(sub);
      if (iter != base->mChildContainers.end())
      {
         base = &iter->second;
         sub.clear();
      }
   }
   return sub.empty() ? base : nullptr;
}

wizard::WsfScriptContainer* wizard::WsfScriptContainer::FindNearestAncestor(const WsfParseTypePath& aTypeKey,
                                                                            WsfParseType&           aSymbolTable)
{
   WsfScriptContainer* child = FindChildContainer(aTypeKey);
   if (child)
      return child;
   const WsfParseType* thisTypePtr = nullptr;
   if (mContainerTypePath.empty())
      return nullptr;
   thisTypePtr = aSymbolTable.FindTypeLocal(mContainerTypePath);
   if (thisTypePtr)
   {
      const WsfParseType* symbolPtr = thisTypePtr->FindTypeLocal(aTypeKey);
      while (symbolPtr)
      {
         WsfParseTypePath typePath;
         symbolPtr->GetPath(typePath);
         WsfScriptContainer* containerPtr = GetRoot()->SearchContainer(typePath);
         if (containerPtr)
            return containerPtr;

         symbolPtr = symbolPtr->GetEffectiveSourceType();
      }
   }
   return nullptr;
}

wizard::WsfScriptContainer* wizard::WsfScriptContainer::GetChildContainer(const WsfParseTypePath& aSaveKey,
                                                                          const WsfParseTypePath& aLoadKey,
                                                                          WsfParseType&           aSymbolTable)
{
   WsfScriptContainer* copyFromContainer = nullptr;
   if (mChildContainers.find(aSaveKey) == mChildContainers.end())
   {
      if (!aLoadKey.empty())
      {
         copyFromContainer = FindNearestAncestor(aLoadKey, aSymbolTable);
      }
   }

   WsfScriptContainer* child = &mChildContainers[aSaveKey];
   if (child->mParent == nullptr)
   {
      child->mScriptDataPtr     = mScriptDataPtr;
      child->mParent            = this;
      child->mConstCtx          = mConstCtx;
      child->mContainerTypePath = aSaveKey;

      if (!copyFromContainer)
      {
         WsfScriptContainer* rootPtr = GetRoot();
         copyFromContainer           = rootPtr->FindChildContainer(aLoadKey);
         if (!copyFromContainer)
         {
            const WsfParseType* loadType = aSymbolTable.FindNestedSymbol(aLoadKey);
            while (loadType && !copyFromContainer)
            {
               const WsfParseType* newType = loadType->GetEffectiveSourceType();
               if (!newType)
                  break;
               WsfParseTypePath newPath;
               newType->GetPath(newPath);
               copyFromContainer = rootPtr->FindChildContainer(newPath);
               loadType          = newType;
            }
         }
      }
      //          if (!mContainerTypePath.empty())
      //          {
      //             if (aSymbolTable.FindTypeLocal(mContainerTypePath, containerTypePtr))
      //             {
      //                WsfParseType* nestedTable = containerTypePtr;
      //                if (nestedTable)
      //                {
      //                   WsfParseType* referencedTypePtr;
      //                   if (WsfParseType::FindNestedSymbol(containerTypePtr, aLoadKey, referencedTypePtr))
      //                   {
      //                      //refPath = WsfParseType::TypeRefToPath(referencedType);
      //                   }
      //                }
      //             }
      //             if (!refPath.empty())
      //             {
      //                copyFromContainer = GetRoot()->FindChildContainer(refPath);
      //             }
      //          }
      if (copyFromContainer == child)
      {
         copyFromContainer = nullptr;
      }
      child->CreateContext(this, copyFromContainer, aLoadKey);
      mScopeToContainer[child->GetContext()->mSym] = child;
   }
   return child;
}

wizard::WsfScriptContainer::WsfScriptContainer()
   : mScriptDataPtr(nullptr)
   , mParent()
   , mConstCtx()
   , mContextPtr()
{
}

wizard::WsfScriptContainer::~WsfScriptContainer()
{
   for (size_t i = 0; i < mScripts.size(); ++i)
   {
      delete mScripts[i];
   }
   delete mContextPtr;
}

wizard::WsfScriptData::WsfScriptData(WsfScriptTypes* aTypesPtr, WsfParseIndex* aParseIndex, WsfParseDefinitions* aDefinitions)
   : mTypesPtr(aTypesPtr)
{
   mParseDefinitions = aDefinitions;
   mParseIndex       = aParseIndex;
   // The symbol table pool is stored in UsConstCtx class for access by the parser
   // but it is owned by this class.
   mRoot.mConstCtx = mTypesPtr->GetConstCtx();
   assert(!mRoot.mConstCtx->mSymbolTablePoolPtr);
   mRoot.mConstCtx->mSymbolTablePoolPtr = new UsSymbolTablePool;
   mRoot.mScriptDataPtr                 = this;
   mRoot.CreateContext(nullptr, nullptr, WsfParseTypePath());
}

UtTextDocumentRange wizard::WsfScriptData::FindSymbolDefinition(UsSymbol* aSymbolPtr)
{
   if (!aSymbolPtr->mTablePtr)
      return UtTextDocumentRange();

   UsSymbol* definitionSymbol = nullptr;
   if (aSymbolPtr->mStorageType == UsSymbol::cSE_EXTERN)
   {
      definitionSymbol   = aSymbolPtr;
      UsSymbolTable* tbl = definitionSymbol->mTablePtr;
      // extern variables need to be searched for after the parse is complete:
      while (tbl)
      {
         UsSymbol* sym = tbl->Search(aSymbolPtr->mName, 10);
         if (sym && sym->mStorageType == UsSymbol::cSE_EXTERN)
         {
            tbl              = sym->mTablePtr->GetParent();
            definitionSymbol = sym;
            continue;
         }
         else if (sym)
         {
            definitionSymbol = sym;
            break;
         }
         else
         {
            break;
         }
      }
   }
   else
   {
      definitionSymbol = aSymbolPtr;
   }

   if (definitionSymbol)
   {
      WsfScriptInfo* scriptPtr = FindScriptFromScope(definitionSymbol->mLexicalScopePtr);
      if (scriptPtr)
      {
         UtTextDocumentRange range;
         range.mSource   = scriptPtr->mLocation.mSource;
         size_t startPos = scriptPtr->mLocation.mPosition + definitionSymbol->mPosition;
         range.SetRange(startPos, startPos + definitionSymbol->mName.size() - 1);
         return range;
         // ViWsfScriptInfo newScriptInfo;
         // newScriptInfo.mBlockType = scriptPtr->mBlockType;
         // SendScintilla(SCI_GETTEXTRANGE, scriptPtr->mLocation.mPosition, currentPos.mPosition, buf);
         //          std::string scriptText = scriptPtr->mRange.Text();
         //          UsCtx* ctx = UsCtx::NewMirror(scriptPtr->mParent->mConstCtx,
         //          scriptPtr->mParent->GetContext()->mSym, 0x7fffffff); ViWsfScriptParser parser(mTypesPtr, this);
         //          ViWsfScriptDetailObserver* obs = new ViWsfScriptDetailObserver(scriptPtr);
         //          parser.ProcessScript(newScriptInfo, scriptText.c_str(), scriptText.size(), *ctx, 0, obs);
         //          obs->mVarDeclLocations.find();
      }
   }
   return UtTextDocumentRange();
}

wizard::WsfScriptInfo* wizard::WsfScriptData::FindScriptFromScope(UsSymbolTable* aTablePtr)
{
   // Get the most specific container given the scope
   WsfScriptContainer* containerPtr = FindContainerFromScope(aTablePtr);
   if (!containerPtr && aTablePtr->GetParent())
   {
      // variables should be found by the first call, but scripts need to check one level up
      containerPtr = FindContainerFromScope(aTablePtr->GetParent());
   }
   if (containerPtr)
   {
      // find the scope that is nested inside the script container's scope
      // This should be the script's scope
      UsSymbolTable* tbl = aTablePtr;
      while (tbl && tbl->GetParent() != containerPtr->GetScope())
      {
         tbl = tbl->GetParent();
      }
      if (tbl)
      {
         WsfScriptInfo* infoPtr = containerPtr->GetScriptByScope(tbl);
         return infoPtr;
      }
   }
   return nullptr;
}

// Given a symbol table pointer, find the associated script context
wizard::WsfScriptContainer* wizard::WsfScriptData::FindContainerFromScope(UsSymbolTable* aTablePtr)
{
   // Because a table can have multiple outer contexts, we need to search both.
   // the script context will only be found in one place
   if (aTablePtr->GetParent() != nullptr)
   {
      for (size_t i = 0; i < aTablePtr->mOuterScopes.size(); ++i)
      {
         WsfScriptContainer* parentScope = FindContainerFromScope(aTablePtr->mOuterScopes[i].mTablePtr);
         if (parentScope)
         {
            WsfScriptContainer* thisContainer = parentScope->GetContainerByScope(aTablePtr);
            if (thisContainer)
               return thisContainer;
         }
      }
   }
   else
   {
      return &mRoot;
   }
   return nullptr;
}


void wizard::WsfScriptDetailObserver::ScriptDefinition(UsSymbolTable* aFuncSymTable, UsSymbol* aSymbolPtr)
{
   mScriptPtr->mSymbolTablePtr = aFuncSymTable;
   mScriptPtr->mSymbolPtr      = aSymbolPtr;
}

void wizard::WsfScriptDetailObserver::AtStat()
{
   // expected = cSTATEMENT;
}

void wizard::WsfScriptDetailObserver::AtDot(UsType* aTypePtr, int aPos)
{
   mLHS_Kind   = cTYPE;
   mLHS_Type   = aTypePtr;
   mDotLHS_Pos = aPos;
}

void wizard::WsfScriptDetailObserver::AtDot(UsVal aVal, int aPos)
{
   mLHS_Kind   = cVALUE;
   mLHS_Val    = aVal;
   mDotLHS_Pos = aPos;
}

void wizard::WsfScriptDetailObserver::PushScope(UsSymbolTable* aSymTablePtr)
{
   mScopeStack.push_back(aSymTablePtr);
}
void wizard::WsfScriptDetailObserver::PopScope(UsSymbolTable* aTablePtr)
{
   if (!mEOF_Hit)
   {
      assert(mScopeStack.back() == aTablePtr);
      mScopeStack.pop_back();
   }
}
void wizard::WsfScriptDetailObserver::VarDecl(UsSymbol* aSymbolPtr, Us::Token* ident)
{
   mVarDeclLocations[aSymbolPtr] = ident->pos;
}

void wizard::WsfScriptDetailObserver::HitEOF()
{
   mEOF_Hit = true;
}

void wizard::WsfScriptDetailObserver::BeginCall(const UsVal& v, int aPos)
{
   if (aPos >= mCursorPos)
      return;
   CallInfo ci;
   ci.mOpenParenPos = ut::cast_to_size_t(aPos);
   ci.mValPtr       = &v;
   if (v.mType && v.mMethodNamePtr)
   {
      ci.mMethodName     = v.mMethodNamePtr;
      ci.mMethodBaseType = v.mType;
   }
   else if (v.mSym && v.mSym->IsPrototype())
   {
      ci.mPrototypeId = v.mSym->mRTypeId;
      ci.mMethodName  = v.mSym->mName;
   }
   mCurrentCalls.push_back(ci);
}

void wizard::WsfScriptDetailObserver::EndCall(const UsVal& v, int aPos)
{
   // if (mEOF_Hit) return;
   while (!mCurrentCalls.empty() && mCurrentCalls.back().mValPtr != &v)
   {
      if (aPos >= mCursorPos)
         return;
      mCurrentCalls.pop_back();
   }
   if (!mCurrentCalls.empty())
   {
      if (aPos >= mCursorPos)
      {
         if (mCurrentCalls.back().mCloseParenPos == ut::npos)
         {
            mCurrentCalls.back().mCloseParenPos = ut::cast_to_size_t(aPos);
         }
      }
      else
      {
         mCurrentCalls.pop_back();
      }
   }
}

void wizard::WsfScriptDetailObserver::CallArgNext(int aPos)
{
   if (aPos >= mCursorPos)
      return;
   if (!mCurrentCalls.empty())
   {
      ++mCurrentCalls.back().mCurrentArg;
   }
}


wizard::WsfScriptData::~WsfScriptData()
{
   assert(mRoot.mConstCtx->mSymbolTablePoolPtr);
   delete mRoot.mConstCtx->mSymbolTablePoolPtr;
   mRoot.mConstCtx->mSymbolTablePoolPtr = nullptr;
}

QString wizard::WsfScriptData::PeekScriptDocumentation(GrammarDocumentation& aDocumentationData,
                                                       UsType*               aClassPtr,
                                                       const std::string&    aMethodName)
{
   QString     text;
   std::string pageName;
   std::string className;
   if (FindScriptMethodDocumentation(aDocumentationData, aClassPtr, aMethodName, pageName, className))
   {
      text = CommandDocumentation::PeekDocumentationText(aDocumentationData, pageName, aMethodName);
   }
   return text;
}

bool wizard::WsfScriptData::FindScriptMethodDocumentation(GrammarDocumentation& aDocumentationData,
                                                          UsType*               aClassPtr,
                                                          const std::string&    aMethodName,
                                                          std::string&          aPageName,
                                                          std::string&          aClassName)
{
   std::set<UsType*> types;
   std::set<UsType*> visitedTypes;
   types.insert(aClassPtr);
   while (!types.empty())
   {
      UsType*     classPtr = *types.begin();
      std::string typeName = classPtr->mName;
      if (typeName.find('<') != std::string::npos) // Strip off the specialied template type
      {
         typeName = typeName.substr(0, typeName.find('<'));
      }
      if (typeName == "__BUILTIN__")
         typeName = "BUILTIN";
      std::string pageName = "docs/script/" + typeName + ".html";
      UtStringUtil::ToLower(pageName);
      QString text =
         CommandDocumentation::PeekDocumentationText(aDocumentationData, pageName, typeName + "." + aMethodName);
      if (!text.isEmpty())
      {
         aClassName = typeName;
         aPageName  = pageName;
         return true;
      }
      types.erase(classPtr);
      visitedTypes.insert(classPtr);
      for (auto&& it : classPtr->mInheritedTypes)
      {
         UsType* basePtr = mTypesPtr->GetConstCtx()->mTypes->GetType(it);
         if (basePtr)
         {
            if (visitedTypes.find(basePtr) == visitedTypes.end())
            {
               types.insert(basePtr);
            }
         }
      }
   }
   return false;
}

bool wizard::WsfScriptData::FindScriptTypeDocumentation(GrammarDocumentation& aDocumentationData,
                                                        UsType*               aClassPtr,
                                                        std::string&          aPageName)
{
   std::string className = aClassPtr->mName;
   std::string pageName  = "docs/script/" + className + ".html";
   UtStringUtil::ToLower(pageName);
   UtPath pagePath = aDocumentationData.mDocumentationDir + pageName;
   if (pagePath.Stat() == UtPath::cFILE)
   {
      aPageName = pageName;
      return true;
   }
   return false;
}

WsfParseType* wizard::WsfScriptData::FindParseType(const WsfParseTypePath& aLoadKey)
{
   WsfParseType* typePtr = mParseDefinitions->mBasicTypes->FindType(aLoadKey);
   if (!typePtr)
   {
      typePtr = mParseIndex->mUserTypes.FindType(aLoadKey);
   }
   return typePtr;
}

// The range of the script is computed given the full range including
// keywords that begin and end the block.
// Here, we walk just past they script/end_script keywords to record the actual
// range of the script.
void wizard::WsfScriptInfo::SetRange(UtTextDocumentRange aRange)
{
   mBlockRange = aRange;

   UtTextDocument*        doc        = aRange.mSource;
   UtTextDocumentIterator docStart   = doc->Begin();
   UtTextDocumentIterator docEnd     = doc->End();
   UtTextDocumentIterator rangeBegin = doc->Begin() + aRange.GetBegin();
   UtTextDocumentIterator rangeEnd   = doc->Begin() + aRange.GetEnd();

   // Remove start keyword
   while (rangeBegin != docEnd && !WsfParseUtil::IsWhitespace(*rangeBegin))
      ++rangeBegin;
   // remove whitespace after
   while (rangeBegin != docEnd && WsfParseUtil::IsWhitespace(*rangeBegin))
      ++rangeBegin;
   // remove end keyword
   while (rangeEnd != docStart && !WsfParseUtil::IsWhitespace(*rangeEnd))
      --rangeEnd;

   mRange.SetRange(rangeBegin.GetIndex(), rangeEnd.GetIndex());
   mRange.mSource = aRange.mSource;
}
