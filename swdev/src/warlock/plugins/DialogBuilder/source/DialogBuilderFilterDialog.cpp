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

#include "DialogBuilderFilterDialog.hpp"

#include "WkfEnvironment.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"

WkDialogBuilder::FilterDialog::FilterDialog(QStringList    aFilterTypes,
                                            const QString& aFilterType,
                                            const QString& aFilterValue,
                                            bool           aIsFiltered,
                                            bool           aIsInclusive,
                                            QWidget*       aParentPtr)
   : QDialog(aParentPtr)
{
   mUi.setupUi(this);

   mUi.filterGroupBox->setChecked(aIsFiltered);
   mUi.inclusiveRadioButton->setChecked(aIsInclusive);

   mUi.typeComboBox->addItems(aFilterTypes);
   if (aFilterTypes.contains(aFilterType))
   {
      mUi.typeComboBox->setCurrentText(aFilterType);
   }
   else if (!aFilterType.isEmpty())
   {
      QString warning = "Cannot filter on \"" + aFilterType + "\". Valid types are: ";
      for (int i = 0; i < aFilterTypes.size(); ++i)
      {
         warning += aFilterTypes[i];
         warning += (i < aFilterTypes.size() - 1) ? ", " : ".";
      }
      QMessageBox::warning(this, "Invalid Filter Type", warning);
   }

   PopulateFilterValues();
   mUi.valueComboBox->setCurrentText(aFilterValue);


   connect(mUi.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(mUi.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
   connect(mUi.typeComboBox, &QComboBox::currentTextChanged, this, [this](const QString&) { PopulateFilterValues(); });
}

void WkDialogBuilder::FilterDialog::PopulateFilterValues()
{
   mUi.valueComboBox->clear();
   auto type = mUi.typeComboBox->currentText();
   if (type == "Side")
   {
      auto map = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamVisibilityMap();
      for (const auto& color : map)
      {
         mUi.valueComboBox->addItem(color.first);
      }
   }
   else if (type == "Type")
   {
      for (const auto& group : wkfEnv.GetGroupings())
      {
         if (group.first.Type() == wkf::PlatformGrouping::eType)
         {
            mUi.valueComboBox->addItem(QString::fromStdString(group.first.Name()));
         }
      }
   }
   else if (type == "Category")
   {
      for (const auto& group : wkfEnv.GetGroupings())
      {
         if (group.first.Type() == wkf::PlatformGrouping::eCategory)
         {
            mUi.valueComboBox->addItem(QString::fromStdString(group.first.Name()));
         }
      }
   }
}
