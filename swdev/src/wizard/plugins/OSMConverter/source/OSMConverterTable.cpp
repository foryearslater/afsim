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

#include "OSMConverterTable.hpp"

#include <QComboBox>

namespace
{

QStringList valueTypes = {"bool", "int", "double", "string", "unitary"};

// Auxiliary data filter for way data table
const std::vector<OSMConverterDataContainer::FilterData> defaultWayTableData{
   // TagKey filter, Value Filter, output name, data type
   {"minspeed", "", "minspeed", OSMConverterDataContainer::ValueType::cUNITARY},
   {"maxspeed", "", "maxspeed", OSMConverterDataContainer::ValueType::cUNITARY},
   {"surface", "", "surface", OSMConverterDataContainer::ValueType::cSTRING},
   {"lanes", "", "lanes", OSMConverterDataContainer::ValueType::cINT},
   {"width", "", "width", OSMConverterDataContainer::ValueType::cINT},
   {"oneway", "", "oneway", OSMConverterDataContainer::ValueType::cBOOL},
   {"access", "", "access", OSMConverterDataContainer::ValueType::cSTRING},
   {"highway", "", "highway", OSMConverterDataContainer::ValueType::cSTRING}};

// Auxiliary data filter for node data table
const std::vector<OSMConverterDataContainer::FilterData> defaultNodeTableData{
   {"highway", "stop", "stop", OSMConverterDataContainer::ValueType::cBOOL},
   {"highway", "traffic_signals", "traffic_signals", OSMConverterDataContainer::ValueType::cBOOL},
   {"highway", "", "highway", OSMConverterDataContainer::ValueType::cSTRING}};

} // namespace

OSMConverterTable::OSMConverterTable(QWidget* parent, Qt::WindowFlags f, TableType tableType)
   : QDialog(parent, f)
   , mTableType(tableType)
{
   mUI.setupUi(this);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   connect(mUI.mAddRowButton, &QPushButton::clicked, this, &OSMConverterTable::InsertRow);
   connect(mUI.mRemoveRowButton, &QPushButton::clicked, this, &OSMConverterTable::RemoveRow);
   connect(mUI.mClearButton, &QPushButton::clicked, [this]() { mUI.mTagTable->setRowCount(0); });
   connect(mUI.mDefaultsButton, &QPushButton::clicked, this, &OSMConverterTable::SetDefaultData);
   connect(mUI.mOkButton, &QPushButton::clicked, this, &OSMConverterTable::SaveTagData);
   connect(mUI.mCancelButton, &QPushButton::clicked, this, &OSMConverterTable::close);
}

void OSMConverterTable::SetTableData(const std::vector<OSMConverterDataContainer::FilterData>& aTableData)
{
   mTableData = aTableData;
   PopulateTableData();
}

// Show previously applied tag values when the dialog is opened
void OSMConverterTable::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);
   mUI.mTagTable->setRowCount(static_cast<int>(mTableData.size()));
   PopulateTableData();
}

void OSMConverterTable::SaveTagData()
{
   mTableData.clear();
   int rowCount = mUI.mTagTable->rowCount();
   for (int i = 0; i < rowCount; ++i)
   {
      OSMConverterDataContainer::FilterData rowData;
      rowData.mTagKey        = mUI.mTagTable->item(i, 0)->text();
      rowData.mValue         = mUI.mTagTable->item(i, 1)->text();
      rowData.mOutputName    = mUI.mTagTable->item(i, 2)->text();
      QComboBox* rowValueBox = dynamic_cast<QComboBox*>(mUI.mTagTable->cellWidget(i, 3));
      rowData.mValueType = rowValueBox ? static_cast<OSMConverterDataContainer::ValueType>(rowValueBox->currentIndex()) :
                                         OSMConverterDataContainer::ValueType::cINT;
      mTableData.emplace_back(rowData);
   }
   hide();
}

void OSMConverterTable::SetDefaultData()
{
   switch (mTableType)
   {
   case TableType::cWAY:
      SetTableData(defaultWayTableData);
      break;
   case TableType::cNODE:
      SetTableData(defaultNodeTableData);
      break;
   default:
      break;
   }
}

void OSMConverterTable::RemoveRow()
{
   if (mUI.mTagTable->rowCount() > 0)
   {
      int  remRow   = mUI.mTagTable->rowCount() - 1;
      bool selected = false;
      if (mUI.mTagTable->currentRow() >= 0)
      {
         remRow   = mUI.mTagTable->currentRow();
         selected = true;
      }

      mUI.mTagTable->removeRow(remRow);
      if (selected && (remRow > 0))
      {
         mUI.mTagTable->selectRow(remRow - 1);
      }
   }
}

void OSMConverterTable::InsertRow()
{
   int  newRow   = mUI.mTagTable->rowCount();
   bool selected = false;
   if (mUI.mTagTable->currentRow() >= 0)
   {
      newRow   = mUI.mTagTable->currentRow() + 1;
      selected = true;
   }

   mUI.mTagTable->insertRow(newRow);
   QTableWidgetItem* rowTagKey     = new QTableWidgetItem("");
   QTableWidgetItem* rowValueKey   = new QTableWidgetItem("");
   QTableWidgetItem* rowOutputName = new QTableWidgetItem("");
   QComboBox*        rowValueBox   = new QComboBox();
   rowValueBox->addItems(valueTypes);
   mUI.mTagTable->setItem(newRow, 0, rowTagKey);
   mUI.mTagTable->setItem(newRow, 1, rowValueKey);
   mUI.mTagTable->setItem(newRow, 2, rowOutputName);
   mUI.mTagTable->setCellWidget(newRow, 3, rowValueBox);
   if (selected && (newRow >= 0))
   {
      mUI.mTagTable->selectRow(newRow);
   }
}

void OSMConverterTable::PopulateTableData()
{
   // Populate the table with currently saved data (if possible)
   if (!mTableData.empty())
   {
      mUI.mTagTable->setRowCount(0);
   }

   for (unsigned int i = 0; i < mTableData.size(); ++i)
   {
      InsertRow();
      QTableWidgetItem* rowTagKey     = new QTableWidgetItem(mTableData[i].mTagKey);
      QTableWidgetItem* rowValueKey   = new QTableWidgetItem(mTableData[i].mValue);
      QTableWidgetItem* rowOutputName = new QTableWidgetItem(mTableData[i].mOutputName);
      QComboBox*        rowValueBox   = new QComboBox();
      rowValueBox->addItems(valueTypes);
      rowValueBox->setCurrentIndex(static_cast<int>(mTableData[i].mValueType));
      mUI.mTagTable->setItem(i, 0, rowTagKey);
      mUI.mTagTable->setItem(i, 1, rowValueKey);
      mUI.mTagTable->setItem(i, 2, rowOutputName);
      mUI.mTagTable->setCellWidget(i, 3, rowValueBox);
   }
}
