// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CommVisEditDialog.hpp"

#include <QTextBlock>
#include <QTextCursor>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "UtTextDocument.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPProxyNode.hpp"

WizCommVis::CommVisEditDialog::~CommVisEditDialog() = default; // Required for abstract class

size_t WizCommVis::CommVisEditDialog::GetNumSpaces(std::string aLine) const noexcept
{
   // if (aLine.empty())
   //{
   //    return 0;
   // }

   size_t numberSpaces = aLine.find_first_not_of(" \f\n\r\t\v");
   size_t tabSize      = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
   numberSpaces        = (numberSpaces != aLine.npos && numberSpaces != 0) ? (numberSpaces) : tabSize;
   return numberSpaces;
}

QString WizCommVis::CommVisEditDialog::GetIndentString(std::string aLine) const
{
   QString spaces;
   spaces.resize(ut::cast_to_int(GetNumSpaces(aLine)), ' ');
   return spaces;
}

void WizCommVis::CommVisEditDialog::AddNewDevice(const UtTextDocumentRange& aDocRange,
                                                 const QString&             aBlockStartStr,
                                                 const QString&             aBlockEndStr)
{
   wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
   editorPtr->GoToRange(aDocRange);
   QTextCursor bottomCursor = editorPtr->textCursor();
   bottomCursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()));
   bottomCursor.setPosition(bottomCursor.block().previous().position());
   bottomCursor.movePosition(QTextCursor::EndOfLine);

   QString spaces       = GetIndentString(bottomCursor.block().text().toStdString());
   QString editBlockStr = spaces + aBlockStartStr;
   int     numberSpaces = spaces.size() + wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
   editBlockStr.append(GetAttributeString(numberSpaces));
   editBlockStr.append("\n" + spaces + aBlockEndStr);

   bottomCursor.insertText("\n" + editBlockStr);
   editorPtr->setTextCursor(bottomCursor);
   editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
   editorPtr->textCursor().clearSelection();
   int curPos = editorPtr->textCursor().blockNumber();
   if (curPos > 0)
   {
      editorPtr->GoToLine(curPos - 1);
   }
}

void WizCommVis::CommVisEditDialog::EditAttribute(const QString& aAttributeStr, const UtTextDocumentRange& aDocRange)
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(aDocRange);
      QTextCursor cursor = editorPtr->textCursor();
      cursor.setPosition(ut::cast_to_int(aDocRange.GetBegin()));
      cursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()) + 1, QTextCursor::KeepAnchor);
      editorPtr->setTextCursor(cursor);
      cursor.insertText(aAttributeStr);
   }
}

void WizCommVis::CommVisEditDialog::AddNewAttribute(const QString& aNewAttributeStr, const WsfPProxyNode& aNode)
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      UtTextDocumentRange docRange = aNode.GetSourceDocumentRange();
      if (docRange.Valid())
      {
         wizard::Editor* editorPtr =
            wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
         editorPtr->GoToRange(docRange);
         QTextCursor bottomCursor = editorPtr->textCursor();
         bottomCursor.setPosition(ut::cast_to_int(docRange.GetEnd()));
         bottomCursor.setPosition(bottomCursor.block().previous().position());
         bottomCursor.movePosition(QTextCursor::EndOfLine);
         QString spaces = GetIndentString(bottomCursor.block().text().toStdString());
         spaces.resize(spaces.size() + wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace(), ' ');

         bottomCursor.insertText("\n" + spaces + aNewAttributeStr);
         editorPtr->setTextCursor(bottomCursor);
         editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
         editorPtr->textCursor().clearSelection();
         int curPos = editorPtr->textCursor().blockNumber();
         if (curPos > 0)
         {
            editorPtr->GoToLine(curPos - 1);
         }
      }
   }
}
