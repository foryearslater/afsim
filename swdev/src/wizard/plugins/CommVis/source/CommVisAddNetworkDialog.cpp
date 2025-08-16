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


#include "CommVisAddNetworkDialog.hpp"

#include <QComboBox>
#include <QTextBlock>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Platform.hpp"


WizCommVis::CommVisAddNetworkDialog::CommVisAddNetworkDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : QDialog(aParent, aFlags)
{
   setAttribute(Qt::WA_DeleteOnClose);
   mUI.setupUi(this);

   Initialize();
}

void WizCommVis::CommVisAddNetworkDialog::Initialize()
{
   connect(mUI.mFileLocationLineEdit, &QLineEdit::textChanged, this, &CommVisAddNetworkDialog::FileChanged);
   connect(mUI.mFileDialogButton, &QPushButton::clicked, this, &CommVisAddNetworkDialog::BrowseFiles);
}

void WizCommVis::CommVisAddNetworkDialog::FileChanged()
{
   ToggleLineEdits(!mUI.mFileLocationLineEdit->text().isEmpty());
}

void WizCommVis::CommVisAddNetworkDialog::ToggleLineEdits(bool aEnabled)
{
   QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
   for (auto curLineEdit : lineEdits)
   {
      curLineEdit->setEnabled(aEnabled);
   }
}

void WizCommVis::CommVisAddNetworkDialog::BrowseFiles()
{
   QString file = QFileDialog::getOpenFileName(this, "Open File", QString(), "Text (*.txt);;");
   if (!file.isEmpty())
   {
      mUI.mFileLocationLineEdit->setText(file);
   }
}


void WizCommVis::CommVisAddNetworkDialog::AddNetwork()
{
   if (!mUI.mFileLocationLineEdit->text().isEmpty())
   {
      QString             file2      = mUI.mFileLocationLineEdit->text();
      wizard::Editor*     editorPtr2 = wizard::Project::Instance()->GotoFile(file2.toStdString());
      UtTextDocumentRange docRange(editorPtr2->GetSource(),
                                   editorPtr2->document()->lastBlock().position(),
                                   editorPtr2->document()->lastBlock().position());
      QString             startBlockStr = "network " + mUI.mNetworkNameLineEdit->text();
      QString             endBlockStr   = "end_network";
      if (!mUI.mNetworkTypeLineEdit->text().isEmpty())
      {
         startBlockStr.append(" " + mUI.mNetworkTypeLineEdit->text());
      }
      AddNewDevice(docRange, startBlockStr, endBlockStr);
   }
}

const QString WizCommVis::CommVisAddNetworkDialog::GetAttributeString(int aTabSize) const
{
   QString spaces;
   spaces.resize(ut::cast_to_int(aTabSize), ' ');

   QString editBlockStr;
   if (!mUI.mNetworkAddressLineEdit->text().isEmpty())
   {
      editBlockStr.append("\n" + spaces + "network_address " + mUI.mNetworkAddressLineEdit->text());
   }
   return editBlockStr;
}


void WizCommVis::CommVisAddNetworkDialog::done(int r)
{
   if (QDialog::Accepted == r)
   {
      if (!mUI.mNetworkNameLineEdit->text().trimmed().isEmpty() && !mUI.mNetworkTypeLineEdit->text().trimmed().isEmpty())
      {
         AddNetwork();
         QDialog::done(r);
      }
      else
      {
         QMessageBox::critical(
            this,
            "Error Adding Network",
            "The Network Name and Network Type fields need to be filled out in order to add a new Network.",
            QMessageBox::Ok);
      }
   }
   else
   {
      QDialog::done(r);
   }
}
