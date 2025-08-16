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
#ifndef DialogBuilderFilterDialog_hpp
#define DialogBuilderFilterDialog_hpp

#include <QDialog>
#include <QWidget>

#include "ui_DialogBuilderFilterDialog.h"

namespace WkDialogBuilder
{
// This class is the dialog that appears when filtering platforms or tracks.
class FilterDialog : public QDialog
{
public:
   FilterDialog(QStringList    aFilterTypes,
                const QString& aFilterType,
                const QString& aFilterValue,
                bool           aIsFiltered,
                bool           aIsInclusive,
                QWidget*       aParentPtr = nullptr);


   QString GetFilterType() const { return mUi.typeComboBox->currentText(); }
   QString GetFilterValue() const { return mUi.valueComboBox->currentText(); }
   bool    IsFiltered() const { return mUi.filterGroupBox->isChecked(); }
   bool    IsInclusive() const { return mUi.inclusiveRadioButton->isChecked(); }

private:
   void PopulateFilterValues();

   Ui::DialogBuilderFilterDialog mUi;
};
} // namespace WkDialogBuilder
#endif
