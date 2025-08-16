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

#ifndef ANALYSISMAPOPTIONSDIALOG_HPP
#define ANALYSISMAPOPTIONSDIALOG_HPP

#include <QDialog>

#include "ui_AnalysisMapOptionsDialog.h"

namespace SPLAT
{
class OutputFilesWidget;

class AnalysisMapOptionsDialog : public QDialog
{
   Q_OBJECT
public:
   AnalysisMapOptionsDialog(OutputFilesWidget* aParent);
   ~AnalysisMapOptionsDialog() override = default;

   void Connect();
   void ApplyChanges();
   void Populate();
   void RestoreDefaults();

   void SetSensorId(const QString& aSensorName);
   void WriteData(std::ostream& aStream);

private:
   Ui::AnalysisMapOptionsDialog mUi;
   QString                      mFileName;
   QString                      mDataTitle;
   QString                      mUnits;
   double                       mColorMin;
   double                       mColorMax;
   double                       mRedVal;
   double                       mGreenVal;
   double                       mBlueVal;
   double                       mAlphaVal;
};
} // namespace SPLAT
#endif