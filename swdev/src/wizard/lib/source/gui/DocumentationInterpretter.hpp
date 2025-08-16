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

#ifndef DOCUMENTATIONINTERPRETTER_HPP
#define DOCUMENTATIONINTERPRETTER_HPP

#include <string>
class QString;
class QUrl;
class UtTextDocumentRange;

namespace wizard
{
class GrammarDocumentation;
class ParseResults;
class Project;

//! Provides documentation using the URL formats:
//  Regular documentation page: ide://documentation/docs/simulation_control_commands.html
//  Single-command page:        ide://documentation/docs/simulation_control_commands.html?doc_section=end_time
//  Search: ide://documentation/find_documentation?source=E:\data\demos\aea_demo\strike.txt&offset1=296&offset2=296
class DocumentationInterpretter
{
public:
   virtual ~DocumentationInterpretter() = default;

   static bool FindDocumentation(GrammarDocumentation&      aDocumentationData,
                                 ParseResults&              aResults,
                                 const UtTextDocumentRange& aRange,
                                 std::string&               aPageName,
                                 std::string&               aSection);

   static QString LookupDocumentationUrl(const QUrl& aUrl, bool& aReturnsURL, QString& aDocumentName);

private:
   static Project* GetProject();
};
} // namespace wizard

#endif
