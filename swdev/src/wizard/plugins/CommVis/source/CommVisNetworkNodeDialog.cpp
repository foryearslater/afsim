// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CommVisNetworkNodeDialog.hpp"

#include <QTextBlock>
#include <QTextDocumentFragment>

#include "CommVisNetworkGraphicsItem.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"

WizCommVis::CommVisNetworkNodeDialog::CommVisNetworkNodeDialog(WsfPProxyNode                    aNetworkProxyNode,
                                                               wkf::CommVisNetworkGraphicsItem* aNetworkPtr,
                                                               QWidget*                         aParent,
                                                               Qt::WindowFlags                  aFlags)
   : wkf::CommVisNetworkNodeDialog(aNetworkPtr, aParent, aFlags)
   , mNetworkProxyNode(aNetworkProxyNode)
{
   setAttribute(Qt::WA_DeleteOnClose);
   Initialize();
}

void WizCommVis::CommVisNetworkNodeDialog::Initialize()
{
   if (mNetworkObjPtr != nullptr)
   {
      mUI.mNetworkNameLineEdit->setText(mNetworkObjPtr->GetName());
      mUI.mNetworkNameLineEdit->setReadOnly(false);
      mUI.mNetworkAddressLineEdit->setText(mNetworkObjPtr->GetAddress());
      mUI.mNetworkAddressLineEdit->setReadOnly(false);
      mUI.mNetworkTypeLineEdit->setText(mNetworkObjPtr->GetNetworkType());
      mUI.mNetworkTypeLineEdit->setReadOnly(false);

      connect(mUI.buttonBox, &QDialogButtonBox::accepted, [this]() { UpdateAttributes(); });
      connect(mUI.mNetworkNameLineEdit, &QLineEdit::textEdited, [this]() { mNameEdited = true; });
      connect(mUI.mNetworkTypeLineEdit, &QLineEdit::textEdited, [this]() { mNameEdited = true; });
   }
}

void WizCommVis::CommVisNetworkNodeDialog::UpdateAttribute(const QString& aAttribute,
                                                           const QString& aScriptKeyword,
                                                           const QString& aValue)
{
   if (aValue.isEmpty())
   {
      // See if this attribute exists, if it does remove it, if it doesn't then there is nothing to do here
      WsfPProxyNode attrNode = mNetworkProxyNode + aAttribute.toStdString();
      if (attrNode.IsValid())
      {
         UtTextDocumentRange docRange = attrNode.GetSourceDocumentRange();
         if (docRange.Valid())
         {
            wizard::Editor* editorPtr =
               wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
            editorPtr->GoToRange(docRange);
            QTextCursor cursor = editorPtr->textCursor();
            cursor.setPosition(ut::cast_to_int(docRange.GetBegin()));
            cursor.setPosition(ut::cast_to_int(docRange.GetEnd()) + 1, QTextCursor::KeepAnchor);
            editorPtr->setTextCursor(cursor);
            cursor.insertText("");
         }
         else
         {
            return;
         }
      }
      else
      {
         return;
      }
   }

   // Special case for updating 'name' of a network as it is part of the start of the network block and not a separate
   // attribute within it...
   if ((aAttribute == "name" || aAttribute == "type") && mNameEdited)
   {
      UtTextDocumentRange docRange = mNetworkProxyNode.GetSourceDocumentRange();
      if (docRange.Valid())
      {
         wizard::Editor* editorPtr =
            wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
         editorPtr->GoToRange(docRange);
         QTextCursor beginCursor = editorPtr->textCursor();
         beginCursor.setPosition(ut::cast_to_int(docRange.GetBegin()));
         beginCursor.movePosition(QTextCursor::StartOfLine);
         size_t      numSpaces     = beginCursor.block().text().toStdString().find_first_not_of(" \t\r\n");
         QStringList strList       = beginCursor.block().text().split(" ", QString::SkipEmptyParts);
         int         networkStrLoc = strList.indexOf("network");
         if (networkStrLoc >= 0)
         {
            if (aAttribute == "name")
            {
               if (strList.size() > networkStrLoc + 1)
               {
                  strList[networkStrLoc + 1] = aValue;
               }
            }
            else if (aAttribute == "type")
            {
               if (strList.size() > networkStrLoc + 2)
               {
                  strList[networkStrLoc + 2] = aValue;
               }
            }
         }
         QString newStr = strList.join(" ");
         QString spaces;
         spaces.resize(ut::cast_to_int(numSpaces), ' ');
         newStr.prepend(spaces);
         beginCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
         beginCursor.insertText(newStr);

         mNameEdited = false;
      }
   }
   else
   {
      WsfPProxyNode attrNode = mNetworkProxyNode + aAttribute.toStdString();
      if (attrNode.IsValid())
      {
         UtTextDocumentRange docRange = attrNode.GetSourceDocumentRange();
         if (docRange.Valid())
         {
            EditAttribute(aScriptKeyword + " " + aValue, docRange);
         }
         else
         {
            // This attribute isn't part of the definition yet, add it
            AddNewAttribute(aScriptKeyword + " " + aValue, mNetworkProxyNode);
         }
      }
   }
}

void WizCommVis::CommVisNetworkNodeDialog::AddNewNetworkBlock()
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      // There isn't a block for this network, add one
      QString         file         = QFileDialog::getOpenFileName(this, "Open File", QString(), "Text (*.txt);;");
      wizard::Editor* editorPtr    = wizard::Project::Instance()->GotoFile(file.toStdString());
      QTextCursor     bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(editorPtr->document()->lastBlock().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces       = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr = "\nnetwork " + mUI.mNetworkNameLineEdit->text() + " " + mUI.mNetworkTypeLineEdit->text();
      if (!mUI.mNetworkAddressLineEdit->text().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "network_address " + mUI.mNetworkAddressLineEdit->text());
      }
      editBlockStr.append("\nend_network");


      bottomCursor.insertText(editBlockStr);
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

void WizCommVis::CommVisNetworkNodeDialog::UpdateAttributes()
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      UtTextDocumentRange docRange = mNetworkProxyNode.GetSourceDocumentRange();
      if (!docRange.Valid())
      {
         AddNewNetworkBlock();
      }
      else
      {
         UpdateAttribute("networkAddress", "network_address", mUI.mNetworkAddressLineEdit->text());

         if (mNameEdited)
         {
            UpdateAttribute("type", "type", mUI.mNetworkTypeLineEdit->text());
            mNameEdited = true;
            UpdateAttribute("name", "name", mUI.mNetworkNameLineEdit->text());
            mNameEdited = false;
            mNetworkObjPtr->SetName(mUI.mNetworkNameLineEdit->text());
         }
      }
   }
}
