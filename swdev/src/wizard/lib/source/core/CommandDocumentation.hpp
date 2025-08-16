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

#ifndef COMMANDDOCUMENTATION_HPP
#define COMMANDDOCUMENTATION_HPP
#include <string>

#include <QString>
#include <QUrl>

#include "WsfParseNode.hpp"

class UtTextDocumentRange;

namespace wizard
{
class GrammarDocumentation;
class CommandDocumentation
{
public:
   static QUrl    GetDocUrl(const UtTextDocumentRange& aRange);
   static QString PeekDocumentationText(GrammarDocumentation& aDocumentationData, WsfParseNode* aLastParseNode);
   static QString PeekDocumentationText(GrammarDocumentation& aDocumentationData,
                                        const std::string&    aPageName,
                                        const std::string&    aAnchor);
   static QString FindDocumentationUrl(GrammarDocumentation& aDocumentationData, WsfParseNode* aLastParseNode);

   static std::string PageURL(GrammarDocumentation& aDocumentationData,
                              const std::string&    aLinkPath,
                              const std::string&    aAnchor);
};
} // namespace wizard
#endif
