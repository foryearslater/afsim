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

#include "CommandDocumentation.hpp"

#include <sstream>

#include <QFile>
#include <QUrlQuery>

#include "GrammarDocumentation.hpp"
#include "UtPath.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRule.hpp"

QUrl wizard::CommandDocumentation::GetDocUrl(const UtTextDocumentRange& aRange)
{
   QUrl url;
   if (aRange.mSource)
   {
      QString path = QString::fromStdString(aRange.mSource->GetFilePath().GetSystemPath());
      url.setScheme("ide");
      url.setHost("documentation");
      url.setPath("/find_documentation");
      QUrlQuery query;
      query.addQueryItem("source", path);
      query.addQueryItem("offset1", QString::number(aRange.GetBegin()));
      query.addQueryItem("offset2", QString::number(aRange.GetEnd()));
      url.setQuery(query);
      QString ttt = url.toString();
   }
   return url;
}

QString wizard::CommandDocumentation::PeekDocumentationText(GrammarDocumentation& aDocumentationData,
                                                            const std::string&    aPageName,
                                                            const std::string&    aAnchor)
{
   UtPath pagePath = aDocumentationData.mDocumentationDir + aPageName;
   if (pagePath.Stat() == UtPath::cFILE)
   {
      QFile pageFile(QString::fromStdString(pagePath.GetSystemPath()));
      pageFile.open(QIODevice::ReadOnly);
      QString           pageText = pageFile.readAll();
      std::stringstream ss;
      ss << "<dt id=\"" << aAnchor << "\">";
      QString anchorText = ss.str().c_str();
      if (aAnchor == "top")
      {
         int bol = pageText.indexOf("<div class=\"documentwrapper\">");
         int pos = bol + 1;
         if (bol > 0)
         {
            int divCount = 1;
            while (divCount > 0)
            {
               pos = pageText.indexOf("div", pos + 1); // look past our current address
               if (pos == -1)                          // if we couldn't find it, we weren't able to parse the document
               {
                  break;
               }
               else if (pageText[pos - 1] == '<') // div open mark, add to the count
               {
                  divCount++;
               }
               else if ((pageText[pos - 1] == '/') && (pageText[pos - 2] == '<')) // div close mark, remove from the count
               {
                  divCount--;
               }
               // else this is not a marking
            }
         }

         if (pos > bol)
         {
            pos += 4;
            pageText = pageText.mid(bol, pos - bol);
            pageText.prepend(QString::fromStdString("<a href=\"ide://documentation/" + aPageName + "#" + aAnchor +
                                                    "\">[ Full Article ]</a>"));

            return pageText;
         }
         // strip the document of all the stuff we don't care about
      }
      int anchorPos = pageText.indexOf(anchorText);
      if (anchorPos != -1)
      {
         int bol = pageText.lastIndexOf("<dl", anchorPos);
         if (bol != -1)
         {
            int tables = 0;
            int pos    = bol;
            while (true)
            {
               int tpos = pageText.indexOf("<dd>", pos);
               int epos = pageText.indexOf("</dl>", pos);
               if (tpos != -1 && tpos < epos)
               {
                  ++tables;
                  pos = tpos + 1;
               }
               else
               {
                  if (epos != -1)
                  {
                     --tables;
                     pos = epos + 5;
                  }
                  else
                  {
                     tables = 0;
                  }
               }
               if (tables == 0)
                  break;
            }
            if (pos > bol)
            {
               pageText = pageText.mid(bol, pos - bol);
               pageText.prepend(QString::fromStdString("<a href=\"ide://documentation/" + aPageName + "#" + aAnchor +
                                                       "\">[ Full Article ]</a><BR/>"));

               return pageText;
            }
         }
      }
   }
   return QString();
}

QString wizard::CommandDocumentation::PeekDocumentationText(GrammarDocumentation& aDocumentationData,
                                                            WsfParseNode*         aLastParseNode)
{
   if (aLastParseNode != nullptr)
   {
      std::string page, anchor;
      if (aDocumentationData.FindDocumentation(aLastParseNode, page, anchor))
      {
         return PeekDocumentationText(aDocumentationData, page, anchor);
      }
   }
   return QString();
}

QString wizard::CommandDocumentation::FindDocumentationUrl(GrammarDocumentation& aDocumentationData,
                                                           WsfParseNode*         aLastParseNode)
{
   if (aLastParseNode != nullptr)
   {
      std::string page, anchor;
      if (aDocumentationData.FindDocumentation(aLastParseNode, page, anchor))
      {
         std::string url = PageURL(aDocumentationData, page, anchor);
         return QString::fromStdString(url);
      }
   }
   return QString();
}

std::string wizard::CommandDocumentation::PageURL(GrammarDocumentation& aDocumentationData,
                                                  const std::string&    aLinkPath,
                                                  const std::string&    aAnchor)
{
   UtPath      pagePath = aDocumentationData.mDocumentationDir + aLinkPath;
   std::string url      = pagePath.GetNormalizedPath();
   if (url.length() > 1 && url.substr(0, 2) == "//")
   {
      url = url.substr(1);
   }
   url = "file:///" + url + "#" + aAnchor;
   return url;
}
