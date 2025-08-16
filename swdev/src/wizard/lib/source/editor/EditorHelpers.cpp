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

#include "EditorHelpers.hpp"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocumentFragment>

// Util Includes
#include "UtEllipsoidalEarth.hpp"
#include "UtLatPos.hpp"
#include "UtLonPos.hpp"

// VTK Includes
#include "VaPosition.hpp"
#include "VaUtils.hpp"

// WKF Includes
#include "WkfUnitsObject.hpp"

// Wizard Includes
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"

namespace wizard
{
namespace EditorHelpers
{

QString GetInsertWithComments(const UtTextDocumentRange& aEntryRange, const QString& aAttributeName, const QString& aInsert)
{
   QStringList replacementList;
   if (!aInsert.isEmpty())
   {
      // Quoted string arguments should not be tokenized: for annotations, there are no commands that accept quoted
      // strings with more than 1 argument
      if (aInsert[0] == '"')
      {
         replacementList.push_back(aInsert);
      }
      else // The command could have multiple arguments (e.g. position with lat/lon, color with rgb). Separate the arguments
      {
         replacementList = aInsert.split(" ");
      }
   }
   else
   {
      replacementList.push_back("");
   }

   QStringList insertList = wizard::Editor::Split(QString::fromStdString(aEntryRange.Text()));
   // If removing a command (replacing with empty string), need to get rid of the attribute as well
   if (aInsert.isEmpty())
   {
      insertList[insertList.indexOf(aAttributeName, 0)] = "";
   }
   int replacementIndex = 0;
   int insertIndex      = !aAttributeName.isEmpty() ? (insertList.indexOf(aAttributeName, 0) + 1) : 0;
   // Replace original command with the modified command
   while (replacementIndex < replacementList.size() && insertIndex >= 0)
   {
      insertIndex = wizard::Editor::FindNextValidToken(insertList, insertIndex);
      if (insertIndex >= 0)
      {
         insertList[insertIndex++] = replacementList[replacementIndex++];
      }
   }
   // Remove All Subsequent Commands, but keep comments. This addresses cases where the original command has more
   // arguments than the replacement
   if (insertIndex >= 0)
   {
      std::vector<int> indices;
      insertIndex = wizard::Editor::FindNextValidToken(insertList, insertIndex);
      while (insertIndex != -1 && insertIndex < insertList.size())
      {
         indices.push_back(insertIndex);
         insertIndex = wizard::Editor::FindNextValidToken(insertList, ++insertIndex);
      }
      for (int i = ut::cast_to_int(indices.size()) - 1; i >= 0; --i)
      {
         insertList.removeAt(indices[i]);
      }
   }
   // If the replacement list had more arguments than the original command, add them to the end
   else if (replacementIndex < replacementList.size())
   {
      while (replacementIndex < replacementList.size())
      {
         insertList.append(" " + replacementList[replacementIndex++]);
      }
   }

   QString retVal = insertList.join("");
   QRegExp reg("[ \f\n\r\v]+");
   if (reg.exactMatch(retVal))
   {
      retVal = "";
   }
   return retVal;
}

bool ReplaceTextInEditor(const UtTextDocumentRange& aEntryRange, const QString& aAttributeName, const QString& aInsert)
{
   // Note:  There are fewer cases where the return status will be true, so default to false.
   bool success{false};
   // Disallow calls to this function without calling wizEnv.BeginUndoCapture() first
   if (!wizEnv.IsCapturingUndo())
   {
      return false;
   }
   // Check if aEntryRange is valid
   if (aEntryRange.Valid())
   {
      // Get the wizard::Editor containing aEntryRange
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aEntryRange.mSource->GetFilePath().GetNormalizedPath());
      if (editorPtr)
      {
         // Get the replacement text
         QString replacement = GetInsertWithComments(aEntryRange, aAttributeName, aInsert);
         if (replacement.isEmpty())
         {
            auto removeLineLambda = [](wizard::Editor* aEditorPtr)
            {
               size_t pos[3];
               aEditorPtr->GetCurrentPosition(pos[0], pos[1], pos[2]);
               std::string lineStr = aEditorPtr->GetLineText(pos[1]);
               // Remove the line containing the command if it is the only command on the line
               if (lineStr.find_first_not_of(" \t\r\n") == ut::npos)
               {
                  aEditorPtr->DeleteLine();
                  return true;
               }
               else
               {
                  return false;
               }
            };
            success = removeLineLambda(editorPtr);
         }
         else
         {
            // Replace the command's value
            QTextCursor cursor = editorPtr->textCursor();
            cursor.setPosition(ut::cast_to_int(aEntryRange.GetBegin()));
            cursor.setPosition(ut::cast_to_int(aEntryRange.GetEnd() + 1), QTextCursor::KeepAnchor);
            editorPtr->setTextCursor(cursor);
            cursor.insertText(replacement);
            success = true;
         }
      }
   }
   return success;
}

QTextCursor FindStartCursor(const WsfPProxyNode& aNode) noexcept
{
   // Get the cursor from the wizard::Editor associated with the proxy node
   auto range       = aNode.GetLastDocumentRange();
   auto fileName    = range.mSource->GetFilePath().GetNormalizedPath();
   auto editor      = Project::Instance()->GotoFile(fileName);
   auto startCursor = editor->textCursor();
   startCursor.setPosition(ut::cast_to_int(range.GetBegin()));
   return startCursor;
}

QTextCursor FindEndCursor(const WsfPProxyNode& aNode) noexcept
{
   auto startCursor = FindStartCursor(aNode);
   startCursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
   auto startBlock = startCursor.selection().toPlainText().trimmed();
   auto blockText  = startCursor.block().text();
   while (!blockText.contains("end_" + startBlock))
   {
      startCursor.movePosition(QTextCursor::NextWord);
      blockText = startCursor.block().text();
   }
   auto endBlockBegin = blockText.indexOf("end_" + startBlock);
   startCursor.setPosition(startCursor.block().position() + endBlockBegin);
   return startCursor;
}

int GetIndentLevel(const QString& aLine) noexcept
{
   for (int index = 0; index < aLine.size(); ++index)
   {
      if (!aLine[index].isSpace())
      {
         return index;
      }
   }
   return -1;
}

QString ConstructTextToAppendToBlock(const QTextCursor& aStartCursor,
                                     const QTextCursor& aEndCursor,
                                     const QString&     aAttributeName,
                                     const QString&     aDataValue) noexcept
{
   // Get the text of the current and start of definition QTextBlocks
   QString currentBlockText{aEndCursor.block().text()};
   QString startBlockText{aStartCursor.block().text()};

   // Acquire the indent level of the start of definition QTextBlock
   int startIndentLevel{wizard::EditorHelpers::GetIndentLevel(startBlockText)};

   // Construct the text-to-insert
   QString textToInsert{'\n'};
   // Acquire the indent preference
   auto    indentPref = ut::safe_cast<int>(wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace());
   QString currentLineIndent{startIndentLevel + indentPref, ' '};
   QString nextLineIndent{startIndentLevel, ' '};
   textToInsert.append(QString{currentLineIndent + "%1 %2\n" + nextLineIndent}.arg(aAttributeName).arg(aDataValue));
   return textToInsert;
}

void AddNewAttributeInText(const WsfPProxyNode& aProxyNode, const QString& aAttributeName, const QString& aDataValue) noexcept
{
   if (!aDataValue.isEmpty())
   {
      // Get the cursors pointing to the start and end of the AFSIM input block
      QTextCursor startBlockCursor{wizard::EditorHelpers::FindStartCursor(aProxyNode)};
      QTextCursor endBlockCursor{wizard::EditorHelpers::FindEndCursor(aProxyNode)};

      QString textToInsert{ConstructTextToAppendToBlock(startBlockCursor, endBlockCursor, aAttributeName, aDataValue)};
      // Consume the whitespace of between the end-of-block identifier and the last attribute
      wizard::EditorHelpers::MoveToPreviousNonWhitespaceToken(endBlockCursor, QTextCursor::KeepAnchor);
      endBlockCursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
      // APPEND the attribute to the AFSIM input block
      endBlockCursor.insertText(textToInsert);

      UtTextDocumentRange range{aProxyNode.GetSourceDocumentRange()};
      if (range.Valid())
      {
         std::string     fileName{range.mSource->GetFilePath().GetNormalizedPath()};
         wizard::Editor* editor{wizard::ProjectWorkspace::Instance()->GotoFile(fileName)};
         if (editor)
         {
            // Set the cursor to the beginning of the newly-added text
            editor->setTextCursor(endBlockCursor);
            editor->textCursor().movePosition(QTextCursor::StartOfLine);
            editor->textCursor().clearSelection();
            size_t curPos{ut::cast_to_size_t(editor->textCursor().blockNumber())};
            editor->GoToLine(curPos - 1);
         }
      }
   }
}

bool MoveToPreviousNonWhitespaceToken(QTextCursor&          aCursor,
                                      QTextCursor::MoveMode aMoveMode,
                                      int                   aOperationCount /* = 1 */) noexcept
{
   bool ok{true};
   // Clear the selection
   aCursor.clearSelection();
   // Move the cursor aOperationCount times (mimic the behavior of QTextCursor::movePosition)
   for (int n = 0; n < aOperationCount; ++n)
   {
      // Move to the previous word
      ok &= aCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
      auto text = aCursor.selection().toPlainText();
      // Keep moving to the previous word until the selected text is not all whitespace
      while (IsWhitespaceString(text))
      {
         ok &= aCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
         text = aCursor.selection().toPlainText();
      }
   }
   // This call is to deselect the text if aMoveMode is QTextCursor::MoveAnchor
   aCursor.setPosition(aCursor.position(), aMoveMode);
   return ok;
}

bool IsWhitespaceString(const QString& aText) noexcept
{
   for (auto elem : aText)
   {
      if (!elem.isSpace())
      {
         return false;
      }
   }
   return true;
}

bool ReplacePositionText(QStringList& aPositionTokens, int aCommandIndex, const vespa::VaPosition& aNewPosition) noexcept
{
   // Get the latitude/longitude format from the preferences
   int latLonFormat{wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting()};
   // Read the absolute latitude
   int latIndex{wizard::Editor::FindNextValidToken(aPositionTokens, aCommandIndex + 1)};
   if (latIndex >= 0)
   {
      // Get the latitude from the position
      UtLatPos lat{aNewPosition.GetLat()};
      // Set the latitude format to the preferred format
      lat.SetFormat(latLonFormat);
      aPositionTokens[latIndex] = QString::fromStdString(vespa::VaUtils::ToString<UtLatPos>(lat));
      // Read the absolute longitude
      int lonIndex{wizard::Editor::FindNextValidToken(aPositionTokens, latIndex + 1)};
      if (lonIndex >= 0)
      {
         // Get the longitude from the position
         UtLonPos lon{aNewPosition.GetLon()};
         // Set the longitude format to the preferred format
         lon.SetFormat(latLonFormat);
         aPositionTokens[lonIndex] = QString::fromStdString(vespa::VaUtils::ToString<UtLonPos>(lon));
         // The replacement succeeded
         return true;
      }
   }
   // The replacement failed
   return false;
}

bool ReplaceMGRS_Text(QStringList& aMGRS_Tokens, int aCommandIndex, const vespa::VaPosition& aNewPosition) noexcept
{
   // Read the absolute MGRS position
   int mgrsIndex{wizard::Editor::FindNextValidToken(aMGRS_Tokens, aCommandIndex + 1)};
   if (mgrsIndex >= 0)
   {
      // Edit the absolute MGRS position
      UtLatPos    lat{aNewPosition.GetLat()};
      UtLonPos    lon{aNewPosition.GetLon()};
      std::string mgrs;
      UtEllipsoidalEarth::ConvertLLToMGRS(lat, lon, mgrs);
      aMGRS_Tokens[mgrsIndex] = QString::fromStdString(mgrs);
      // The replacement succeeded
      return true;
   }
   // The replacement failed
   return false;
}

void InsertTextAndResetCursor(wizard::Editor* aEditorPtr, QTextCursor& aCursor, const QString& aTextToInsert) noexcept
{
   aCursor.insertText(aTextToInsert);
   aCursor.movePosition(QTextCursor::StartOfBlock);
   aEditorPtr->setTextCursor(aCursor);
}

QString GetPositionString(const vespa::VaPosition& aPosition) noexcept
{
   // Get the latitude/longitude format from the preferences
   int latLonFormat{wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting()};
   // Get the latitude from the position
   UtLatPos latitudeValue{aPosition.GetLat()};
   // Set the latitude format to the preferred format
   latitudeValue.SetFormat(latLonFormat);
   // Covert the latitude to a string
   auto latitudeValueStr = vespa::VaUtils::ToString<UtLatPos>(latitudeValue);
   // Get the longitude from the position
   UtLonPos longitudeValue{aPosition.GetLon()};
   // Set the longitude format to the preferred format
   longitudeValue.SetFormat(latLonFormat);
   // Covert the longitude to a string
   auto longitudeValueStr = vespa::VaUtils::ToString<UtLonPos>(longitudeValue);
   // The position contains the latitude AND latitude strings delimited by a single ' ' character
   return QString::fromStdString(latitudeValueStr + ' ' + longitudeValueStr);
}

} // namespace EditorHelpers
} // namespace wizard
