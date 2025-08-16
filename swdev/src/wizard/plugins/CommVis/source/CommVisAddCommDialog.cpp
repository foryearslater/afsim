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


#include "CommVisAddCommDialog.hpp"

#include <QComboBox>
#include <QTextBlock>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Platform.hpp"

WizCommVis::CommVisAddCommDialog::CommVisAddCommDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : QDialog(aParent, aFlags)
{
   setAttribute(Qt::WA_DeleteOnClose);
   mUI.setupUi(this);

   Initialize();
}

void WizCommVis::CommVisAddCommDialog::Initialize()
{
   // Populate the platforms
   WsfPM_Root        proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_PlatformMap platforms = proxyRoot.platforms();

   for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
   {
      WsfPM_Platform curPlatform  = platforms.AtT(i);
      std::string    platformName = curPlatform.GetName();
      mUI.mPlatformNameComboBox->addItem(QString::fromStdString(platformName));
   }

   connect(mUI.mPlatformNameComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           this,
           &CommVisAddCommDialog::ToggleLineEdits);
}

void WizCommVis::CommVisAddCommDialog::ToggleLineEdits()
{
   bool              enable    = (mUI.mPlatformNameComboBox->currentText() != "<Choose Platform>");
   QList<QLineEdit*> lineEdits = this->findChildren<QLineEdit*>();
   for (auto curLineEdit : lineEdits)
   {
      curLineEdit->setEnabled(enable);
   }
}

void WizCommVis::CommVisAddCommDialog::AddComm()
{
   // Find the platform in the proxy
   WsfPM_Root     proxyRoot(wizard::Project::Instance()->Proxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + mUI.mPlatformNameComboBox->currentText().toStdString();

   if (platformNode.IsValid())
   {
      // Find the document this platform is in and populate that into the filename field
      UtTextDocumentRange docRange = platformNode.GetSourceDocumentRange();
      if (docRange.Valid())
      {
         QString startBlockStr = "add comm " + mUI.mCommNameLineEdit->text();
         QString endBlockStr   = "end_comm";
         if (!mUI.mCommTypeLineEdit->text().isEmpty())
         {
            startBlockStr.append(" " + mUI.mCommTypeLineEdit->text());
         }

         AddNewDevice(docRange, startBlockStr, endBlockStr);
      }
   }
}

const QString WizCommVis::CommVisAddCommDialog::GetAttributeString(int aTabSize) const
{
   QString spaces;
   spaces.resize(ut::cast_to_int(aTabSize), ' ');

   QString editBlockStr;
   if (!mUI.mNetworkAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "network_address " + mUI.mNetworkAddressLineEdit->text());
   }
   if (!mUI.mAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "address " + mUI.mAddressLineEdit->text());
   }
   if (!mUI.mNetworkNameLineEdit->text().isEmpty() && mUI.mNetworkNameLineEdit->text().toLower() != "default")
   {
      editBlockStr.append("\n" + spaces + "network_name " + mUI.mNetworkNameLineEdit->text());
   }
   if (!mUI.mLinkPlatformLineEdit->text().isEmpty() && !mUI.mLinkCommLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "link " + mUI.mLinkPlatformLineEdit->text() + " " +
                          mUI.mLinkCommLineEdit->text());
   }
   if (!mUI.mLocalLinkLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "local_link " + mUI.mLocalLinkLineEdit->text());
   }
   if (!mUI.mLinkAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "link_address " + mUI.mLinkAddressLineEdit->text());
   }

   if (!mUI.mRouterNameLineEdit->text().isEmpty() && mUI.mRouterNameLineEdit->text() != "default")
   {
      editBlockStr.append("\n" + spaces + "router_name " + mUI.mRouterNameLineEdit->text());
   }
   if (!mUI.mGatewayPlatformLineEdit->text().isEmpty() && !mUI.mGatewayCommLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "gateway " + mUI.mGatewayPlatformLineEdit->text() + " " +
                          mUI.mGatewayCommLineEdit->text());
   }

   return editBlockStr;
}

void WizCommVis::CommVisAddCommDialog::done(int r)
{
   if (QDialog::Accepted == r)
   {
      if (!mUI.mCommNameLineEdit->text().trimmed().isEmpty() && !mUI.mCommTypeLineEdit->text().trimmed().isEmpty() &&
          (mUI.mPlatformNameComboBox->currentText() != "<Choose Platform>"))
      {
         AddComm();
         QDialog::done(r);
         return;
      }
      else
      {
         if (mUI.mPlatformNameComboBox->currentText() == "<Choose Platform>")
         {
            QMessageBox::critical(this,
                                  "Error Adding Comm Device",
                                  "Please select a platform to add this new Comm Device to.",
                                  QMessageBox::Ok);
         }
         else
         {
            QMessageBox::critical(
               this,
               "Error Adding Comm Device",
               "The Comm Name and Comm Type fields need to be filled out in order to add a new Comm Device.",
               QMessageBox::Ok);
         }
         return;
      }
   }
   else
   {
      QDialog::done(r);
      return;
   }
}
