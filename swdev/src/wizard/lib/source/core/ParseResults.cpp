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

#include "ParseResults.hpp"

#include <iostream>
#include <stack>

#include "ParseUtil.hpp"
#include "Project.hpp"
#include "ProxyMerge.hpp"
#include "ProxyUtil.hpp"
#include "TextSource.hpp"
#include "UtStringUtil.hpp"
#include "WsfExe.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPProxy.hpp"
#include "WsfPProxyIndex.hpp"
#include "WsfPProxyObjectMap.hpp"
#include "WsfParseAlternate.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRule.hpp"
#include "WsfParseSourceInclude.hpp"
#include "WsfParseTypeLoadRule.hpp"
#include "WsfParser.hpp"
#include "WsfScriptData.hpp"

wizard::ParseResults::ParseResults(Project* aProjectPtr)
   : ParseResultsTiny(aProjectPtr)
   , mParserPtr()
   , mParseTreePtr(nullptr)
   , mLastNodePtr(nullptr)
   , mRootInclude(nullptr)
   , mTypeNamesIndexed(false)
{
   mPlatformPartKinds.push_back(UtStringRef("comms"));
   mPlatformPartKinds.push_back(UtStringRef("movers"));
   mPlatformPartKinds.push_back(UtStringRef("processors"));
   mPlatformPartKinds.push_back(UtStringRef("weapons"));
   mPlatformPartKinds.push_back(UtStringRef("sensors"));
}

wizard::ParseResults::~ParseResults()
{
   Clear();
}

void wizard::ParseResults::Clear()
{
   for (size_t i = 0; i < mErrors.size(); ++i)
   {
      delete mErrors[i];
   }
   mErrors.clear();
   mParseTreePtr = nullptr;
   delete mRootInclude;
   mRootInclude = nullptr;
   mFileNotifications.clear();
   mTypeNamesIndexed = false;
   mFileTransitionNodes.clear();

   delete mScriptDataPtr;
   mScriptDataPtr = nullptr;
   FreeScriptTypes();
}


void wizard::ParseResults::AssignResults(Project*                                           aProjectPtr,
                                         WsfParser*                                         aParserPtr,
                                         const WsfParseSourceInclude*                       aRootInclude,
                                         WsfParseNode*                                      aParseTreePtr,
                                         WsfParseNode*                                      aLastNodePtr,
                                         std::map<UtTextDocument*, WsfParseSourceInclude*>& aFirstIncludes)
{
   const static int cMAX_ERROR_COUNT = 100;

   Clear();
   mRootInclude                              = aRootInclude;
   mProjectPtr                               = aProjectPtr;
   mParserPtr                                = aParserPtr;
   mParseTreePtr                             = aParseTreePtr;
   mLastNodePtr                              = aLastNodePtr;
   const std::vector<WsfParseError*>& errors = mParserPtr->GetErrors();
   for (size_t i = 0; i < errors.size() && i < (size_t)cMAX_ERROR_COUNT; ++i)
   {
      mErrors.push_back(new WsfParseError(*errors[i]));
   }
   std::swap(mFirstIncludes, aFirstIncludes);
   mIncludePaths = mParserPtr->GetParseIndex()->GetIncludePath();
}

bool wizard::ParseResults::FindIncludeAtPoint(UtTextDocument*         aSourcePtr,
                                              const UtTextRange&      aLocation,
                                              WsfParseSourceInclude*& aSourceIncludePtr,
                                              int&                    aEntryIndex,
                                              TextSource*&            aTargetIncludeSourcePtr)
{
   aSourceIncludePtr = nullptr;
   aEntryIndex       = -1;
   aSourceIncludePtr = FindSourceData(aSourcePtr);
   if (aSourceIncludePtr)
   {
      for (size_t i = 0; i < aSourceIncludePtr->mEntries.size(); ++i)
      {
         WsfParseSourceInclude::Entry& include = aSourceIncludePtr->mEntries[i];
         if (include.mType == WsfParseSourceInclude::cINCLUDE_COMMAND)
         {
            if (include.mLocation.Intersects(aLocation))
            {
               aEntryIndex             = (int)i;
               aTargetIncludeSourcePtr = static_cast<TextSource*>(include.mTargetSource);
               return true;
            }
         }
      }
   }
   return false;
}

WsfParseSourceInclude* wizard::ParseResults::FindSourceData(UtTextDocument* aSourcePtr)
{
   std::map<UtTextDocument*, WsfParseSourceInclude*>::iterator iter = mFirstIncludes.find(aSourcePtr);
   if (iter != mFirstIncludes.end())
   {
      return iter->second;
   }
   return nullptr;
}

//! Finds the last WsfParseNode intersecting the specified range.
//! WIZARD_TODO: Need way to efficiently find nodes.  Includes allow nodes to be segmented
//! preventing an efficient approach.  Possible solution is to store ranges of nodes
//! valid for a given source file in the UtTextDocumentData class -- allowing binary search.
WsfParseNode* wizard::ParseResults::FindLastParseNode(const UtTextDocumentRange& aPosition)
{
   std::vector<WsfParseNode*> nodeRanges;
   if (GetRangeNodes(aPosition, nodeRanges) && nodeRanges.size() > 1)
   {
      return nodeRanges.back();
   }
   return nullptr;
}

WsfParseNode* wizard::ParseResults::FindBlock(WsfParseNode* aNodePtr)
{
   bool        found     = false;
   std::string blockText = "block";

   WsfParseNode* nodePtr = aNodePtr;
   while (nodePtr->GetParent() != nullptr)
   {
      nodePtr = nodePtr->GetParent();
      if (nodePtr->mType == blockText)
      {
         found = true;
         break;
      }
      WsfParseNode* childBlockPtr = nodePtr->FindChildByType(blockText);
      if (childBlockPtr != nullptr)
      {
         nodePtr = childBlockPtr;
         found   = true;
         break;
      }
   }
   if (found)
   {
      return nodePtr;
   }
   return nullptr;
}

void wizard::ParseResults::FindDefinitions(const std::string&          aName,
                                           const std::string&          aDefinitionKind,
                                           std::vector<WsfParseNode*>& aNodes)
{
   WsfParseNode* nodePtr = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & (WsfParseNode::cTYPE_NAME_NODE | WsfParseNode::cLAZY_TYPENAME_NODE))
      {
         WsfParseTypePath typeKey;
         bool             isNested;
         if (wizard::ParseUtil::FindReferenceType(nodePtr, typeKey, isNested) && typeKey.size() >= 2)
         {
            const std::string& name = typeKey[1].Get();
            if ((name == aName) && (aDefinitionKind.empty() || typeKey[0] == aDefinitionKind))
            {
               aNodes.push_back(nodePtr);
            }
         }
      }
      nodePtr = nodePtr->Next();
   }
}

void wizard::ParseResults::FindReferences(const std::string&          aName,
                                          const std::string&          aDefinitionKind,
                                          std::vector<WsfParseNode*>& aNodes)
{
   const int     cIMPORTANT_FLAGS = WsfParseNode::cTYPE_REFERENCE_NODE | WsfParseNode::cLAZY_TYPE_REFERENCE_NODE;
   WsfParseNode* nodePtr          = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & cIMPORTANT_FLAGS)
      {
         WsfParseTypePath typeKey;
         bool             isNested;
         if (wizard::ParseUtil::FindReferenceType(nodePtr, typeKey, isNested) && typeKey.size() >= 2)
         {
            if (typeKey.back() == aName)
            {
               if (aDefinitionKind.empty() || aDefinitionKind == typeKey[0])
               {
                  aNodes.push_back(nodePtr);
               }
            }
         }
      }
      nodePtr = nodePtr->Next();
   }
}

void wizard::ParseResults::FindDefinitionsOfType(const std::string&          aDefinitionType,
                                                 std::vector<WsfParseNode*>& aNodes,
                                                 bool                        aAllowNested)
{
   const int     cTYPENAME_FLAGS = WsfParseNode::cTYPE_NAME_NODE | WsfParseNode::cLAZY_TYPENAME_NODE;
   WsfParseNode* nodePtr         = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & cTYPENAME_FLAGS)
      {
         WsfParseTypePath typeKey;
         bool             isNested;
         if (wizard::ParseUtil::FindReferenceType(nodePtr, typeKey, isNested) && typeKey.size() >= 2)
         {
            if ((aAllowNested || !isNested) && aDefinitionType == typeKey[0])
            {
               aNodes.push_back(nodePtr);
            }
         }
      }
      nodePtr = nodePtr->Next();
   }
}
void wizard::ParseResults::FindDefinitionsOfType(const WsfParseTypePath&     aPath,
                                                 std::vector<WsfParseNode*>& aNodes,
                                                 bool                        aAllowNested)
{
   const int     cTYPENAME_FLAGS = WsfParseNode::cTYPE_NAME_NODE | WsfParseNode::cLAZY_TYPENAME_NODE;
   WsfParseNode* nodePtr         = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & cTYPENAME_FLAGS)
      {
         WsfParseTypePath typeKey;
         bool             isNested;
         if (wizard::ParseUtil::FindReferenceType(nodePtr, typeKey, isNested))
         {
            if ((aAllowNested || !isNested) && typeKey.size() >= aPath.size())
            {
               bool isMatch(true);
               for (size_t i = 0; i < aPath.size(); ++i)
               {
                  if (!(aPath[i] == typeKey[i]))
                  {
                     isMatch = false;
                  }
               }
               if (isMatch)
               {
                  aNodes.push_back(nodePtr);
               }
            }
         }
      }
      nodePtr = nodePtr->Next();
   }
}

void wizard::ParseResults::FindNamedNodes(const std::string&          aCategory,
                                          const std::string&          aName,
                                          std::vector<WsfParseNode*>& aNodes)
{
   WsfParseNode* nodePtr = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & WsfParseNode::cNAMED_NODE)
      {
         if (nodePtr->mValue == aName && nodePtr->mType == aCategory)
         {
            aNodes.push_back(nodePtr);
         }
      }
      nodePtr = nodePtr->Next();
   }
}

void wizard::ParseResults::GetBaseTypes(const WsfParseTypePath&   aKind,
                                        std::vector<std::string>& aBaseTypeNames,
                                        bool                      aIncludeUserTypes)
{
   if (aKind.empty())
   {
      return;
   }
   if (mProxy->GetRegistry() == nullptr)
   {
      return;
   }
   WsfPProxyValue root = mProxy->GetRegistry()->GetBasicRoot();
   // std::cerr << mProxy->root << '\n';
   std::string    kindStr      = WsfParseTypePathString(aKind);
   WsfPProxyPath  path         = WsfPProxyPath::FromString(root, kindStr);
   WsfPProxyValue typeLists[2] = {root.Lookup(path), WsfPProxyValue()};
   if (aIncludeUserTypes && mProxy->mRoot)
   {
      typeLists[1] = mProxy->mRoot.Lookup(path);
   }
   for (int i = 0; i < 2; ++i)
   {
      WsfPProxyObjectMap* mapPtr = typeLists[i].GetObjectMap();
      if (mapPtr)
      {
         auto& values = mapPtr->GetValues();
         for (auto i = values.begin(); i != values.end(); ++i)
         {
            aBaseTypeNames.push_back(i->first);
         }
      }
   }
   //    //if (aKind == "platform_type")
   //    //{
   //       //aBaseTypeNames.push_back("WSF_PLATFORM");
   //    //}
   //    //else
   //    {
   //       WsfParseType* typeBase = mParserPtr->GetDefinitions()->mBasicTypes->FindType(aKind);
   //       if (typeBase && typeBase->Table())
   //       {
   //          WsfParseSymbolTable::SymbolTable& sym = typeBase->Table()->mSymbols;
   //          for (WsfParseSymbolTable::SymbolTable::const_iterator i = sym.begin(); i != sym.end(); ++i)
   //          {
   //             aBaseTypeNames.push_back(i->first.Get());
   //          }
   //       }
   //    }
}

void wizard::ParseResults::FindNames(const std::string& aNameType, std::vector<std::string>& aNameList)
{
   WsfParseNode* nodePtr = mParseTreePtr;
   while (nodePtr)
   {
      if ((nodePtr->mFlags & WsfParseNode::cNAMED_NODE) && (aNameType == nodePtr->mType) && nodePtr->mValue.Valid())
      {
         std::string name = nodePtr->mValue.Text();
         aNameList.push_back(name);
      }
      nodePtr = nodePtr->Next();
   }
   std::sort(aNameList.begin(), aNameList.end());
   aNameList.erase(std::unique(aNameList.begin(), aNameList.end()), aNameList.end());
}
//
// std::string wizard::ParseResults::GetPartBaseType(WsfParseNode* aPartNodePtr)
// {
//    WsfParseNode* partTyping = aPartNodePtr->FindChildByType("platform-part-typing");
//    if (partTyping)
//    {
//       WsfParseNode* typeNodePtr = partTyping->FindChildByType("part-type");
//       if (! typeNodePtr)
//       {
//          WsfParseNode* nameNodePtr = partTyping->FindChildByType("part-name");
//          mParseIndexPtr->FindP
//       }
//    }
// }
void wizard::ParseResults::FindIncludes(TextSource* aSourcePtr, std::vector<UtTextDocumentRange>& aIncludes)
{
   if (!mRootInclude)
   {
      return;
   }
   UtTextDocument*                          srcFind = aSourcePtr->GetSource();
   std::stack<const WsfParseSourceInclude*> s;
   s.push(mRootInclude);
   while (!s.empty())
   {
      const WsfParseSourceInclude* incPtr = s.top();
      s.pop();
      for (size_t i = 0; i < incPtr->mEntries.size(); ++i)
      {
         const WsfParseSourceInclude::Entry& entry = incPtr->mEntries[i];
         if (incPtr->mEntries[i].mType == WsfParseSourceInclude::cINCLUDE_COMMAND)
         {
            if (entry.mTargetSource == srcFind && !entry.mIsVirtualInclude)
            {
               aIncludes.push_back(UtTextDocumentRange(incPtr->mSourcePtr, entry.mLocation));
            }
            if (incPtr->mEntries[i].mIncludePtr)
            {
               s.push(incPtr->mEntries[i].mIncludePtr.get());
            }
         }
      }
   }
}

void wizard::ParseResults::FindReferencedFiles(std::vector<WsfParseNode*>& aFileReferenceNodes)
{
   WsfParseNode* nodePtr = mParseTreePtr;
   while (nodePtr != nullptr)
   {
      if (nodePtr->GetAuxiliaryValue() && nodePtr->GetAuxiliaryValue()->mDataType == WsfParseAuxData::cFILE_REFERENCE)
      {
         aFileReferenceNodes.push_back(nodePtr);
      }
      nodePtr = nodePtr->Next();
   }
}

wizard::ParseResults::FileNotificationList* wizard::ParseResults::GetFileNotifications(TextSource* aSourcePtr)
{
   FileNotificationMap::iterator i = mFileNotifications.find(aSourcePtr);
   if (i != mFileNotifications.end())
   {
      return &i->second;
   }
   if (!mProxy.get())
   {
      return nullptr;
   }
   if (!mProxy->mRoot.IsValid() || mProxy->mIndex == nullptr || isProxyDataOld())
   {
      return nullptr;
   }

   if (!mTypeNamesIndexed)
   {
      IndexNames();
   }
   FileNotificationList& notifications = mFileNotifications[aSourcePtr];

   std::map<UtTextDocument*, WsfParseSourceInclude*>::iterator includeIter = mFirstIncludes.find(aSourcePtr);
   if (includeIter != mFirstIncludes.end())
   {
      WsfParseSourceInclude* incPtr = includeIter->second;
      for (size_t i = 0; i < incPtr->mEntries.size(); ++i)
      {
         WsfParseSourceInclude::Entry& entry = incPtr->mEntries[i];
         if (entry.mType == WsfParseSourceInclude::cINCLUDE_COMMAND)
         {
            FileNotification note(entry.mLocation, cINCLUDE_KIND);
            note.mNotificationType |= cINFORMATION_BIT;
            if (!incPtr->mSourcePtr)
            {
               note.mNotificationType |= cERROR_BIT;
            }
            notifications.push_back(note);
         }
      }
   }

   size_t                     size = aSourcePtr->GetText().Size();
   UtTextDocumentRange        range(aSourcePtr, 0, size - 1);
   std::vector<WsfParseNode*> nodes;
   GetRangeNodes(range, nodes);

   for (int i = 0; i < (int)nodes.size(); i += 2)
   {
      WsfParseNode* n    = nodes[i];
      WsfParseNode* last = nodes[i + 1];
      last               = last->Next();
      while (n != last)
      {
         if (n->mFlags & WsfParseNode::cLOAD_TYPE_NODE)
         {
            WsfParseTypeInfoData* auxPtr = (WsfParseTypeInfoData*)n->GetAuxiliaryValue();
            WsfParseNode*         tmp    = n->Left();
            while (tmp)
            {
               if (tmp->mFlags & WsfParseNode::cTYPE_REFERENCE_NODE)
               {
                  FileNotification typeNote(tmp->mValue, cTYPE_KIND);
                  typeNote.mNotificationType |= cINFORMATION_BIT;
                  if (auxPtr->mFlags & WsfParseTypeInfoData::cINVALID_TYPE)
                  {
                     typeNote.mNotificationType |= cERROR_BIT;
                  }
                  notifications.push_back(typeNote);
               }
               else if (tmp->mFlags & WsfParseNode::cTYPE_NAME_NODE)
               {
                  FileNotification nameNote(tmp->mValue, cTYPE_KIND);
                  nameNote.mNotificationType |= cINFORMATION_BIT;
                  if (auxPtr->mFlags & WsfParseTypeInfoData::cDUPLICATE_NAME)
                  {
                     nameNote.mNotificationType |= cERROR_BIT;
                  }
                  notifications.push_back(nameNote);
               }
               tmp = tmp->Left();
            }
         }
         else if (n->mFlags & WsfParseNode::cLAZY_TYPE_REFERENCE_NODE && n->mType != nullptr)
         {
            bool foundType = false;
            if (n->mType[0] == '.')
            {
               const WsfPProxyPath* currentTypePtr = mProxy->mIndex->FindCurrentPath(n);

               if (currentTypePtr)
               {
                  WsfPProxyPath path = *currentTypePtr;
                  while (!foundType)
                  {
                     WsfPProxyValue container = mProxy->mRoot.Lookup(path);
                     UtStringRef    kind(new std::string(n->mType + 1));
                     // WsfParseType* lookupTypePtr;
                     if (kind == "platform_part")
                     {
                        if (container.IsStruct() && container.GetType()->mTypeName == "Platform")
                        {
                           const char* partMapNames[] = {"sensors", "processors", "weapons", "comms"};

                           std::string partName = n->mValue.Text();
                           for (size_t i = 0; i < 4 && !foundType; ++i)
                           {
                              WsfPProxyObjectMap* partMap = container[partMapNames[i]].GetObjectMap();
                              if (partMap)
                              {
                                 if (partMap->GetAt(partName).IsValid())
                                 {
                                    foundType = true;
                                    break;
                                 }
                              }
                           }
                        }
                     }
                     else
                     {
                        WsfPProxyValue refVal = container[kind][n->mValue.Text()];
                        if (refVal.IsValid())
                        {
                           foundType = true;
                        }
                     }
                     if (path.Empty())
                     {
                        break;
                     }
                     path.Pop();
                  }
               }
            }
            else
            {
               if (mProxy->mRoot[n->mType][n->mValue.Text()].IsValid())
               {
                  foundType = true;
               }
               else
               {
                  if (mProxy->GetRegistry()->GetBasicRoot()[n->mType][n->mValue.Text()].IsValid())
                  {
                     foundType = true;
                  }
               }
               if (!foundType)
               {
                  WsfParseTypePath tp;
                  tp.push_back(UtStringRef(n->mType));
                  tp.push_back(UtStringRef(n->mValue.Text()));
                  if (mTypeNames.find(tp) != mTypeNames.end())
                  {
                     foundType = true;
                  }
               }
            }
            FileNotification refNote(n->mValue, cREFERENCE_KIND);
            refNote.mNotificationType |= cINFORMATION_BIT;
            if (!foundType)
            {
               refNote.mNotificationType |= cWARNING_BIT;
            }
            notifications.push_back(refNote);
         }
         else if (n->mFlags & WsfParseNode::cLAZY_TYPENAME_NODE && n->mType != nullptr)
         {
            FileNotification refNote(n->mValue, cTYPE_KIND);
            refNote.mNotificationType |= cINFORMATION_BIT;
            notifications.push_back(refNote);
         }
         n = n->Next();
      }
   }
   for (size_t i = 0; i < mErrors.size(); ++i)
   {
      if (aSourcePtr == mErrors[i]->mRange.mSource)
      {
         FileNotification refNote(mErrors[i]->mRange, cSYNTAX_ERROR_KIND);
         refNote.mNotificationType |= cERROR_BIT;
         notifications.push_back(refNote);
      }
   }
   return &notifications;
}

wizard::ParseResults::FileNotification wizard::ParseResults::GetNotification(const UtTextDocumentLocation& aLocation)
{
   wizard::TextSource* sourcePtr = static_cast<wizard::TextSource*>(aLocation.mSource);
   if (sourcePtr)
   {
      FileNotificationList* notificationListPtr = GetFileNotifications(sourcePtr);
      if (notificationListPtr)
      {
         for (size_t i = 0; i < notificationListPtr->size(); ++i)
         {
            ParseResults::FileNotification& note = (*notificationListPtr)[i];
            if (note.mRange.Contains(aLocation.mPosition))
            {
               return note;
            }
         }
      }
   }
   return FileNotification(UtTextRange(), 0);
}

//! Returns the ranges of nodes containing the values in the given range.
//! @param aRange  The range of text to find nodes in.
//! @param aNodes  A vector containing a pair of nodes for each range marking the begin and end.
//! @note  Run time is O([Tokens In File] - [Tokens in range] + [Files in scenario])
//!        When requesting the entire file range, O([Files in scenario])
bool wizard::ParseResults::GetRangeNodes(const UtTextDocumentRange& aRange, std::vector<WsfParseNode*>& aNodes)
{
   bool found = false;
   for (size_t i = 0; i < mFileTransitionNodes.size(); ++i)
   {
      WsfParseNode* transNodePtr = mFileTransitionNodes[i];
      if ((transNodePtr != nullptr) && (transNodePtr->mValue.mSource == aRange.mSource))
      {
         if (transNodePtr->mValue.GetBegin() > aRange.GetEnd())
         {
            continue;
         }
         WsfParseNode* lastPtr = nullptr;
         if (i + 1 < mFileTransitionNodes.size())
         {
            lastPtr = mFileTransitionNodes[i + 1]->PreviousLeaf();
         }
         else
         {
            lastPtr = mLastNodePtr;
         }
         if (lastPtr != nullptr)
         {
            UtTextRange range = transNodePtr->mValue;
            range.Extend(lastPtr->mValue);
            if (range.Intersects(aRange))
            {
               while (transNodePtr->mValue.GetEnd() < aRange.GetBegin())
               {
                  WsfParseNode* n = transNodePtr->NextLeaf();
                  if (!n)
                  {
                     transNodePtr = nullptr;
                     break;
                  }
                  else
                  {
                     transNodePtr = n;
                  }
               }
               if (transNodePtr)
               {
                  if (!(lastPtr && lastPtr->mValue.Intersects(aRange)))
                  {
                     while (lastPtr)
                     {
                        WsfParseNode* n = lastPtr->PreviousLeaf();
                        if (!n || n->mValue.mSource != aRange.mSource)
                        {
                           break;
                        }
                        else if (n->mValue.GetEnd() < aRange.GetEnd())
                        {
                           if (n->mValue.Intersects(aRange))
                           {
                              lastPtr = n;
                           }
                           break;
                        }
                        else
                        {
                           lastPtr = n;
                        }
                     }
                  }
                  if (lastPtr)
                  {
                     // Adjust the first node up until we reach a node containing other leafs
                     while (!transNodePtr->Left())
                     {
                        WsfParseNode* p = transNodePtr->GetParent();
                        if (p && p->Down() == transNodePtr)
                        {
                           transNodePtr = p;
                        }
                        else
                        {
                           break;
                        }
                     }
                     aNodes.push_back(transNodePtr);
                     aNodes.push_back(lastPtr);
                     found = true;
                  }
               }
            }
         }
      }
   }
   return found;
}

//! Build an index of 'names' defined in the input file.
//! These are types that do not produce an entry in the parser's type dictionary
void wizard::ParseResults::IndexNames()
{
   mTypeNames.clear();
   WsfParseNode* n = mParseTreePtr;
   while (n != nullptr)
   {
      if (n->mFlags & WsfParseNode::cLAZY_TYPENAME_NODE)
      {
         WsfParseTypePath key;
         key.push_back(new std::string(n->mType));
         key.push_back(n->mValue.Text());
         mTypeNames.insert(key);
      }
      n = n->Next();
   }
   mTypeNamesIndexed = true;
}

//! Return the type lookup for the type located at aNodePtr
WsfParseType* wizard::ParseResults::FindCurrentType(WsfParseNode* aNodePtr)
{
   WsfParseNode*                 nodePtr = aNodePtr;
   std::vector<WsfParseTypePath> typeStack;
   while (nodePtr)
   {
      if ((nodePtr->mFlags & WsfParseNode::cLOAD_TYPE_NODE) && nodePtr->GetAuxiliaryValue())
      {
         WsfParseTypeInfoData* typeDataPtr = (WsfParseTypeInfoData*)nodePtr->GetAuxiliaryValue();
         if (!typeDataPtr->mSaveKey.empty())
         {
            typeStack.push_back(typeDataPtr->mSaveKey);
         }
         else
         {
            typeStack.push_back(typeDataPtr->mLoadKey);
         }
         if (!(typeDataPtr->mFlags & (WsfParseTypeInfoData::cNESTED_SAVE | WsfParseTypeInfoData::cNESTED_LOAD)))
         {
            break;
         }
      }
      if (nodePtr->Left())
      {
         nodePtr = nodePtr->Left();
      }
      else
      {
         nodePtr = nodePtr->GetParent();
      }
   }
   const WsfParseType* currentTablePtr = nullptr;
   WsfParseType*       typeRefPtr      = nullptr;
   for (int i = (int)typeStack.size() - 1; i >= 0; --i)
   {
      const WsfParseType* typePtr = nullptr;
      if (currentTablePtr != nullptr)
      {
         typePtr = currentTablePtr->FindType(typeStack[i]);
      }
      else
      {
         typePtr = GetParseIndex()->mUserTypes.FindType(typeStack[i]);
      }
      if (typePtr)
      {
         typeRefPtr      = (WsfParseType*)typePtr;
         currentTablePtr = typePtr;
      }
      else
      {
         break;
      }
   }
   return typeRefPtr;
}

//! Attempts to find type information given a location.  The location should point to the text
//! referring to the type.
bool wizard::ParseResults::TypeInformationFromPosition(const UtTextDocumentLocation& aLocation, TypeInformation& aInformation)
{
   WsfParseNode* nodePtr = FindLastParseNode(UtTextDocumentRange(aLocation, 1));
   if (nodePtr)
   {
      return TypeInformationFromNode(nodePtr, aInformation);
   }
   return false;
}

//! Fills aInformation given the type key.  Returns 'true' if the type information was found successfully.
bool wizard::ParseResults::FindTypeInfo(TypeInformation& aInformation)
{
   WsfParseType* basicTypesPtr = mParserPtr->GetDefinitions()->mBasicTypes;
   WsfParseType& userTypes     = GetParseIndex()->mUserTypes;
   bool          foundType     = false;
   WsfParseType* foundTypePtr  = nullptr;
   if (aInformation.mParent && !aInformation.mTypeKey.empty())
   {
      if (aInformation.mTypeKey[0] == "platform_part")
      {
         for (size_t i = 0; i < mPlatformPartKinds.size() && !foundType; ++i)
         {
            WsfParseTypePath key;
            key.push_back(mPlatformPartKinds[i]);
            key.push_back(aInformation.mTypeKey.back());
            if (aInformation.mParent)
            {
               WsfParseType* lookupTypePtr = aInformation.mParent->FindNestedSymbol(key);
               if (lookupTypePtr)
               {
                  aInformation.mTypeKey = key;
                  aInformation.mType    = lookupTypePtr;
                  foundType             = true;
                  break;
               }
            }
         }
      }
      else
      {
         WsfParseType* lookupTypePtr;
         if (aInformation.mParent)
         {
            lookupTypePtr = aInformation.mParent->FindNestedSymbol(aInformation.mTypeKey);
            if (lookupTypePtr)
            {
               aInformation.mType = lookupTypePtr;
               foundType          = true;
            }
         }
      }
   }
   if (!foundType)
   {
      // Check for basic type
      foundTypePtr = basicTypesPtr->FindType(aInformation.mTypeKey);
      if (foundTypePtr)
      {
         aInformation.mType  = foundTypePtr;
         aInformation.mFlags = TypeInformation::cBASIC_TYPE;
         foundType           = true;
      }
      else
      {
         // Check for user type
         UtStringRef& name = aInformation.mTypeKey.back();
         if (!name.empty())
         {
            WsfParseTypePath path = aInformation.mTypeKey;
            foundTypePtr          = (WsfParseType*)userTypes.FindType(path);
            if (foundTypePtr)
            {
               aInformation.mType = foundTypePtr;
               foundType          = true;
            }
            else
            {
               // Check for the 'lazy type'
               if (mTypeNames.find(aInformation.mTypeKey) != mTypeNames.end())
               {
                  aInformation.mFlags |= TypeInformation::cLAZY_TYPE;
                  foundType = true;
               }
            }
         }
      }
   }
   if (!foundType)
   {
      aInformation.mFlags |= TypeInformation::cTYPE_LOOKUP_FAIL;
   }
   return foundType;
}

//! Fills aDocumentation with the string containing the user's documentation in comments above the given type.
//! Returns 'true' if the documentation is found.
bool wizard::ParseResults::FindUserTypeDocumentation(TypeInformation& aType, std::string& aDocumentation)
{
   WsfParseNode* leafPtr = FindTypeDefinitionNode(aType);
   if (leafPtr)
   {
      if (leafPtr && leafPtr->mValue.Valid())
      {
         TextSource* sourcePtr = static_cast<TextSource*>(leafPtr->mValue.mSource);
         size_t      line      = sourcePtr->PositionToLineNumber(leafPtr->mValue.GetBegin());
         size_t      lastLine  = line - 1;
         size_t      firstLine = lastLine;
         std::string docString;
         while (firstLine != ut::npos)
         {
            std::string lineText = sourcePtr->GetLineString(firstLine);
            if (lineText.size() > 2)
            {
               if (lineText[0] == '#')
               {
                  docString = lineText.substr(1) + docString;
               }
               else if (lineText[0] == '/' && lineText[1] == '/')
               {
                  docString = lineText.substr(2) + docString;
               }
               else
               {
                  break;
               }
            }
            else
            {
               break;
            }
            --firstLine;
         }
         if (!docString.empty())
         {
            UtStringUtil::TrimWhiteSpace(docString);
            aDocumentation = docString;
            return true;
         }
      }
   }
   return false;
}

// Fills aInformation given a node that references a type.
bool wizard::ParseResults::TypeInformationFromNode(WsfParseNode* aNodePtr, TypeInformation& aInformation)
{
   if (!GetParseIndex() || !aNodePtr || !aNodePtr->mValue.Valid())
   {
      return false;
   }
   aInformation.mType    = nullptr;
   aInformation.mParent  = nullptr;
   aInformation.mFlags   = 0;
   aInformation.mTypeKey = WsfParseTypePath();

   UtStringRef typeName;
   aNodePtr->mValue.Text(typeName);
   // scan forward looking for the node containing type information
   WsfParseNode* typeLoadNode = aNodePtr;
   for (int i = 0; i < 10 && typeLoadNode && !(typeLoadNode->mFlags & WsfParseNode::cLOAD_TYPE_NODE); ++i)
   {
      typeLoadNode = typeLoadNode->Right();
   }
   if (typeLoadNode && (typeLoadNode->mFlags & WsfParseNode::cLOAD_TYPE_NODE) && typeLoadNode->GetAuxiliaryValue())
   {
      WsfParseTypeInfoData* typeInfoPtr = (WsfParseTypeInfoData*)typeLoadNode->GetAuxiliaryValue();
      if (aNodePtr->mFlags & WsfParseNode::cTYPE_REFERENCE_NODE)
      {
         aInformation.mTypeKey = typeInfoPtr->mLoadKey;
         if (typeInfoPtr->mFlags & WsfParseTypeInfoData::cNESTED_LOAD)
         {
            aInformation.mParent = FindCurrentType(aNodePtr);
         }
         if (typeInfoPtr->mFlags & WsfParseTypeInfoData::cINVALID_TYPE)
         {
            aInformation.mFlags |= TypeInformation::cTYPE_LOOKUP_FAIL;
         }
      }
      else if (aNodePtr->mFlags & WsfParseNode::cTYPE_NAME_NODE)
      {
         aInformation.mTypeKey = typeInfoPtr->mSaveKey;
         if (typeInfoPtr->mFlags & WsfParseTypeInfoData::cNESTED_SAVE)
         {
            aInformation.mParent = FindCurrentType(aNodePtr);
         }
         if (typeInfoPtr->mFlags & WsfParseTypeInfoData::cDUPLICATE_NAME)
         {
            aInformation.mFlags |= TypeInformation::cDUPLICATE_NAME;
         }
      }
   }
   if (aNodePtr->mFlags & WsfParseNode::cLAZY_TYPENAME_NODE && aNodePtr->mType)
   {
      aInformation.mTypeKey.clear();
      aInformation.mTypeKey.push_back(new std::string(aNodePtr->mType));
      aInformation.mTypeKey.push_back(typeName);
   }
   if (aNodePtr->mFlags & WsfParseNode::cLAZY_TYPE_REFERENCE_NODE && aNodePtr->mType)
   {
      aInformation.mTypeKey.clear();
      if (aNodePtr->mType[0] == '.')
      {
         aInformation.mTypeKey.push_back(new std::string(aNodePtr->mType + 1));
         aInformation.mParent = FindCurrentType(aNodePtr);
      }
      else
      {
         aInformation.mTypeKey.push_back(new std::string(aNodePtr->mType));
      }
      aInformation.mTypeKey.push_back(typeName);
   }
   if (!aInformation.mTypeKey.empty())
   {
      FindTypeInfo(aInformation);
      return true;
   }
   return false;
}

//! Searches for the type information of aType's base type.
bool wizard::ParseResults::FindBaseType(const TypeInformation& aType, TypeInformation& aBaseType)
{
   if (aType.mFlags & TypeInformation::cBASIC_TYPE)
   {
      return false;
   }
   if (aType.mFlags & TypeInformation::cLAZY_TYPE)
   {
      return false;
   }

   WsfParseNode* nodePtr = FindCreateTypeNode(aType);
   if (nodePtr)
   {
      WsfParseNode* n = nodePtr->Left();
      while (n)
      {
         if (n->mFlags & WsfParseNode::cTYPE_REFERENCE_NODE)
         {
            return TypeInformationFromNode(n, aBaseType);
         }
         n = n->Left();
      }
   }
   return false;
}

//! Searches for the parse node that created the specified type.
WsfParseNode* wizard::ParseResults::FindCreateTypeNode(const TypeInformation& aType)
{
   if (aType.mType == nullptr)
   {
      return nullptr;
   }
   if (aType.mFlags & TypeInformation::cBASIC_TYPE)
   {
      return nullptr;
   }
   if (aType.mFlags & TypeInformation::cLAZY_TYPE)
   {
      return nullptr;
   }
   WsfParseType* typeContext = aType.mType;
   if (typeContext)
   {
      // WIZARD_TODO: Not sure if this is the right level?
      typeContext = typeContext->GetContainer();
   }
   std::vector<WsfParseType*> contextAncestry;
   if (typeContext)
   {
      typeContext->GetAncestry(contextAncestry);
   }
   int           contextDepth = typeContext ? typeContext->GetDepth() : 0;
   int           bestMatch    = -1;
   WsfParseNode* matchPtr     = nullptr;

   WsfParseNode* nodePtr = mParseTreePtr;
   while (nodePtr)
   {
      if (nodePtr->GetFlags() & WsfParseNode::cLOAD_TYPE_NODE)
      {
         WsfParseTypeInfoData* typeInfoPtr = (WsfParseTypeInfoData*)nodePtr->GetAuxiliaryValue();
         if (typeInfoPtr)
         {
            if (typeInfoPtr->mOperation == WsfParseTypeInfoData::cCREATE_TYPE)
            {
               if (typeInfoPtr->mSaveKey == aType.mTypeKey)
               {
                  // WIZARD_TODO: Double check this
                  if (aType.mType->GetDepth() > 1) // we are trying to find a nested type
                  {
                     WsfParseNode* n = nodePtr;
                     while (n && 0 == (n->GetFlags() & WsfParseNode::cTYPE_NAME_NODE))
                     {
                        n = n->Left();
                     }
                     if (n)
                     {
                        WsfParseType*              current = FindCurrentType(n);
                        int                        i       = 0;
                        std::vector<WsfParseType*> currentAncestry;
                        current->GetAncestry(currentAncestry);
                        for (; i < contextDepth && i < current->GetDepth() && contextAncestry[i] == currentAncestry[i]; ++i)
                        {
                        }
                        if (bestMatch < (int)i)
                        {
                           bestMatch = (int)i;
                           matchPtr  = nodePtr;
                        }
                     }
                  }
                  if (bestMatch < 0)
                  {
                     bestMatch = 0;
                     matchPtr  = nodePtr;
                  }
               }
            }
         }
      }
      nodePtr = nodePtr->Next();
   }
   return matchPtr;
}

std::vector<WsfParseNode*> wizard::ParseResults::FindTypeReferenceNodes(const TypeInformation& aType)
{
   std::vector<WsfParseNode*> refs;
   if (aType.mType == nullptr)
   {
      return refs;
   }

   const int typeFlags = WsfParseNode::cNAMED_NODE | WsfParseNode::cLOAD_TYPE_NODE |
                         WsfParseNode::cLAZY_TYPE_REFERENCE_NODE | WsfParseNode::cLAZY_TYPENAME_NODE;
   WsfParseNode* n = mParseTreePtr;

   while (n)
   {
      if ((n->GetFlags() & typeFlags) && !aType.mTypeKey.empty() && n->mValue == aType.mTypeKey.back())
      {
         TypeInformation info;
         if (TypeInformationFromNode(n, info))
         {
            if (TypeKeyMatches(info.mTypeKey, aType.mTypeKey))
            {
               bool match = false;
               if (aType.mType->GetDepth() > 1)
               {
                  if (aType.mType == info.mType)
                  {
                     match = true;
                  }
               }
               else
               {
                  match = true;
               }
               if (match)
               {
                  refs.push_back(n);
               }
            }
         }
      }
      n = n->NextLeaf();
   }
   return refs;
}

//! Searches for the parse node which defined aType.
WsfParseNode* wizard::ParseResults::FindTypeDefinitionNode(const TypeInformation& aType)
{
   WsfParseNode* nodePtr = FindCreateTypeNode(aType);
   if (!nodePtr)
   {
      WsfParseNode* n = mParseTreePtr;
      while (n)
      {
         if (n->mFlags & WsfParseNode::cLAZY_TYPENAME_NODE)
         {
            if (n->mType && n->mType == aType.mTypeKey[0].Get())
            {
               if (n->mValue == aType.mTypeKey.back().Get())
               {
                  return n;
               }
            }
         }
         n = n->NextLeaf();
      }
   }
   else
   {
      while (nodePtr && ((nodePtr->mFlags & WsfParseNode::cTYPE_NAME_NODE) == 0))
      {
         nodePtr = nodePtr->Left();
      }
   }
   return nodePtr;
}

WsfPProxyPath wizard::ParseResults::FindProxyPathFromLocation(const UtTextDocumentRange& aLocation, int& aEntryTypeRef)
{
   WsfPProxyPath   path;
   WsfPProxyIndex* indexPtr = ProxyIndex();
   if (indexPtr)
   {
      WsfParseNode* nodePtr = FindLastParseNode(aLocation);
      if (nodePtr)
      {
         size_t                             lexIndex = nodePtr->LexicalIndex();
         size_t                             subEntry = 0;
         WsfPProxyIndex::ReverseIndexEntry* entry    = indexPtr->FindByParseNodeIndex(lexIndex, subEntry);
         if (entry && entry->mNodePtr)
         {
            if (entry->mNodePtr->mEntries.size() > subEntry)
            {
               aEntryTypeRef = entry->mNodePtr->mEntries[subEntry].mEntryType;
               entry->mNodePtr->GetPath(path);
            }
         }
      }
   }
   return path;
}

WsfPProxyNode wizard::ParseResults::FindProxyReference(WsfPProxyNode aContext, WsfParseTypePath aPath)
{
   if (aPath.empty())
   {
      return aContext;
   }
   std::string tok = aPath[0].Get();
   if (tok == ".platform_part" && aPath.size() >= 2)
   {
      if (aContext.IsA_StructTypeOf("Platform"))
      {
         WsfPM_Platform plat(aContext);
         WsfPProxyNode  part = plat.FindPart(aPath[1]);
         if (part)
         {
            WsfParseTypePath newPath(aPath.begin() + 2, aPath.end());
            WsfPProxyNode    result = FindProxyReference(part, newPath);
            if (result)
            {
               return result;
            }
         }
      }
   }
   else if (!tok.empty())
   {
      if (tok[0] == '.')
      {
         tok = tok.substr(1);
      }
      WsfPProxyNode n = aContext + tok;
      if (n)
      {
         return FindProxyReference(n, WsfParseTypePath(aPath.begin() + 1, aPath.end()));
      }
   }
   if (!aContext.GetPath().Empty())
   {
      return FindProxyReference(aContext.GetParent(), aPath);
   }
   return WsfPProxyNode();
}

WsfParseIndex* wizard::ParseResults::GetParseIndex()
{
   if (mParserPtr)
   {
      return mParserPtr->GetParseIndex();
   }
   return nullptr;
}


wizard::ParseResultsTiny::ParseResultsTiny(Project* aProjectPtr)
   : mSequenceNumber(0)
   , mProjectPtr(aProjectPtr)
   , mScriptDataPtr(nullptr)
   , mScriptTypesPtr(nullptr)
{
}

wizard::ParseResultsTiny::~ParseResultsTiny()
{
   if (mScriptDataPtr)
   {
      delete mScriptDataPtr;
      mScriptDataPtr = nullptr;
   }
   FreeScriptTypes();
}

//! Copies a minimal subset of results from aResults
void wizard::ParseResultsTiny::TakeResults(ParseResults& aResults)
{
   mSequenceNumber = aResults.mSequenceNumber;
   mProjectPtr     = aResults.mProjectPtr;
   mIncludePaths.clear();
   std::swap(mIncludePaths, aResults.mIncludePaths);
   mFirstIncludes.clear();
   std::swap(mFirstIncludes, aResults.mFirstIncludes);

   if (mProxy)
   {
      mProxy->Registry(std::make_shared<WsfPProxyRegistry>());
      mProxy->mRoot.Delete();
   }
   SwapProxy(aResults, true);

   if (mScriptDataPtr)
   {
      delete mScriptDataPtr;
      mScriptDataPtr = nullptr;
   }

   FreeScriptTypes();
   std::swap(mScriptTypesPtr, aResults.mScriptTypesPtr);
   std::swap(mScriptDataPtr, aResults.mScriptDataPtr);
}

bool wizard::ParseResultsTiny::IncludesFile(TextSource* aSourcePtr) const
{
   return mFirstIncludes.find(aSourcePtr) != mFirstIncludes.end();
}

bool wizard::ParseResultsTiny::FindBestRelativePath(UtPath aFullPath, std::string& aRelativeIncludePath)
{
   bool ok = false;
   for (size_t i = 0; i < mIncludePaths.size(); ++i)
   {
      UtPath pathDir = mIncludePaths[i].Get();
      pathDir        = mProjectPtr->WorkingDirectory() + pathDir;
      UtPath relPath = pathDir.GetRelativePath(aFullPath);
      if (!relPath.GetNormalizedPath().empty())
      {
         if (!ok || relPath.GetNormalizedPath().length() < aRelativeIncludePath.length())
         {
            aRelativeIncludePath = relPath.GetNormalizedPath();
            ok                   = true;
         }
      }
   }
   return ok;
}

void wizard::ParseResultsTiny::FreeScriptTypes()
{
   if (mScriptTypesPtr)
   {
      if (mProjectPtr && mProjectPtr->GetExe())
      {
         mProjectPtr->GetExe()->GiveScriptTypes(mScriptTypesPtr);
      }
      else
      {
         delete mScriptTypesPtr;
      }
      mScriptTypesPtr = nullptr;
   }
}

void wizard::ParseResultsProxyData::GiveProxy(std::unique_ptr<WsfPProxy> aProxy, std::unique_ptr<ProxyHash> aProxyHashPtr)
{
   // Take ownership of the proxy
   if (mProxy != aProxy)
   {
      mProxy = std::move(aProxy);
   }

   // Take ownership of the proxy hash
   if (mProxyHashPtr != aProxyHashPtr)
   {
      mProxyHashPtr = std::move(aProxyHashPtr);
   }

   // Sync the root values between proxy and hash
   if (mProxy && mProxyHashPtr)
   {
      mProxy->mRoot = mProxyHashPtr->RootValue();
   }
}

// Swap the proxy data between this and aOtherParseResults
// if aSwapProxyPointer is true, the underlying WsfPProxy* is swapped
//  otherwise, the contents of the proxy is swapped
//  This distiction is necessary because WsfPProxyNode keeps a pointer to
//  the proxy, and careful management of this allows references to the proxy to remain vaild
void wizard::ParseResultsProxyData::SwapProxy(ParseResultsProxyData& aOtherParseResults, bool aSwapProxyPointer)
{
   std::swap(mProxyHashPtr, aOtherParseResults.mProxyHashPtr);

   if (aSwapProxyPointer)
   {
      std::swap(mProxy, aOtherParseResults.mProxy);
   }
   else
   {
      if (!mProxy)
      {
         mProxy = ut::make_unique<WsfPProxy>();
      }
      if (!aOtherParseResults.mProxy)
      {
         aOtherParseResults.mProxy = ut::make_unique<WsfPProxy>();
      }
      mProxy->Swap(*aOtherParseResults.mProxy);
   }
}

void wizard::ParseResultsProxyData::FindProxySourcePaths(const WsfPProxyPath& aPath, std::vector<WsfPProxyPath>& aSources)
{
   if (!mProxy)
   {
      return;
   }

   // add the current path to the source list
   aSources.emplace_back(aPath);
   WsfPProxyPath parentPath = aPath;
   // traverse the hierarchy for more sources
   while (!parentPath.Empty())
   {
      WsfPProxyStructValue p(parentPath.Lookup(mProxy->mRoot));
      while (p.IsValid() && p.GetBase())
      {
         WsfPProxyValue ancestor = mProxy->mRoot.Lookup(*p.GetBase());
         if (ancestor.IsValid())
         {
            // get the ancestor path
            WsfPProxyPath ancestorPath = *p.GetBase();
            if (parentPath.size() != aPath.size()) // don't return the path to the type
            {
               // append the current path to the ancestor path
               for (size_t i = parentPath.size(); i < aPath.size(); ++i)
               {
                  ancestorPath += aPath[i];
               }
               if (mProxy->mRoot.Lookup(ancestorPath).IsValid())
               {
                  // add the path to the source list
                  aSources.push_back(ancestorPath);
                  p = ancestor;
               }
               else
               {
                  break;
               }
            }
            else
            {
               break;
            }
         }
         else
         {
            break;
         }
      }
      parentPath.Pop();
   }
}

UtTextDocumentRange wizard::ParseResultsProxyData::FindDefinitionLocation(const WsfPProxyPath& aPath)
{
   std::vector<UtTextDocumentRange> locations;
   FindModificationLocations(aPath, locations);
   if (locations.empty())
   {
      return UtTextDocumentRange();
   }
   return locations[0];
}

void wizard::ParseResultsProxyData::FindDefinitionStartLocations(const WsfPProxyPath&              aPath,
                                                                 std::vector<UtTextDocumentRange>& aLocations)
{
   if (mProxy)
   {
      const auto& rootIndex = mProxy->mIndex;
      if (rootIndex)
      {
         auto pathIndex = rootIndex->Find(aPath);
         if (pathIndex)
         {
            ProxyUtil::FindBlockStartLocations(pathIndex->mEntries, aLocations);
         }
      }
   }
}

void wizard::ParseResultsProxyData::FindModificationLocations(const WsfPProxyPath&              aPath,
                                                              std::vector<UtTextDocumentRange>& aLocations)
{
   if (mProxy)
   {
      const auto& rootIndex = mProxy->mIndex;
      if (rootIndex)
      {
         auto pathIndex = rootIndex->Find(aPath);
         if (pathIndex)
         {
            ProxyUtil::FindBlockLocations(pathIndex->mEntries, aLocations);
         }
      }
   }
}

void wizard::ParseResultsProxyData::FindModificationLocationsExtended(const WsfPProxyPath&              aPath,
                                                                      std::vector<UtTextDocumentRange>& aLocations)
{
   std::vector<WsfPProxyPath> sources;
   FindProxySourcePaths(aPath, sources);
   for (const auto& source : sources)
   {
      FindModificationLocations(source, aLocations);
   }
}

void wizard::ParseResultsProxyData::FindDefinitionStartLocationsExtended(const WsfPProxyPath&              aPath,
                                                                         std::vector<UtTextDocumentRange>& aLocations)
{
   std::vector<WsfPProxyPath> sources;
   FindProxySourcePaths(aPath, sources);
   for (const auto& source : sources)
   {
      FindDefinitionStartLocations(source, aLocations);
   }
}

void wizard::ParseResultsProxyData::FindDerivedTypes(const WsfPProxyPath&        aTypePath,
                                                     std::vector<WsfPProxyPath>& aDerivedTypePaths)
{
   if (!mProxy->mRoot.IsValid())
   {
      return;
   }
   if (aTypePath.Empty())
   {
      return;
   }
   WsfPProxyPath typeListPath = aTypePath;
   typeListPath.Pop();
   std::vector<WsfPProxyPath> maps;
   maps.push_back(typeListPath);

   std::vector<std::string> pathStr = typeListPath.ToStringList(mProxy->mRoot);

   // Search platforms derived from the platform type too:
   if (!pathStr.empty() && pathStr.back() == "platformType")
   {
      pathStr.back() = "platform";
      maps.push_back(WsfPProxyPath::FromStringList(mProxy->mRoot, pathStr));
   }

   for (auto&& mapPath : maps)
   {
      WsfPProxyObjectMap* mapPtr = mProxy->mRoot.Lookup(mapPath).GetObjectMap();
      if (!mapPtr)
      {
         continue;
      }
      auto& values = mapPtr->GetValues();
      for (auto&& val : values)
      {
         WsfPProxyPath* basePathPtr = WsfPProxyStructValue(val.second).GetBase();
         if (basePathPtr != nullptr && *basePathPtr == aTypePath)
         {
            aDerivedTypePaths.push_back(mapPath + val.first);
         }
      }
   }
}
