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

#include "GrammarDocumentation.hpp"

#include <ctype.h>
#include <fstream>
#include <sstream>

#include "ParseUtil.hpp"
#include "UtStringUtil.hpp"
#include "WsfParseAlternate.hpp"
#include "WsfParseDefinitions.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRecurrence.hpp"
#include "WsfParseRule.hpp"
#include "WsfParseSequence.hpp"

namespace
{

struct CompareVectorString
{
   bool operator()(const std::vector<std::string>& aLhs, const std::vector<std::string>& aRhs) const
   {
      size_t mi = std::min(aLhs.size(), aRhs.size());
      for (size_t i = 0; i < mi; ++i)
      {
         if (aLhs[i] < aRhs[i])
            return true;
         if (aLhs[i] > aRhs[i])
            return false;
      }
      return aLhs.size() < aRhs.size();
   }
};

} // namespace

float CompareTitleToRuleName(const std::string& aTitle, const std::string& aRuleName)
{
   std::string ruleText;
   size_t      dotPos = aRuleName.find('.');
   if (dotPos != std::string::npos)
   {
      ruleText = aRuleName.substr(dotPos + 1);
   }
   else
   {
      ruleText = aRuleName;
   }
   std::string titleText(aTitle);
   UtStringUtil::ToLower(ruleText);
   UtStringUtil::ToLower(titleText);
   if (ruleText == titleText)
   {
      return 0.95f;
   }
   if (ruleText.size() > titleText.size() && std::equal(titleText.begin(), titleText.end(), ruleText.begin()))
   {
      return 0.9f;
   }
   return 0.5f;
}

void GetAllAlternates(WsfParseRule* aRulePtr, std::set<WsfParseRule*>& aProcessed, std::vector<WsfParseAlternate*>& aAlternates)
{
   if (!aRulePtr || !aProcessed.insert(aRulePtr).second)
   {
      return;
   }
   WsfParseRuleDictionary* nested = aRulePtr->GetNestedRules();
   if (nested)
   {
      for (auto&& n2r : nested->mNameToRule)
      {
         GetAllAlternates(n2r.second, aProcessed, aAlternates);
      }
   }
   if (aRulePtr->Type() == WsfParseRule::cALTERNATE)
   {
      aAlternates.push_back(static_cast<WsfParseAlternate*>(aRulePtr));
   }
   std::vector<WsfParseRule*> seq = aRulePtr->GetSequence();
   std::vector<WsfParseRule*> alt = aRulePtr->GetAlternates();
   for (auto&& s : seq)
   {
      GetAllAlternates(s, aProcessed, aAlternates);
   }
   for (auto&& a : alt)
   {
      GetAllAlternates(a, aProcessed, aAlternates);
   }
}

void wizard::GrammarDocumentation::Process(WsfParseDefinitions& aDefinitions, const std::string& aWikiIndexFile)
{
   mDocumentationDir = aWikiIndexFile;
   mDocumentationDir.Up();
   // WIZARD_TODO: memory leaks!
   mRuleCommandData.clear();
   mReaderToRuleName.clear();
   mPageTitleToLink.clear();
   std::vector<WsfParseAlternate*> alternates;
   if (!aDefinitions.GetRoot())
      return;

   std::set<WsfParseRule*> visitedRules;
   GetAllAlternates(aDefinitions.GetGlobalRule(), visitedRules, alternates);

   for (auto&& readerPtr : alternates)
   {
      std::string                       ruleName = aDefinitions.GetFullRuleName(*readerPtr->GetBestContext());
      CommandFormatGroup                group;
      const std::vector<WsfParseRule*>& subAlternates = readerPtr->GetAlternates();
      for (auto&& sequencePtr : subAlternates)
      {
         std::vector<WsfParseRule*> ruleSequence = sequencePtr->GetSequence();
         CommandFormatData          cmd;
         cmd.mReaderPtr = sequencePtr;
         for (auto&& word : ruleSequence)
         {
            if (!word)
               continue;
            if (word->Type() == WsfParseRule::cLITERAL)
            {
               WsfParseLiteral* expectedValuePtr = (WsfParseLiteral*)word;
               mReaderToRule[expectedValuePtr]   = sequencePtr;
               cmd.mWords.push_back(expectedValuePtr->mText);
            }
            else if (word->Type() == WsfParseRule::cALTERNATE)
            {
               cmd.mSubCommands.push_back(static_cast<WsfParseAlternate*>(word));
            }
            else if (word->Type() == WsfParseRule::cRECURRENCE)
            {
               WsfParseRule* recurRule = ((WsfParseRecurrence*)word)->GetSubordinateRule();
               if (recurRule->Type() == WsfParseRule::cALTERNATE)
               {
                  cmd.mSubCommands.push_back(static_cast<WsfParseAlternate*>(recurRule));
               }
            }
         }
         if (!cmd.mWords.empty())
         {
            cmd.mWords.resize(1); // multi-Word commands are too inconsistent to be matched
            group.mCommands.push_back(cmd);
         }
         if (!group.mCommands.empty())
         {
            mReaderToRuleName[sequencePtr] = ruleName;
         }
      }
      if (!group.mCommands.empty())
      {
         group.mName                = ruleName;
         mRuleCommandData[ruleName] = group;
      }
   }

   typedef std::multimap<std::string, PageCommand*> PageFinderMap;
   PageFinderMap                                    pageCommandFinder;
   std::map<std::string, PageData>                  pages;
   std::ifstream                                    wikFile(aWikiIndexFile.c_str());
   PageData*                                        pagePtr = nullptr;
   std::string                                      anchor;
   char                                             line[1001];
   line[1000] = 0;
   while (wikFile.getline(line, 1000))
   {
      // There is a problem with the wiki indexing script where some titles are
      // resulting in lines filled with tabs, just throw away whitespace-only lines
      bool isEmptyLine = true;
      for (char* p = line; *p; ++p)
      {
         if (!isspace(*p))
         {
            isEmptyLine = false;
            break;
         }
      }
      if (isEmptyLine)
         continue;

      if (line[0] != ' ')
      {
         pagePtr        = &pages[line];
         pagePtr->mLink = line;
      }
      else if (pagePtr)
      {
         std::string       lineStr = line + 1;
         std::stringstream ss(lineStr);
         std::string       word;
         ss >> word;
         if (word == "cmd_anchor")
         {
            ss >> anchor;
         }
         else if (word == "cmd_format")
         {
            PageCommand* c = new PageCommand;
            c->mAnchor     = anchor;
            while (ss >> word)
            {
               if (word[0] != '<')
               {
                  c->mFormat.push_back(word);
               }
            }
            if (!c->mFormat.empty())
            {
               c->mFormat.resize(1); // multi-Word commands are too inconsistent to be matched
               c->mPagePtr = pagePtr;
               pagePtr->mCommands.push_back(c);
               pageCommandFinder.insert(make_pair(c->mFormat[0], c));
            }
            else
            {
               delete c;
            }
         }
         else if (word == "title")
         {
            while (ss >> word)
            {
               if (!pagePtr->mTitle.empty())
                  pagePtr->mTitle += ' ';
               pagePtr->mTitle += word;
            }
            mPageTitleToLink[pagePtr->mTitle] = pagePtr->mLink;
         }
         else if (word == "page_name")
         {
            std::string name;
            ss >> name;
            mPageLinkToName[pagePtr->mLink] = name;
         }
         else if (word == "rule_reference")
         {
            std::string rule;
            ss >> rule;
            pagePtr->mRuleReferences.push_back(rule);
         }
      }
   }

   int matchedCommands   = 0;
   int unmatchedCommands = 0;
   for (auto&& i : mRuleCommandData)
   {
      CommandFormatGroup&      group = i.second;
      std::map<PageData*, int> pageHits;
      for (auto&& cfd : group.mCommands)
      {
         PageFinderMap::iterator lb = pageCommandFinder.lower_bound(cfd.mWords[0]);
         PageFinderMap::iterator ub = pageCommandFinder.upper_bound(cfd.mWords[0]);
         while (lb != ub)
         {
            PageCommand* cmd = lb->second;
            pageHits[cmd->mPagePtr]++;
            ++lb;
         }
      }
      for (auto&& cfd : group.mCommands)
      {
         PageFinderMap::iterator lb                = pageCommandFinder.lower_bound(cfd.mWords[0]);
         PageFinderMap::iterator ub                = pageCommandFinder.upper_bound(cfd.mWords[0]);
         PageCommand*            bestPageCommand   = nullptr;
         float                   bestPageScore     = -1.0f;
         float                   bestPageNameScore = -1.0f;
         while (lb != ub)
         {
            PageCommand* cmd           = lb->second;
            float        pageNameScore = CompareTitleToRuleName(cmd->mPagePtr->mTitle, group.mName);
            for (auto&& rref : cmd->mPagePtr->mRuleReferences)
            {
               if (rref == group.mName)
               {
                  pageNameScore = 0.96f;
               }
            }
            float pageScore = pageHits[cmd->mPagePtr] * pageNameScore;
            if (pageScore > bestPageScore)
            {
               bestPageNameScore = pageNameScore;
               bestPageScore     = pageScore;
               bestPageCommand   = cmd;
            }
            ++lb;
         }
         // some global commands aren't marked as commands because they have their own page
         // Don't use the global page if the previously found page had a similar name to the grammar rule,
         // as an attempt to guess context.
         if (bestPageCommand == nullptr || bestPageNameScore <= 0.5f)
         {
            std::map<std::string, std::string>::iterator titleIter = mPageTitleToLink.find(cfd.mWords[0]);
            if (titleIter != mPageTitleToLink.end())
            {
               cfd.mAnchor   = "top";
               cfd.mPageLink = titleIter->second;
            }
         }
         if (!cfd.mPageLink.empty())
         {
            ++matchedCommands;
         }
         else if (bestPageCommand)
         {
            cfd.mAnchor   = bestPageCommand->mAnchor;
            cfd.mPageLink = bestPageCommand->mPagePtr->mLink;
            ++matchedCommands;
         }
         else
         {
            --unmatchedCommands;
         }
      }
   }

   // Clean up memory
   for (PageFinderMap::iterator iter = pageCommandFinder.begin(), iterEnd = pageCommandFinder.end(); iter != iterEnd; ++iter)
   {
      delete iter->second;
   }
}
bool wizard::GrammarDocumentation::FindDocumentation(WsfParseNode* aNodePtr, std::string& aPageLink, std::string& aAnchor)
{
   // object types are handled specially:
   if (aNodePtr->GetFlags() & WsfParseNode::cTYPE_REFERENCE_NODE)
   {
      WsfParseTypePath typeKey;
      bool             isNested;
      if (wizard::ParseUtil::FindReferenceType(aNodePtr, typeKey, isNested))
      {
         if ((typeKey.size() >= 2) && mPageTitleToLink.find(typeKey[1]) != mPageTitleToLink.end())
         {
            aPageLink = mPageTitleToLink[typeKey[1]];
            aAnchor   = "top";
            return true;
         }
      }
   }
   WsfParseNode* nodePtr = aNodePtr;
   while (nodePtr)
   {
      if (nodePtr->Left())
      {
         nodePtr = nodePtr->Left();
         continue;
      }
      if (nodePtr->mValue.Valid())
      {
         CommandFormatGroup* grpPtr = FindCommandFormatGroup(nodePtr->mRulePtr);
         if (grpPtr != nullptr)
         {
            std::string         nodeText = nodePtr->mValue.Text();
            CommandFormatGroup& grp      = *grpPtr;
            for (auto&& cfd : grp.mCommands)
            {
               if (cfd.mWords[0] == nodeText)
               {
                  if (!cfd.mPageLink.empty())
                  {
                     aPageLink = cfd.mPageLink;
                     aAnchor   = cfd.mAnchor;
                     return true;
                  }
               }
            }
         }
      }
      nodePtr = nodePtr->GetParent();
   }
   return false;
}

wizard::GrammarDocumentation::CommandFormatGroup* wizard::GrammarDocumentation::FindCommandFormatGroup(WsfParseRule* aReaderPtr)
{
   std::map<WsfParseRule*, std::string>::iterator iter = mReaderToRuleName.find(aReaderPtr);
   if (iter != mReaderToRuleName.end())
   {
      return &mRuleCommandData[iter->second];
   }
   std::map<WsfParseRule*, WsfParseRule*>::iterator iter2 = mReaderToRule.find(aReaderPtr);
   if (iter2 != mReaderToRule.end())
   {
      iter = mReaderToRuleName.find(iter2->second);
      if (iter != mReaderToRuleName.end())
      {
         return &mRuleCommandData[iter->second];
      }
   }
   return nullptr;
}

std::string wizard::GrammarDocumentation::GetPageName(std::string aPageLink)
{
   return mPageLinkToName[aPageLink];
}
