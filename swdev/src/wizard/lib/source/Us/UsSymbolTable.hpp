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
#ifndef USSYMBOLTABLE_HPP
#define USSYMBOLTABLE_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string.h>
#include <vector>

#include "UtMemoryDebug.hpp"
#include "Util.hpp"

class UsType;
class UsFunction;

// Stores an array of dynamic size.  If the array is smaller than ARRAY_SIZE, no memory allocations are made.
// The array is grown by increments of size 'GROW_SIZE'
template<typename T, int ARRAY_SIZE, int GROW_SIZE = 10>
class DynamicArray
{
public:
   DynamicArray()
      : mSize(0)
      , mDynamicBuffer(nullptr)
   {
   }
   DynamicArray(const DynamicArray<T, ARRAY_SIZE>& aRhs)
   {
      mSize                 = aRhs.mSize;
      mDynamicBuffer        = nullptr;
      int arrayValuesToCopy = std::min(mSize, ARRAY_SIZE);
      std::copy(&aRhs.mArray[0], &aRhs.mArray[0] + arrayValuesToCopy, &mArray[0]);
      if (aRhs.mDynamicBuffer && mSize > ARRAY_SIZE)
      {
         mDynamicBuffer = new T[mSize - ARRAY_SIZE];
         std::copy(aRhs.mDynamicBuffer, aRhs.mDynamicBuffer + mSize - ARRAY_SIZE, mDynamicBuffer);
      }
   }
   ~DynamicArray() { delete[] mDynamicBuffer; }
   void resize(int aSize)
   {
      if (mSize != aSize)
      {
         if (mSize < ARRAY_SIZE)
         {
            delete mDynamicBuffer;
            mDynamicBuffer = nullptr;
         }
         else
         {
            if (((mSize - ARRAY_SIZE) % GROW_SIZE) != ((aSize - ARRAY_SIZE) % GROW_SIZE))
            {
               int newMemSize = ((aSize - ARRAY_SIZE + GROW_SIZE - 1) / GROW_SIZE) * GROW_SIZE;
               T*  newBuf     = new T[newMemSize];
               if (mDynamicBuffer)
               {
                  for (int i = ARRAY_SIZE; i < mSize; ++i)
                  {
                     newBuf[i - ARRAY_SIZE] = mDynamicBuffer[i - ARRAY_SIZE];
                  }
                  delete[] mDynamicBuffer;
               }
               mDynamicBuffer = newBuf;
            }
         }
      }
      mSize = aSize;
   }
   T& operator[](int i)
   {
      if (i < ARRAY_SIZE)
      {
         return mArray[i];
      }
      return mDynamicBuffer[i - ARRAY_SIZE];
   }
   T&       operator[](size_t i) { return operator[]((int)i); }
   const T& operator[](int i) const
   {
      if (i < ARRAY_SIZE)
      {
         return mArray[i];
      }
      return mDynamicBuffer[i - ARRAY_SIZE];
   }
   const T& operator[](size_t i) const { return operator[]((int)i); }
   void     push_back(const T& aVal)
   {
      int i = size();
      resize(i + 1);
      (*this)[i] = aVal;
   }

   int size() const { return mSize; }

private:
   int mSize;
   T   mArray[ARRAY_SIZE];
   T*  mDynamicBuffer;
};

// A function prototype
class UsFunction
{
public:
   UsFunction()
      : mFlags()
      , mReturnType()
   {
   }
   enum Flags
   {
      cAPP_METHOD = 1,
      cSTATIC     = 2,
      cVARARGS    = 4
   };
   bool                 operator<(const UsFunction& aRhs) const;
   bool                 operator==(const UsFunction& aRhs) const;
   int                  SpecializeType(int aThisType, std::pair<UsType*, UsType*> aArgs, int aType);
   void                 Specialize(int aThisType, const std::pair<UsType*, UsType*>& aTempArgs);
   int                  mFlags;
   int                  mReturnType;
   DynamicArray<int, 4> mArgs;
};

// Contains the set of all types for the script system.
class UsTypeList
{
public:
   typedef std::pair<int, int>             TemplateArgs;
   typedef std::pair<int, TemplateArgs>    TemplateType;
   typedef std::map<TemplateType, UsType*> TemplateTypeMap;

   UsTypeList();

   ~UsTypeList();

   UsType* GetType(int aTypeId);
   UsType* GetType(TemplateType aTypeId);

   std::pair<UsType*, UsType*> GetTemplateArgs(TemplateArgs aArgs);

   const char*       GetString(const char* aString);
   std::string       GetTypeName(int aTypeId);
   UsType*           FindType(const std::string& aTypeName);
   UsType*           FindType(const char* aTypeStr);
   UsType*           FindTypeOrMakeTemplate(const std::string& aTypeName);
   static int        SplitTemplateType(const std::string& aTypeName,
                                       std::string&       aBaseName,
                                       std::string&       aTemplateArg1,
                                       std::string&       aTemplateArg2);
   void              AddType(const std::string& aTypeName, UsType* aType);
   int               AddPrototype(const UsFunction& aPrototype);
   const UsFunction* GetPrototype(int aId);
   std::string       GetFunctionPrototype(const std::string& aName, const UsFunction& aFunction);


   typedef std::set<const char*, wizard::UtilCStringCompare> StrMap;
   typedef std::map<UsFunction, int>                         PrototypeMap;

   // To store type information about both types a function prototypes,
   // type ID's lower than this number are interpreted as normal type ID's
   // other type ID's are interpreted as function prototypes
   static const int cPROTOTYPE_ID_START = 100000;

   // Map from type name to type ID
   typedef std::map<const char*, int, wizard::UtilCStringCompare> NameMap;
   NameMap                                                        mNameMap;
   // List of all types indexed by type ID
   // First entry is 0
   std::vector<UsType*> mTypes;
   // map of specialized template types
   TemplateTypeMap mTemplateTypes;
   UsType*         mMapBase;
   UsType*         mSetBase;
   UsType*         mArrayBase;
   // Function prototypes are stored on demand so that a function may have a type ID maps to
   // information about return type and argument types.
   std::vector<const UsFunction*> mPrototypes;

   PrototypeMap mPrototypeMap;

   // To reduce the cost of storing strings, some strings are stored as pointers
   // and stored here for cleanup.  No user-defined strings should be stored here.
   StrMap mStrings;
   UT_MEMORY_DEBUG_MARKER(cMDB_UsTypeList);
};

// A script type / class
class UsType
{
public:
   enum Flags
   {
      cCONSTRUCTIBLE        = 1,
      cCLONEABLE            = 2,
      cCONTAINER            = 4,
      cLESS_THAN_COMPARABLE = 8,
      cEQUALITY_COMPARABLE  = 16
   };
   explicit UsType(UsTypeList* aTypes)
      : mTypeList(aTypes)
      , mTypeId(-1)
      , mFlags()
   {
   }
   UsType*     Specialize(std::pair<UsType*, UsType*> aTemplateArgs) const;
   void        ReSpecialize(UsType* aTemplateBase);
   int         Id() const { return mTypeId; }
   bool        IsImplicitlyCastable(int aTypeId) const;
   bool        IsExplicitlyCastable(int aTypeId) const;
   bool        IsBaseType(int aTypeId) const;
   bool        HasStaticMethod(const char* aName);
   UsFunction* FindMethod(const char* aName, std::vector<int>& aArgTypes);

   void AddInheritedType(int aTypeId)
   {
      if (mTypeId != aTypeId)
      {
         if (std::find(mInheritedTypes.begin(), mInheritedTypes.end(), aTypeId) == mInheritedTypes.end())
         {
            mInheritedTypes.push_back(aTypeId);
         }
      }
   }
   void AddImplicitCastType(int aTypeId)
   {
      if (std::find(mImplicitCastTypes.begin(), mImplicitCastTypes.end(), aTypeId) == mImplicitCastTypes.end())
      {
         mImplicitCastTypes.push_back(aTypeId);
      }
   }
   void AddExplicitCastType(int aTypeId)
   {
      if (std::find(mExplicitCastTypes.begin(), mExplicitCastTypes.end(), aTypeId) == mExplicitCastTypes.end())
      {
         mExplicitCastTypes.push_back(aTypeId);
      }
   }
   bool HasMethod(const char* aName, const UsFunction& aFunc) const;
   // Parent type list
   UsTypeList* mTypeList;
   // Unique ID for the type
   int mTypeId;
   // Flags that apply to this type
   int         mFlags;
   std::string mName;
   // If this is a specialized template type, 1 or both of these is non-null
   std::pair<UsType*, UsType*> mTemplateArgs;

   typedef std::multimap<const char*, UsFunction, wizard::UtilCStringCompare> MethodMap;
   // All methods available for this class.  Overloading occurs if more than one shares a single name.
   // The key is a string returned by UsTypeList::GetString()
   MethodMap mMethods;
   // List of type id's this type inherits from
   std::vector<int> mInheritedTypes;
   // List of type id's to which this type may be implicitly cast
   std::vector<int> mImplicitCastTypes;
   // List of type id's to which this type may be explicitly cast
   std::vector<int> mExplicitCastTypes;
   // All variables available for this class. The key is the variable name.
   using VarMap = std::map<std::string, UsType*>;
   VarMap mVariables;
};
class UsSymbolTable;

// A symbol is a named entity that needs to be stored for later access
// in a script related context
class UsSymbol
{
public:
   enum EntryType
   {
      cSE_VARIABLE,
      cSE_FUNCTION,
      cSE_APP_FUNCTION,
      cSE_SCRIPT
   };

   //! If the 'EntryType' is cSE_VARIABLE, there is
   //! a corresponding 'StorageType'
   enum StorageType
   {
      cSE_NOT_APPLICABLE,
      cSE_AUTOMATIC,
      cSE_PARAMETER,
      cSE_GLOBAL,
      cSE_STATIC,
      cSE_EXTERN
   };

   bool IsPrototype() const { return mRTypeId >= UsTypeList::cPROTOTYPE_ID_START; }
   // The symbol table containing this symbol
   UsSymbolTable* mTablePtr;
   // The symbol table of the lexical scope.  This is the same as mTablePtr for normal variables, but may be
   // a child of mTablePtr for 'global' variables
   UsSymbolTable* mLexicalScopePtr;

   // Name of the symbol as specified by the script
   std::string mName;
   // EntryType value
   unsigned char mEntryType;
   // StorageType value
   unsigned char mStorageType;
   // If this is a definition, this stores the offset in characters from the start of the script
   unsigned short mPosition;
   // TypeID of the symbol if the symbol has a type
   int mRTypeId;
   // Indicates the 'time' at which the symbol is first added to the symbol table
   int mSequenceNumber;
};

class UsSymbolTable;
class UsSymbolTablePool
{
public:
   friend class UsSymbolTable;
   UsSymbolTablePool() {}
   ~UsSymbolTablePool();

private:
   std::vector<UsSymbolTable*> mSymbolTables;
};

// Symbols are stored in a symbol table.
// Symbol tables are a directed acyclic graph with a single root symbol table
class UsSymbolTable
{
   UsSymbolTable(const UsSymbolTable& aRhs);
   UsSymbolTable& operator=(const UsSymbolTable& aRhs);

public:
   // When doing a search for symbols, there is limited depth.
   // The linkage determines how the search takes place
   enum TableLinkage
   {
      cOPEN_NEW_SCOPE = 0x1,
      cADD_DEPTH_FLAG = 0x2,
      // this links to another table with the same scope,
      // as if the inner symbol table is actually a copy of the outer one
      cTRANSPARENT_COPY = 0,
      // this links to a new local scope which uses this table's variables and does not add search depth
      cLOCAL_SCOPE = cOPEN_NEW_SCOPE,
      // this links to a new scope which may use this table's variables, but adds to the 'depth' when searched
      cNEW_SCOPE = cADD_DEPTH_FLAG | cOPEN_NEW_SCOPE,
   };
   struct TableLink
   {
      TableLink(UsSymbolTable* aTablePtr, TableLinkage aLinkage)
         : mTablePtr(aTablePtr)
         , mLinkage(aLinkage)
      {
      }
      UsSymbolTable* mTablePtr;
      TableLinkage   mLinkage;
   };

   explicit UsSymbolTable(UsSymbolTablePool* aPoolPtr)
      : mPoolPtr(aPoolPtr)
   {
      mPoolPtr->mSymbolTables.push_back(this);
      mThisTypePtr = nullptr;
   }
   UsSymbolTable* AddInner(TableLinkage aLinkage);
   void           AddOuter(TableLinkage aLinkage, UsSymbolTable* aSymbolTablePtr);
   UsSymbol*      Search(const std::string& aName, int aDepth = 1, int aSequenceNumberLimit = 0x7fffffff);
   UsSymbol*      SearchTransCopy(const std::string& aName, int aSequenceNumberLimit = 0x7fffffff);

   UsSymbolTable* GetParentScope() { return mOuterScopes.empty() ? nullptr : mOuterScopes[0].mTablePtr; }

   UsSymbolTable* GetParent() const
   {
      if (mOuterScopes.empty())
         return nullptr;
      return mOuterScopes.back().mTablePtr;
   }

   bool IsInheriting(UsSymbolTable* aParent)
   {
      if (GetParent())
      {
         return GetParent() == aParent || GetParent()->IsInheriting(aParent);
      }
      return false;
   }
   // The first outer scope acts as the parent symbol table, and in most cases there will be only one outer scope
   std::vector<TableLink> mOuterScopes;
   // Inner scopes at this point are only used for deletion later
   std::vector<TableLink> mInnerScopes;
   // The map of entries in this symbol table.  Only one entry per name.
   typedef std::map<std::string, UsSymbol> EntryMap;
   EntryMap                                mEntries;
   // One symbol may operate as a 'this' symbol.  It is implicitly used for method calls.
   // Note: The 'this' pointer is just the type of 'this'.  It's a good enough solution for now.
   UsType* mThisTypePtr;

   UsSymbolTablePool* mPoolPtr;

   UT_MEMORY_DEBUG_MARKER(cMDB_UsSymbolTable);
};

// Light-weight reference to a script value (symbol or function)
struct UsValRef
{
   UsValRef()
      : mFunctionNamePtr(nullptr)
   {
      mSymbolPtr = nullptr;
   }
   UsValRef(UsSymbol* aSymPtr)
      : mFunctionNamePtr(nullptr)
   {
      mSymbolPtr = aSymPtr;
   }
   UsValRef(UsType* aObjectTypePtr, const char* aFunctionNamePtr)
      : mFunctionNamePtr(aFunctionNamePtr)
   {
      mFunctionTypePtr = aObjectTypePtr;
   }
   bool IsNull() const { return mSymbolPtr == nullptr; }
   bool operator==(const UsValRef& aRhs) const
   {
      return mSymbolPtr == aRhs.mSymbolPtr && mFunctionNamePtr == aRhs.mFunctionNamePtr;
   }
   bool operator<(const UsValRef& aRhs) const
   {
      if (mSymbolPtr < aRhs.mSymbolPtr)
      {
         return true;
      }
      if (mSymbolPtr > aRhs.mSymbolPtr)
      {
         return false;
      }
      return mFunctionNamePtr < aRhs.mFunctionNamePtr;
   }
   union
   {
      UsSymbol* mSymbolPtr;
      UsType*   mFunctionTypePtr;
   };
   const char* mFunctionNamePtr;
};

#endif // UsSymbolTable_h__
