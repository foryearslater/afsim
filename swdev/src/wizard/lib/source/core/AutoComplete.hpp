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

#ifndef AUTOCOMPLETE_HPP
#define AUTOCOMPLETE_HPP
#include <string>
#include <vector>

#include "WsfParseType.hpp"
class WsfParseRule;
class WsfParseNode;
class WsfParseType;
class WsfParseSequence;

namespace wizard
{
class ParseResults;
class TextSource;

//! Computes suggestions for the auto-complete popup
//! The algorithm is as follows:
//!   Given a starting containing block node B (may be null)
//!   and given a starting insert position P:
//!     1. Combine all words, W, from P to either start of line, or end of previously parsed node.
//!     2. Grab rule, R, used to create nodes for block B (or root rule if B is null)
//!     3. Recursively expand and prune the tree of rules beginning at R with all possible following words for
//!        words 1..N where N is the number of words in W.  While pruning words that do not match W.
//!     4. Return the list of valid words for the last word in W.
//!
//!  This words in most cases, but there are a few problems:
//!
//!  * Extending valid commands:
//!
//!    processor \<internal-link\>                        (implicit mover command) <br>
//!    processor \<name\> \<type\> ... end_processor      (platform command)       <br>
//!
//!    matches:  <br>
//!
//!    processor myproc WSF_|     ('processor myproc' matches the first rule, leaving us to autocomplete only on WSF_)
//!    <br> end_time 1 m|              ('end_time 1 m' matches a rule, leaving no way to autocomplete for 'minutes')
//!
//!
class AutoComplete
{
public:
   struct Results
   {
      std::vector<std::string> mSuggestionList;
      WsfParseNode*            mParentNodePtr;
      size_t                   mInsertPosition;
      size_t                   mReplaceCharacters;
      bool                     mLastToken;
      bool                     mPartialToken;
   };
   AutoComplete();
   void GetSuggestions(ParseResults&      aParseData,
                       TextSource&        aSource,
                       WsfParseNode*      aNodePtr,
                       const std::string& aText,
                       size_t             aInsertPosition,
                       Results&           aResults);
   using Syntax     = std::vector<std::string>;
   using SyntaxList = std::vector<Syntax>;
   struct SyntaxResults
   {
      size_t mWordIndex;
      // The number of words that are in the base command
      // For instance:   platform <name> <type> ... end_platform
      //               has 3 concrete words.  We stop showing syntax once '...' is reached
      size_t     mConcreteWordCount;
      SyntaxList mSequences;
   };
   void GetSyntax(ParseResults&      aParseData,
                  TextSource&        aSource,
                  WsfParseNode*      aNodePtr,
                  const std::string& aText,
                  size_t             aInsertPosition,
                  SyntaxResults&     aResults);

protected:
   enum Operation
   {
      cAUTOCOMPLETE,
      cSYNTAX
   };

   struct Entry
   {
      enum EntryType
      {
         cNONE,
         cUNEXPANDED,
         cWORD,
         cSEQUENCE,
         cALTERNATE,
         cTYPE_CHANGE
      };
      enum Flags
      {
         cNAMED_WORD        = 1,
         cTYPE_REF_WORD     = 2,
         cBLOCK_PLACEHOLDER = 4
      };
      Entry();
      Entry(const std::string& aText, bool aWordIsPattern);
      Entry(const Entry& aRhs);
      Entry& operator=(const Entry&) = default; // Copy assignment
      ~Entry();
      void Set(const std::string& aText);
      void SetTypeRef(const WsfParseTypePath& aRefType, bool aIsNestedLookup);
      void SetNameRef(const std::string& aRefType);
      int  GetWordCount() const;

      EntryType           mEntryType;
      std::string         mWordText;
      WsfParseRule*       mReaderPtr;
      WsfParseSequence*   mParentCommandPtr;
      const WsfParseType* mCurrentTypeChangePtr;
      size_t              mCommandWordIndex;
      std::vector<Entry*> mChildren;
      int                 mFlags;
      bool                mComplete;
      bool                mWordIsPattern;
      int                 mWordCount;
      WsfParseTypePath    mRefType;
      bool                mNestedLookup;
      // std::string             mRefType;
      std::string mRuleName;
   };

   struct Query
   {
   public:
      ~Query() = default;
      std::vector<std::string> mQueryWords;
      size_t                   mQueryWordIndex;

      Operation mOperation;
   };

   SyntaxList CollectSyntax(Entry* aEntryTreePtr, bool aFirstWord = true);
   SyntaxList JoinSyntax(const SyntaxList& aLhs, const SyntaxList& aRhs);
   void GetIncludeSuggestions(const std::string& aText, TextSource& aSource, ParseResults& aParseData, Results& aResults);
   bool FullyExpandWord(size_t aWordIndex) const;
   void DigestInput(const std::string& aText, size_t aInputPosition, size_t& aReplaceCharacters, size_t& aInsertPosition);
   int  ExpandTree(Entry& aEntry, const WsfParseType* aCurrentTypePtr, int aWordIndex, int aMaxWordIndex);
   bool TextMatches(const std::string& aText, size_t aWordIndex);
   bool TextMatches(const std::string& aText, const std::string& aQuery);

   WsfParseNode* FindBlock(WsfParseNode* aNodePtr, int aInsertPosition);

   bool ExpandRule(int                 aWordIndex,
                   WsfParseRule*       aReaderPtr,
                   const WsfParseType* aCurrentTypePtr,
                   Entry&              aEntry,
                   bool                aExpandBlock = false);
   void ExpandRefType(Entry& aEntry, const WsfParseType* aCurrentTypePtr, int aWordIndex);
   void Prune(int aQueryWordIndex, Entry*& aEntry);
   void CollectSuggestions(Entry* aRootEntry, size_t aWordIndex, std::vector<std::string>& aSuggestions, bool& aLastToken);
   bool          FindSuggestionTerminator(Entry* aEntryWord, std::string& aTerminatorToken);
   WsfParseNode* FindNextBlock(WsfParseNode* aBlockPtr);
   int           CountSuggestions(Entry* aRootEntry);

   const WsfParseType* FindCurrentType(WsfParseNode* aNodePtr);

   bool          mExpanded;
   ParseResults* mParseResultsPtr;
   // WsfParseType*           mCurrentTypePtr;
   const WsfParseType* mCurrentTypeRef;
   bool                mExpandTypeCommand;

   Query mCurrentQuery;
};
} // namespace wizard

#endif
