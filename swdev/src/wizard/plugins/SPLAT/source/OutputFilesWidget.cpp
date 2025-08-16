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

#include "OutputFilesWidget.hpp"

#include <ostream>

#include <QDir>

#include "AnalysisMapOptionsDialog.hpp"
#include "PlotOptionsDialog.hpp"


SPLAT::OutputFilesWidget::OutputFilesWidget(PlotType aPlotType, PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
   , mAnalysisMapDialog(new AnalysisMapOptionsDialog(this))
   , mContourLevel(ContourLevel())
   , mPlotType(aPlotType)
{
   mUi.setupUi(this);
   Initialize();
   Connect();
}

void SPLAT::OutputFilesWidget::WriteData(std::ostream& aStream)
{
   if ((mPlotType == PlotType::cHORIZONTAL_MAP) || (mPlotType == PlotType::cVERTICAL_MAP) ||
       (mPlotType == PlotType::cSPHERICAL_MAP))
   {
      if (mUi.mPdCheckBox->isChecked())
      {
         aStream << "   pd_map_file " << mPDFile.toStdString() << "\n";
      }

      if (mUi.mGnuplotCheckBox->isChecked())
      {
         aStream << "   gnuplot_file " << mGnuplotFile.toStdString() << "\n";
      }

      if (mUi.mPlayerCheckBox->isChecked())
      {
         aStream << "   gnuplot_player_file " << mGnuplotPlayerFile.toStdString() << "\n";
      }

      aStream << "\n";
      if (mPlotType == PlotType::cHORIZONTAL_MAP)
      {
         if (mUi.mContourLevelGroupBox->isChecked())
         {
            aStream << "   contour_level " << mContourLevel.mLevel << "\n";
            aStream << "      line_width " << mContourLevel.mWidth << "\n";
            aStream << "      line_color " << mContourLevel.mRedVal << " " << mContourLevel.mGreenVal << " "
                    << mContourLevel.mBlueVal << "\n";
            aStream << "   end_contour_level"
                    << "\n\n";

            if (mUi.mKMLCheckBox->isChecked())
            {
               aStream << "   kml_file " << mKML.toStdString() << "\n";
            }

            if (mUi.mShapefileCheckBox->isChecked())
            {
               aStream << "   shapefile " << mShapeFile.toStdString() << "\n";
            }

            if (mUi.mSeditCheckBox->isChecked())
            {
               aStream << "   sedit_file " << mSeditFile.toStdString() << "\n";
            }
            aStream << "\n";
         }
         if (mUi.mAnalysisMapCheckBox->isChecked())
         {
            mAnalysisMapDialog->WriteData(aStream);
            aStream << "\n";
         }
         if (mUi.mTiffCheckBox->isChecked())
         {
            aStream << "   tiff_file " << mTiffFile.toStdString() << "\n";
         }
      }
   }
   else if ((mPlotType == PlotType::cHORIZONTAL_COVERAGE) || (mPlotType == PlotType::cVERTICAL_COVERAGE))
   {
      if (!mOutputFile.isEmpty())
      {
         aStream << "   output_file " << mOutputFile.toStdString() << "\n";
      }

      if (!mAltitudeUnits.isEmpty())
      {
         aStream << "   altitude_units " << mAltitudeUnits.toStdString() << "\n";
      }

      if (!mRangeUnits.isEmpty())
      {
         aStream << "   range_units " << mRangeUnits.toStdString() << "\n";
      }

      if (!mX_Units.isEmpty())
      {
         aStream << "   x_units " << mX_Units.toStdString() << "\n";
      }

      if (!mY_Units.isEmpty())
      {
         aStream << "   y_units " << mY_Units.toStdString() << "\n";
      }

      if (mUi.mOutputMaxHeight->isChecked())
      {
         aStream << "   output_max_height\n";
      }

      if (mUi.mOutputMaxRange->isChecked())
      {
         aStream << "   output_max_range\n";
      }
   }
}

bool SPLAT::OutputFilesWidget::ApplyChanges()
{
   SetOutputFiles();

   QString gnuPlotFilename = "";
   if (mUi.mGnuplotCheckBox->isChecked())
   {
      gnuPlotFilename = mGnuplotFile;
   }
   emit GnuplotFileChanged(gnuPlotFilename);

   // TODO - is a check needed for output files and gnuplot files
   // QString outputFilename = "";
   // if (mUi.mOutputFileCheckBox->isChecked())
   //{
   //   outputFilename = mOutputFile;
   //}
   emit OutputFileChanged(mOutputFile);

   return true;
}

void SPLAT::OutputFilesWidget::Populate()
{
   mUi.mPdLineEdit->setText(mPDFile);
   mUi.mGnuplotLineEdit->setText(mGnuplotFile);
   mUi.mPlayerLineEdit->setText(mGnuplotPlayerFile);
   mUi.mReportFileLineEdit->setText(mReportFile);
   mUi.mTiffLineEdit->setText(mTiffFile);

   mUi.mKMLLineEdit->setText(mKML);
   mUi.mShapefileLineEdit->setText(mShapeFile);
   mUi.mSeditLineEdit->setText(mSeditFile);

   mUi.mWidthSpinBox->setValue(mContourLevel.mWidth);
   mUi.mLevelSpinBox->setValue(mContourLevel.mLevel);
   mUi.mRedSpinBox->setValue(mContourLevel.mRedVal);
   mUi.mGreenSpinBox->setValue(mContourLevel.mGreenVal);
   mUi.mBlueSpinBox->setValue(mContourLevel.mBlueVal);

   mUi.mOutputFileLineEdit->setText(mOutputFile);

   mUi.mAltitudeUnitsLineEdit->setText(mAltitudeUnits);
   mUi.mRangeUnitsLineEdit->setText(mRangeUnits);
   mUi.mX_UnitsLineEdit->setText(mX_Units);
   mUi.mY_UnitsLineEdit->setText(mY_Units);
}

void SPLAT::OutputFilesWidget::RestoreDefaults()
{
   Initialize();
   Populate();
}

void SPLAT::OutputFilesWidget::SetSensorId(const QString& aSensorName)
{
   // set default filenames
   if (mPlotType == PlotType::cHORIZONTAL_COVERAGE)
   {
      mOutputFile = "HCD_" + aSensorName + ".plt";
   }
   else if (mPlotType == PlotType::cHORIZONTAL_MAP)
   {
      mPDFile            = "HM_" + aSensorName + ".pd";
      mGnuplotFile       = "HM_" + aSensorName + ".plt";
      mGnuplotPlayerFile = "HM_player.plt";
      mReportFile        = "HM_report_file.txt";
      mTiffFile          = "HM_" + aSensorName + ".tif";
      mKML               = "HM_kml_" + aSensorName + ".kml";
      mShapeFile         = "HM_shapefile.shp";
      mSeditFile         = "HM_sedit.txt";
   }
   else if (mPlotType == PlotType::cSPHERICAL_MAP)
   {
      mPDFile            = "SM_" + aSensorName + ".pd";
      mGnuplotFile       = "SM_" + aSensorName + ".plt";
      mGnuplotPlayerFile = "SM_player.plt";
   }
   else if (mPlotType == PlotType::cVERTICAL_COVERAGE)
   {
      mOutputFile = "VCD_" + aSensorName + ".plt";
   }
   else if (mPlotType == PlotType::cVERTICAL_MAP)
   {
      mPDFile            = "VM_" + aSensorName + ".pd";
      mGnuplotFile       = "VM_" + aSensorName + ".plt";
      mGnuplotPlayerFile = "VM_player.plt";
   }

   mAnalysisMapDialog->SetSensorId(aSensorName);
   Populate();
}

void SPLAT::OutputFilesWidget::Initialize()
{
   mUi.mPdCheckBox->setChecked(true);
   mUi.mGnuplotCheckBox->setChecked(true);

   mUi.mPlayerLineEdit->setEnabled(false);
   mUi.mReportFileLineEdit->setEnabled(false);
   mUi.mTiffLineEdit->setEnabled(false);

   mUi.mKMLLineEdit->setEnabled(false);
   mUi.mShapefileLineEdit->setEnabled(false);
   mUi.mSeditLineEdit->setEnabled(false);

   mUi.mOutputMaxHeight->setChecked(false);
   mUi.mOutputMaxRange->setChecked(false);

   switch (mPlotType)
   {
   case PlotType::cHORIZONTAL_COVERAGE:
      mUi.mContourLevelGroupBox->hide();
      mUi.mPdGnuplotGroupBox->hide();
      mUi.mReportFilesGroupBox->hide();
      break;
   case PlotType::cHORIZONTAL_MAP:
      mUi.mOutputFilesGroupBox->hide();
      mUi.mUnitsSelectionGroupBox->hide();
      break;
   case PlotType::cSPHERICAL_MAP:
      mUi.mContourLevelGroupBox->hide();
      mUi.mOutputFilesGroupBox->hide();
      mUi.mUnitsSelectionGroupBox->hide();
      break;
   case PlotType::cVERTICAL_COVERAGE:
      mUi.mContourLevelGroupBox->hide();
      mUi.mPdGnuplotGroupBox->hide();
      mUi.mReportFilesGroupBox->hide();
      break;
   case PlotType::cVERTICAL_MAP:
      mUi.mContourLevelGroupBox->hide();
      mUi.mOutputFilesGroupBox->hide();
      mUi.mUnitsSelectionGroupBox->hide();
      break;
   default:
      break;
   }
}

void SPLAT::OutputFilesWidget::Connect()
{
   // Enable/Disable Widgets
   connect(mUi.mPdCheckBox, &QCheckBox::stateChanged, this, [this](int aState) { mUi.mPdLineEdit->setEnabled(aState); });
   connect(mUi.mGnuplotCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mGnuplotLineEdit->setEnabled(aState); });
   connect(mUi.mPlayerCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mPlayerLineEdit->setEnabled(aState); });

   connect(mUi.mReportFileCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mReportFileLineEdit->setEnabled(aState); });
   connect(mUi.mTiffCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mTiffLineEdit->setEnabled(aState); });

   connect(mUi.mKMLCheckBox, &QCheckBox::stateChanged, this, [this](int aState) { mUi.mKMLLineEdit->setEnabled(aState); });
   connect(mUi.mShapefileCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mShapefileLineEdit->setEnabled(aState); });
   connect(mUi.mSeditCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](int aState) { mUi.mSeditLineEdit->setEnabled(aState); });

   // Analysis Map Options Dialog
   connect(mUi.mOptionsButton, &QPushButton::pressed, this, [this]() { mAnalysisMapDialog->show(); });
}

void SPLAT::OutputFilesWidget::SetOutputFiles()
{
   mPDFile            = mUi.mPdLineEdit->text();
   mGnuplotFile       = mUi.mGnuplotLineEdit->text();
   mGnuplotPlayerFile = mUi.mPlayerLineEdit->text();

   mReportFile = mUi.mReportFileLineEdit->text();
   mTiffFile   = mUi.mTiffLineEdit->text();

   mContourLevel.mWidth    = mUi.mWidthSpinBox->value();
   mContourLevel.mLevel    = mUi.mLevelSpinBox->value();
   mContourLevel.mRedVal   = mUi.mRedSpinBox->value();
   mContourLevel.mGreenVal = mUi.mGreenSpinBox->value();
   mContourLevel.mBlueVal  = mUi.mBlueSpinBox->value();

   mKML       = mUi.mKMLLineEdit->text();
   mShapeFile = mUi.mShapefileLineEdit->text();
   mSeditFile = mUi.mSeditLineEdit->text();

   mOutputFile = mUi.mOutputFileLineEdit->text();

   mAltitudeUnits = mUi.mAltitudeUnitsLineEdit->text();
   mRangeUnits    = mUi.mRangeUnitsLineEdit->text();
   mX_Units       = mUi.mX_UnitsLineEdit->text();
   mY_Units       = mUi.mY_UnitsLineEdit->text();
}
