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

#include "CommVisRouterNodeDialog.hpp"

#include <QTextBlock>
#include <QTextDocumentFragment>

#include "CommVisRouterGraphicsItem.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "WkfEnvironment.hpp"

WizCommVis::CommVisRouterNodeDialog::CommVisRouterNodeDialog(WsfPM_Router                    aRouterProxyNode,
                                                             wkf::CommVisRouterGraphicsItem* aRouterPtr,
                                                             QWidget*                        aParent,
                                                             Qt::WindowFlags                 aFlags)
   : wkf::CommVisRouterNodeDialog(aRouterPtr, aParent, aFlags)
   , mRouterProxyNode(aRouterProxyNode)
{
   setAttribute(Qt::WA_DeleteOnClose);
   Initialize();
}

void WizCommVis::CommVisRouterNodeDialog::Initialize()
{
   if (mRouterObjPtr != nullptr)
   {
      mUI.mRouterNameLineEdit->setText(mRouterObjPtr->GetName());
      mUI.mRouterNameLineEdit->setReadOnly(false);
      mUI.mRouterTypeLineEdit->setText(mRouterObjPtr->GetRouterType());
      mUI.mRouterTypeLineEdit->setReadOnly(false);
      mUI.mPlatformNameLineEdit->setText(mRouterObjPtr->GetPlatform());
      mUI.mPlatformNameLineEdit->setReadOnly(false);
      mUI.mGatewayLineEdit->setText(mRouterObjPtr->GetGateway());
      mUI.mGatewayLineEdit->setReadOnly(false);
      mUI.mGatewayAddressLineEdit->setText(mRouterObjPtr->GetAddress());
      mUI.mGatewayAddressLineEdit->setReadOnly(false);
      mUI.mHopLimitLineEdit->setText(QString::number(mRouterObjPtr->GetHopLimit()));
      mUI.mHopLimitLineEdit->setReadOnly(false);
      mUI.mAILCheckbox->setChecked(mRouterObjPtr->UseAutomatedInterfaceLinking());
      mUI.mDefaultProtocolCheckbox->setChecked(mRouterObjPtr->UseDefaultProtocol());
      mUI.mMulticastProtocolCheckbox->setChecked(mRouterObjPtr->UseMulticastProtocol());

      connect(mUI.buttonBox, &QDialogButtonBox::accepted, [this]() { UpdateRouterBlock(); });

      if (mUI.mHopLimitLineEdit->text() != "64")
      {
         mUI.mHopLimitLineEdit->setModified(true);
      }
   }
}

void WizCommVis::CommVisRouterNodeDialog::UpdateRouterBlock()
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      UtTextDocumentRange docRange = mRouterProxyNode.GetSourceDocumentRange();

      if (docRange.Valid())
      {
         auto platName = mRouterObjPtr->GetPlatform().toStdString();
         if (!mUI.mRouterNameLineEdit->text().isEmpty() && !platName.empty())
         {
            WsfPM_Root     proxyRoot(mRouterProxyNode.Root());
            WsfPM_Platform curPlatform = proxyRoot.platforms() + platName;

            wizard::Editor* editorPtr =
               wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
            QTextCursor cursor = editorPtr->textCursor();
            cursor.setPosition(ut::cast_to_int(docRange.GetBegin()));
            cursor.movePosition(QTextCursor::StartOfLine);
            QString firstStr = cursor.block().text();
            cursor.setPosition(ut::cast_to_int(docRange.GetEnd()) + 1, QTextCursor::KeepAnchor);

            QString editBlockStr = GenerateRouterString(docRange, curPlatform);
            cursor.insertText(editBlockStr);
         }
      }

      else
      {
         // The router is not defined on this platform (most likely a default router), need to add it
         // There isn't an 'edit' block for this comm on this platform.  Add one.
         WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
         WsfPM_Platform curPlatform = proxyRoot.platforms() + mUI.mPlatformNameLineEdit->text().toStdString();
         if (curPlatform.IsValid())
         {
            docRange = curPlatform.GetSourceDocumentRange();
            if (docRange.Valid())
            {
               QString editBlockStr = GenerateRouterString(docRange, curPlatform);
               editBlockStr.prepend("\n");

               wizard::Editor* editorPtr =
                  wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
               editorPtr->GoToRange(docRange);
               QTextCursor bottomCursor = editorPtr->textCursor();
               bottomCursor.setPosition(ut::cast_to_int(docRange.GetEnd()));
               bottomCursor.setPosition(bottomCursor.block().previous().position());
               bottomCursor.movePosition(QTextCursor::EndOfLine);

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
      }
   }
}

QString WizCommVis::CommVisRouterNodeDialog::GenerateRouterString(const UtTextDocumentRange& aDocRange,
                                                                  const WsfPM_Platform&      aPlatform)
{
   wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
   QTextCursor     cursor    = editorPtr->textCursor();
   cursor.setPosition(ut::cast_to_int(aDocRange.GetBegin()));
   cursor.movePosition(QTextCursor::StartOfLine);
   QString firstStr = cursor.block().text();
   cursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()) + 1, QTextCursor::KeepAnchor);

   QString spaces = GetIndentString(firstStr.toStdString());
   // Need to determine if this is an existing comm (edit) or a new comm (add)
   QString addEditStr = "edit";
   auto    platName   = aPlatform.GetName();
   if (!mUI.mRouterNameLineEdit->text().isEmpty() && !aPlatform.GetName().empty())
   {
      WsfPM_Router router = aPlatform.Comms() + mUI.mRouterNameLineEdit->text().toStdString();
      if (!router.IsValid() || firstStr.trimmed().startsWith("add"))
      {
         // This is comm doesn't currently exist, add it
         addEditStr = "add";
      }
   }

   QString editBlockStr = spaces + addEditStr + " router " + mUI.mRouterNameLineEdit->text();
   if (addEditStr == "add")
   {
      editBlockStr.append(" " + mUI.mRouterTypeLineEdit->text());
   }

   if (!mUI.mGatewayLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + spaces + "gateway " + mUI.mGatewayLineEdit->text());
   }
   if (!mUI.mGatewayAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + spaces + "gateway_address " + mUI.mGatewayAddressLineEdit->text());
   }
   if (!mUI.mHopLimitLineEdit->text().isEmpty() && (mUI.mHopLimitLineEdit->text() != mHopLimitDefault))
   {
      editBlockStr.append("\n" + spaces + spaces + "hop_limit " + mUI.mHopLimitLineEdit->text());
   }
   else
   {
      // Check to see if the file explicitly has a value defined in it already, if so keep that definition and give it the new value
      if (aDocRange.Text().find("hop_limit") != std::string::npos)
      {
         editBlockStr.append("\n" + spaces + spaces + "hop_limit " + mUI.mHopLimitLineEdit->text());
      }
   }

   QString boolStr = mUI.mAILCheckbox->isChecked() ? "true" : "false";
   if (mUI.mAILCheckbox->isChecked() != mAILDefault)
   {
      editBlockStr.append("\n" + spaces + spaces + "automated_interface_linking " + boolStr);
   }
   else
   {
      // Check to see if the file explicitly has a value defined in it already, if so keep that definition and give it the new value
      if (aDocRange.Text().find("automated_interface_linking") != std::string::npos)
      {
         editBlockStr.append("\n" + spaces + spaces + "automated_interface_linking " + boolStr);
      }
   }

   boolStr = mUI.mDefaultProtocolCheckbox->isChecked() ? "true" : "false";
   if (mUI.mDefaultProtocolCheckbox->isChecked() != mUseDefaultProtocolDefault)
   {
      editBlockStr.append("\n" + spaces + spaces + "use_default_protocol " + boolStr);
   }
   else
   {
      // Check to see if the file explicitly has a value defined in it already, if so keep that definition and give it the new value
      if (aDocRange.Text().find("use_default_protocol") != std::string::npos)
      {
         editBlockStr.append("\n" + spaces + spaces + "use_default_protocol " + boolStr);
      }
   }

   boolStr = mUI.mMulticastProtocolCheckbox->isChecked() ? "true" : "false";
   if (mUI.mMulticastProtocolCheckbox->isChecked() != mUseMulticastDefault)
   {
      editBlockStr.append("\n" + spaces + spaces + "use_multicast_protocol " + boolStr);
   }
   else
   {
      // Check to see if the file explicitly has a value defined in it already, if so keep that definition and give it the new value
      if (aDocRange.Text().find("use_default_protocol") != std::string::npos)
      {
         editBlockStr.append("\n" + spaces + spaces + "use_multicast_protocol " + boolStr);
      }
   }

   editBlockStr.append("\n" + spaces + "end_router");
   return editBlockStr;
}
