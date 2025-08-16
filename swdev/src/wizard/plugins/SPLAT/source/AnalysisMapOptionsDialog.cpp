// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AnalysisMapOptionsDialog.hpp"

#include <ostream>

#include <QDir>

#include "OutputFilesWidget.hpp"


SPLAT::AnalysisMapOptionsDialog::AnalysisMapOptionsDialog(OutputFilesWidget* aParent)
   : QDialog(aParent)
   , mFileName("")
   , mDataTitle("Unknown")
   , mUnits("Unknown")
   , mColorMin(0)
   , mColorMax(1)
   , mRedVal(0)
   , mGreenVal(0)
   , mBlueVal(0)
   , mAlphaVal(1)
{
   mUi.setupUi(this);
   Connect();
   Populate();
}

void SPLAT::AnalysisMapOptionsDialog::Connect()
{
   connect(mUi.mButtonBox, &QDialogButtonBox::accepted, this, [this]() { ApplyChanges(); });
   connect(mUi.mButtonBox, &QDialogButtonBox::rejected, this, [this]() { close(); });
}

void SPLAT::AnalysisMapOptionsDialog::ApplyChanges()
{
   mFileName  = mUi.mFileNameEdit->text();
   mDataTitle = mUi.mDataTitleEdit->text();
   mUnits     = mUi.mUnitsEdit->text();

   mColorMin = mUi.mColorMinSpinBox->value();
   mColorMax = mUi.mColorMaxSpinBox->value();

   mRedVal   = mUi.mRedSpinBox->value();
   mGreenVal = mUi.mGreenSpinBox->value();
   mBlueVal  = mUi.mBlueSpinBox->value();
   mAlphaVal = mUi.mAlphaSpinBox->value();
}

void SPLAT::AnalysisMapOptionsDialog::Populate()
{
   mUi.mFileNameEdit->setText(mFileName);
   mUi.mDataTitleEdit->setText(mDataTitle);
   mUi.mUnitsEdit->setText(mUnits);

   mUi.mColorMinSpinBox->setValue(mColorMin);
   mUi.mColorMaxSpinBox->setValue(mColorMax);

   mUi.mRedSpinBox->setValue(mRedVal);
   mUi.mGreenSpinBox->setValue(mGreenVal);
   mUi.mBlueSpinBox->setValue(mBlueVal);
   mUi.mAlphaSpinBox->setValue(mAlphaVal);
}

void SPLAT::AnalysisMapOptionsDialog::RestoreDefaults()
{
   mDataTitle = "Unknown";
   mUnits     = "Unknown";
   mColorMin  = 0;
   mColorMax  = 1;
   mRedVal    = 0;
   mGreenVal  = 0;
   mBlueVal   = 0;
   mAlphaVal  = 1;

   Populate();
}

void SPLAT::AnalysisMapOptionsDialog::SetSensorId(const QString& aSensorName)
{
   mFileName = "analysis_map_" + aSensorName + ".txt";
   Populate();
}

void SPLAT::AnalysisMapOptionsDialog::WriteData(std::ostream& aStream)
{
   aStream << "   analysis_map\n";
   aStream << "      file " << mFileName.toStdString() << "\n";
   aStream << "      data_title " << mDataTitle.toStdString() << "\n";
   aStream << "      units " << mUnits.toStdString() << "\n";
   if (mUi.mColorRangeGroupBox->isChecked())
   {
      aStream << "      color_range " << mColorMin << " to " << mColorMax;
      aStream << " " << mRedVal << " " << mGreenVal << " " << mBlueVal << " " << mAlphaVal << "\n";
   }
   aStream << "   end_analysis_map\n";
}
