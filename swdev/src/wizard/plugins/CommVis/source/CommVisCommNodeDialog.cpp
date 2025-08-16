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

#include "CommVisCommNodeDialog.hpp"

#include <QTextBlock>
#include <QTextDocumentFragment>

#include "CommVisCommGraphicsItem.hpp"
#include "CommVisViewLinksDialog.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "UtQtMemory.hpp"
#include "WkfEnvironment.hpp"

WizCommVis::CommVisCommNodeDialog::CommVisCommNodeDialog(WsfPProxyNode                 aCommProxyNode,
                                                         wkf::CommVisCommGraphicsItem* aCommPtr,
                                                         QWidget*                      aParent,
                                                         Qt::WindowFlags               aFlags)
   : wkf::CommVisCommNodeDialog(aCommPtr, aParent, aFlags)
   , mCommProxyNode(aCommProxyNode)
{
   setAttribute(Qt::WA_DeleteOnClose);
   Initialize();
}

void WizCommVis::CommVisCommNodeDialog::Initialize()
{
   if (mCommObjPtr != nullptr)
   {
      mUI.mNetworkNameLineEdit->setText(mCommObjPtr->GetNetworkName());
      mUI.mNetworkNameLineEdit->setReadOnly(false);
      mUI.mCommNameLineEdit->setText(mCommObjPtr->GetName());
      mUI.mCommNameLineEdit->setReadOnly(false);
      mUI.mCommAddressLineEdit->setText(mCommObjPtr->GetAddress());
      mUI.mCommAddressLineEdit->setReadOnly(false);
      mUI.mPlatformNameLineEdit->setText(mCommObjPtr->GetPlatformName());
      mUI.mPlatformNameLineEdit->setReadOnly(false);
      mUI.mCommTypeLineEdit->setText(mCommObjPtr->GetType());
      mUI.mCommTypeLineEdit->setReadOnly(false);
      mUI.mNetworkAddressLineEdit->setText(mCommObjPtr->GetNetworkAddress());
      mUI.mNetworkAddressLineEdit->setReadOnly(false);
      mUI.mGatewayPlatformLineEdit->setText(mCommObjPtr->GetGatewayPlatform());
      mUI.mGatewayPlatformLineEdit->setReadOnly(false);
      mUI.mRouterNameLineEdit->setText(mCommObjPtr->GetRouterName());
      mUI.mRouterNameLineEdit->setReadOnly(false);
      mUI.mGatewayCommLineEdit->setText(mCommObjPtr->GetGatewayName());
      mUI.mGatewayCommLineEdit->setReadOnly(false);

      // Connections
      connect(mUI.buttonBox, &QDialogButtonBox::accepted, [this]() { UpdateAttributes(); });

      mUI.mDeleteCommPushButton->show();
      connect(mUI.mDeleteCommPushButton, &QPushButton::clicked, [this]() { DeleteComm(); });

      if (mLinksDialog)
      {
         delete mLinksDialog;
      }
      mLinksDialog = ut::qt::make_qt_ptr<CommVisViewLinksDialog>(mCommObjPtr->GetLinks(), this, windowFlags());
      connect(mUI.mViewLinksPushButton, &QPushButton::clicked, mLinksDialog.data(), &QDialog::show);
      connect(mLinksDialog.data(), &QDialog::accepted, [this]() { UpdateAttributes(); });
   }
}


void WizCommVis::CommVisCommNodeDialog::AddNewCommBlock()
{
   // Need to determine if this is an existing comm (edit) or a new comm (add)
   QString addEditStr = "edit";
   auto    platName   = mCommObjPtr->GetPlatformName().toStdString();
   if (!mUI.mCommNameLineEdit->text().isEmpty() && !platName.empty())
   {
      WsfPM_Root     proxyRoot(mCommProxyNode.Root());
      WsfPM_Platform curPlatform = proxyRoot.platforms() + platName;
      WsfPM_Comm     comm        = curPlatform.Comms() + mUI.mCommNameLineEdit->text().toStdString();
      if (!comm.IsValid())
      {
         // This is comm doesn't currently exist, add it
         addEditStr = "add";
      }
   }

   // There isn't an 'edit' block for this comm on this platform.  Add one
   UtTextDocumentRange docRange = mCommProxyNode.GetParent().GetParent().GetSourceDocumentRange();
   if (docRange.Valid())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(docRange);
      QTextCursor bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(ut::cast_to_int(docRange.GetEnd()));
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces       = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr = "\n" + spaces + addEditStr + " comm " + mUI.mCommNameLineEdit->text();
      if (addEditStr == "add")
      {
         editBlockStr.append(" " + mUI.mCommTypeLineEdit->text());
      }
      if (!mUI.mNetworkAddressLineEdit->text().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "network_address " + mUI.mNetworkAddressLineEdit->text());
      }
      if (!mUI.mCommAddressLineEdit->text().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "address " + mUI.mCommAddressLineEdit->text());
      }
      if (!mUI.mNetworkNameLineEdit->text().isEmpty() && mUI.mNetworkNameLineEdit->text().toLower() != "default")
      {
         editBlockStr.append("\n" + spaces + spaces + "network_name " + mUI.mNetworkNameLineEdit->text());
      }

      editBlockStr.append(GetLinksString(spaces));

      if (!mUI.mRouterNameLineEdit->text().isEmpty() && mUI.mRouterNameLineEdit->text() != "default")
      {
         editBlockStr.append("\n" + spaces + spaces + "router_name " + mUI.mRouterNameLineEdit->text());
      }
      if (!mUI.mGatewayPlatformLineEdit->text().isEmpty() && !mUI.mGatewayCommLineEdit->text().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "gateway " + mUI.mGatewayPlatformLineEdit->text() + " " +
                             mUI.mGatewayCommLineEdit->text());
      }
      editBlockStr.append("\n" + spaces + "end_comm");


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


void WizCommVis::CommVisCommNodeDialog::UpdateCommBlock()
{
   UtTextDocumentRange docRange = mCommProxyNode.GetSourceDocumentRange();
   wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
   QTextCursor     cursor    = editorPtr->textCursor();

   cursor.setPosition(ut::cast_to_int(docRange.GetBegin()));
   cursor.movePosition(QTextCursor::StartOfLine);
   QString firstStr = cursor.block().text();
   cursor.setPosition(ut::cast_to_int(docRange.GetEnd()) + 1, QTextCursor::KeepAnchor);
   QString spaces = GetIndentString(firstStr.toStdString());

   // Need to determine if this is an existing comm (edit) or a new comm (add)
   QString addEditStr = "edit";
   auto    platName   = mCommObjPtr->GetPlatformName().toStdString();
   if (!mUI.mCommNameLineEdit->text().isEmpty() && !platName.empty())
   {
      WsfPM_Root     proxyRoot(mCommProxyNode.Root());
      WsfPM_Platform curPlatform = proxyRoot.platforms() + platName;
      WsfPM_Comm     comm        = curPlatform.Comms() + mUI.mCommNameLineEdit->text().toStdString();
      if (!comm.IsValid() || firstStr.trimmed().startsWith("add"))
      {
         // This is comm doesn't currently exist, add it
         addEditStr = "add";
      }
   }


   QString editBlockStr = spaces + addEditStr + " comm " + mUI.mCommNameLineEdit->text();
   if (addEditStr == "add")
   {
      editBlockStr.append(" " + mUI.mCommTypeLineEdit->text());
   }
   if (!mUI.mNetworkAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + spaces + "network_address " + mUI.mNetworkAddressLineEdit->text());
   }
   if (!mUI.mCommAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + spaces + "address " + mUI.mCommAddressLineEdit->text());
   }
   if (!mUI.mNetworkNameLineEdit->text().isEmpty() && mUI.mNetworkNameLineEdit->text().toLower() != "default")
   {
      editBlockStr.append("\n" + spaces + spaces + "network_name " + mUI.mNetworkNameLineEdit->text());
   }

   editBlockStr.append(GetLinksString(spaces));

   if (!mUI.mRouterNameLineEdit->text().isEmpty() && mUI.mRouterNameLineEdit->text() != "default")
   {
      editBlockStr.append("\n" + spaces + spaces + "router_name " + mUI.mRouterNameLineEdit->text());
   }
   if (!mUI.mGatewayPlatformLineEdit->text().isEmpty() && !mUI.mGatewayCommLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + spaces + "gateway " + mUI.mGatewayPlatformLineEdit->text() + " " +
                          mUI.mGatewayCommLineEdit->text());
   }
   editBlockStr.append("\n" + spaces + "end_comm");

   cursor.insertText(editBlockStr);
}

QString WizCommVis::CommVisCommNodeDialog::GetLinksString(const QString& aSpaces) const
{
   QString linksStr;

   if (mLinksDialog)
   {
      const QTableWidget* linksTable = mLinksDialog->GetLinksTable();
      for (int i = 0; i < linksTable->rowCount(); ++i)
      {
         // Check if address is set first, if so no need to get the other information
         QLineEdit* linkAddressLineEdit = qobject_cast<QLineEdit*>(linksTable->cellWidget(i, 2));
         if (linkAddressLineEdit && !linkAddressLineEdit->text().isEmpty())
         {
            QString commAddress = linkAddressLineEdit->text();
            linksStr.append("\n" + aSpaces + aSpaces + "link_address " + commAddress);
            continue;
         }
         else
         {
            QString    platformName, commName;
            QLineEdit* linkPlatformLineEdit = qobject_cast<QLineEdit*>(linksTable->cellWidget(i, 0));
            if (linkPlatformLineEdit)
            {
               platformName = linkPlatformLineEdit->text();
            }
            QLineEdit* linkCommLineEdit = qobject_cast<QLineEdit*>(linksTable->cellWidget(i, 1));
            if (linkCommLineEdit)
            {
               commName = linkCommLineEdit->text();
            }
            if ((platformName.isEmpty() || platformName == mCommObjPtr->GetPlatformName() || platformName == "local_link") &&
                !commName.isEmpty())
            {
               // This is a local link
               linksStr.append("\n" + aSpaces + aSpaces + "local_link " + commName);
            }
            else if (!platformName.isEmpty() && !commName.isEmpty())
            {
               linksStr.append("\n" + aSpaces + aSpaces + "link " + platformName + " " + commName);
            }
         }
      }
   }

   return linksStr;
}

void WizCommVis::CommVisCommNodeDialog::UpdateAttributes()
{
   if (wizard::Project::Instance()->ParseIsUpToDate())
   {
      UtTextDocumentRange docRange = mCommProxyNode.GetSourceDocumentRange();
      if (!docRange.Valid())
      {
         AddNewCommBlock();
      }
      else
      {
         UpdateCommBlock();
      }
   }
}

void WizCommVis::CommVisCommNodeDialog::DeleteComm()
{
   if (QMessageBox::warning(this,
                            "Delete Comm?",
                            "Are you sure you want to delete this comm device?",
                            QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
   {
      UtTextDocumentRange docRange = mCommProxyNode.GetSourceDocumentRange();
      if (docRange.Valid())
      {
         wizard::Editor* editorPtr =
            wizard::Project::Instance()->GotoFile(docRange.mSource->GetFilePath().GetNormalizedPath());
         QTextCursor cursor = editorPtr->textCursor();

         cursor.setPosition(ut::cast_to_int(docRange.GetBegin()));
         cursor.movePosition(QTextCursor::StartOfLine);
         cursor.setPosition(ut::cast_to_int(docRange.GetEnd()) + 1, QTextCursor::KeepAnchor);
         cursor.insertText("");
      }
      else
      {
         // This is an inherited comm, do not allow deletion at this level
         QMessageBox::information(this, "Delete Comm?", "This comm device is inherited from a platform type and cannot be deleted at the individual platform level.");
      }
      close();
   }
}
