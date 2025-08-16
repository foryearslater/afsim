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
#include "UsSymbolTable.hpp"

#include <sstream>

#include "WsfParseUtil.hpp"

int UsFunction::SpecializeType(int aThisType, std::pair<UsType*, UsType*> aArgs, int aType)
{
   if (aType == -1)
   {
      return aArgs.first ? aArgs.first->mTypeId : -1;
   }
   if (aType == -2)
   {
      return aArgs.second ? aArgs.second->mTypeId : -1;
   }
   if (aType == -3)
   {
      return aThisType;
   }
   return aType;
}

void UsFunction::Specialize(int aThisType, const std::pair<UsType*, UsType*>& aTempArgs)
{
   mReturnType = SpecializeType(aThisType, aTempArgs, mReturnType);
   for (int i = 0; i < mArgs.size(); ++i)
   {
      mArgs[i] = SpecializeType(aThisType, aTempArgs, mArgs[i]);
   }
}

bool UsFunction::operator==(const UsFunction& aRhs) const
{
   if (!(mFlags == aRhs.mFlags && mReturnType == aRhs.mReturnType && mArgs.size() == aRhs.mArgs.size()))
   {
      return false;
   }
   for (int i = 0; i < mArgs.size(); ++i)
   {
      if (mArgs[i] != aRhs.mArgs[i])
      {
         return false;
      }
   }
   return true;
}

bool UsFunction::operator<(const UsFunction& aRhs) const
{
   if (mArgs.size() < aRhs.mArgs.size())
   {
      return true;
   }
   if (mArgs.size() > aRhs.mArgs.size())
   {
      return false;
   }
   if (mReturnType < aRhs.mReturnType)
   {
      return true;
   }
   if (mReturnType > aRhs.mReturnType)
   {
      return false;
   }
   if (mFlags < aRhs.mFlags)
   {
      return true;
   }
   if (mFlags > aRhs.mFlags)
   {
      return false;
   }
   for (int i = 0; i < mArgs.size(); ++i)
   {
      int lhs = mArgs[i];
      int rhs = aRhs.mArgs[i];
      if (lhs < rhs)
      {
         return true;
      }
      if (lhs > rhs)
      {
         return false;
      }
   }
   return false;
}


bool UsType::IsImplicitlyCastable(int aTypeId) const
{
   return std::find(mImplicitCastTypes.begin(), mImplicitCastTypes.end(), aTypeId) != mImplicitCastTypes.end();
}
bool UsType::IsExplicitlyCastable(int aTypeId) const
{
   return std::find(mExplicitCastTypes.begin(), mExplicitCastTypes.end(), aTypeId) != mExplicitCastTypes.end();
}
bool UsType::IsBaseType(int aTypeId) const
{
   for (size_t i = 0; i < mInheritedTypes.size(); ++i)
   {
      if (mInheritedTypes[i] == aTypeId)
      {
         return true;
      }
   }
   return false;
}
bool UsType::HasStaticMethod(const char* aName)
{
   MethodMap::iterator i = mMethods.lower_bound(aName);
   while (i != mMethods.end() && strcmp(i->first, aName) == 0)
   {
      UsFunction& fn = i->second;
      if (fn.mFlags & UsFunction::cSTATIC)
      {
         return true;
      }
      ++i;
   }
   return false;
}

UsFunction* UsType::FindMethod(const char* aName, std::vector<int>& aArgTypes)
{
   MethodMap::iterator i = mMethods.lower_bound(aName);
   if (i == mMethods.end() || strcmp(aName, i->first) != 0)
   {
      return nullptr;
   }
   const char* namePtr = i->first;
   do
   {
      UsFunction& fn = i->second;
      if ((fn.mArgs.size() == (int)aArgTypes.size()) ||
          ((fn.mFlags & UsFunction::cVARARGS) && (fn.mArgs.size() <= (int)aArgTypes.size() + 1)))
      {
         bool ok = true;
         for (int i = 0; i < (int)aArgTypes.size(); ++i)
         {
            // varargs repeats the last argument any number of times
            int j = std::min(i, fn.mArgs.size() - 1);
            ok    = false;
            if (aArgTypes[i] != fn.mArgs[j])
            {
               UsType* inType = mTypeList->GetType(aArgTypes[i]);
               if (inType)
               {
                  int expectedTypeId = fn.mArgs[j];
                  if (inType->IsImplicitlyCastable(expectedTypeId) || inType->IsBaseType(expectedTypeId))
                  {
                     ok = true;
                  }
               }
            }
            else
            {
               ok = true;
            }
            if (!ok)
            {
               break;
            }
         }
         if (ok)
         {
            return &fn;
         }
      }
      ++i;
   } while (i != mMethods.end() && i->first == namePtr);
   return nullptr;
}

UsSymbolTable* UsSymbolTable::AddInner(TableLinkage aLinkage)
{
   UsSymbolTable* i = new UsSymbolTable(mPoolPtr);
   i->AddOuter(aLinkage, this);
   mInnerScopes.push_back(TableLink(i, aLinkage));
   return i;
}

void UsSymbolTable::AddOuter(TableLinkage aLinkage, UsSymbolTable* aSymbolTablePtr)
{
   mOuterScopes.push_back(TableLink(aSymbolTablePtr, aLinkage));
   if (aLinkage == cTRANSPARENT_COPY || mOuterScopes.size() == 1)
   {
      mThisTypePtr = aSymbolTablePtr->mThisTypePtr;
   }
}

UsSymbol* UsSymbolTable::Search(const std::string& aName, int aDepth, int aSequenceNumberLimit)
{
   UsSymbol* r = SearchTransCopy(aName, aSequenceNumberLimit);
   if (!r)
   {
      for (size_t i = 0; i < mOuterScopes.size() && !r; ++i)
      {
         TableLink l = mOuterScopes[i];
         if (l.mLinkage != cTRANSPARENT_COPY)
         {
            if (aDepth > 0)
            {
               int nextDepth = aDepth - ((l.mLinkage & cADD_DEPTH_FLAG) ? 1 : 0);
               r             = l.mTablePtr->Search(aName, nextDepth, aSequenceNumberLimit);
               break;
            }
         }
      }
   }
   return r;
}

// Search for the symbol in table and all symbol tables this is copied from
UsSymbol* UsSymbolTable::SearchTransCopy(const std::string& aName, int aSequenceNumberLimit)
{
   UsSymbol*          r = nullptr;
   EntryMap::iterator i = mEntries.find(aName);
   if (i != mEntries.end() && i->second.mSequenceNumber < aSequenceNumberLimit)
   {
      r = &i->second;
   }
   else
   {
      for (size_t i = 0; i < mOuterScopes.size() && !r; ++i)
      {
         TableLink l = mOuterScopes[i];
         if (l.mLinkage == cTRANSPARENT_COPY)
         {
            r = l.mTablePtr->SearchTransCopy(aName, aSequenceNumberLimit);
            break;
         }
      }
   }
   return r;
}

UsType* UsTypeList::FindType(const std::string& aTypeName)
{
   return FindType(aTypeName.c_str());
}

UsType* UsTypeList::FindType(const char* aTypeStr)
{
   NameMap::iterator i = mNameMap.find(aTypeStr);
   if (i != mNameMap.end())
   {
      return mTypes[i->second];
   }
   return nullptr;
}

int UsTypeList::SplitTemplateType(const std::string& aTypeName,
                                  std::string&       aBaseName,
                                  std::string&       aTemplateArg1,
                                  std::string&       aTemplateArg2)
{
   size_t laPos, cPos = 0, raPos = 0;
   laPos = aTypeName.find('<');
   if (laPos != std::string::npos)
   {
      // If there is any whitespace, remove it and recurse
      {
         std::string newType;
         for (size_t i = 0; i < aTypeName.size(); ++i)
         {
            if (WsfParseUtil::IsWhitespace(aTypeName[i]))
            {
               newType = aTypeName.substr(0, i);
               for (size_t j = i; i < aTypeName.size(); ++i)
               {
                  char c = aTypeName[j];
                  if (!WsfParseUtil::IsWhitespace(c))
                  {
                     newType += c;
                  }
               }
               return SplitTemplateType(newType, aBaseName, aTemplateArg1, aTemplateArg2);
            }
         }
      }

      // We have a new template specialization, create it now
      aBaseName     = aTypeName.substr(0, laPos);
      int nestLevel = 0;
      for (size_t i = laPos + 1; i < aTypeName.size(); ++i)
      {
         char c = aTypeName[i];
         if (c == '<')
         {
            ++nestLevel;
         }
         else if (c == '>')
         {
            if (--nestLevel < 0)
            {
               raPos = i;
               break;
            }
         }
         else if (c == ',')
         {
            if (nestLevel == 0)
            {
               cPos = i;
            }
         }
      }
      if (raPos > 0)
      {
         if (cPos != 0)
         {
            aTemplateArg1 = aTypeName.substr(laPos + 1, cPos - laPos - 1);
            aTemplateArg2 = aTypeName.substr(cPos + 1, raPos - cPos - 1);
            return 2;
         }
         else
         {
            aTemplateArg1 = aTypeName.substr(laPos + 1, raPos - laPos - 1);
            return 1;
         }
      }
   }
   else
   {
      aBaseName = aTypeName;
   }
   return 0;
}

UsType* UsTypeList::FindTypeOrMakeTemplate(const std::string& aTypeName)
{
   size_t laPos, cPos = 0, raPos = 0;
   laPos = aTypeName.find('<');
   if (laPos != std::string::npos)
   {
      // If there is any whitespace, remove it and recurse
      {
         std::string newType;
         for (size_t i = 0; i < aTypeName.size(); ++i)
         {
            if (WsfParseUtil::IsWhitespace(aTypeName[i]))
            {
               newType = aTypeName.substr(0, i);
               for (size_t j = i; i < aTypeName.size(); ++i)
               {
                  char c = aTypeName[j];
                  if (!WsfParseUtil::IsWhitespace(c))
                  {
                     newType += c;
                  }
               }
               return FindTypeOrMakeTemplate(newType);
            }
         }
      }

      // Return if it's already made
      UsType* existingInstance = FindType(aTypeName);
      if (existingInstance)
      {
         return existingInstance;
      }

      // We have a new template specialization, create it now
      UsType* basePtr = FindType(aTypeName.substr(0, laPos));
      if (basePtr)
      {
         int nestLevel = 0;
         for (size_t i = laPos + 1; i < aTypeName.size(); ++i)
         {
            char c = aTypeName[i];
            if (c == '<')
            {
               ++nestLevel;
            }
            else if (c == '>')
            {
               if (--nestLevel < 0)
               {
                  raPos = i;
                  break;
               }
            }
            else if (c == ',')
            {
               if (nestLevel == 0)
               {
                  cPos = i;
               }
            }
         }
         if (raPos > 0)
         {
            if (cPos != 0)
            {
               UsType* a = FindTypeOrMakeTemplate(aTypeName.substr(laPos + 1, cPos - laPos - 1));
               UsType* b = FindTypeOrMakeTemplate(aTypeName.substr(cPos + 1, raPos - cPos - 1));
               if (a && b)
               {
                  return GetType(TemplateType(basePtr->Id(), TemplateArgs(a->Id(), b->Id())));
               }
            }
            else
            {
               UsType* a = FindTypeOrMakeTemplate(aTypeName.substr(laPos + 1, raPos - laPos - 1));
               return GetType(TemplateType(basePtr->Id(), TemplateArgs(a->Id(), 0)));
            }
         }
      }
   }
   else
   {
      return FindType(aTypeName);
   }
   return nullptr;
}

void UsTypeList::AddType(const std::string& aTypeName, UsType* aType)
{
   aType->mTypeId = (int)mTypes.size();
   mTypes.push_back(aType);
   int& id = mNameMap[GetString(aTypeName.c_str())];
   assert(id == 0);
   id = aType->mTypeId;
}

UsType* UsTypeList::GetType(int aTypeId)
{
   if (aTypeId >= 0 && aTypeId < (int)mTypes.size())
   {
      return mTypes[aTypeId];
   }
   return nullptr;
}

namespace
{
std::string BuildTemplateTypeName(const std::string& aName, std::pair<UsType*, UsType*> aArgs)
{
   if (aArgs.second)
   {
      return aName + '<' + aArgs.first->mName + ',' + aArgs.second->mName + '>';
   }
   else if (aArgs.first)
   {
      return aName + '<' + aArgs.first->mName + '>';
   }
   return aName; // error
}
} // namespace

UsType* UsTypeList::GetType(TemplateType aTypeId)
{
   TemplateTypeMap::iterator iter = mTemplateTypes.find(aTypeId);
   if (iter != mTemplateTypes.end())
   {
      return iter->second;
   }
   else
   {
      UsType* basePtr = GetType(aTypeId.first);
      if (basePtr)
      {
         if (aTypeId.second.first != 0)
         {
            UsType* spec = basePtr->Specialize(GetTemplateArgs(aTypeId.second));
            if (spec)
            {
               mTemplateTypes[aTypeId] = spec;
               spec->mTypeId           = (int)mTypes.size();
               spec->mName             = BuildTemplateTypeName(basePtr->mName, spec->mTemplateArgs);
               mTypes.push_back(spec);
               int& id = mNameMap[GetString(spec->mName.c_str())];
               assert(id == 0);
               id = spec->mTypeId;
               return spec;
            }
         }
      }
      return basePtr;
   }
   return nullptr;
}
std::pair<UsType*, UsType*> UsTypeList::GetTemplateArgs(TemplateArgs aArgs)
{
   std::pair<UsType*, UsType*> rval;
   if (aArgs.first)
   {
      rval.first = GetType(aArgs.first);
   }
   if (aArgs.second)
   {
      rval.second = GetType(aArgs.second);
   }
   return rval;
}
std::string UsTypeList::GetTypeName(int aTypeId)
{
   UsType* typePtr = GetType(aTypeId);
   return typePtr ? typePtr->mName : std::string();
}

const UsFunction* UsTypeList::GetPrototype(int aId)
{
   int id = aId - cPROTOTYPE_ID_START;
   if (id >= 0 && id < (int)mPrototypes.size())
   {
      return mPrototypes[id];
   }
   return nullptr;
}
int UsTypeList::AddPrototype(const UsFunction& aPrototype)
{
   PrototypeMap::iterator iter = mPrototypeMap.find(aPrototype);
   if (iter == mPrototypeMap.end())
   {
      int                                     protoId = (int)mPrototypes.size() + cPROTOTYPE_ID_START;
      std::pair<PrototypeMap::iterator, bool> result  = mPrototypeMap.insert(std::make_pair(aPrototype, protoId));
      mPrototypes.push_back(&result.first->first);
      return protoId;
   }
   else
   {
      return iter->second;
   }
}

const char* UsTypeList::GetString(const char* aString)
{
   StrMap::iterator iter = mStrings.find(aString);
   if (iter != mStrings.end())
   {
      return *iter;
   }
   int   len    = (int)strlen(aString);
   char* newStr = new char[len + 1];
   for (int i = 0; i < len + 1; ++i)
   {
      newStr[i] = aString[i];
   }
   mStrings.insert(newStr);
   return newStr;
}

UsTypeList::UsTypeList()
{
   mTypes.push_back(nullptr); // don't use 0 as a type id
   mMapBase = mSetBase = mArrayBase = nullptr;
}

UsTypeList::~UsTypeList()
{
   for (StrMap::iterator i = mStrings.begin(); i != mStrings.end(); ++i)
   {
      delete[] * i;
   }
   for (size_t i = 0; i < mTypes.size(); ++i)
   {
      delete mTypes[i];
   }
}

std::string UsTypeList::GetFunctionPrototype(const std::string& aName, const UsFunction& aFunction)
{
   std::stringstream ss;
   ss << GetTypeName(aFunction.mReturnType) << ' ' << aName << "(";
   for (int i = 0; i < aFunction.mArgs.size(); ++i)
   {
      if (i != 0)
      {
         ss << ",";
      }
      ss << GetTypeName(aFunction.mArgs[i]);
   }
   return ss.str();
}


UsSymbolTablePool::~UsSymbolTablePool()
{
   for (size_t i = 0; i < mSymbolTables.size(); ++i)
   {
      delete mSymbolTables[i];
   }
}

UsType* UsType::Specialize(std::pair<UsType*, UsType*> aTemplateArgs) const
{
   UsType* s        = new UsType(*this);
   s->mTemplateArgs = aTemplateArgs;

   for (UsType::MethodMap::iterator i = s->mMethods.begin(); i != s->mMethods.end(); ++i)
   {
      i->second.Specialize(mTypeId, aTemplateArgs);
   }
   if (mTypeList && !mName.empty())
   {
      if (mTypeList->mArrayBase && mName[0] == 'A')
      {
         s->mImplicitCastTypes.push_back(mTypeList->mArrayBase->Id());
      }
      if (mTypeList->mSetBase && mName[0] == 'S')
      {
         s->mImplicitCastTypes.push_back(mTypeList->mSetBase->Id());
      }
      if (mTypeList->mMapBase && mName[0] == 'M')
      {
         s->mImplicitCastTypes.push_back(mTypeList->mMapBase->Id());
      }
   }
   return s;
}
void UsType::ReSpecialize(UsType* aTemplateBase)
{
   mMethods = aTemplateBase->mMethods;
   for (UsType::MethodMap::iterator i = mMethods.begin(); i != mMethods.end(); ++i)
   {
      i->second.Specialize(mTypeId, mTemplateArgs);
   }
}

bool UsType::HasMethod(const char* aName, const UsFunction& aFunc) const
{
   MethodMap::const_iterator i = mMethods.lower_bound(aName);
   while (i != mMethods.end() && strcmp(i->first, aName) == 0)
   {
      if (i->second == aFunc)
      {
         return true;
      }
      ++i;
   }
   return false;
}
