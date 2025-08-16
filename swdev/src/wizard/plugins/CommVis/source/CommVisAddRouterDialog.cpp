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


#include "CommVisAddRouterDialog.hpp"

#include <QComboBox>
#include <QTextBlock>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Platform.hpp"


WizCommVis::CommVisAddRouterDialog::CommVisAddRouterDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : QDialog(aParent, aFlags)
{
   setAttribute(Qt::WA_DeleteOnClose);
   mUI.setupUi(this);

   Initialize();
}

void WizCommVis::CommVisAddRouterDialog::Initialize()
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
           &CommVisAddRouterDialog::PlatformChanged);
}

void WizCommVis::CommVisAddRouterDialog::PlatformChanged()
{
   bool enable = (mUI.mPlatformNameComboBox->currentText() != "<Choose Platform>");
   ToggleLineEdits(enable);
   ToggleCheckBoxes(enable);
}

void WizCommVis::CommVisAddRouterDialog::ToggleLineEdits(bool aEnabled)
{
   QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
   for (auto curLineEdit : lineEdits)
   {
      curLineEdit->setEnabled(aEnabled);
   }
}

void WizCommVis::CommVisAddRouterDialog::ToggleCheckBoxes(bool aEnabled)
{
   QList<QCheckBox*> checkBoxes = findChildren<QCheckBox*>();
   for (auto curCheckBox : checkBoxes)
   {
      curCheckBox->setEnabled(aEnabled);
   }
}

void WizCommVis::CommVisAddRouterDialog::AddRouter()
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
         QString startBlockStr = "add router " + mUI.mRouterNameLineEdit->text();
         QString endBlockStr   = "end_router";
         if (!mUI.mRouterTypeLineEdit->text().isEmpty())
         {
            startBlockStr.append(" " + mUI.mRouterTypeLineEdit->text());
         }
         AddNewDevice(docRange, startBlockStr, endBlockStr);

         // Need to find the comm on the platform that was designated for this router and set its router name
         WsfPM_CommMap comms = platformNode.Comms();

         // Iterate through the comms and gather information about them
         for (WsfPM_CommMap::iterator j = comms.begin(); j != comms.end(); ++j)
         {
            auto curComm = comms.AtT(j);
            if (curComm.Name() == mUI.mGatewayLineEdit->text().toStdString())
            {
               if (curComm.RouterName().empty())
               {
                  AddNewAttribute("router_name " + mUI.mRouterNameLineEdit->text(), curComm);
               }
               else
               {
                  UtTextDocumentRange commDocRange = curComm.GetSourceDocumentRange();
                  if (commDocRange.Valid())
                  {
                     EditAttribute("router_name " + mUI.mRouterNameLineEdit->text(), commDocRange);
                  }
               }
            }
         }
      }
   }
}

const QString WizCommVis::CommVisAddRouterDialog::GetAttributeString(int aTabSize) const
{
   QString spaces;
   spaces.resize(ut::cast_to_int(aTabSize), ' ');

   QString editBlockStr;
   if (!mUI.mGatewayLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "gateway " + mUI.mGatewayLineEdit->text());
   }
   if (!mUI.mGatewayAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "gateway_address " + mUI.mGatewayAddressLineEdit->text());
   }
   if (!mUI.mHopLimitLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "hop_limit " + mUI.mHopLimitLineEdit->text());
   }

   QString boolStr = mUI.mAILCheckbox->isChecked() ? "true" : "false";
   editBlockStr.append("\n" + spaces + "automated_interface_linking " + boolStr);

   boolStr = mUI.mDefaultProtocolCheckbox->isChecked() ? "true" : "false";
   editBlockStr.append("\n" + spaces + "use_default_protocol " + boolStr);

   boolStr = mUI.mMulticastProtocolCheckbox->isChecked() ? "true" : "false";
   editBlockStr.append("\n" + spaces + "use_multicast_protocol " + boolStr);

   return editBlockStr;
}

void WizCommVis::CommVisAddRouterDialog::done(int r)
{
   if (QDialog::Accepted == r)
   {
      if (!mUI.mRouterNameLineEdit->text().trimmed().isEmpty() && !mUI.mRouterTypeLineEdit->text().trimmed().isEmpty() &&
          (mUI.mPlatformNameComboBox->currentText() != "<Choose Platform>"))
      {
         AddRouter();
         QDialog::done(r);
         return;
      }
      else
      {
         if (mUI.mPlatformNameComboBox->currentText() == "<Choose Platform>")
         {
            QMessageBox::critical(this,
                                  "Error Adding Router",
                                  "Please select a platform to add this new router to.",
                                  QMessageBox::Ok);
         }
         else
         {
            QMessageBox::critical(
               this,
               "Error Adding Router",
               "The Router Name and Router Type fields need to be filled out in order to add a new Router.",
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
