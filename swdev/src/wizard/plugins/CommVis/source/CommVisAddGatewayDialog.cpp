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


#include "CommVisAddGatewayDialog.hpp"

#include <QTextBlock>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Platform.hpp"


WizCommVis::CommVisAddGatewayDialog::CommVisAddGatewayDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : QDialog(aParent, aFlags)
{
   setAttribute(Qt::WA_DeleteOnClose);
   mUI.setupUi(this);

   Initialize();
}

void WizCommVis::CommVisAddGatewayDialog::Initialize()
{
   // Populate the platforms
   WsfPM_Root        proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_PlatformMap platforms = proxyRoot.platforms();

   for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
   {
      WsfPM_Platform curPlatform  = platforms.AtT(i);
      std::string    platformName = curPlatform.GetName();
      mUI.mPlatformNameComboBox->addItem(QString::fromStdString(platformName));
      mUI.mRemotePlatformComboBox->addItem(QString::fromStdString(platformName));
   }

   connect(mUI.mPlatformNameComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           this,
           &CommVisAddGatewayDialog::PlatformChanged);
   connect(mUI.mRemotePlatformComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           this,
           &CommVisAddGatewayDialog::RemotePlatformChanged);
   connect(mUI.mRouterComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           this,
           &CommVisAddGatewayDialog::RouterChanged);
}

void WizCommVis::CommVisAddGatewayDialog::PlatformChanged()
{
   // Clear the router combo box and reload it
   mUI.mRouterComboBox->setEnabled(true);
   mUI.mRouterComboBox->clear();
   mUI.mRouterComboBox->addItem("<Choose Router>");

   // Clear the comm combo box and reload it
   mUI.mGatewayCommComboBox->setEnabled(true);
   mUI.mGatewayCommComboBox->clear();
   mUI.mGatewayCommComboBox->addItem("<Choose Comm>");

   WsfPM_Root     proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + mUI.mPlatformNameComboBox->currentText().toStdString();
   if (platformNode.IsValid())
   {
      // Populate the routers
      mUI.mRouterComboBox->addItem("default");
      WsfPM_RouterMap routers = platformNode.Routers();
      for (WsfPM_RouterMap::iterator j = routers.begin(); j != routers.end(); ++j)
      {
         WsfPM_Router curRouter = routers.AtT(j);
         mUI.mRouterComboBox->addItem(QString::fromStdString(curRouter.Name()));
      }

      // Populate the Comms
      WsfPM_CommMap comms = platformNode.Comms();
      for (WsfPM_CommMap::iterator j = comms.begin(); j != comms.end(); ++j)
      {
         WsfPM_Comm curComm = comms.AtT(j);
         mUI.mGatewayCommComboBox->addItem(QString::fromStdString(curComm.Name()));
      }
   }
   else
   {
      mUI.mRouterComboBox->setEnabled(false);
      mUI.mGatewayCommComboBox->setEnabled(false);
   }
}

void WizCommVis::CommVisAddGatewayDialog::RemotePlatformChanged()
{
   // Clear the router combo box and reload it
   mUI.mRemoteCommComboBox->setEnabled(true);
   mUI.mRemoteCommComboBox->clear();
   mUI.mRemoteCommComboBox->addItem("<Choose Comm>");
   WsfPM_Root     proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + mUI.mRemotePlatformComboBox->currentText().toStdString();
   if (platformNode.IsValid())
   {
      WsfPM_CommMap comms = platformNode.Comms();
      for (WsfPM_RouterMap::iterator j = comms.begin(); j != comms.end(); ++j)
      {
         WsfPM_Comm curComm = comms.AtT(j);
         mUI.mRemoteCommComboBox->addItem(QString::fromStdString(curComm.Name()));
      }
   }
   else
   {
      mUI.mRemoteCommComboBox->setEnabled(false);
   }
}


void WizCommVis::CommVisAddGatewayDialog::RouterChanged()
{
   bool toggleOn =
      !mUI.mRouterComboBox->currentText().isEmpty() && (mUI.mRouterComboBox->currentText() != "<Choose Router>");

   ToggleLineEdits(toggleOn);
}

void WizCommVis::CommVisAddGatewayDialog::ToggleLineEdits(bool aEnabled)
{
   QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
   for (auto curLineEdit : lineEdits)
   {
      curLineEdit->setEnabled(aEnabled);
   }
   mUI.mRemotePlatformComboBox->setEnabled(aEnabled);
}

void WizCommVis::CommVisAddGatewayDialog::AddGateway()
{
   // Find the platform in the proxy
   WsfPM_Root     proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + mUI.mPlatformNameComboBox->currentText().toStdString();
   WsfPM_Router   routerNode   = platformNode.Routers() + mUI.mRouterComboBox->currentText().toStdString();
   if (routerNode.IsValid())
   {
      UtTextDocumentRange docRange = routerNode.GetSourceDocumentRange();
      if (docRange.Valid())
      {
         // Need to figure out if a remote interface was correctly set up and if so, compare the docrange of it to the
         // docrange of this router and make sure edit the "bottom" one first
         if (mUI.mGatewayCommComboBox->currentText() != "<Choose Comm>")
         {
            WsfPM_Comm          commNode = platformNode.Comms() + mUI.mGatewayCommComboBox->currentText().toStdString();
            UtTextDocumentRange commDocRange = commNode.GetSourceDocumentRange();

            if (commDocRange.Valid())
            {
               if (commDocRange.GetEnd() > docRange.GetEnd())
               {
                  EditComm(commDocRange);
                  EditRouter(docRange);
               }
               else
               {
                  EditRouter(docRange);
                  EditComm(commDocRange);
               }
            }
            else
            {
               // Need to add an edit block for this comm on this platform...
               UtTextDocumentRange platformDocRange = platformNode.GetSourceDocumentRange();
               if (platformDocRange.Valid())
               {
                  if (platformDocRange.GetEnd() > docRange.GetEnd())
                  {
                     AddEditCommToPlatform(platformDocRange);
                     EditRouter(docRange);
                  }
                  else
                  {
                     EditRouter(docRange);
                     AddEditCommToPlatform(platformDocRange);
                  }
               }
            }
         }
         else
         {
            EditRouter(docRange);
         }
      }
   }

   // If the router the user is trying to edit is not defined in the docrange of the platform, that means it is defined
   // in the platform_type or is the default router. Need to add an edit block for the router along with all other components
   else
   {
      UtTextDocumentRange platformDocRange = platformNode.GetSourceDocumentRange();
      // Need to figure out if a remote interface was correctly set up and if so, compare the docrange of it to the
      // docrange of this router and make sure edit the "bottom" one first
      if (mUI.mGatewayCommComboBox->currentText() != "<Choose Comm>")
      {
         WsfPM_Comm          commNode = platformNode.Comms() + mUI.mGatewayCommComboBox->currentText().toStdString();
         UtTextDocumentRange commDocRange = commNode.GetSourceDocumentRange();

         if (commDocRange.Valid())
         {
            if (commDocRange.GetEnd() > platformDocRange.GetEnd())
            {
               EditComm(commDocRange);
               AddEditRouterToPlatform(platformDocRange);
            }
            else
            {
               AddEditRouterToPlatform(platformDocRange);
               EditComm(commDocRange);
            }
         }
         else
         {
            AddEditRouterToPlatform(platformDocRange);
            AddEditCommToPlatform(platformDocRange);
         }
      }
      else
      {
         AddEditRouterToPlatform(platformDocRange);
      }
   }
}

void WizCommVis::CommVisAddGatewayDialog::EditRouter(const UtTextDocumentRange& aDocRange)
{
   if (aDocRange.Valid())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(aDocRange);
      QTextCursor bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()));
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr;
      if (!mUI.mGatewayCommComboBox->currentText().trimmed().isEmpty())
      {
         editBlockStr.append("\n" + spaces + "gateway " + mUI.mGatewayCommComboBox->currentText());
      }
      if (!mUI.mGatewayAddressLineEdit->text().trimmed().isEmpty())
      {
         editBlockStr.append("\n" + spaces + "gateway_address " + mUI.mGatewayAddressLineEdit->text());
      }
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

void WizCommVis::CommVisAddGatewayDialog::EditComm(const UtTextDocumentRange& aDocRange)
{
   if (aDocRange.Valid())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(aDocRange);
      QTextCursor bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()));
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr =
         spaces + "gateway " + mUI.mRemotePlatformComboBox->currentText() + " " + mUI.mRemoteCommComboBox->currentText();
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
}

void WizCommVis::CommVisAddGatewayDialog::AddEditCommToPlatform(const UtTextDocumentRange& aDocRange)
{
   if (aDocRange.Valid())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(aDocRange);
      QTextCursor bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()));
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces       = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr = spaces + "edit comm " + mUI.mGatewayCommComboBox->currentText();
      editBlockStr.append("\n" + spaces + spaces + "router_name " + mUI.mRouterComboBox->currentText());
      editBlockStr.append("\n" + spaces + spaces + "gateway " + mUI.mRemotePlatformComboBox->currentText() + " " +
                          mUI.mRemoteCommComboBox->currentText());
      editBlockStr.append("\n" + spaces + "end_comm");
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
}

void WizCommVis::CommVisAddGatewayDialog::AddEditRouterToPlatform(const UtTextDocumentRange& aDocRange)
{
   if (aDocRange.Valid())
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(aDocRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(aDocRange);
      QTextCursor bottomCursor = editorPtr->textCursor();
      bottomCursor.setPosition(ut::cast_to_int(aDocRange.GetEnd()));
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);

      QString spaces       = GetIndentString(bottomCursor.block().text().toStdString());
      QString editBlockStr = spaces + "edit router " + mUI.mRouterComboBox->currentText();

      if (mUI.mRouterComboBox->currentText() == "default")
      {
         editBlockStr = spaces + "add router default WSF_COMM_ROUTER";
      }
      if (!mUI.mGatewayCommComboBox->currentText().trimmed().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "gateway " + mUI.mGatewayCommComboBox->currentText());
      }
      if (!mUI.mGatewayAddressLineEdit->text().trimmed().isEmpty())
      {
         editBlockStr.append("\n" + spaces + spaces + "gateway_address " + mUI.mGatewayAddressLineEdit->text());
      }
      editBlockStr.append("\n" + spaces + "end_router");
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
}

void WizCommVis::CommVisAddGatewayDialog::done(int r)
{
   if (QDialog::Accepted == r)
   {
      if (mUI.mGatewayCommComboBox->currentText() != "<Choose Comm>" ||
          !mUI.mGatewayAddressLineEdit->text().trimmed().isEmpty())
      {
         AddGateway();
         QDialog::done(r);
      }
      else
      {
         QMessageBox::critical(
            this,
            "Error Adding Gateway",
            "The Gateway Name or Gateway Address field needs to be filled out in order to add a new Gateway.",
            QMessageBox::Ok);
      }
   }
   else
   {
      QDialog::done(r);
   }
}
