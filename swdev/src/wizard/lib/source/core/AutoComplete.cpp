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

#include "AutoComplete.hpp"

#include "ParseResults.hpp"
#include "Project.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "UtScanDir.hpp"
#include "WsfParseAlternate.hpp"
#include "WsfParseBasicRules.hpp"
#include "WsfParseIndex.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRecurrence.hpp"
#include "WsfParseRule.hpp"
#include "WsfParseSequence.hpp"
#include "WsfParseTypeLoadRule.hpp"
#include "WsfParser.hpp"

namespace
{
// Suggestions are sorted by case-insensitive order,
// preferring results that begin with '<', as that usually indicates the type of entry that is expected.
bool SpecialCharCompare(char a, char b)
{
   if (a == '<')
   {
      return b != '<';
   }
   else
   {
      if (b == '<')
      {
         return false;
      }
      return tolower(a) < tolower(b);
   }
}
bool SpecialSort(const std::string& aLhs, const std::string& aRhs)
{
   return std::lexicographical_compare(aLhs.begin(), aLhs.end(), aRhs.begin(), aRhs.end(), SpecialCharCompare);
}

} // namespace

wizard::AutoComplete::AutoComplete()
   : mExpanded(false)
   , mParseResultsPtr(nullptr)
   , mCurrentTypeRef(nullptr)
   , mExpandTypeCommand(true)
   , mCurrentQuery()
{
}
void wizard::AutoComplete::GetSyntax(ParseResults&      aParseData,
                                     TextSource&        aSource,
                                     WsfParseNode*      aBlockNodePtr,
                                     const std::string& aText,
                                     size_t             aInsertPosition,
                                     SyntaxResults&     aResults)
{
   mCurrentTypeRef          = nullptr;
   mParseResultsPtr         = &aParseData;
   WsfParseRule* rootReader = aParseData.parser()->GetDefinitions()->GetRoot();
   if (aBlockNodePtr && aBlockNodePtr->mRulePtr)
   {
      rootReader      = aBlockNodePtr->mRulePtr;
      mCurrentTypeRef = FindCurrentType(aBlockNodePtr);
   }

   mCurrentQuery            = Query();
   mCurrentQuery.mOperation = cSYNTAX;
   size_t replaceCharacters, insPos;
   DigestInput(aText, aInsertPosition, replaceCharacters, insPos);

   aResults.mWordIndex         = mCurrentQuery.mQueryWordIndex;
   aResults.mConcreteWordCount = 0;
   // Require that at least one full word is in the query
   if (mCurrentQuery.mQueryWordIndex < 1)
   {
      if (mCurrentQuery.mQueryWords.empty())
      {
         return;
      }
      if (mCurrentQuery.mQueryWords.size() == 1)
      {
         mCurrentQuery.mQueryWords.emplace_back("");
      }
      mCurrentQuery.mQueryWordIndex = 1;
   }
   Entry* entryPtr      = new Entry;
   entryPtr->mEntryType = Entry::cALTERNATE;

   // Normally, we only have to expand a single block's reader.
   // However, if the block has no terminator, the command could be valid for the next block.
   // Walk up the hierarchy until we run out of blocks, or we reach a block that has a terminator
   WsfParseNode* blockNodePtr = aBlockNodePtr;
   do
   {
      Entry* nodeEntry = new Entry;
      ExpandRule(0, rootReader, mCurrentTypeRef, *nodeEntry, true);

      WsfParseNode* nextBlockPtr = nullptr;
      if (rootReader->Type() == WsfParseRule::cRECURRENCE)
      {
         const std::string& endKeyword = ((WsfParseRecurrence*)rootReader)->mEndKeyword;
         if (endKeyword.empty())
         {
            nextBlockPtr = FindNextBlock(blockNodePtr);
            rootReader   = nextBlockPtr->mRulePtr;
         }
         else
         {
            nodeEntry->mChildren.push_back(new Entry(endKeyword, false));
         }
      }

      Prune(0, nodeEntry);
      if (nodeEntry != nullptr)
      {
         entryPtr->mChildren.push_back(nodeEntry);
      }

      blockNodePtr = nextBlockPtr;
   } while (blockNodePtr && rootReader);

   int maxExpandedWordIndex = 20;
   // WIZARD_TODO: If a grammar rule is recursive, this code could take forever!
   //      Need to limit computation by some other factors than depth.
   //      At the time, the only recursive rule is used has a '*' which
   //      avoids this behavior.
   do
   {
      // repeatedly expand and prune until no more can be done
      mExpanded = false;
      ExpandTree(*entryPtr, mCurrentTypeRef, 0, maxExpandedWordIndex);
      Prune(0, entryPtr);
      if (!entryPtr)
      {
         break;
      }
   } while (mExpanded);

   if (entryPtr)
   {
      aResults.mSequences = CollectSyntax(entryPtr);
      std::sort(aResults.mSequences.begin(), aResults.mSequences.end());
      aResults.mSequences.erase(std::unique(aResults.mSequences.begin(), aResults.mSequences.end()),
                                aResults.mSequences.end());
      size_t largestExtent = 0;
      for (auto&& seq : aResults.mSequences)
      {
         bool stop = false;
         for (size_t j = 0; j < seq.size(); ++j)
         {
            if (seq[j] == "...")
            {
               if (j - 1 > largestExtent)
               {
                  largestExtent = j - 1;
                  stop          = true;
               }
            }
         }
         if (!stop && largestExtent < seq.size())
         {
            largestExtent = seq.size();
         }
      }
      aResults.mConcreteWordCount = largestExtent;
      delete entryPtr;
      entryPtr = nullptr;
   }
}

void wizard::AutoComplete::GetSuggestions(ParseResults&      aParseData,
                                          TextSource&        aSource,
                                          WsfParseNode*      aBlockNodePtr,
                                          const std::string& aText,
                                          size_t             aInsertPosition,
                                          Results&           aResults)
{
   aResults.mPartialToken = false;
   aResults.mLastToken    = true;

   mCurrentTypeRef          = nullptr;
   mParseResultsPtr         = &aParseData;
   WsfParseRule* rootReader = aParseData.parser()->GetDefinitions()->GetRoot();
   if (aBlockNodePtr && aBlockNodePtr->mRulePtr)
   {
      rootReader      = aBlockNodePtr->mRulePtr;
      mCurrentTypeRef = FindCurrentType(aBlockNodePtr);
   }

   mCurrentQuery            = Query();
   mCurrentQuery.mOperation = cAUTOCOMPLETE;
   DigestInput(aText, aInsertPosition, aResults.mReplaceCharacters, aResults.mInsertPosition);
   Entry* entryPtr      = new Entry;
   entryPtr->mEntryType = Entry::cALTERNATE;
   if (!mCurrentQuery.mQueryWords.empty() &&
       (mCurrentQuery.mQueryWords[0] == "include" || mCurrentQuery.mQueryWords[0] == "include_once"))
   {
      std::string text;
      if (mCurrentQuery.mQueryWords.size() > 1)
      {
         text = mCurrentQuery.mQueryWords[1];
      }
      GetIncludeSuggestions(text, aSource, aParseData, aResults);
      return;
   }

   // Normally, we only have to expand a single block's reader.
   // However, if the block has no terminator, the command could be valid for the next block.
   // Walk up the hierarchy until we run out of blocks, or we reach a block that has a terminator
   WsfParseNode* blockNodePtr = aBlockNodePtr;
   do
   {
      Entry* nodeEntry = new Entry;
      ExpandRule(0, rootReader, mCurrentTypeRef, *nodeEntry, true);

      WsfParseNode* nextBlockPtr = nullptr;
      if (rootReader && rootReader->Type() == WsfParseRule::cRECURRENCE)
      {
         const std::string& endKeyword = ((WsfParseRecurrence*)rootReader)->mEndKeyword;
         if (endKeyword.empty())
         {
            nextBlockPtr = FindNextBlock(blockNodePtr);
            rootReader   = nextBlockPtr->mRulePtr;
         }
         else
         {
            nodeEntry->mChildren.push_back(new Entry(endKeyword, false));
         }
      }

      Prune(0, nodeEntry);
      if (nodeEntry != nullptr)
      {
         entryPtr->mChildren.push_back(nodeEntry);
      }

      blockNodePtr = nextBlockPtr;
   } while (blockNodePtr);

   int maxExpandedWordIndex = (int)mCurrentQuery.mQueryWords.size();

   do
   {
      // repeatedly expand and prune until no more can be done
      mExpanded = false;
      ExpandTree(*entryPtr, mCurrentTypeRef, 0, maxExpandedWordIndex);
      Prune(0, entryPtr);
      if (!entryPtr)
      {
         break;
      }
   } while (mExpanded);

   if (entryPtr)
   {
      CollectSuggestions(entryPtr, mCurrentQuery.mQueryWordIndex, aResults.mSuggestionList, aResults.mLastToken);

      // Sometimes a command is in the grammar more than once.  Sort and remove duplicate entries.
      std::sort(aResults.mSuggestionList.begin(), aResults.mSuggestionList.end(), SpecialSort);
      aResults.mSuggestionList.erase(std::unique(aResults.mSuggestionList.begin(), aResults.mSuggestionList.end()),
                                     aResults.mSuggestionList.end());
      delete entryPtr;
      entryPtr = nullptr;
   }
}

wizard::AutoComplete::SyntaxList wizard::AutoComplete::JoinSyntax(const SyntaxList& aLhs, const SyntaxList& aRhs)
{
   static const size_t cMAX_SYNTAX_LIST_SIZE = 50;
   if (aLhs.empty())
   {
      return aRhs;
   }
   if (aRhs.empty())
   {
      return aLhs;
   }
   if (aRhs.size() * aLhs.size() > cMAX_SYNTAX_LIST_SIZE)
   {
      // If the maximum syntax list size is going to be exceeded,
      // replace the RHS words with <...>.
      SyntaxList    rval;
      std::set<int> rhsWordCounts;
      Syntax        rhsWords = aRhs[0];
      rhsWordCounts.insert((int)aRhs[0].size());
      for (size_t j = 1; j < aRhs.size(); ++j)
      {
         Syntax rhsSyn = aRhs[j];
         for (size_t i = 0; i < rhsWords.size(); ++i)
         {
            if (i >= rhsSyn.size() || rhsSyn[i] != rhsWords[i])
            {
               rhsWords[i] = "<...>"; // more than one option
            }
         }
         rhsWordCounts.insert((int)aRhs[j].size());
      }
      Syntax rhs;
      for (int wc = 1; wc <= (int)rhsWords.size(); ++wc)
      {
         rhs.push_back(rhsWords[wc - 1]);
         if (rhsWordCounts.find(wc) != rhsWordCounts.end())
         {
            for (Syntax s : aLhs)
            {
               s.insert(s.end(), rhs.begin(), rhs.end());
               rval.push_back(s);
            }
         }
      }
      return rval;
   }
   else
   {
      SyntaxList rval;
      for (const auto& aRh : aRhs)
      {
         for (Syntax s : aLhs)
         {
            s.insert(s.end(), aRh.begin(), aRh.end());
            rval.push_back(s);
         }
      }
      return rval;
   }
}

wizard::AutoComplete::SyntaxList wizard::AutoComplete::CollectSyntax(Entry* aEntryTreePtr, bool aFirstWord)
{
   SyntaxList slist;
   if (aEntryTreePtr->mEntryType == Entry::cALTERNATE)
   {
      SyntaxList prev;
      for (auto&& child : aEntryTreePtr->mChildren)
      {
         SyntaxList syn = CollectSyntax(child, aFirstWord);
         if (syn != prev)
         {
            slist.insert(slist.end(), syn.begin(), syn.end());
            std::swap(syn, prev);
         }
      }
   }
   else if (aEntryTreePtr->mEntryType == Entry::cSEQUENCE)
   {
      if (!aFirstWord && aEntryTreePtr->mReaderPtr &&
          (aEntryTreePtr->mReaderPtr->GetReaderFlags() & WsfParseRule::cIS_SINGLE_TOKEN) &&
          !aEntryTreePtr->mRuleName.empty())
      {
         slist.emplace_back(1, "<" + aEntryTreePtr->mRuleName + ">");
      }
      else
      {
         if (!aEntryTreePtr->mChildren.empty())
         {
            slist = CollectSyntax(aEntryTreePtr->mChildren[0], aFirstWord);
         }
         for (size_t i = 1; i < aEntryTreePtr->mChildren.size(); ++i)
         {
            SyntaxList next = CollectSyntax(aEntryTreePtr->mChildren[i], false);
            slist           = JoinSyntax(slist, next);
         }
      }
   }
   else if (aEntryTreePtr->mEntryType == Entry::cWORD)
   {
      slist.emplace_back(1, aEntryTreePtr->mWordText);
   }
   return slist;
}

// Handles suggestions for include files
void wizard::AutoComplete::GetIncludeSuggestions(const std::string& aText,
                                                 TextSource&        aSource,
                                                 ParseResults&      aParseData,
                                                 Results&           aResults)
{
   if (!aParseData.mProjectPtr)
   {
      return;
   }
   UtPath      currentInputDirectory = aText;
   std::string inputFileName         = currentInputDirectory.GetFileName();
   if (aText.empty() || (aText[aText.size() - 1] == '/' || aText[aText.size() - 1] == '\\'))
   {
      inputFileName.clear();
   }
   else
   {
      currentInputDirectory.Up();
   }
   std::string inputFileFilter = inputFileName + "*";
   UtPath      workingDir      = aParseData.mProjectPtr->WorkingDirectory();
   for (auto&& includePath : mParseResultsPtr->mIncludePaths)
   {
      UtPath includeDir(includePath);
      UtPath fullIncludeDir = workingDir + includeDir;
      UtPath inputDir       = fullIncludeDir + currentInputDirectory;

      if (inputDir.Stat() == UtPath::cDIRECTORY)
      {
         UtScanDir::DirEntries entries;
         UtScanDir::ScanDir(inputDir.GetSystemPath(), entries, inputFileFilter);
         for (auto&& e : entries)
         {
            std::string filePath = (currentInputDirectory + e.mFilename).GetNormalizedPath();
            if (e.mDirectory)
            {
               if (e.mFilename == "." || e.mFilename == ".." || e.mFilename == "CVS")
               {
                  continue;
               }
               filePath += '/';
               aResults.mPartialToken = true;
            }
            aResults.mSuggestionList.push_back(filePath);
         }
      }
   }

   // include paths can lead to the same file being listed multiple times
   std::sort(aResults.mSuggestionList.begin(), aResults.mSuggestionList.end(), SpecialSort);
   aResults.mSuggestionList.erase(std::unique(aResults.mSuggestionList.begin(), aResults.mSuggestionList.end()),
                                  aResults.mSuggestionList.end());
}

bool wizard::AutoComplete::TextMatches(const std::string& aText, size_t aWordIndex)
{
   if (aWordIndex < mCurrentQuery.mQueryWords.size())
   {
      if (aWordIndex == mCurrentQuery.mQueryWordIndex)
      {
         // match beginning of string ONLY
         return TextMatches(aText, mCurrentQuery.mQueryWords[aWordIndex]);
      }
      return aText == mCurrentQuery.mQueryWords[aWordIndex];
   }
   return true;
}

bool wizard::AutoComplete::TextMatches(const std::string& aText, const std::string& aQuery)
{
   bool   match   = true;
   size_t strSize = aQuery.size();
   if (aText.size() < aQuery.size())
   {
      strSize = aText.size();
   }
   for (size_t i = 0; i < strSize && match; ++i)
   {
      match = (aText[i] == aQuery[i]);
   }
   return match;
}

const WsfParseType* wizard::AutoComplete::FindCurrentType(WsfParseNode* aNodePtr)
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
   const WsfParseType* typeRef         = nullptr;
   for (int i = (int)typeStack.size() - 1; i >= 0; --i)
   {
      const WsfParseType* typePtr = nullptr;
      if (currentTablePtr != nullptr)
      {
         typePtr = currentTablePtr->FindType(typeStack[i]);
      }
      else
      {
         typePtr = mParseResultsPtr->GetParseIndex()->mUserTypes.FindType(typeStack[i]);
         if (!typePtr)
         {
            typePtr = mParseResultsPtr->parser()->GetDefinitions()->mBasicTypes->FindType(typeStack[i]);
         }
      }

      if (typePtr)
      {
         typeRef         = typePtr;
         currentTablePtr = typePtr;
      }
      else
      {
         break;
      }
   }
   return typeRef;
}

void wizard::AutoComplete::DigestInput(const std::string& aText,
                                       size_t             aInputPosition,
                                       size_t&            aReplaceCharacters,
                                       size_t&            aInsertPosition)
{
   bool        gotWord        = false;
   bool        foundQueryWord = false;
   std::string wordText;
   size_t      replaceStart = aInputPosition;
   size_t      replaceEnd   = aText.size() - 1;
   size_t      wordStart    = 0;
   for (size_t i = 0; i <= aText.size(); ++i)
   {
      char c = ' ';
      if (i < aText.size())
      {
         c = aText[i];
      }
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
      {
         if (!wordText.empty())
         {
            if (!foundQueryWord && aInputPosition <= i)
            {
               mCurrentQuery.mQueryWordIndex = mCurrentQuery.mQueryWords.size();
               replaceStart                  = wordStart;
               replaceEnd                    = i;
               foundQueryWord                = true;
            }
            mCurrentQuery.mQueryWords.push_back(wordText);
            wordText.clear();
            if (gotWord)
            {
               gotWord    = false;
               replaceEnd = i - 1;
            }
         }
      }
      else
      {
         if (wordText.empty())
         {
            wordStart = i;
            if (!foundQueryWord && aInputPosition < i)
            {
               foundQueryWord                = true;
               mCurrentQuery.mQueryWordIndex = mCurrentQuery.mQueryWords.size();
               mCurrentQuery.mQueryWords.emplace_back(""); // appears the user is trying to enter a new word
               replaceStart = aInputPosition;
               replaceEnd   = aInputPosition;
               break;
            }
         }
         if (i < aInputPosition) // don't add characters after input to query
         {
            wordText += c;
         }
      }
   }
   if (!foundQueryWord)
   {
      mCurrentQuery.mQueryWordIndex = mCurrentQuery.mQueryWords.size();
      mCurrentQuery.mQueryWords.emplace_back(""); // appears the user is trying to enter a new word
      replaceStart = aInputPosition;
      replaceEnd   = aInputPosition;
   }
   while (mCurrentQuery.mQueryWordIndex < mCurrentQuery.mQueryWords.size() - 1)
   {
      mCurrentQuery.mQueryWords.pop_back();
   }
   if (mCurrentQuery.mOperation == cAUTOCOMPLETE)
   {
      // Quick HACK:
      //      Don't worry about filtering based on the partially-typed word
      if (!mCurrentQuery.mQueryWords.empty())
      {
         if (!(mCurrentQuery.mQueryWords[0] == "include" || mCurrentQuery.mQueryWords[0] == "include_once"))
         {
            if (!mCurrentQuery.mQueryWords.back().empty())
            {
               mCurrentQuery.mQueryWords.back().clear();
            }
         }
      }
   }
   aReplaceCharacters = replaceEnd - replaceStart;
   aInsertPosition    = replaceStart;
}

WsfParseNode* wizard::AutoComplete::FindBlock(WsfParseNode* aNodePtr, int aInsertPosition)
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
         UtTextRange blockRange = childBlockPtr->GetBoundingRange(aNodePtr->mValue.mSource);
         UtTextRange pos(aInsertPosition, aInsertPosition + 1);
         if (blockRange.Intersects(pos))
         {
            nodePtr = childBlockPtr;
            found   = true;
            break;
         }
      }
   }
   if (found)
   {
      return nodePtr;
   }
   return nullptr;
}

namespace
{
std::string GetBestName(WsfParseRule* aRulePtr)
{
   WsfParseNamedRule* ctxPtr = aRulePtr->GetBestContext();
   if (ctxPtr)
   {
      return ctxPtr->mName;
   }
   return "rule";
}
} // namespace

// 'Expands' a reader into an entry which defines the acceptable inputs for the reader
bool wizard::AutoComplete::ExpandRule(int                 aQueryWordIndex,
                                      WsfParseRule*       aReaderPtr,
                                      const WsfParseType* aCurrentTypePtr,
                                      Entry&              aEntry,
                                      bool                aExpandBlock)
{
   if (aReaderPtr == nullptr)
   {
      return false;
   }
   bool expanded = false;
   switch (aReaderPtr->Type())
   {
   case WsfParseRule::cREAL:
      aEntry.Set("<real>");
      aEntry.mWordIsPattern = true;
      break;
   case WsfParseRule::cINT:
      aEntry.Set("<integer>");
      aEntry.mWordIsPattern = true;
      break;
   case WsfParseRule::cSTRING:
   {
      aEntry.Set("<string>");
      WsfParseStringRule* readerPtr = (WsfParseStringRule*)aReaderPtr;
      if (aEntry.mParentCommandPtr != nullptr /* && aEntry.mParentCommandPtr->mHasVariables*/)
      {
         if (0 == (aEntry.mFlags & Entry::cTYPE_REF_WORD))
         {
            for (size_t i = aEntry.mCommandWordIndex + 1; i < aEntry.mParentCommandPtr->Sequence().size(); ++i)
            {
               WsfParseRule* rulePtr = aEntry.mParentCommandPtr->Sequence()[i];
               if (rulePtr->Type() == WsfParseRule::cTYPE_LOAD)
               {
                  WsfParseTypeLoadRule* typeLoadPtr = (WsfParseTypeLoadRule*)rulePtr;
                  if (typeLoadPtr->mLoadType.mOrdinal == aEntry.mCommandWordIndex)
                  {
                     aEntry.SetTypeRef(typeLoadPtr->mLoadType.mPath, typeLoadPtr->mLoadType.mNestedLookup);
                  }
                  break;
               }
            }
         }
      }
      if (readerPtr->mFlags & WsfParseNode::cLAZY_TYPE_REFERENCE_NODE)
      {
         WsfParseTypePath kind = aEntry.mRefType;
         if (kind.empty())
         {
            kind.emplace_back(((WsfParseStringRule*)aReaderPtr)->mNodeType);
         }
         if (!kind.empty() && 0 == (aEntry.mFlags & Entry::cTYPE_REF_WORD))
         {
            const std::string& typeStr = kind[0].Get();
            aEntry.SetTypeRef(kind, !typeStr.empty() && typeStr[0] == '.');
            expanded = true;
         }
      }
      if (readerPtr->mFlags & WsfParseNode::cNAMED_NODE)
      {
         std::string nameType = ((WsfParseStringRule*)aReaderPtr)->mNodeType;
         if (!nameType.empty() && 0 == (aEntry.mFlags & Entry::cNAMED_WORD))
         {
            aEntry.SetNameRef(nameType);
            expanded = true;
         }
      }
      if (readerPtr->mFlags & WsfParseNode::cLAZY_TYPENAME_NODE)
      {
         std::string kind = readerPtr->mNodeType;
         aEntry.mWordText = "<" + kind + "-name>";
      }
      else if (!readerPtr->mNodeType.empty())
      {
         aEntry.mWordText = "<" + readerPtr->mNodeType + ">";
      }
      aEntry.mWordIsPattern = true;
   }
   break;
   case WsfParseRule::cLINE_STRING:
      aEntry.Set("<line-string>");
      aEntry.mWordIsPattern = true;
      break;
   case WsfParseRule::cQUOTED_STRING:
      if (((WsfParseQuotedStringRule*)aReaderPtr)->mFileReference)
      {
         UtStringRef ft = ((WsfParseQuotedStringRule*)aReaderPtr)->mFileType;
         if (!ft.empty())
         {
            std::string fileType = "<";
            fileType += ft.Get();
            fileType += "-file>";
            aEntry.Set(fileType);
         }
         else
         {
            aEntry.Set("<file-path>");
         }
      }
      else
      {
         aEntry.Set("<string>");
      }
      aEntry.mWordIsPattern = true;
      break;
   case WsfParseRule::cLITERAL:
   {
      WsfParseLiteral* readerPtr = (WsfParseLiteral*)aReaderPtr;
      aEntry.Set(readerPtr->mText);
   }
   break;
   case WsfParseRule::cDELIMITED:
   {
      aEntry.Set("<" + GetBestName(aReaderPtr) + ">");
      aEntry.mWordIsPattern = true;
   }
   break;
   case WsfParseRule::cSEQUENCE:
   {
      WsfParseSequence* readerPtr = static_cast<WsfParseSequence*>(aReaderPtr);
      if (!(readerPtr->mNodeFlags & WsfParseNode::cERROR_INCOMPLETE)) // ignore error rules
      {
         //             if (readerPtr->mValueReaders.size() == 1)
         //             {
         //                ExpandRule(aQueryWordIndex, readerPtr->mValueReaders.front().mRulePtr, aCurrentTypePtr, aEntry);
         //             }
         //             else
         //             {
         aEntry.mEntryType = Entry::cSEQUENCE;

         aEntry.mRuleName  = GetBestName(readerPtr);
         aEntry.mReaderPtr = aReaderPtr;
         if (readerPtr->GetReaderFlags() & WsfParseRule::cIS_SINGLE_TOKEN)
         {
            aEntry.mWordCount = 1;
         }
         const WsfParseType* currentTypePtr = aCurrentTypePtr;

         for (size_t i = 0; i < readerPtr->Sequence().size(); ++i)
         {
            WsfParseRule* childRule     = readerPtr->Sequence()[i];
            Entry*        childPtr      = new Entry;
            childPtr->mParentCommandPtr = readerPtr;
            childPtr->mCommandWordIndex = i;
            if (i == 0)
            {
               expanded = ExpandRule(aQueryWordIndex, childRule, currentTypePtr, *childPtr) || expanded;
               if (childPtr->mEntryType == Entry::cTYPE_CHANGE)
               {
                  currentTypePtr = childPtr->mCurrentTypeChangePtr;
               }
            }
            else
            {
               childPtr->mEntryType = Entry::cUNEXPANDED;
               childPtr->mReaderPtr = childRule;
               expanded             = true;
            }
            aEntry.mChildren.push_back(childPtr);
         }
      }
   }
   break;
   case WsfParseRule::cRULE_REFERENCE:
   {
      WsfParseRuleReference* ref = (WsfParseRuleReference*)aReaderPtr;
      return ExpandRule(aQueryWordIndex, ref->GetRulePtr(), aCurrentTypePtr, aEntry, aExpandBlock);
   }
   case WsfParseRule::cRECURRENCE:
   {
      if (aExpandBlock)
      {
         WsfParseRecurrence* rec = (WsfParseRecurrence*)aReaderPtr;
         return ExpandRule(aQueryWordIndex, rec->GetSubordinateRule(), aCurrentTypePtr, aEntry, aExpandBlock);
      }
      else
      {
         aEntry.mEntryType = Entry::cSEQUENCE;
         aEntry.mChildren.push_back(new Entry("...", false));
         aEntry.mChildren.back()->mFlags |= Entry::cBLOCK_PLACEHOLDER;
         break;
      }
   }
   case WsfParseRule::cALTERNATE:
   {
      WsfParseAlternate* readerPtr = (WsfParseAlternate*)aReaderPtr;
      aEntry.mEntryType            = Entry::cALTERNATE;
      aEntry.mReaderPtr            = aReaderPtr;
      if (readerPtr->GetReaderFlags() & WsfParseRule::cIS_SINGLE_TOKEN)
      {
         aEntry.mWordCount = 1;
      }
      for (auto&& reader : readerPtr->GetReaders())
      {
         Entry* childPtr = new Entry;
         ExpandRule(aQueryWordIndex, reader, aCurrentTypePtr, *childPtr);
         aEntry.mChildren.push_back(childPtr);
      }
   }
   break;
   case WsfParseRule::cTYPE_LOAD:
   {
      WsfParseTypeLoadRule* readerPtr = (WsfParseTypeLoadRule*)aReaderPtr;
      if (readerPtr->GetOperation() == WsfParseTypeInfoData::cLOAD_TYPE)
      {
         const WsfParseTypeName& loadRef = readerPtr->GetLoadType();
         if (loadRef.mNestedLookup && aCurrentTypePtr != nullptr)
         {
            // Allow the loading of types when specified explicitly; i.e. (load (subtype template))
            if (loadRef.mOrdinal == ut::npos)
            {
               const WsfParseType* typeRef = aCurrentTypePtr->FindNestedSymbol(loadRef.mPath);
               aEntry.mEntryType           = Entry::cTYPE_CHANGE;
               if (typeRef)
               {
                  aEntry.mCurrentTypeChangePtr = typeRef;
               }
               aEntry.mComplete  = true;
               aEntry.mWordCount = 0;
            }
         }
      }
   }
   break;
   case WsfParseRule::cTYPE_COMMAND:
      if (aCurrentTypePtr != nullptr && aCurrentTypePtr->GetRule())
      {
         if (mExpandTypeCommand)
         {
            bool ok  = ExpandRule(aQueryWordIndex, aCurrentTypePtr->GetRule(), aCurrentTypePtr, aEntry, false);
            expanded = ok || expanded;
         }
      }
      break;
   case WsfParseRule::cNAMED_RULE:
   case WsfParseRule::cSTRUCT:
   case WsfParseRule::cVALUE:
   {
      WsfParseNamedRule* namedPtr = (WsfParseNamedRule*)aReaderPtr;
      return ExpandRule(aQueryWordIndex, namedPtr->GetDefinition(), aCurrentTypePtr, aEntry, aExpandBlock);
   }
   break;
   default:
      break;
   }
   return expanded || (aEntry.mEntryType != Entry::cUNEXPANDED);
}

// Walks tree of entries, expanding any entries if possible.
// ExpandTree does a limited amount of work, usually only expanding one level.
// This allows Prune() to reduce the number of entries that are expanded.
// mExpanded is set to true if an entry was expanded.
int wizard::AutoComplete::ExpandTree(Entry& aEntry, const WsfParseType* aCurrentTypePtr, int aWordIndex, int aMaxWordIndex)
{
   if (aEntry.mComplete)
   {
      if (aEntry.mWordCount < 0)
      {
         return aEntry.mWordCount;
      }
      return aEntry.mWordCount + aWordIndex;
   }

   switch (aEntry.mEntryType)
   {
   case Entry::cWORD:
      return aWordIndex + 1;
      break;
   case Entry::cUNEXPANDED:
   {
      Entry newEntry(aEntry);
      if (!aEntry.mRefType.empty())
      {
         if (FullyExpandWord(aWordIndex))
         {
            ExpandRefType(newEntry, aCurrentTypePtr, aWordIndex);
         }
         else
         {
            newEntry            = aEntry;
            newEntry.mEntryType = Entry::cWORD;
         }
      }
      else
      {
         mExpanded = ExpandRule(aWordIndex, aEntry.mReaderPtr, aCurrentTypePtr, newEntry) || mExpanded;
      }
      std::swap(aEntry, newEntry);
      return aWordIndex + aEntry.GetWordCount();
   }
   break;
   case Entry::cSEQUENCE:
   {
      int                 wordIndex      = aWordIndex;
      bool                complete       = true;
      const WsfParseType* currentTypePtr = aCurrentTypePtr;
      for (size_t i = 0; i < aEntry.mChildren.size() && (int)wordIndex < aMaxWordIndex; ++i)
      {
         Entry* childPtr = aEntry.mChildren[i];
         if (childPtr->mEntryType == Entry::cTYPE_CHANGE)
         {
            currentTypePtr = childPtr->mCurrentTypeChangePtr;
         }
         wordIndex = ExpandTree(*childPtr, currentTypePtr, wordIndex, aMaxWordIndex);
         complete  = complete && childPtr->mComplete;
      }
      if (complete && (int)aEntry.mChildren.size() <= aMaxWordIndex)
      {
         aEntry.mWordCount = aEntry.GetWordCount();
         aEntry.mComplete  = true;
      }
      return wordIndex;
   }
   case Entry::cALTERNATE:
   {
      bool complete  = true;
      int  wordIndex = aWordIndex;
      for (size_t i = 0; i < aEntry.mChildren.size(); ++i)
      {
         Entry* childPtr = aEntry.mChildren[i];
         int    index    = ExpandTree(*childPtr, aCurrentTypePtr, aWordIndex, aMaxWordIndex);
         if (i == 0)
         {
            wordIndex = index;
         }
         else if (wordIndex != index)
         {
            wordIndex = -1;
         }
         complete = complete && childPtr->mComplete;
      }
      if (complete)
      {
         aEntry.mWordCount = aEntry.GetWordCount();
         aEntry.mComplete  = true;
      }
      return wordIndex;
   }
   default:
      return -1;
   }
}

// Expand an entry previously marked as a type reference.
void wizard::AutoComplete::ExpandRefType(Entry& aEntry, const WsfParseType* aCurrentTypePtr, int aWordIndex)
{
   assert(!aEntry.mRefType.empty());
   Entry newEntry(aEntry);
   mExpanded           = true;
   newEntry.mEntryType = Entry::cALTERNATE;
   // Always allow the user to enter an arbitrary string:
   newEntry.mChildren.push_back(new Entry(aEntry.mWordText, true));
   // If this is the queried word, look up types if possible
   if (aEntry.mFlags & Entry::cTYPE_REF_WORD)
   {
      if (newEntry.mNestedLookup && !newEntry.mRefType.empty())
      {
         if (aCurrentTypePtr)
         {
            std::vector<const WsfParseType*> ancestry;
            aCurrentTypePtr->GetAncestry(ancestry);
            for (auto&& typePtr : ancestry)
            {
               if (typePtr)
               {
                  const WsfParseType::SymbolTable& symbols = typePtr->GetSymbols();

                  // Some kind strings have a '.' in front of them (ie: .comms and .processors)
                  // Strip off the '.' for the following string comparisons
                  std::string kind = newEntry.mRefType[0];
                  if (kind[0] == '.')
                  {
                     kind = kind.substr(1);
                  }

                  std::vector<std::string> kinds;
                  if (kind == "platform_part")
                  {
                     kinds.emplace_back("movers");
                     kinds.emplace_back("comms");
                     kinds.emplace_back("processors");
                     kinds.emplace_back("weapons");
                     kinds.emplace_back("sensors");
                  }
                  else
                  {
                     kinds.push_back(kind);
                  }
                  for (auto&& kind : kinds)
                  {
                     WsfParseType::SymbolTable::const_iterator iter = symbols.lower_bound(kind);
                     while (iter != symbols.end() && iter->first == kind)
                     {
                        const WsfParseType::SymbolTable& symbols = iter->second->GetSymbols();
                        for (auto&& symIt : symbols)
                        {
                           Entry* typeEntry = new Entry;
                           typeEntry->Set(symIt.first);
                           ;
                           newEntry.mChildren.push_back(typeEntry);
                        }
                        ++iter;
                     }
                  }
               }
            }
         }
      }
      else
      {
         std::vector<WsfParseNode*> defs;
         mParseResultsPtr->FindDefinitionsOfType(newEntry.mRefType, defs);
         for (auto&& def : defs)
         {
            Entry* typeEntry = new Entry;
            typeEntry->Set(def->mValue.Text());
            newEntry.mChildren.push_back(typeEntry);
         }
         std::vector<std::string> baseTypes;
         mParseResultsPtr->GetBaseTypes(newEntry.mRefType, baseTypes);
         for (auto&& type : baseTypes)
         {
            Entry* typeEntry = new Entry;
            typeEntry->Set(type);
            newEntry.mChildren.push_back(typeEntry);
         }
      }
   }
   else if (aEntry.mFlags & Entry::cNAMED_WORD)
   {
      std::vector<std::string> nameList;
      if (aEntry.mRefType.size() == 1)
      {
         mParseResultsPtr->FindNames(aEntry.mRefType[0], nameList);
         // plug-ins may also have suggestions here
         wizSignals->RequestNameSuggestions(aEntry.mRefType[0], nameList);
         for (auto&& name : nameList)
         {
            Entry* typeEntry = new Entry;
            typeEntry->Set(name);
            newEntry.mChildren.push_back(typeEntry);
         }
      }
      else
      {
         assert(!"unexpected");
      }
   }
   std::swap(aEntry, newEntry);
}

// Walk the entry tree, removing entries which can not match the query.
void wizard::AutoComplete::Prune(int aQueryWordIndex, Entry*& aEntry)
{
   assert(aEntry != nullptr);
   if (aEntry != nullptr && aQueryWordIndex < (int)mCurrentQuery.mQueryWords.size())
   {
      switch (aEntry->mEntryType)
      {
      case Entry::cNONE:
         delete aEntry;
         aEntry = nullptr;
         break;
      case Entry::cWORD:
         if (!aEntry->mWordIsPattern && !TextMatches(aEntry->mWordText, aQueryWordIndex))
         {
            delete aEntry;
            aEntry = nullptr;
         }
         break;
      case Entry::cUNEXPANDED:
         break;
      case Entry::cALTERNATE:
         for (size_t i = 0; i < aEntry->mChildren.size();)
         {
            Prune(aQueryWordIndex, aEntry->mChildren[i]);
            if (aEntry->mChildren[i] == nullptr)
            {
               aEntry->mChildren.erase(aEntry->mChildren.begin() + i);
            }
            else
            {
               ++i;
            }
         }
         if (aEntry->mChildren.empty())
         {
            delete aEntry;
            aEntry = nullptr;
         }
         break;
      case Entry::cSEQUENCE:
      {
         int  wordIndex = aQueryWordIndex;
         bool pruned    = false;
         for (size_t i = 0; i < aEntry->mChildren.size() && !pruned; ++i)
         {
            Prune(wordIndex, aEntry->mChildren[i]);
            if (aEntry->mChildren[i] == nullptr)
            {
               pruned = true;
            }
            else
            {
               int wc = aEntry->mChildren[i]->GetWordCount();
               if (wc >= 0)
               {
                  wordIndex += wc;
               }
               else
               {
                  break;
               }
            }
         }
         if (pruned)
         {
            delete aEntry;
            aEntry = nullptr;
         }
         break;
      }
      default:
         break;
      }
   }
}

//! Attempt to find a terminator for a command.
//! For example, given the rule:  platform \<name\> \<type\> \<BLOCK\> end_platform <br>
//! Input:  plat|    Suggestion->     platform...end_platform
bool wizard::AutoComplete::FindSuggestionTerminator(Entry* aEntryWord, std::string& aTerminatorToken)
{
   bool hasTerminator = false;
   if (aEntryWord->mParentCommandPtr && aEntryWord->mCommandWordIndex == 0 &&
       aEntryWord->mParentCommandPtr->Sequence().size() >= 3)
   {
      int           readerCount = (int)aEntryWord->mParentCommandPtr->Sequence().size();
      WsfParseRule* nodePtr     = aEntryWord->mParentCommandPtr->Sequence()[readerCount - 2];
      if (nodePtr->Type() == WsfParseRule::cRECURRENCE)
      {
         WsfParseRecurrence* blockPtr = (WsfParseRecurrence*)nodePtr;
         if (!blockPtr->mEndKeyword.empty())
         {
            aTerminatorToken = blockPtr->mEndKeyword;
            hasTerminator    = true;
         }
      }
   }
   return hasTerminator;
}

void wizard::AutoComplete::CollectSuggestions(Entry*                    aEntry,
                                              size_t                    aWordIndex,
                                              std::vector<std::string>& aSuggestions,
                                              bool&                     aLastToken)
{
   if (aWordIndex == ut::npos)
   {
      if ((aEntry->mFlags & Entry::cBLOCK_PLACEHOLDER) == 0)
      {
         aLastToken = false;
      }
      return;
   }
   switch (aEntry->mEntryType)
   {
   case Entry::cWORD:
      if (aWordIndex == 0)
      {
         if ((aEntry->mFlags & Entry::cBLOCK_PLACEHOLDER) == 0)
         {
            if (!aEntry->mWordText.empty())
            {
               std::string suggestionText;
               if (aEntry->mWordText[0] == '<' && !aEntry->mWordIsPattern)
               {
                  suggestionText = '\\' + aEntry->mWordText;
               }
               else
               {
                  suggestionText = aEntry->mWordText;
               }
               std::string endTerm;
               if (FindSuggestionTerminator(aEntry, endTerm))
               {
                  suggestionText = suggestionText + "..." + endTerm;
               }
               aSuggestions.push_back(suggestionText);
            }
         }
      }
      break;
   case Entry::cALTERNATE:
      for (auto&& c : aEntry->mChildren)
      {
         CollectSuggestions(c, aWordIndex, aSuggestions, aLastToken);
      }
      break;
   case Entry::cSEQUENCE:
   {
      size_t wordIndex = aWordIndex;
      for (auto&& c : aEntry->mChildren)
      {
         int words = c->GetWordCount();
         CollectSuggestions(c, wordIndex, aSuggestions, aLastToken);
         wordIndex -= words;
      }
   }
   break;
   case Entry::cUNEXPANDED:
      break;
   default:
      break;
   }
}


int wizard::AutoComplete::CountSuggestions(Entry* aEntry)
{
   switch (aEntry->mEntryType)
   {
   case Entry::cWORD:
      return 1;
      break;
   case Entry::cALTERNATE:
   {
      int sum = 0;
      for (auto&& c : aEntry->mChildren)
      {
         sum += CountSuggestions(c);
      }
      return sum;
   }
   break;
   case Entry::cSEQUENCE:
   {
      int sequenceCount = 0;
      for (auto&& c : aEntry->mChildren)
      {
         int optionCount = CountSuggestions(c);

         if (sequenceCount == 0)
         {
            sequenceCount = optionCount;
         }
         else
         {
            if (sequenceCount * optionCount < 15)
            {
               sequenceCount *= optionCount;
            }
            else
            {
               break;
            }
         }
      }
      return sequenceCount;
   }
   break;
   default:
      return 0;
   }
}

WsfParseNode* wizard::AutoComplete::FindNextBlock(WsfParseNode* aBlockPtr)
{
   WsfParseNode* nodePtr = aBlockPtr->GetParent();
   std::string   blockStr("block");
   while (nodePtr->GetParent() != nullptr)
   {
      if (nodePtr->mFlags & WsfParseNode::cBLOCK_NODE)
      {
         break;
      }
      nodePtr = nodePtr->GetParent();
   }
   return nodePtr;
}

bool wizard::AutoComplete::FullyExpandWord(size_t aWordIndex) const
{
   return mCurrentQuery.mQueryWordIndex == aWordIndex && mCurrentQuery.mOperation == cAUTOCOMPLETE;
}

wizard::AutoComplete::Entry::Entry()
   : mEntryType(cNONE)
   , mReaderPtr(nullptr)
   , mParentCommandPtr(nullptr)
   , mCurrentTypeChangePtr(nullptr)
   , mCommandWordIndex(0)
   , mFlags(0)
   , mComplete(false)
   , mWordIsPattern(false)
   , mWordCount(-1)
   , mNestedLookup(false)
{
}

wizard::AutoComplete::Entry::Entry(const std::string& aText, bool aWordIsPattern = false)
   : mReaderPtr()
   , mParentCommandPtr()
   , mCurrentTypeChangePtr()
   , mCommandWordIndex()
   , mFlags(0)
   , mWordIsPattern(false)
   , mNestedLookup(false)
{
   Set(aText);
   mWordCount     = 1;
   mWordIsPattern = aWordIsPattern;
}

wizard::AutoComplete::Entry::Entry(const Entry& aRhs)
   : mEntryType(aRhs.mEntryType)
   , mWordText(aRhs.mWordText)
   , mReaderPtr(aRhs.mReaderPtr)
   , mParentCommandPtr(aRhs.mParentCommandPtr)
   , mCurrentTypeChangePtr(aRhs.mCurrentTypeChangePtr)
   , mCommandWordIndex(aRhs.mCommandWordIndex)
   , mChildren()
   , mFlags(aRhs.mFlags)
   , mComplete(aRhs.mComplete)
   , mWordIsPattern(aRhs.mWordIsPattern)
   , mWordCount(aRhs.mWordCount)
   , mRefType(aRhs.mRefType)
   , mNestedLookup(aRhs.mNestedLookup)
   , mRuleName(aRhs.mRuleName)
{
}

wizard::AutoComplete::Entry::~Entry()
{
   for (auto& child : mChildren)
   {
      delete child;
   }
}

void wizard::AutoComplete::Entry::Set(const std::string& aText)
{
   mWordText  = aText;
   mEntryType = cWORD;
   mComplete  = true;
   mWordCount = 1;
}

void wizard::AutoComplete::Entry::SetTypeRef(const WsfParseTypePath& aRefType, bool aIsNestedLookup)
{
   mRefType = aRefType;
   // remove the variable reference if it exists
   if (!mRefType.empty() && mRefType.back()[0] == '$')
   {
      mRefType.pop_back();
   }
   mEntryType = cUNEXPANDED;
   mWordCount = 1;
   mComplete  = false;
   mFlags |= cTYPE_REF_WORD;
   mNestedLookup = aIsNestedLookup;
}

void wizard::AutoComplete::Entry::SetNameRef(const std::string& aRefType)
{
   mRefType.assign(1, aRefType);
   mNestedLookup = false;
   mEntryType    = cUNEXPANDED;
   mWordCount    = 1;
   mComplete     = false;
   mFlags |= cNAMED_WORD;
}


int wizard::AutoComplete::Entry::GetWordCount() const
{
   if (mComplete)
   {
      return mWordCount;
   }
   switch (mEntryType)
   {
   case cWORD:
      return 1;
   case cUNEXPANDED:
      return -1;
   case cSEQUENCE:
   {
      // Take sum of sequence counts as word count
      int sum = 0;
      for (auto&& c : mChildren)
      {
         int val = c->GetWordCount();
         if (val < 0)
         {
            return val;
         }
         sum += val;
      }
      return sum;
   }
   case cALTERNATE:
   {
      // Takes largest possible word count within alternate as word count, or the invalid count if not complete
      int max = 0;
      for (auto&& c : mChildren)
      {
         int val = c->GetWordCount();
         if (val < 0)
         {
            return val;
         }
         max = val > max ? val : max;
      }
      return max;
   }
   default:
      return -1;
   }
}
