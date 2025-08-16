// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ColorUtils.hpp"

#include <algorithm>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Editor.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "UtColor.hpp"
#include "UtQt.hpp"
#include "UtTextDocument.hpp"
#include "WsfParseUtil.hpp"
#include "WsfScriptLookup.hpp"

std::string ColorUtils::IdentifierAtPosition(wizard::Editor* aEditorPtr, size_t aPos, size_t& aStartingPos, bool aScriptOnly)
{
   std::string identifier;
   if (aPos >= 0)
   {
      wizard::ParseResults* parseResults = aEditorPtr->GetProject()->GetParseResults();
      if (parseResults && parseResults->mScriptDataPtr)
      {
         size_t line, start;
         aEditorPtr->GetLineColumn(aPos, line, start);
         std::string text = aEditorPtr->GetLineText(line);
         if (text.empty())
         {
            start = 0;
         }
         else
         {
            start = std::min(start, text.size() - 1);
         }

         size_t end = start;
         while (start != 0)
         {
            char c = text[start];
            if (!(isalpha(c) || isdigit(c) || c == '_'))
            {
               ++start;
               break;
            }
            --start;
         }

         while (end < text.size())
         {
            char c = text[end];
            if (!(isalpha(c) || isdigit(c) || c == '_'))
            {
               break;
            }
            ++end;
         }
         if (!text.empty())
         {
            end = std::min(end, text.size());
         }

         if (end > start)
         {
            std::string             id = text.substr(start, end - start);
            wizard::WsfScriptLookup lookup(aEditorPtr->GetProject());
            lookup.SetLocation(UtTextDocumentLocation(aEditorPtr->GetSource(), aPos));
            wizard::WsfScriptLookup::UpdateResult result = lookup.Update();
            if (result != wizard::WsfScriptLookup::cNO_SCRIPT || !aScriptOnly)
            {
               aStartingPos = aEditorPtr->GetSource()->GetSource()->GetLinePosition(line) + start;
               identifier   = id;
            }
         }
      }
   }
   return identifier;
}

// Note: This assumes a valid or empty color string
QColor ColorUtils::GetColorFromString(const std::string& aColorStr)
{
   QColor color;
   if (!aColorStr.empty())
   {
      // Assumes the quoted value is a hex value
      if (aColorStr[0] == '"')
      {
         auto colorStr = aColorStr;
         colorStr.erase(std::remove(colorStr.begin(), colorStr.end(), '"'), colorStr.end());
         color = UtQtColor(UtColor(colorStr));
      }
      else
      {
         float             rgb[3];
         float             alpha = 1;
         size_t            count = 0;
         std::string       val;
         std::stringstream ss(aColorStr);
         while (ss.good() && std::getline(ss, val, ','))
         {
            // Set the rgb value
            if (count < 3)
            {
               rgb[count] = std::stof(val) / 255.0F;
            }
            else
            {
               // Has optional alpha value to set
               alpha = std::stof(val) / 255.0F;
            }
            count++;
         }
         color = UtQtColor(UtColor(rgb, alpha));
      }
   }
   return color;
}

bool ColorUtils::IsValidColor(const std::string& aColorStr)
{
   bool isValidColor = true;
   if (!aColorStr.empty())
   {
      if (aColorStr[0] == '"')
      {
         // Should be of the format "######" or "########" where # is 0-9 or A-F.
         // The last optional two are for the alpha value
         QRegularExpression hex("^\"[0-9A-F]{6}([0-9A-F]{2})?\"$");
         isValidColor = hex.match(QString::fromStdString(aColorStr)).hasMatch();
      }
      else
      {
         size_t            count = 0;
         std::string       val;
         std::stringstream ss(aColorStr);
         // For each comma separated value, check if it is a 0-255 integer
         while (ss.good() && std::getline(ss, val, ','))
         {
            UtStringUtil::TrimWhiteSpace(val);
            if (std::find_if_not(std::begin(val), std::end(val), [](char c) { return WsfParseUtil::IsDigit(c); }) !=
                   std::end(val) ||
                std::stof(val) < 0 || std::stof(val) > 255 || count > 3)
            {
               isValidColor = false;
               break;
            }
            count++;
         }
      }
   }
   return isValidColor;
}

std::string ColorUtils::GetColorString(wizard::Editor* aEditorPtr, const size_t& aStartPos)
{
   std::string colorStr;
   size_t      line, col;
   aEditorPtr->GetLineColumn(aStartPos, line, col);
   auto lineText = aEditorPtr->GetLineText(line);
   auto linePos  = aEditorPtr->GetSource()->GetSource()->GetLinePosition(line);
   if (lineText[aStartPos - linePos] == '(')
   {
      size_t openParenPos = aStartPos - linePos + 1;
      auto   closingPos   = lineText.find_first_of(')', openParenPos);
      if (closingPos < lineText.size())
      {
         colorStr = lineText.substr(openParenPos, closingPos - openParenPos);
      }
   }
   return colorStr;
}
