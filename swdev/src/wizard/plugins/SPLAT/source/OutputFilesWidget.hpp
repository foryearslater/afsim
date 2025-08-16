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

#ifndef OUTPUTFILESWIDGET_HPP
#define OUTPUTFILESWIDGET_HPP

#include "PlotOptionsWidget.hpp"
#include "ui_OutputFilesWidget.h"

namespace SPLAT
{
class AnalysisMapOptionsDialog;

class OutputFilesWidget : public PlotOptionsWidget
{
   Q_OBJECT
   struct ContourLevel
   {
      double       mLevel;
      unsigned int mWidth;
      double       mRedVal;
      double       mGreenVal;
      double       mBlueVal;

      ContourLevel()
         : mLevel(0.5)
         , mWidth(3)
         , mRedVal(0)
         , mGreenVal(0)
         , mBlueVal(0)
      {
      }
   };

public:
   OutputFilesWidget(PlotType aPlotType, PlotOptionsDialog* aParent);
   ~OutputFilesWidget() override = default;
   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void Populate();
   void RestoreDefaults() override;
   void SetSensorId(const QString& aSensorName);

signals:
   void GnuplotFileChanged(const QString& aFilename);
   void OutputFileChanged(const QString& aFilename);

private:
   void Initialize();
   void Connect();
   void SetOutputFiles();

   Ui::OutputFilesWidget     mUi;
   AnalysisMapOptionsDialog* mAnalysisMapDialog; //!< Parented by Qt
   ContourLevel              mContourLevel;
   PlotType                  mPlotType;

   // Pd, GnuPlot & Player Files
   QString mPDFile{""};
   QString mGnuplotFile{""};
   QString mGnuplotPlayerFile{""};

   // Report File
   QString mReportFile{""};

   QString mTiffFile{""};

   // Contour Plots (Horizontal Map only)
   QString mKML{""};
   QString mShapeFile{""};
   QString mSeditFile{""};

   // Output Files/Base (Coverage Diagrams Only)
   QString mOutputFile{""};

   // Units and Selection
   QString mAltitudeUnits{"m"};
   QString mRangeUnits{"m"};
   QString mX_Units{"m"};
   QString mY_Units{"m"};

   // QString mSensorName;
   // unsigned int mRunNumber;
};
} // namespace SPLAT
#endif
