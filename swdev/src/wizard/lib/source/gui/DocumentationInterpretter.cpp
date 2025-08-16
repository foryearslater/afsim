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

#include "DocumentationInterpretter.hpp"

#include <QNetworkReply>
#include <QUrlQuery>

#include "CommandDocumentation.hpp"
#include "GrammarDocumentation.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "RunEnvManager.hpp"
#include "TextSource.hpp"
#include "WsfExe.hpp"
#include "WsfScriptLookup.hpp"

QString wizard::DocumentationInterpretter::LookupDocumentationUrl(const QUrl& aUrl, bool& aReturnsURL, QString& aDocumentName)
{
   QUrlQuery query(aUrl);
   if (aUrl.path() == "/find_documentation")
   {
      Project* projectPtr = GetProject();
      if (projectPtr)
      {
         QString source  = query.queryItemValue("source");
         QString offset1 = query.queryItemValue("offset1");
         QString offset2 = query.queryItemValue("offset2");
         if (!source.isEmpty() && !offset1.isEmpty() && !offset2.isEmpty())
         {
            int         offsetInt1 = offset1.toInt();
            int         offsetInt2 = offset2.toInt();
            TextSource* sourcePtr  = projectPtr->GetWorkspace()->FindSource(source.toStdString(), false, true);
            if (sourcePtr)
            {
               UtTextDocumentRange range(sourcePtr, offsetInt1, offsetInt2);
               if (projectPtr->GetExe())
               {
                  GrammarDocumentation* docPtr = projectPtr->GetExe()->GetGrammarDocumentation().get();
                  if (docPtr)
                  {
                     std::string   pageName;
                     std::string   sectionName;
                     ParseResults* resultsPtr = projectPtr->WaitForParseResults();

                     if (resultsPtr &&
                         DocumentationInterpretter::FindDocumentation(*docPtr, *resultsPtr, range, pageName, sectionName))
                     {
                        aDocumentName = pageName.c_str();
                        QUrl newUrl;
                        newUrl.setScheme("ide");
                        newUrl.setHost("documentation");
                        newUrl.setPath("/" + QString::fromStdString(pageName));
                        //                        if (!sectionName.empty() && sectionName != "top")
                        {
                           QUrlQuery query(newUrl);
                           query.addQueryItem("doc_section", QString::fromStdString(sectionName));
                           newUrl.setQuery(query);
                        }
                        aReturnsURL = false;
                        return LookupDocumentationUrl(newUrl.toString(), aReturnsURL, aDocumentName);
                     }
                  }
               }
            }
         }
      }
      aReturnsURL = true;
      return "";
   }

   QString section = query.queryItemValue("doc_section");
   if (!section.isEmpty())
   {
      // Detect if the user is trying to access another anchor:
      if (aUrl.hasFragment())
      {
         QUrl newUrl(aUrl);
         newUrl.setFragment("");
         QUrlQuery query(newUrl);
         query.removeQueryItem("doc_section");
         newUrl.setQuery(query);
         return LookupDocumentationUrl(newUrl, aReturnsURL, aDocumentName);
      }
      Project* projectPtr = GetProject();
      if (projectPtr)
      {
         std::string page = aUrl.path().mid(1).toStdString();
         aDocumentName    = page.c_str();

         WsfExe* exePtr = projectPtr->GetExe();
         if (exePtr)
         {
            std::shared_ptr<GrammarDocumentation> doc = exePtr->GetGrammarDocumentation();
            QString text = CommandDocumentation::PeekDocumentationText(*doc, page, section.toStdString());
            aReturnsURL  = false;
            return text;
         }
      }
   }
   else
   {
      aReturnsURL          = true;
      std::string urlpath  = aUrl.path().mid(1).toStdString();
      std::string fragment = aUrl.fragment().toStdString();
      return QString::fromStdString(RunEnvManager::DocumentationURL(urlpath, fragment));
   }
   aReturnsURL = true;
   return "";
}

wizard::Project* wizard::DocumentationInterpretter::GetProject()
{
   return ProjectWorkspace::Instance()->GetProject();
}


bool wizard::DocumentationInterpretter::FindDocumentation(GrammarDocumentation&      aDocumentationData,
                                                          ParseResults&              aResults,
                                                          const UtTextDocumentRange& aRange,
                                                          std::string&               aPageName,
                                                          std::string&               aSection)
{
   WsfParseNode* nodePtr = aResults.FindLastParseNode(aRange);
   if (nodePtr != nullptr)
   {
      if (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK)
      {
         UtTextDocumentRange range = WsfScriptLookup::IdentifierRange(aRange.GetPosition());
         if (range.Valid())
         {
            WsfScriptLookup lookup(aResults.mProjectPtr);
            lookup.SetLocation(range.GetPosition());
            lookup.Update();
            std::string identName = range.Text();
            // Interpret as type:
            UsType* typePtr = lookup.FindType(identName);
            if (typePtr)
            {
               std::string pageName;
               if (aResults.mScriptDataPtr->FindScriptTypeDocumentation(aDocumentationData, typePtr, pageName))
               {
                  aPageName = pageName;
                  aSection  = "top";
                  return true;
               }
            }
            // Interpret as method call:
            UsValRef                   valRef;
            WsfScriptLookup::SymbolMap symMap;
            if (lookup.AddMethodSymbols(symMap))
            {
               if (symMap.find(identName) != symMap.end())
               {
                  valRef = symMap[identName];
               }
            }
            else
            {
               lookup.AddSymbolsFromThis(symMap);
               lookup.AddSymbolsFromBuiltins(symMap);
               if (symMap.find(identName) != symMap.end())
               {
                  valRef = symMap[identName];
               }
            }
            if (valRef.mFunctionNamePtr)
            {
               std::string pageName;
               std::string className;
               if (aResults.mScriptDataPtr->FindScriptMethodDocumentation(aDocumentationData,
                                                                          valRef.mFunctionTypePtr,
                                                                          valRef.mFunctionNamePtr,
                                                                          pageName,
                                                                          className))
               {
                  aPageName = pageName;
                  aSection  = className + "." + valRef.mFunctionNamePtr;
                  return true;
               }
            }
            return false;
         }
      }
      return aDocumentationData.FindDocumentation(nodePtr, aPageName, aSection);
   }
   return false;
}
