// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "DialogBuilderScriptCreator.hpp"

#include <iostream>

#include <QCheckBox>

#include "DialogBuilderFilterDialog.hpp"
#include "WkfEnvironment.hpp"

WkDialogBuilder::ScriptCreator::ScriptCreator(QWidget* parent /*= nullptr*/)
   : QDialog(parent)
{
   mUi.setupUi(this);

   mUi.displayNameLineEdit->setValidator(new QRegExpValidator(QRegExp("^[-a-zA-Z0-9_\\s]*$")));

   connect(mUi.browseIconPushButton, &QPushButton::clicked, this, &ScriptCreator::BrowseIconHandler);
   connect(mUi.contextComboBox,
           QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
           this,
           &ScriptCreator::ContextChanged);
   connect(mUi.numArgsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ScriptCreator::UpdateScriptArgsTable);
   connect(mUi.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(mUi.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   mUi.tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
   mUi.tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
   mUi.tableWidget->horizontalHeader()->resizeSection(1, 50);

   for (auto i : ScriptContextToStringMap)
   {
      mUi.contextComboBox->addItem(i.second);
   }
}

bool WkDialogBuilder::ScriptCreator::Execute(ScriptDefinition& aDef)
{
   Populate(aDef);
   if (exec() == QDialog::Accepted)
   {
      GetScript(aDef);
      return true;
   }
   return false;
}

void WkDialogBuilder::ScriptCreator::ContextChanged(const QString& aText)
{
   // Enable the platformTypeLineEdit only if the selection is Platform context.
   mUi.platformTypeLineEdit->setEnabled(aText == ScriptContextToStringMap.at(warlock::script::PLATFORM));
   mUi.clampPlatformSelectionCheckbox->setEnabled(aText == ScriptContextToStringMap.at(warlock::script::PLATFORM));
}

void WkDialogBuilder::ScriptCreator::GetScript(ScriptDefinition& aDef)
{
   aDef.displayName = mUi.displayNameLineEdit->text();
   aDef.icon        = mUi.displayIconLineEdit->text();
   aDef.color       = mUi.colorPushButton->GetColor();
   // If useDefaultColor is true, and the color of the button is already set,
   //  when the user changes the theme, the button color does not update.
   //  Not important enough to fix at this point in time.
   aDef.useDefaultColor        = (aDef.color == qApp->palette().color(QPalette::Button));
   aDef.scriptName             = mUi.scriptNameLineEdit->text();
   aDef.context                = (warlock::script::Context)mUi.contextComboBox->currentIndex();
   aDef.platformType           = mUi.platformTypeLineEdit->text();
   aDef.promptUser             = mUi.promptUserCheckBox->isChecked();
   aDef.link                   = mUi.linkLineEdit->text();
   aDef.clampPlatformSelection = mUi.clampPlatformSelectionCheckbox->isChecked();

   aDef.args.clear();
   aDef.filters.clear();
   for (int row = 0; row < mUi.tableWidget->rowCount(); ++row)
   {
      warlock::script::Argument arg;
      arg.data = mUi.tableWidget->item(row, 0)->text();
      aDef.args.push_back(arg);
      auto filter = qvariant_cast<warlock::script::Filter>(mUi.tableWidget->item(row, 1)->data(Qt::UserRole));
      aDef.filters.push_back(filter);
   }
}

void WkDialogBuilder::ScriptCreator::CreateAndShowFilterDialog(const int aRow)
{
   auto        data   = mUi.tableWidget->item(aRow, 1)->data(Qt::UserRole);
   auto        filter = qvariant_cast<warlock::script::Filter>(data);
   QStringList filterTypes{"Side", "Type", "Category"};

   auto dialogPtr = new FilterDialog(filterTypes, filter.type, filter.value, filter.isFiltered, filter.isInclusive, this);
   if (dialogPtr->exec())
   {
      warlock::script::Filter filter;
      filter.isFiltered  = dialogPtr->IsFiltered();
      filter.isInclusive = dialogPtr->IsInclusive();
      filter.type        = dialogPtr->GetFilterType();
      filter.value       = dialogPtr->GetFilterValue();
      QVariant f;
      f.setValue(filter);
      mUi.tableWidget->item(aRow, 1)->setData(Qt::UserRole, f);
      QString str = "Not filtered";
      if (filter.isFiltered)
      {
         str = filter.type + (filter.isInclusive ? " = " : " != ") + filter.value;
      }
      mUi.tableWidget->cellWidget(aRow, 1)->setToolTip(str);
   }
}

void WkDialogBuilder::ScriptCreator::Populate(const ScriptDefinition& aDef)
{
   mUi.displayNameLineEdit->setText(aDef.displayName);
   mUi.displayIconLineEdit->setText(aDef.icon);
   mUi.colorPushButton->SetColor(aDef.color);
   mUi.scriptNameLineEdit->setText(aDef.scriptName);
   mUi.contextComboBox->setCurrentIndex((int)aDef.context);
   mUi.platformTypeLineEdit->setText(aDef.platformType);
   mUi.promptUserCheckBox->setChecked(aDef.promptUser);
   mUi.linkLineEdit->setText(aDef.link);
   mUi.clampPlatformSelectionCheckbox->setChecked(aDef.clampPlatformSelection);

   for (int i = 0; i < aDef.args.size(); ++i)
   {
      warlock::script::Filter filter;
      if (i < aDef.filters.size())
      {
         filter = aDef.filters[i];
      }
      AddArgument(aDef.args[i], filter);
   }
   // Set this after the Arguments have been added
   mUi.numArgsSpinBox->setValue(aDef.args.size());
}

void WkDialogBuilder::ScriptCreator::AddArgument(const warlock::script::Argument& aArgument,
                                                 const warlock::script::Filter&   aFilter)
{
   // Get the new row number which is equal to the rowCount
   int row = mUi.tableWidget->rowCount();
   mUi.tableWidget->insertRow(row);
   mUi.tableWidget->setItem(row, 0, new QTableWidgetItem(aArgument.data.toString()));
   mUi.tableWidget->setItem(row, 1, new QTableWidgetItem());
   if (aArgument.type == "WsfPlatform" || aArgument.type == "WsfTrack" || aArgument.type.empty())
   {
      QPushButton* filterButton = new QPushButton("+");
      filterButton->setFixedWidth(50);

      QString str = "Not filtered";
      if (aFilter.isFiltered)
      {
         str = aFilter.type + (aFilter.isInclusive ? " = " : " != ") + aFilter.value;
      }
      filterButton->setToolTip(str);

      QVariant f;
      f.setValue(aFilter);
      mUi.tableWidget->setCellWidget(row, 1, filterButton);
      mUi.tableWidget->item(row, 1)->setData(Qt::UserRole, f);

      connect(filterButton, &QPushButton::clicked, [this, row]() { CreateAndShowFilterDialog(row); });
   }
}

void WkDialogBuilder::ScriptCreator::BrowseIconHandler()
{
   QString filepath = wkf::getOpenFileName(this, "Select the Image", "", "Images (*.png *.xpm *.jpg)");
   mUi.displayIconLineEdit->setText(filepath);
}

void WkDialogBuilder::ScriptCreator::UpdateScriptArgsTable(int aValue)
{
   int currentNumOfRows = mUi.tableWidget->rowCount();
   while (currentNumOfRows > aValue)
   {
      mUi.tableWidget->removeRow(aValue);
      currentNumOfRows--;
   }
   while (currentNumOfRows < aValue)
   {
      AddArgument(warlock::script::Argument(), warlock::script::Filter());
      currentNumOfRows++;
   }
}
