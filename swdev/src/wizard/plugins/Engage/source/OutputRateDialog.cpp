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

#include "OutputRateDialog.hpp"

#include "OutputRateItem.hpp"

Engage::OutputRateDialog::OutputRateDialog(OutputRateItem& aItem, QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
   , mOutputRateItem(aItem)
{
   mUi.setupUi(this);
   connect(mUi.mAddButton, &QPushButton::clicked, this, &OutputRateDialog::AddRateCB);
   connect(mUi.mRemoveButton, &QPushButton::clicked, this, &OutputRateDialog::RemoveRateCB);

   // populate the fields if necessary
   if (mOutputRateItem.GetFileName() != "")
   {
      mUi.mOutputRateNameLineEdit->setText(mOutputRateItem.GetFileName());

      for (const auto& curVar : mOutputRateItem.GetItems())
      {
         // create the row necessary
         AddRateCB();
         int        curRow        = mUi.mOutputRateTableWidget->rowCount() - 1;
         QLineEdit* timeLE        = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(curRow, 0));
         QLineEdit* timeUnitsLE   = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(curRow, 1));
         QLineEdit* periodLE      = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(curRow, 2));
         QLineEdit* periodUnitsLE = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(curRow, 3));

         timeLE->setText(curVar.mTime);
         timeUnitsLE->setText(curVar.mTimeUnits);
         periodLE->setText(curVar.mPeriod);
         periodUnitsLE->setText(curVar.mPeriodUnits);
      }
   }
}

void Engage::OutputRateDialog::accept()
{
   mOutputRateItem.SetFileName(mUi.mOutputRateNameLineEdit->text());
   mOutputRateItem.ClearItems();

   for (int i = 0; i < mUi.mOutputRateTableWidget->rowCount(); i++)
   {
      QLineEdit* timeLE        = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(i, 0));
      QLineEdit* timeUnitsLE   = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(i, 1));
      QLineEdit* periodLE      = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(i, 2));
      QLineEdit* periodUnitsLE = dynamic_cast<QLineEdit*>(mUi.mOutputRateTableWidget->cellWidget(i, 3));


      mOutputRateItem.AddItem(mUi.mOutputRateNameLineEdit->text(),
                              timeLE->text(),
                              timeUnitsLE->text(),
                              periodLE->text(),
                              periodUnitsLE->text());
   }
   QDialog::accept();
}

void Engage::OutputRateDialog::AddRateCB()
{
   int curRow = mUi.mOutputRateTableWidget->rowCount();
   mUi.mOutputRateTableWidget->insertRow(curRow);
   QLineEdit* timeLineEdit = new QLineEdit(mUi.mOutputRateTableWidget);
   mUi.mOutputRateTableWidget->setCellWidget(curRow, 0, timeLineEdit);

   QLineEdit* timeUnitsLineEdit = new QLineEdit(mUi.mOutputRateTableWidget);
   mUi.mOutputRateTableWidget->setCellWidget(curRow, 1, timeUnitsLineEdit);

   QLineEdit* periodLineEdit = new QLineEdit(mUi.mOutputRateTableWidget);
   mUi.mOutputRateTableWidget->setCellWidget(curRow, 2, periodLineEdit);

   QLineEdit* periodUnitsLineEdit = new QLineEdit(mUi.mOutputRateTableWidget);
   mUi.mOutputRateTableWidget->setCellWidget(curRow, 3, periodUnitsLineEdit);
}

void Engage::OutputRateDialog::RemoveRateCB()
{
   mUi.mOutputRateTableWidget->removeRow(mUi.mOutputRateTableWidget->currentRow());
}
