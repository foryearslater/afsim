// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RunOutputDialog.hpp"

#include "OutputItem.hpp"

Engage::RunOutputDialog::RunOutputDialog(OutputItem& aItem, QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
   , mOutputItem(aItem)
{
   mUi.setupUi(this);
   connect(mUi.mAddVariableButton, &QPushButton::clicked, this, &RunOutputDialog::AddVariableCB);
   connect(mUi.mRemoveVariableButton, &QPushButton::clicked, this, &RunOutputDialog::RemoveVariableCB);

   // populate the fields if necessary
   if (mOutputItem.GetFileName() != "" || mOutputItem.GetPhase() != "")
   {
      mUi.mFileName->setText(mOutputItem.GetFileName());
      mUi.mPhaseComboBox->setCurrentText(mOutputItem.GetPhase());
      mUi.mAddEventOutput->setChecked(mOutputItem.GetAddEventOutput());
      mUi.mAddSummaryOutput->setChecked(mOutputItem.GetAddSummaryOutput());

      std::vector<OutputVariable*> items = mOutputItem.GetItems();
      for (auto&& itemIter : items)
      {
         OutputVariable* curVar = itemIter;
         // create the row necessary
         AddVariableCB();
         int        curRow   = mUi.mVariableTableWidget->rowCount() - 1;
         QLineEdit* nameLE   = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(curRow, 0));
         QLineEdit* unitsLE  = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(curRow, 1));
         QLineEdit* formatLE = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(curRow, 2));

         nameLE->setText(curVar->mName);
         unitsLE->setText(curVar->mUnits);
         formatLE->setText(curVar->mFormat);
      }
   }
}

void Engage::RunOutputDialog::accept()
{
   mOutputItem.SetFileName(mUi.mFileName->text());
   mOutputItem.SetPhase(mUi.mPhaseComboBox->currentText());
   mOutputItem.SetAddEventOutput(mUi.mAddEventOutput->isChecked());
   mOutputItem.SetAddSummaryOutput(mUi.mAddSummaryOutput->isChecked());

   mOutputItem.ClearItems();

   for (int i = 0; i < mUi.mVariableTableWidget->rowCount(); i++)
   {
      QLineEdit* nameLE   = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(i, 0));
      QLineEdit* unitsLE  = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(i, 1));
      QLineEdit* formatLE = dynamic_cast<QLineEdit*>(mUi.mVariableTableWidget->cellWidget(i, 2));

      mOutputItem.AddItem(nameLE->text(), unitsLE->text(), formatLE->text());
   }
   QDialog::accept();
}

void Engage::RunOutputDialog::AddVariableCB()
{
   int curRow = mUi.mVariableTableWidget->rowCount();
   mUi.mVariableTableWidget->insertRow(curRow);
   QLineEdit* nameLineEdit = new QLineEdit(mUi.mVariableTableWidget);
   mUi.mVariableTableWidget->setCellWidget(curRow, 0, nameLineEdit);

   QLineEdit* unitsLineEdit = new QLineEdit(mUi.mVariableTableWidget);
   mUi.mVariableTableWidget->setCellWidget(curRow, 1, unitsLineEdit);

   QLineEdit* formatLineEdit = new QLineEdit(mUi.mVariableTableWidget);
   mUi.mVariableTableWidget->setCellWidget(curRow, 2, formatLineEdit);

   nameLineEdit->setFocus();
}

void Engage::RunOutputDialog::RemoveVariableCB()
{
   mUi.mVariableTableWidget->removeRow(mUi.mVariableTableWidget->currentRow());
}
