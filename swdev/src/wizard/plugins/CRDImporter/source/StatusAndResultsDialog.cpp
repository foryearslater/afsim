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

#include "StatusAndResultsDialog.hpp"

#include <iostream>

#include <QDir>

StatusAndResults::Dialog::Dialog(QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
{
   mUI.setupUi(this);

   connect(mUI.cancelButton, &QPushButton::clicked, this, &Dialog::CancelButtonClicked);
   connect(mUI.closeButton, &QPushButton::clicked, this, &Dialog::CloseDialog);

   setModal(true);
   mUI.treeWidget->setColumnCount(1);
}

void StatusAndResults::Dialog::done(int r)
{
   mUI.treeWidget->clear();
   QDialog::done(r);
}

void StatusAndResults::Dialog::SetStatus(const QString& aStatus)
{
   mUI.statusLabel->setText(aStatus);
   // also disable Cancel button, since there is no longer a conversion in progress
   mUI.cancelButton->setDisabled(true);
}

void StatusAndResults::Dialog::ConversionStarted()
{
   mUI.cancelButton->setDisabled(false);
}

void StatusAndResults::Dialog::CancelButtonClicked()
{
   mImporter->CancelCurrentImport();
}

void StatusAndResults::Dialog::CloseDialog()
{
   mImporter->CancelCurrentImport();
   this->setVisible(false);
   mUI.treeWidget->clear();
   done(QDialog::Accepted);
}

void StatusAndResults::Dialog::SetCrdImporter(CrdImporter::CrdFileImporter& aCrdImporter, CrdImporter::Dialog* aDialog)
{
   mImporter       = &aCrdImporter;
   mImporterDialog = aDialog;
}

void StatusAndResults::Dialog::SetOutputTree(const std::map<std::string, std::vector<std::string>>& aInputAndOutputMap)
{
   mUI.treeWidget->clear();
   for (const auto& iter : aInputAndOutputMap)
   {
      // put current entry into the tree widget
      auto* inputFileWidget = new QTreeWidgetItem(mUI.treeWidget);
      inputFileWidget->setText(0, QString::fromStdString(iter.first));
      mUI.treeWidget->addTopLevelItem(inputFileWidget);

      for (const std::string& currFileName : iter.second)
      {
         auto* outputFileWidget = new QTreeWidgetItem(inputFileWidget);
         outputFileWidget->setText(0, QString::fromStdString(currFileName));
         inputFileWidget->addChild(outputFileWidget);
      }
   }
}
