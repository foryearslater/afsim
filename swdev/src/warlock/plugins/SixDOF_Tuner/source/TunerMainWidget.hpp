// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TUNERMAINDIALOG_HPP
#define TUNERMAINDIALOG_HPP

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QTextStream>

#include "TunerSimInterface.hpp"
#include "WkfUnitComboBox.hpp"
#include "ui_TunerMainWidget.h"

class UtQtGL2DPlotWidget;

namespace wkf
{
class Platform;
}

namespace WkSixDOF_Tuner
{
class MainWidget : public QWidget
{
   Q_OBJECT

public:
   MainWidget(SimInterface* aSimInterface, QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~MainWidget() override;

   void Export();

   void SaveAndClose();

   void UpdateData();

   void SimulationInitializing();
   void SetExportFile(const QString& aFileName) { mExportFileName = aFileName; }

signals:
   void InitializeAutoTune(int aPid);

protected:
   void showEvent(QShowEvent* aEvent) override;
   void closeEvent(QCloseEvent* aEvent) override;

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatformPtr);
   void SetPlatformOfInterest(const QString& aPlatformName);
   void SendPositionPressed();
   void AltitudeFreezeToggled(bool aInput);
   void PositionFreezeToggled(bool aInput);
   void SpeedFreezeToggled(bool aInput);
   void PitchFreezeToggled(bool aInput);
   void RollFreezeToggled(bool aInput);
   void YawFreezeToggled(bool aInput);
   void FuelFreezeToggled(bool aInput);

   void PID_SelectionChanged(int aInput);
   void PInputChanged();
   void IInputChanged();
   void DInputChanged();
   void AlphaInputChanged();
   void EnableMaxAccumulationToggled(bool aInput);
   void MaxAccumulationChanged();

   void ClearPlotButtonPressed();
   void ContinuousCommand1ButtonPressed();
   void ContinuousCommand2ButtonPressed();
   void ExportButtonPressed();
   void TimeManagedCommandButtonPressed();
   void Pid1ButtonPressed();
   void Pid2ButtonPressed();
   void Pid3ButtonPressed();
   void UpdateUnitComboBox(int aInput);

   void AutotunePIDButtonPressed();
   void AutoTuneModelButtonPressed();

   void AdvanceAutoTuneModel();
   void HandleAdvanceToTimeComplete();
   void HandleAutoTuneComplete(int aPid);
   void HandleAutoTuneCanceled();
   void HandleNewVehicle(const std::string& aName, int aId);
   void HandleDeletedVehicle(const std::string& aName);

   enum SeriesEnum
   {
      SERIES_OUTPUT = 1,
      SERIES_SETPOINT,
      SERIES_D,
      SERIES_I,
      SERIES_P,
      SERIES_FF
   };

   unsigned int GetTickMarkSize(double aX_AxisWidth);

   void InitializePidSelectionComboBox(int aPlatformIndex);
   void InitializeData();
   void InitializeLimitDisplay(QLabel*        aLimitLabel,
                               QLineEdit*     aLineEdit,
                               const QString& aText,
                               double         aLimitValue,
                               QLabel*        aUnitLabel,
                               const QString& aLimitUnits);
   void InitializePidButtons();
   void InitializePlot(UtQtGL2DPlotWidget* aPlotWidget);
   void InitializeUnitComboBoxes();
   void InitializeSpeedUnitComboBox();
   void HandleFreezeIcon();

   void PopulateResponsePlotYAxisLabel();

   void SetUpAutoTune(wsf::six_dof::Pid::Type aPid);

   void    WriteFile(QString aFilename);
   QString WriteG_LimitsFile(const QString& aFilename, const wsf::six_dof::AutopilotLimitsAndSettings& aLimits);
   void    WritePIDBlock(QTextStream&                                 aStream,
                         const QString&                               aBlockName,
                         const QString&                               aBlockWhiteSpace,
                         std::map<double, wsf::six_dof::PidGainData>& aGainData,
                         double                                       aUpdateInterval);

   // Selected PID is the PID that the user has selected in the pidSelectionComboBox
   wsf::six_dof::Pid::Type mSelectedPID = static_cast<wsf::six_dof::Pid::Type>(1);

   // Displayed PID is the PID whose data is currently being displayed, this can be
   // different than the selected PID. For example, if the user selects Altitude PID,
   // it depends on Alpha and Vertical Speed, the user can then choose to display the
   // data for the Alpha pid
   wsf::six_dof::Pid::Type mDisplayedPID = static_cast<wsf::six_dof::Pid::Type>(1);
   ;

   int    mSelectedPlatformId       = -1;
   double mSelectedControllingValue = 0.0;
   double mLastSimTime              = -1.0;
   bool   mModified                 = false;
   bool   mRetrieveData             = true;
   size_t mAutoTuneModelPidIndex    = 0;

   std::map<QString, int> mVehicleMap;

   SimInterface* mSimInterface{nullptr};

   Ui::TunerMainWidget mUi;

   wkf::UnitComboBox* continuousCommand1UnitComboBox{nullptr};
   wkf::UnitComboBox* continuousCommand2UnitComboBox{nullptr};
   wkf::UnitComboBox* timeManagedCommandUnitComboBox{nullptr};
   QComboBox*         speedUnitsComboBox{nullptr};
   QProgressDialog*   mAutoTuneProgressDialog{nullptr};

   QString mExportFileName = "";

   bool mInitialized = false;
};
} // namespace WkSixDOF_Tuner
#endif
