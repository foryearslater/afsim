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

#ifndef GRAMMARDOCUMENTATION_HPP
#define GRAMMARDOCUMENTATION_HPP
class WsfParseDefinitions;
class WsfParseNode;
class WsfParseSequence;
class WsfParseAlternate;
class WsfParseRule;
#include <map>
#include <string>
#include <vector>

#include "UtPath.hpp"

namespace wizard
{
class GrammarDocumentation
{
public:
   struct CommandFormatGroup;

   void Process(WsfParseDefinitions& aDefinitions, const std::string& aWikiIndexFile);

   bool FindDocumentation(WsfParseNode* aNodePtr, std::string& aPageLink, std::string& aAnchor);

   std::string GetPageName(std::string aPageLink);

   CommandFormatGroup* FindCommandFormatGroup(WsfParseRule* aReaderPtr);

   struct CommandFormatData
   {
      WsfParseRule*                   mReaderPtr;
      std::vector<WsfParseAlternate*> mSubCommands;
      std::vector<std::string>        mWords;
      std::string                     mPageLink;
      std::string                     mAnchor;
   };
   struct CommandFormatGroup
   {
      std::string                    mName;
      std::vector<CommandFormatData> mCommands;
   };
   struct PageData;
   typedef std::vector<std::string> StringList;
   struct PageCommand
   {
      PageData*   mPagePtr;
      std::string mAnchor;
      StringList  mFormat;
   };

   struct PageData
   {
      std::string               mLink;
      std::string               mName;
      std::string               mTitle;
      std::vector<PageCommand*> mCommands;
      // set of rule-names specifically referenced by the writer of the wiki page
      std::vector<std::string> mRuleReferences;
   };

   // std::map<WsfParseRule*, CommandFormatGroup>   mReaderData;
   std::map<std::string, CommandFormatGroup> mRuleCommandData;
   std::map<WsfParseRule*, std::string>      mReaderToRuleName;
   std::map<WsfParseRule*, WsfParseRule*>    mReaderToRule;
   std::map<std::string, std::string>        mPageTitleToLink;
   std::map<std::string, std::string>        mPageLinkToName;
   UtPath                                    mDocumentationDir;
};
} // namespace wizard
#endif
