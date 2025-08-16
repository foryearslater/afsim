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

#include "TunerMainWidget.hpp"

#include <iostream>

#include <QDate>
#include <QDoubleValidator>
#include <QFile>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTextStream>

#include "TunerData.hpp"
#include "UtCast.hpp"
#include "UtPath.hpp"
#include "UtSleep.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WkSixDOF_Tuner::MainWidget::MainWidget(WkSixDOF_Tuner::SimInterface* aSimInterface, QWidget* aParent, Qt::WindowFlags aFlags)
   : QWidget(aParent, aFlags)
   , mSimInterface(aSimInterface)
{
   mUi.setupUi(this);

   for (const auto& i : Data::SpeedTypeMap)
   {
      mUi.speedComboBox->addItem(i.second);
   }

   auto* validator = new QDoubleValidator(this);
   mUi.kpLineEdit->setValidator(validator);
   mUi.kiLineEdit->setValidator(validator);
   mUi.kdLineEdit->setValidator(validator);
   mUi.filterAlphaLineEdit->setValidator(validator);
   mUi.maxAccumulationLineEdit->setValidator(validator);
   mUi.innerLimit1LineEdit->setValidator(validator);
   mUi.innerLimit2LineEdit->setValidator(validator);
   mUi.middleLimit1LineEdit->setValidator(validator);
   mUi.middleLimit2LineEdit->setValidator(validator);
   mUi.altitudeLineEdit->setValidator(validator);
   mUi.speedLineEdit->setValidator(validator);
   mUi.continuousCommand1LineEdit->setValidator(validator);
   mUi.continuousCommand2LineEdit->setValidator(validator);
   mUi.timeManagedCommandValue->setValidator(validator);
   mUi.timeManagedLengthLineEdit->setValidator(validator);

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &MainWidget::PlatformOfInterestChanged);

   connect(mUi.setStatePushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::SendPositionPressed);
   connect(mUi.speedComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &WkSixDOF_Tuner::MainWidget::InitializeSpeedUnitComboBox);
   connect(mUi.altitudeFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::AltitudeFreezeToggled);
   connect(mUi.positionFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::PositionFreezeToggled);
   connect(mUi.speedFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::SpeedFreezeToggled);
   connect(mUi.pitchFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::PitchFreezeToggled);
   connect(mUi.rollFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::RollFreezeToggled);
   connect(mUi.yawFreezeCheckBox, &QCheckBox::toggled, this, &WkSixDOF_Tuner::MainWidget::YawFreezeToggled);
   connect(mUi.fuelFreezeGroupBox, &QGroupBox::toggled, this, &WkSixDOF_Tuner::MainWidget::FuelFreezeToggled);

   connect(mUi.pidSelectionComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &WkSixDOF_Tuner::MainWidget::PID_SelectionChanged);
   connect(mUi.kpLineEdit, &QLineEdit::textChanged, this, &WkSixDOF_Tuner::MainWidget::PInputChanged);
   connect(mUi.kiLineEdit, &QLineEdit::textChanged, this, &WkSixDOF_Tuner::MainWidget::IInputChanged);
   connect(mUi.kdLineEdit, &QLineEdit::textChanged, this, &WkSixDOF_Tuner::MainWidget::DInputChanged);
   connect(mUi.filterAlphaLineEdit, &QLineEdit::textChanged, this, &WkSixDOF_Tuner::MainWidget::AlphaInputChanged);
   connect(mUi.maxAccumulationGroupBox, &QGroupBox::toggled, this, &WkSixDOF_Tuner::MainWidget::EnableMaxAccumulationToggled);
   connect(mUi.maxAccumulationLineEdit, &QLineEdit::textChanged, this, &WkSixDOF_Tuner::MainWidget::MaxAccumulationChanged);

   connect(mUi.clearPlotPushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::ClearPlotButtonPressed);
   connect(mUi.continuousCommand1PushButton,
           &QPushButton::clicked,
           this,
           &WkSixDOF_Tuner::MainWidget::ContinuousCommand1ButtonPressed);
   connect(mUi.continuousCommand2PushButton,
           &QPushButton::clicked,
           this,
           &WkSixDOF_Tuner::MainWidget::ContinuousCommand2ButtonPressed);
   connect(mUi.saveExportPushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::ExportButtonPressed);
   connect(mUi.timeManagedCommandPushButton,
           &QPushButton::clicked,
           this,
           &WkSixDOF_Tuner::MainWidget::TimeManagedCommandButtonPressed);
   connect(mUi.pid1PushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::Pid1ButtonPressed);
   connect(mUi.pid2PushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::Pid2ButtonPressed);
   connect(mUi.pid3PushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::Pid3ButtonPressed);

   connect(mUi.autoTunePIDPushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::AutotunePIDButtonPressed);
   connect(mUi.autoTuneModelPushButton, &QPushButton::clicked, this, &WkSixDOF_Tuner::MainWidget::AutoTuneModelButtonPressed);

   connect(mSimInterface, &WkSixDOF_Tuner::SimInterface::NewPlatform, this, &WkSixDOF_Tuner::MainWidget::HandleNewVehicle);
   connect(mSimInterface,
           &WkSixDOF_Tuner::SimInterface::DeletedPlatform,
           this,
           &WkSixDOF_Tuner::MainWidget::HandleDeletedVehicle);
   connect(mSimInterface,
           &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
           this,
           &WkSixDOF_Tuner::MainWidget::HandleAutoTuneComplete);
   connect(mSimInterface,
           &WkSixDOF_Tuner::SimInterface::AdvanceToTimeAchieved,
           this,
           &WkSixDOF_Tuner::MainWidget::HandleAdvanceToTimeComplete);

   // Set the pid selection to the first item in the enumeration
   mUi.pidSelectionComboBox->setCurrentIndex(0);
   InitializePidButtons();

   InitializeUnitComboBoxes();
   mUi.speedComboBox->setCurrentIndex(Data::CustomCommand::KTAS);
   InitializeSpeedUnitComboBox();

   // The result of this operation isn't retained. This forces the combo box to be initialized
   // so that we can convert our displayed altitude from the design unit to the displayed unit.
   mUi.altitudeUnitsComboBox->ConvertToStandard(0);

   InitializePlot(mUi.signalPlot);
   UtQtGL2DPlot* signalPlot = mUi.signalPlot->GetPlot();
   signalPlot->SetSeriesName("P", SERIES_P);
   signalPlot->SetSeriesName("I", SERIES_I);
   signalPlot->SetSeriesName("D", SERIES_D);
   signalPlot->SetSeriesName("FF", SERIES_FF);
   signalPlot->SetSeriesName("Out", SERIES_OUTPUT);
   signalPlot->SetLegendPosition(0.93, 0.65);
   signalPlot->SetSeriesColor(QColor(255, 0, 0), SERIES_OUTPUT);
   signalPlot->SetSeriesColor(QColor(170, 30, 150), SERIES_D);
   signalPlot->SetSeriesColor(QColor(30, 170, 30), SERIES_I);
   signalPlot->SetSeriesColor(QColor(50, 150, 255), SERIES_P);
   signalPlot->SetSeriesColor(QColor(170, 170, 170), SERIES_FF);
   signalPlot->SetTitle("PID Signal Composition");
   signalPlot->SetLabelXAxis("Time (seconds)");
   signalPlot->SetLabelYAxis("Signal Magnitude");

   InitializePlot(mUi.responsePlot);
   UtQtGL2DPlot* responsePlot = mUi.responsePlot->GetPlot();
   responsePlot->SetSeriesName("Platform State", SERIES_OUTPUT);
   responsePlot->SetSeriesName("Target State", SERIES_SETPOINT);
   responsePlot->SetLegendPosition(0.78, 0.755);
   responsePlot->SetTitle("Platform Response");
   responsePlot->SetLabelXAxis("Time (seconds)");
   responsePlot->SetLabelYAxis("y");
}

WkSixDOF_Tuner::MainWidget::~MainWidget()
{
   delete continuousCommand1UnitComboBox;
   delete continuousCommand2UnitComboBox;
   delete timeManagedCommandUnitComboBox;
   delete speedUnitsComboBox;
}

void WkSixDOF_Tuner::MainWidget::showEvent(QShowEvent* aEvent)
{
   // make sure to call base class
   QWidget::showEvent(aEvent);
   mSimInterface->SetLogData(true);
}

void WkSixDOF_Tuner::MainWidget::closeEvent(QCloseEvent* aEvent)
{
   if (mModified)
   {
      if (QMessageBox::Yes ==
          QMessageBox::question(this,
                                "Unsaved Modifications",
                                "Tuning modifications were made and not saved.\nDo you want to save these changes?",
                                QMessageBox::Yes | QMessageBox::No))
      {
         Export();
      }
   }
   mSimInterface->SetLogData(false);
   mSimInterface->ClearPidLog();
   mUi.responsePlot->GetPlot()->ClearData();
   mUi.signalPlot->GetPlot()->ClearData();
}

void WkSixDOF_Tuner::MainWidget::PlatformOfInterestChanged(wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      SetPlatformOfInterest(QString::fromStdString(aPlatformPtr->GetName()));

      QString platformName = QString::fromStdString(aPlatformPtr->GetName());
   }
   else
   {
      mUi.selectedPlatformLabel->setText("None");
      mSelectedPlatformId = 0;
   }
}

void WkSixDOF_Tuner::MainWidget::SetPlatformOfInterest(const QString& aPlatformName)
{
   auto entry = mVehicleMap.find(aPlatformName);
   if (entry != mVehicleMap.end())
   {
      mUi.selectedPlatformLabel->setText(aPlatformName);
      mSelectedPlatformId = entry->second;
   }
   else
   {
      mUi.selectedPlatformLabel->setText("None");
      mSelectedPlatformId = 0;
   }

   mSimInterface->SetPlatformOfInterest(mSelectedPlatformId);

   // Update the Pre-Position state to reflect the initial test vehicle state
   double speed_fps     = 0.0;
   double latitude_deg  = 0.0;
   double longitude_deg = 0.0;
   double altitude_ft   = 0.0;
   mSimInterface->GetSpeed(mSelectedPlatformId, speed_fps);
   mSimInterface->GetPosition(mSelectedPlatformId, latitude_deg, longitude_deg, altitude_ft);

   mUi.speedComboBox->setCurrentIndex(Data::CustomCommand::KTAS);
   double speed_mps      = UtUnitLength::ConvertToStandard(speed_fps, UtUnitLength::BaseUnit::cFEET);
   double speed_guiUnits = UtUnitSpeed::ConvertFromStandard(speed_mps, mUi.speedComboBox->currentIndex());
   mUi.speedLineEdit->setText(QString::number(speed_guiUnits));

   double altitude_m         = UtUnitLength::ConvertToStandard(altitude_ft, UtUnitLength::BaseUnit::cFEET);
   double altitude_GUI_Units = UtUnitLength::ConvertFromStandard(altitude_m, mUi.altitudeUnitsComboBox->currentIndex());
   mUi.altitudeLineEdit->setText(QString::number(altitude_GUI_Units));

   InitializePidSelectionComboBox(mSelectedPlatformId);
   InitializeData();
}

void WkSixDOF_Tuner::MainWidget::SendPositionPressed()
{
   if (mSelectedPID == wsf::six_dof::Pid::TaxiSpeed || mSelectedPID == wsf::six_dof::Pid::TaxiHeading ||
       mSelectedPID == wsf::six_dof::Pid::TaxiYawRate)
   {
      mSimInterface->SendTaxiCommand(true);
   }
   else
   {
      if (mUi.speedComboBox->currentIndex() == Data::CustomCommand::Mach)
      {
         mSimInterface->SendSetPositionCommand(mUi.altitudeUnitsComboBox->ConvertToStandard(
                                                  mUi.altitudeLineEdit->text().toDouble()),
                                               (Data::CustomCommand)mUi.speedComboBox->currentIndex(),
                                               mUi.speedLineEdit->text().toDouble());
      }
      else
      {
         auto* temp = dynamic_cast<wkf::UnitComboBox*>(speedUnitsComboBox);

         if (temp != nullptr)
         {
            mSimInterface->SendSetPositionCommand(mUi.altitudeUnitsComboBox->ConvertToStandard(
                                                     mUi.altitudeLineEdit->text().toDouble()),
                                                  (Data::CustomCommand)mUi.speedComboBox->currentIndex(),
                                                  temp->ConvertToStandard(mUi.speedLineEdit->text().toDouble()));
         }
      }
   }
}

void WkSixDOF_Tuner::MainWidget::AltitudeFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_ALTITUDE, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::PositionFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_POSITION, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::SpeedFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_SPEED, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::PitchFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_PITCH, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::RollFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_ROLL, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::YawFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_YAW, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::FuelFreezeToggled(bool aInput)
{
   mSimInterface->SendFreezeCommand(WkSixDOF_Tuner::SimInterface::FRZ_FUEL, aInput);
   HandleFreezeIcon();
}

void WkSixDOF_Tuner::MainWidget::HandleFreezeIcon()
{
   if (mUi.altitudeFreezeCheckBox->isChecked() || mUi.positionFreezeCheckBox->isChecked() ||
       mUi.speedFreezeCheckBox->isChecked() || mUi.pitchFreezeCheckBox->isChecked() ||
       mUi.rollFreezeCheckBox->isChecked() || mUi.yawFreezeCheckBox->isChecked() || mUi.fuelFreezeGroupBox->isChecked())
   {
      mUi.toolBox->setItemIcon(3, QIcon::fromTheme("alert_yellow"));
   }
   else
   {
      mUi.toolBox->setItemIcon(3, QIcon());
   }
}

void WkSixDOF_Tuner::MainWidget::PID_SelectionChanged(int aInput)
{
   if (mUi.pidSelectionComboBox->count() != 0)
   {
      mSelectedPID = (wsf::six_dof::Pid::Type)mUi.pidSelectionComboBox->currentData().toInt();
   }
   else
   {
      mSelectedPID = wsf::six_dof::Pid::Unknown;
   }

   mDisplayedPID = mSelectedPID;

   InitializePidButtons();
   InitializeData();
   InitializeUnitComboBoxes();
}

void WkSixDOF_Tuner::MainWidget::PInputChanged()
{
   mModified = true;
   mSimInterface->SendPValue(mDisplayedPID, mSelectedControllingValue, mUi.kpLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::IInputChanged()
{
   mModified = true;
   mSimInterface->SendIValue(mDisplayedPID, mSelectedControllingValue, mUi.kiLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::DInputChanged()
{
   mModified = true;
   mSimInterface->SendDValue(mDisplayedPID, mSelectedControllingValue, mUi.kdLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::AlphaInputChanged()
{
   mModified = true;
   mSimInterface->SendAlphaValue(mDisplayedPID, mSelectedControllingValue, mUi.filterAlphaLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::EnableMaxAccumulationToggled(bool aInput)
{
   mSimInterface->SendMaxAccumulation(mDisplayedPID,
                                      aInput,
                                      mSelectedControllingValue,
                                      mUi.maxAccumulationLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::MaxAccumulationChanged()
{
   // mUi.maxAccumulationGroupBox->isChecked() is always true, if LineEdit changed
   mSimInterface->SendMaxAccumulation(mDisplayedPID,
                                      true,
                                      mSelectedControllingValue,
                                      mUi.maxAccumulationLineEdit->text().toDouble());
}

void WkSixDOF_Tuner::MainWidget::ExportButtonPressed()
{
   // If the preferences indicate that Warlock was opened via mover creator,
   // then change the functionality and text on the export button
   if (!mExportFileName.isEmpty())
   {
      WriteFile(mExportFileName);
      mModified = false;
      wkfEnv.GetMainWindow()->close();
   }
   else
   {
      Export();
   }
}

void WkSixDOF_Tuner::MainWidget::ClearPlotButtonPressed()
{
   mUi.signalPlot->GetPlot()->ClearData();
   mUi.signalPlot->Update(); // This forces a redraw, so the plot is cleared immediately
   mUi.responsePlot->GetPlot()->ClearData();
   mUi.responsePlot->Update(); // This forces a redraw, so the plot is cleared immediately
}

void WkSixDOF_Tuner::MainWidget::ContinuousCommand1ButtonPressed()
{
   if (mSelectedPlatformId != -1 && continuousCommand1UnitComboBox != nullptr && isVisible())
   {
      mSimInterface->SendRunCommand();
      mSimInterface->SendCommandBasedOnPID(mSelectedPID,
                                           continuousCommand1UnitComboBox->ConvertToStandard(
                                              mUi.continuousCommand1LineEdit->text().toDouble()));
   }
}

void WkSixDOF_Tuner::MainWidget::ContinuousCommand2ButtonPressed()
{
   if (mSelectedPlatformId != -1 && continuousCommand2UnitComboBox != nullptr && isVisible())
   {
      mSimInterface->SendRunCommand();
      mSimInterface->SendCommandBasedOnPID(mSelectedPID,
                                           continuousCommand2UnitComboBox->ConvertToStandard(
                                              mUi.continuousCommand2LineEdit->text().toDouble()));
   }
}

void WkSixDOF_Tuner::MainWidget::TimeManagedCommandButtonPressed()
{
   if (mSelectedPlatformId != -1 && timeManagedCommandUnitComboBox != nullptr && isVisible())
   {
      SendPositionPressed();

      mSimInterface->SendRunCommand();
      mSimInterface->SendCommandBasedOnPID(mSelectedPID,
                                           timeManagedCommandUnitComboBox->ConvertToStandard(
                                              mUi.timeManagedCommandValue->text().toDouble()));

      double endTime = mSimInterface->GetSimTime() + mUi.timeManagedLengthLineEdit->text().toDouble();
      mSimInterface->SendAdvanceToTimeCommand(endTime, mUi.runRealTimeCheckBox->isChecked());

      // Sets time window for plots equal to time managed run time
      if (mUi.timeWindowSpinBox->text().toDouble() != mUi.timeManagedLengthLineEdit->text().toDouble())
      {
         mUi.timeWindowSpinBox->setValue(mUi.timeManagedLengthLineEdit->text().toInt());
      }
      mUi.timeManagedCommandPushButton->setEnabled(false);
   }
}

void WkSixDOF_Tuner::MainWidget::Pid1ButtonPressed()
{
   mUi.pid2PushButton->setChecked(false);
   mUi.pid3PushButton->setChecked(false);
   mDisplayedPID = Data::PID_InnerMap.at(mSelectedPID);
   InitializeData();
}

void WkSixDOF_Tuner::MainWidget::Pid2ButtonPressed()
{
   mUi.pid1PushButton->setChecked(false);
   mUi.pid3PushButton->setChecked(false);
   mDisplayedPID = Data::PID_MiddleMap.at(mSelectedPID);
   InitializeData();
}

void WkSixDOF_Tuner::MainWidget::Pid3ButtonPressed()
{
   mUi.pid1PushButton->setChecked(false);
   mUi.pid2PushButton->setChecked(false);
   mDisplayedPID = Data::PID_OuterMap.at(mSelectedPID);
   InitializeData();
}

void WkSixDOF_Tuner::MainWidget::UpdateUnitComboBox(int aInput)
{
   continuousCommand1UnitComboBox->setCurrentIndex(aInput);
   continuousCommand2UnitComboBox->setCurrentIndex(aInput);
   timeManagedCommandUnitComboBox->setCurrentIndex(aInput);
}

void WkSixDOF_Tuner::MainWidget::AutotunePIDButtonPressed()
{
   if (mSelectedPlatformId != -1 && mDisplayedPID != wsf::six_dof::Pid::Unknown)
   {
      mAutoTuneProgressDialog = new QProgressDialog(this, Qt::CustomizeWindowHint);
      mAutoTuneProgressDialog->setMinimum(1);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneProgress,
              mAutoTuneProgressDialog,
              &QProgressDialog::setValue);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneMaximum,
              mAutoTuneProgressDialog,
              &QProgressDialog::setMaximum);
      mAutoTuneProgressDialog->setModal(true);
      mAutoTuneProgressDialog->setAutoClose(true);
      connect(mAutoTuneProgressDialog, &QProgressDialog::canceled, this, &WkSixDOF_Tuner::MainWidget::HandleAutoTuneCanceled);

      SetUpAutoTune(mDisplayedPID);

      mAutoTuneProgressDialog->show();
   }
}

void WkSixDOF_Tuner::MainWidget::AutoTuneModelButtonPressed()
{
   if (mSelectedPlatformId != -1)
   {
      mAutoTuneModelPidIndex = 0;
      mUi.pidSelectionComboBox->setCurrentIndex(ut::cast_to_int(mAutoTuneModelPidIndex));

      disconnect(mSimInterface,
                 &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
                 this,
                 &WkSixDOF_Tuner::MainWidget::HandleAutoTuneComplete);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
              this,
              &WkSixDOF_Tuner::MainWidget::AdvanceAutoTuneModel);

      mAutoTuneProgressDialog = new QProgressDialog(this, Qt::CustomizeWindowHint);
      mAutoTuneProgressDialog->setMinimum(1);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneProgress,
              mAutoTuneProgressDialog,
              &QProgressDialog::setValue);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneMaximum,
              mAutoTuneProgressDialog,
              &QProgressDialog::setMaximum);
      mAutoTuneProgressDialog->setModal(true);
      mAutoTuneProgressDialog->setAutoClose(true);
      connect(mAutoTuneProgressDialog, &QProgressDialog::canceled, this, &WkSixDOF_Tuner::MainWidget::HandleAutoTuneCanceled);

      int aPid = mUi.pidSelectionComboBox->itemData(ut::cast_to_int(mAutoTuneModelPidIndex)).toInt();
      SetUpAutoTune(static_cast<wsf::six_dof::Pid::Type>(aPid));

      mAutoTuneProgressDialog->show();
   }
}

void WkSixDOF_Tuner::MainWidget::AdvanceAutoTuneModel()
{
   wsf::six_dof::TunerGASpecimen aOptimum;
   int  aPid  = mUi.pidSelectionComboBox->itemData(ut::cast_to_int(mAutoTuneModelPidIndex)).toInt();
   auto tuned = static_cast<wsf::six_dof::Pid::Type>(aPid);
   mSimInterface->GetOptimumSpecimen(aOptimum);
   mSimInterface->SendPValue(tuned, mSelectedControllingValue, aOptimum.GetKp());
   mSimInterface->SendIValue(tuned, mSelectedControllingValue, aOptimum.GetKi());
   mSimInterface->SendDValue(tuned, mSelectedControllingValue, aOptimum.GetKd());
   mSimInterface->SendAlphaValue(tuned, mSelectedControllingValue, aOptimum.GetLowPassAlpha());

   if (tuned == mDisplayedPID)
   {
      mUi.kpLineEdit->setText(QString::number(aOptimum.GetKp()));
      mUi.kiLineEdit->setText(QString::number(aOptimum.GetKi()));
      mUi.kdLineEdit->setText(QString::number(aOptimum.GetKd()));
      mUi.filterAlphaLineEdit->setText(QString::number(aOptimum.GetLowPassAlpha()));
   }

   ++mAutoTuneModelPidIndex;

   // Tuner relies on the GUI-active PID, so we must switch the combo box
   // and update the pane before we start tuning.
   mUi.pidSelectionComboBox->setCurrentIndex(ut::cast_to_int(mAutoTuneModelPidIndex));

   // Give the GUI thread ample time to do what it needs before tuning.
   UtSleep::Sleep(1.0);

   aPid = mUi.pidSelectionComboBox->itemData(ut::cast_to_int(mAutoTuneModelPidIndex)).toInt();
   SetUpAutoTune(static_cast<wsf::six_dof::Pid::Type>(aPid));

   if (mAutoTuneModelPidIndex == static_cast<size_t>(mUi.pidSelectionComboBox->count() - 1))
   {
      disconnect(mSimInterface,
                 &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
                 this,
                 &WkSixDOF_Tuner::MainWidget::AdvanceAutoTuneModel);
      connect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
              this,
              &WkSixDOF_Tuner::MainWidget::HandleAutoTuneComplete);
   }
}

void WkSixDOF_Tuner::MainWidget::HandleAdvanceToTimeComplete()
{
   mUi.timeManagedCommandPushButton->setEnabled(true);
}

void WkSixDOF_Tuner::MainWidget::HandleAutoTuneComplete(int aPid)
{
   wsf::six_dof::TunerGASpecimen aOptimum;
   auto                          tuned = static_cast<wsf::six_dof::Pid::Type>(aPid);
   mSimInterface->GetOptimumSpecimen(aOptimum);
   mSimInterface->SendPValue(tuned, mSelectedControllingValue, aOptimum.GetKp());
   mSimInterface->SendIValue(tuned, mSelectedControllingValue, aOptimum.GetKi());
   mSimInterface->SendDValue(tuned, mSelectedControllingValue, aOptimum.GetKd());
   mSimInterface->SendAlphaValue(tuned, mSelectedControllingValue, aOptimum.GetLowPassAlpha());

   if (tuned == mDisplayedPID)
   {
      mUi.kpLineEdit->setText(QString::number(aOptimum.GetKp()));
      mUi.kiLineEdit->setText(QString::number(aOptimum.GetKi()));
      mUi.kdLineEdit->setText(QString::number(aOptimum.GetKd()));
      mUi.filterAlphaLineEdit->setText(QString::number(aOptimum.GetLowPassAlpha()));
   }

   mRetrieveData = true;
   disconnect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneProgress,
              mAutoTuneProgressDialog,
              &QProgressDialog::setValue);
   disconnect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneMaximum,
              mAutoTuneProgressDialog,
              &QProgressDialog::setMaximum);
   disconnect(mAutoTuneProgressDialog, &QProgressDialog::canceled, this, &WkSixDOF_Tuner::MainWidget::HandleAutoTuneCanceled);
   mAutoTuneProgressDialog->hide();
   delete mAutoTuneProgressDialog;

   // reset freezes to what is specified in the dialog
   AltitudeFreezeToggled(mUi.altitudeFreezeCheckBox->isChecked());
   SpeedFreezeToggled(mUi.speedFreezeCheckBox->isChecked());
   PositionFreezeToggled(mUi.positionFreezeCheckBox->isChecked());
   FuelFreezeToggled(mUi.fuelFreezeGroupBox->isChecked());
   RollFreezeToggled(mUi.rollFreezeCheckBox->isChecked());
   PitchFreezeToggled(mUi.pitchFreezeCheckBox->isChecked());
   YawFreezeToggled(mUi.yawFreezeCheckBox->isChecked());
}

void WkSixDOF_Tuner::MainWidget::HandleAutoTuneCanceled()
{
   mSimInterface->CancelAutoTuneCommand();
   mSimInterface->SendPValue(mDisplayedPID, mSelectedControllingValue, mUi.kpLineEdit->text().toDouble());
   mSimInterface->SendIValue(mDisplayedPID, mSelectedControllingValue, mUi.kiLineEdit->text().toDouble());
   mSimInterface->SendDValue(mDisplayedPID, mSelectedControllingValue, mUi.kdLineEdit->text().toDouble());
   mSimInterface->SendAlphaValue(mDisplayedPID, mSelectedControllingValue, mUi.filterAlphaLineEdit->text().toDouble());

   disconnect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneProgress,
              mAutoTuneProgressDialog,
              &QProgressDialog::setValue);
   disconnect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneMaximum,
              mAutoTuneProgressDialog,
              &QProgressDialog::setMaximum);
   disconnect(mAutoTuneProgressDialog, &QProgressDialog::canceled, this, &WkSixDOF_Tuner::MainWidget::HandleAutoTuneCanceled);
   delete mAutoTuneProgressDialog;
   mRetrieveData = true;

   disconnect(mSimInterface,
              &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
              this,
              &WkSixDOF_Tuner::MainWidget::AdvanceAutoTuneModel);
   connect(mSimInterface,
           &WkSixDOF_Tuner::SimInterface::AutoTuneComplete,
           this,
           &WkSixDOF_Tuner::MainWidget::HandleAutoTuneComplete);

   // reset freezes to what is specified in the dialog
   AltitudeFreezeToggled(mUi.altitudeFreezeCheckBox->isChecked());
   SpeedFreezeToggled(mUi.speedFreezeCheckBox->isChecked());
   PositionFreezeToggled(mUi.positionFreezeCheckBox->isChecked());
   FuelFreezeToggled(mUi.fuelFreezeGroupBox->isChecked());
   RollFreezeToggled(mUi.rollFreezeCheckBox->isChecked());
   PitchFreezeToggled(mUi.pitchFreezeCheckBox->isChecked());
   YawFreezeToggled(mUi.yawFreezeCheckBox->isChecked());
}

void WkSixDOF_Tuner::MainWidget::HandleNewVehicle(const std::string& aName, int aId)
{
   QString name      = QString::fromStdString(aName);
   mVehicleMap[name] = aId;

   // If the newly discovered Mover belongs to the Platform Of Interest, go ahead and update the displays.
   auto* platformOfInterest = wkfEnv.GetPlatformOfInterest();
   if (platformOfInterest && platformOfInterest->GetName() == aName)
   {
      SetPlatformOfInterest(name);
   }
}

void WkSixDOF_Tuner::MainWidget::HandleDeletedVehicle(const std::string& aName)
{
   QString name = QString::fromStdString(aName);
   if (mUi.selectedPlatformLabel->text() == name)
   {
      mUi.selectedPlatformLabel->setText("None");
   }
   mVehicleMap.erase(name);
}

void WkSixDOF_Tuner::MainWidget::SimulationInitializing()
{
   // Clear cached data in the interface, and clear the current plot data.
   mSimInterface->ClearPidLog();

   // If the preferences indicate that warlock was opened via mover creator, change the functionality
   // and text on the export button
   if (!mExportFileName.isEmpty())
   {
      mUi.saveExportPushButton->setText("Save And Exit");
   }

   InitializeData();
}

void WkSixDOF_Tuner::MainWidget::Export()
{
   QString filename = wkf::getSaveFileName(this, "Save to File", "", "*.txt");
   if (!filename.isEmpty())
   {
      WriteFile(filename);
      mModified = false;
   }
}

void WkSixDOF_Tuner::MainWidget::SaveAndClose()
{
   WriteFile(mExportFileName);
   mModified = false;
   wkfEnv.GetMainWindow()->close();
}

unsigned int WkSixDOF_Tuner::MainWidget::GetTickMarkSize(double aX_AxisWidth)
{
   if (aX_AxisWidth < 10)
   {
      return 1;
   }
   else if (aX_AxisWidth < 20)
   {
      return 2;
   }
   else if (aX_AxisWidth < 60)
   {
      return 5;
   }
   else if (aX_AxisWidth < 120)
   {
      return 10;
   }
   else if (aX_AxisWidth < 240)
   {
      return 30;
   }
   else
   {
      return 60;
   }
}

void WkSixDOF_Tuner::MainWidget::InitializePidSelectionComboBox(int aPlatformIndex)
{
   mUi.pidSelectionComboBox->clear();
   std::vector<wsf::six_dof::Pid::Type> platformPids;
   mSimInterface->GetRelevantPids(aPlatformIndex, platformPids);

   for (const auto& iter : platformPids)
   {
      mUi.pidSelectionComboBox->addItem(Data::PID_NameMap.at(iter), iter);
   }
}

void WkSixDOF_Tuner::MainWidget::InitializeData()
{
   mUi.responsePlot->GetPlot()->ClearData();
   mUi.responsePlot->GetPlot()->RemoveAllOverlayText();
   mUi.signalPlot->GetPlot()->ClearData();
   mUi.signalPlot->GetPlot()->RemoveAllOverlayText();

   // Inform the plots to update now, so that the plots are redrawn with no data.
   mUi.signalPlot->Update();
   mUi.responsePlot->Update();

   mSimInterface->SetPidOfInterest(mDisplayedPID);

   if (mSelectedPlatformId == -1)
   {
      return;
   }
   mUi.responsePlot->setWindowTitle(Data::PID_NameMap.at(mDisplayedPID));
   mUi.gainsGroupBox->setTitle(Data::PID_NameMap.at(mDisplayedPID) + " Gains");

   // Update the labels for the Send Command Buttons
   const QString& commandValueLabel = Data::PID_SendCommandLabelMap.at(mSelectedPID);
   mUi.continuousCommand1Label->setText(commandValueLabel);
   mUi.continuousCommand2Label->setText(commandValueLabel);
   mUi.timeManagedCommandLabel->setText(commandValueLabel);

   // Update the labels for the Relevant Limits Display
   double      limit1, limit2;
   std::string limitUnits;
   mSimInterface->GetRelevantLimits(mSelectedPlatformId, Data::PID_InnerMap.at(mSelectedPID), limit1, limit2, limitUnits);
   InitializeLimitDisplay(mUi.innerLimit1Label,
                          mUi.innerLimit1LineEdit,
                          Data::PID_RelevantLimit1Map.at(Data::PID_InnerMap.at(mSelectedPID)),
                          limit1,
                          mUi.innerLimit1UnitLabel,
                          QString::fromStdString(limitUnits));
   InitializeLimitDisplay(mUi.innerLimit2Label,
                          mUi.innerLimit2LineEdit,
                          Data::PID_RelevantLimit2Map.at(Data::PID_InnerMap.at(mSelectedPID)),
                          limit2,
                          mUi.innerLimit2UnitLabel,
                          QString::fromStdString(limitUnits));
   mSimInterface->GetRelevantLimits(mSelectedPlatformId, Data::PID_MiddleMap.at(mSelectedPID), limit1, limit2, limitUnits);
   InitializeLimitDisplay(mUi.middleLimit1Label,
                          mUi.middleLimit1LineEdit,
                          Data::PID_RelevantLimit2Map.at(Data::PID_MiddleMap.at(mSelectedPID)),
                          limit1,
                          mUi.middleLimit1UnitLabel,
                          QString::fromStdString(limitUnits));
   InitializeLimitDisplay(mUi.middleLimit2Label,
                          mUi.middleLimit2LineEdit,
                          Data::PID_RelevantLimit2Map.at(Data::PID_MiddleMap.at(mSelectedPID)),
                          limit2,
                          mUi.middleLimit2UnitLabel,
                          QString::fromStdString(limitUnits));

   double maxAccum;
   mSimInterface->GetMaxAccumulationValues(mSelectedPlatformId, mDisplayedPID, mSelectedControllingValue, maxAccum);

   bool useMax = (maxAccum < std::numeric_limits<float>::max());

   mUi.maxAccumulationGroupBox->setChecked(useMax);
   mUi.maxAccumulationLineEdit->setText(QString::number(maxAccum));

   double p, i, d, alpha;
   mSimInterface->GetInputValuesForPid(mSelectedPlatformId, mDisplayedPID, mSelectedControllingValue, p, i, d, alpha);
   mUi.kpLineEdit->setText(QString::number(p));
   mUi.kiLineEdit->setText(QString::number(i));
   mUi.kdLineEdit->setText(QString::number(d));
   mUi.filterAlphaLineEdit->setText(QString::number(alpha));

   PopulateResponsePlotYAxisLabel();
   UpdateData();
}

void WkSixDOF_Tuner::MainWidget::UpdateData()
{
   if (mSelectedPlatformId != -1 && mSimInterface->IsDataValid() && mRetrieveData)
   {
      UtQtGL2DPlot* signalPlot   = mUi.signalPlot->GetPlot();
      UtQtGL2DPlot* responsePlot = mUi.responsePlot->GetPlot();

      bool platformCrashed;
      mSimInterface->GetPlatformCrashed(mSelectedPlatformId, platformCrashed);

      if (platformCrashed)
      {
         signalPlot->AddOverlayText("Platform Crashed", .5, .5, QColor(255, 0, 0), 0, 12, Qt::AlignCenter);
         responsePlot->AddOverlayText("Platform Crashed", .5, .5, QColor(255, 0, 0), 0, 12, Qt::AlignCenter);
      }

      // Don't store the same data point multiple times within the plot
      double simTime = mSimInterface->GetSimTime();
      if (mLastSimTime != simTime)
      {
         mLastSimTime = simTime;

         std::vector<wsf::six_dof::TunerGASpecimen::PidLogElement> pidLog;
         mSimInterface->GetPidLog(pidLog);
         mSimInterface->ClearPidLog();

         double minX, minY, maxX, maxY;
         for (const auto& iter : pidLog)
         {
            signalPlot->AddPoint(iter.timeStamp, iter.pidValues.FFValue, SERIES_FF);
            signalPlot->AddPoint(iter.timeStamp, iter.pidValues.KdValue, SERIES_D);
            signalPlot->AddPoint(iter.timeStamp, iter.pidValues.KiValue, SERIES_I);
            signalPlot->AddPoint(iter.timeStamp, iter.pidValues.KpValue, SERIES_P);

            responsePlot->AddPoint(iter.timeStamp, iter.pidValues.CurrentValue, SERIES_OUTPUT);
            responsePlot->AddPoint(iter.timeStamp, iter.pidValues.SetPoint, SERIES_SETPOINT);
         }

         // If the signalPlot is not zoomed, resize the plot accordingly
         if (!mUi.signalPlot->IsZoomed())
         {
            signalPlot->MakePlotPrettyY();
            signalPlot->GetDataMinMax(minX, minY, maxX, maxY);
            signalPlot->SetTickIncrementX(GetTickMarkSize(mUi.timeWindowSpinBox->value()));
            signalPlot->SetXAxisBounds(std::max(maxX - mUi.timeWindowSpinBox->value(), 0.0), maxX);
         }

         // If the responsePlot is not zoomed, resize the plot accordingly
         if (!mUi.responsePlot->IsZoomed())
         {
            responsePlot->MakePlotPrettyY();
            responsePlot->GetDataMinMax(minX, minY, maxX, maxY);
            responsePlot->SetTickIncrementX(GetTickMarkSize(mUi.timeWindowSpinBox->value()));
            responsePlot->SetXAxisBounds(std::max(maxX - mUi.timeWindowSpinBox->value(), 0.0), maxX);
         }
      }
   }
}

void WkSixDOF_Tuner::MainWidget::InitializeLimitDisplay(QLabel*        aLimitLabel,
                                                        QLineEdit*     aLineEdit,
                                                        const QString& aText,
                                                        double         aLimitValue,
                                                        QLabel*        aUnitLabel,
                                                        const QString& aLimitUnits)
{
   if (aText == "")
   {
      aLimitLabel->hide();
      aLineEdit->hide();
      aUnitLabel->hide();
   }
   else
   {
      aLimitLabel->show();
      aLineEdit->show();
      aLimitLabel->setText(aText);
      aLineEdit->setText(QString::number(aLimitValue));
      aUnitLabel->show();
      aUnitLabel->setText(aLimitUnits);
   }
}

void WkSixDOF_Tuner::MainWidget::InitializePidButtons()
{
   wsf::six_dof::Pid::Type inner  = Data::PID_InnerMap.at(mSelectedPID);
   wsf::six_dof::Pid::Type middle = Data::PID_MiddleMap.at(mSelectedPID);
   wsf::six_dof::Pid::Type outer  = Data::PID_OuterMap.at(mSelectedPID);

   if (inner != wsf::six_dof::Pid::Unknown)
   {
      mUi.pid1PushButton->setText(Data::PID_NameMap.at(inner));
   }
   if (middle != wsf::six_dof::Pid::Unknown)
   {
      mUi.pid2PushButton->setText(Data::PID_NameMap.at(middle));
   }
   if (outer != wsf::six_dof::Pid::Unknown)
   {
      mUi.pid3PushButton->setText(Data::PID_NameMap.at(outer));
   }

   mUi.pid1PushButton->setVisible(true);
   mUi.pid2PushButton->setVisible(middle != wsf::six_dof::Pid::Unknown);
   mUi.pid3PushButton->setVisible(outer != wsf::six_dof::Pid::Unknown);
}

void WkSixDOF_Tuner::MainWidget::InitializePlot(UtQtGL2DPlotWidget* aPlotWidget)
{
   aPlotWidget->SetEnableCrosshairMouseControlled(true);
   UtQtGL2DPlot* plot = aPlotWidget->GetPlot();
   plot->SetAutoScaleEnabled(false);
   plot->SetDefaultPlotStyle(UtQtGL2DPlot::LINE);
   plot->SetCrosshairsShowAllSeries(true);
   plot->SetShowLegend(true);
   plot->SetTickJustifiedX(false);
   plot->SetTickIncrementX(5);
   plot->SetUseMultipleColorsForSeries(true);
}

void WkSixDOF_Tuner::MainWidget::InitializeUnitComboBoxes()
{
   if (continuousCommand1UnitComboBox != nullptr)
   {
      delete continuousCommand1UnitComboBox;
      delete continuousCommand2UnitComboBox;
      delete timeManagedCommandUnitComboBox;
   }

   switch (mSelectedPID)
   {
   case wsf::six_dof::Pid::Alpha:
   case wsf::six_dof::Pid::PitchAngle:
   case wsf::six_dof::Pid::FlightPathAngle:
   case wsf::six_dof::Pid::DeltaPitch:
   case wsf::six_dof::Pid::Beta:
   case wsf::six_dof::Pid::YawHeading:
   case wsf::six_dof::Pid::TaxiHeading:
   case wsf::six_dof::Pid::DeltaRoll:
   case wsf::six_dof::Pid::BankAngle:
   case wsf::six_dof::Pid::RollHeading:
   {
      continuousCommand1UnitComboBox = new wkf::AngleComboBox(mUi.continuousGroupBox);
      continuousCommand2UnitComboBox = new wkf::AngleComboBox(mUi.continuousGroupBox);
      timeManagedCommandUnitComboBox = new wkf::AngleComboBox(mUi.timeManagedGroupBox);
      break;
   }
   case wsf::six_dof::Pid::Speed:
   case wsf::six_dof::Pid::TaxiSpeed:
   {
      continuousCommand1UnitComboBox = new wkf::SpeedComboBox(mUi.continuousGroupBox);
      continuousCommand2UnitComboBox = new wkf::SpeedComboBox(mUi.continuousGroupBox);
      timeManagedCommandUnitComboBox = new wkf::SpeedComboBox(mUi.timeManagedGroupBox);
      break;
   }
   case wsf::six_dof::Pid::VerticalSpeed:
   {
      continuousCommand1UnitComboBox = new wkf::SpeedComboBox(mUi.continuousGroupBox);
      continuousCommand2UnitComboBox = new wkf::SpeedComboBox(mUi.continuousGroupBox);
      timeManagedCommandUnitComboBox = new wkf::SpeedComboBox(mUi.timeManagedGroupBox);

      // The first time the user changes to vertical speed, override vertical speed to fpm units.
      if (!mInitialized)
      {
         mInitialized = true;
         // Hacky fix to force the combo boxes to initialize now.
         continuousCommand1UnitComboBox->ConvertToStandard(0);
         continuousCommand2UnitComboBox->ConvertToStandard(0);
         timeManagedCommandUnitComboBox->ConvertToStandard(0);
         continuousCommand1UnitComboBox->setCurrentIndex(
            continuousCommand1UnitComboBox->findData(UtUnitSpeed::cFEET_PER_MINUTE));
         continuousCommand2UnitComboBox->setCurrentIndex(
            continuousCommand2UnitComboBox->findData(UtUnitSpeed::cFEET_PER_MINUTE));
         timeManagedCommandUnitComboBox->setCurrentIndex(
            timeManagedCommandUnitComboBox->findData(UtUnitSpeed::cFEET_PER_MINUTE));
      }
      break;
   }
   case wsf::six_dof::Pid::PitchRate:
   case wsf::six_dof::Pid::RollRate:
   case wsf::six_dof::Pid::YawRate:
   case wsf::six_dof::Pid::TaxiYawRate:
   {
      continuousCommand1UnitComboBox = new wkf::AngularRateComboBox(mUi.continuousGroupBox);
      continuousCommand2UnitComboBox = new wkf::AngularRateComboBox(mUi.continuousGroupBox);
      timeManagedCommandUnitComboBox = new wkf::AngularRateComboBox(mUi.timeManagedGroupBox);
      break;
   }
   case wsf::six_dof::Pid::Altitude:
   {
      continuousCommand1UnitComboBox = new wkf::AltitudeComboBox(mUi.continuousGroupBox);
      continuousCommand2UnitComboBox = new wkf::AltitudeComboBox(mUi.continuousGroupBox);
      timeManagedCommandUnitComboBox = new wkf::AltitudeComboBox(mUi.timeManagedGroupBox);
      break;
   }
   default:
   {
      continuousCommand1UnitComboBox = nullptr;
      continuousCommand2UnitComboBox = nullptr;
      timeManagedCommandUnitComboBox = nullptr;
      return;
   }
   }

   continuousCommand1UnitComboBox->setObjectName(QStringLiteral("continuousCommand1UnitComboBox"));
   mUi.gridLayout_7->addWidget(continuousCommand1UnitComboBox, 0, 2);

   continuousCommand2UnitComboBox->setObjectName(QStringLiteral("continuousCommand2UnitComboBox"));
   mUi.gridLayout_7->addWidget(continuousCommand2UnitComboBox, 3, 2);

   timeManagedCommandUnitComboBox->setObjectName(QStringLiteral("continuousCommand1UnitComboBox"));
   mUi.gridLayout_2->addWidget(timeManagedCommandUnitComboBox, 0, 2);

   connect(continuousCommand1UnitComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &WkSixDOF_Tuner::MainWidget::UpdateUnitComboBox);
   connect(continuousCommand2UnitComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &WkSixDOF_Tuner::MainWidget::UpdateUnitComboBox);
   connect(timeManagedCommandUnitComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &WkSixDOF_Tuner::MainWidget::UpdateUnitComboBox);
}

void WkSixDOF_Tuner::MainWidget::InitializeSpeedUnitComboBox()
{
   if (speedUnitsComboBox != nullptr)
   {
      delete speedUnitsComboBox;
   }

   switch (mUi.speedComboBox->currentIndex())
   {
   case Data::CustomCommand::KTAS:
   case Data::CustomCommand::KCAS:
      speedUnitsComboBox = new wkf::SpeedComboBox(mUi.page_3);
      break;

   case Data::CustomCommand::Mach:
      speedUnitsComboBox = new QComboBox(mUi.page_3);
      speedUnitsComboBox->addItem("mach");
      break;

   case Data::CustomCommand::DynamicPressure:
      speedUnitsComboBox = new wkf::PressureComboBox(mUi.page_3);
      break;

   default:
      speedUnitsComboBox = nullptr;
      break;
   }

   mUi.gridLayout_4->addWidget(speedUnitsComboBox, 1, 2);
}

void WkSixDOF_Tuner::MainWidget::PopulateResponsePlotYAxisLabel()
{
   std::string yAxisTitle = Data::PID_NameMap.at(mDisplayedPID).toStdString();

   switch (mDisplayedPID)
   {
   case wsf::six_dof::Pid::Alpha:
   case wsf::six_dof::Pid::PitchAngle:
   case wsf::six_dof::Pid::FlightPathAngle:
   case wsf::six_dof::Pid::DeltaPitch:
   case wsf::six_dof::Pid::Beta:
   case wsf::six_dof::Pid::YawHeading:
   case wsf::six_dof::Pid::TaxiHeading:
   case wsf::six_dof::Pid::DeltaRoll:
   case wsf::six_dof::Pid::BankAngle:
   case wsf::six_dof::Pid::RollHeading:
      yAxisTitle += " (deg)";
      break;

   case wsf::six_dof::Pid::VerticalSpeed:
      yAxisTitle += " (fpm)";
      break;

   case wsf::six_dof::Pid::PitchRate:
   case wsf::six_dof::Pid::YawRate:
   case wsf::six_dof::Pid::RollRate:
   case wsf::six_dof::Pid::TaxiYawRate:
      yAxisTitle += " (deg/s)";
      break;

   case wsf::six_dof::Pid::Altitude:
      yAxisTitle += " (feet)";
      break;

   case wsf::six_dof::Pid::Speed:
   case wsf::six_dof::Pid::TaxiSpeed:
      yAxisTitle += " (ft/s)";
      break;

   default:
      break;
   }
   mUi.responsePlot->GetPlot()->SetLabelYAxis(QString::fromStdString(yAxisTitle));
}

void WkSixDOF_Tuner::MainWidget::SetUpAutoTune(wsf::six_dof::Pid::Type aPid)
{
   auto*  altitudeUnits = dynamic_cast<wkf::AltitudeComboBox*>(mUi.altitudeUnitsComboBox);
   double altitude_m    = 0.0;
   double speed_SI      = 0.0;

   if (altitudeUnits)
   {
      double altitude_unit = mUi.altitudeLineEdit->text().toDouble();
      altitude_m           = altitudeUnits->ConvertToStandard(altitude_unit);
   }

   if (mUi.speedComboBox->currentIndex() == Data::CustomCommand::Mach)
   {
      speed_SI = mUi.speedLineEdit->text().toDouble();
   }
   else
   {
      auto* speedUnits = dynamic_cast<wkf::UnitComboBox*>(speedUnitsComboBox);

      if (speedUnits != nullptr)
      {
         speed_SI = speedUnits->ConvertToStandard(mUi.speedLineEdit->text().toDouble());
      }
   }

   mSimInterface->InitializeAutoTuneCommand(mSelectedPlatformId,
                                            aPid,
                                            altitude_m,
                                            (Data::CustomCommand)mUi.speedComboBox->currentIndex(),
                                            speed_SI);

   mSimInterface->ClearPidLog();
   mRetrieveData = false;
   emit InitializeAutoTune((int)aPid);

   mAutoTuneProgressDialog->setLabelText(QString::fromStdString("Auto-Tuning " + Data::PID_NameMap.at(aPid).toStdString()));
}

void WkSixDOF_Tuner::MainWidget::WriteFile(QString aFilename)
{
   QFile file(aFilename);
   if (file.open(QIODevice::WriteOnly))
   {
      QTextStream stream(&file);

      QDate Today = QDate::currentDate();
      stream << "# ##################################################################################################"
             << endl;
      stream << "# " << endl;
      stream << "# This file was auto-generated by the Vehicle Tuner Plug-In for AFSIM Warlock." << endl;
      stream << "# " << endl;
      stream << "# Creation Date:  " << Today.toString(Qt::RFC2822Date) << endl;
      stream << "# " << endl;
      stream << "# ##################################################################################################"
             << endl;
      stream << endl;
      stream << "# **************************************************************************************************"
             << endl;
      stream << "# This file defines the configuration of the autopilot, including its PIDs. A PID is a proportional,"
             << endl;
      stream << "# integral, derivative feedback control technique. Multiple PIDs are used to 'define' the autopilot."
             << endl;
      stream << "# Each control output may be influenced by one or more PIDs." << endl;
      stream << "# **************************************************************************************************"
             << endl;
      stream << endl;
      stream << "autopilot_config" << endl;
      stream << endl;
      stream << endl;

      wsf::six_dof::Control::Method             controlMethod = mSimInterface->GetControlMethod(mSelectedPlatformId);
      WkSixDOF_Tuner::SimInterface::VehicleType vehicleType   = mSimInterface->GetType(mSelectedPlatformId);

      switch (controlMethod)
      {
      case wsf::six_dof::Control::BankToTurnNoYaw:
         stream << "  control_method                    BANK_TO_TURN_NO_YAW" << endl;
         break;
      case wsf::six_dof::Control::BankToTurnWithYaw:
         stream << "  control_method                    BANK_TO_TURN_WITH_YAW" << endl;
         break;
      case wsf::six_dof::Control::YawToTurnNoRoll:
         stream << "  control_method                    YAW_TO_TURN_NO_ROLL" << endl;
         break;
      case wsf::six_dof::Control::YawToTurnRollRate:
         stream << "  control_method                    YAW_TO_TURN_ROLL_RATE" << endl;
         break;
      case wsf::six_dof::Control::YawToTurnZeroBank:
         stream << "  control_method                    YAW_TO_TURN_ZERO_BANK" << endl;
         break;
      default:
         stream << "  control_method                    YAW_TO_TURN_NO_ROLL" << endl;
      }

      if (vehicleType == WkSixDOF_Tuner::SimInterface::VehicleType::RigidBody)
      {
         stream << endl;
         stream << "  min_taxi_turn_radius              50.0 ft   # This can be adjusted after testing" << endl;
         stream << endl;
         stream << "  use_simple_yaw_damper             false     # This can be set to true to damp yaw oscillations"
                << endl;
         stream << endl;
      }

      stream << "  # ------------------------------------------------------------" << endl;
      stream << endl;
      stream << "  pid_group" << endl;
      stream << endl;

      QString whitespace = QString("    ");

      std::map<double, wsf::six_dof::PidGainData> gains =
         mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::Alpha);
      double interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::Alpha);
      WritePIDBlock(stream, QString("pid_alpha"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::Altitude);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::Altitude);
      WritePIDBlock(stream, QString("pid_altitude"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::BankAngle);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::BankAngle);
      WritePIDBlock(stream, QString("pid_bank_angle"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::Beta);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::Beta);
      WritePIDBlock(stream, QString("pid_beta"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::DeltaPitch);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::DeltaPitch);
      WritePIDBlock(stream, QString("pid_delta_pitch"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::DeltaRoll);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::DeltaRoll);
      WritePIDBlock(stream, QString("pid_delta_roll"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::FlightPathAngle);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::FlightPathAngle);
      WritePIDBlock(stream, QString("pid_flightpath_angle"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::ForwardAccel);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::ForwardAccel);
      WritePIDBlock(stream, QString("pid_forward_accel"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::PitchAngle);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::PitchAngle);
      WritePIDBlock(stream, QString("pid_pitch_angle"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::PitchRate);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::PitchRate);
      WritePIDBlock(stream, QString("pid_pitch_rate"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::RollHeading);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::RollHeading);
      WritePIDBlock(stream, QString("pid_roll_heading"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::RollRate);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::RollRate);
      WritePIDBlock(stream, QString("pid_roll_rate"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::Speed);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::Speed);
      WritePIDBlock(stream, QString("pid_speed"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiForwardAccel);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiForwardAccel);
      WritePIDBlock(stream, QString("pid_taxi_forward_accel"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiHeading);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiHeading);
      WritePIDBlock(stream, QString("pid_taxi_heading"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiSpeed);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiSpeed);
      WritePIDBlock(stream, QString("pid_taxi_speed"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiYawRate);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::TaxiYawRate);
      WritePIDBlock(stream, QString("pid_taxi_yaw_rate"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::VerticalSpeed);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::VerticalSpeed);
      WritePIDBlock(stream, QString("pid_vert_speed"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::YawHeading);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::YawHeading);
      WritePIDBlock(stream, QString("pid_yaw_heading"), whitespace, gains, interval_sec);

      gains        = mSimInterface->GetInputValuesForPid(mSelectedPlatformId, wsf::six_dof::Pid::YawRate);
      interval_sec = mSimInterface->GetUpdateIntervalForPid(mSelectedPlatformId, wsf::six_dof::Pid::YawRate);
      WritePIDBlock(stream, QString("pid_yaw_rate"), whitespace, gains, interval_sec);

      wsf::six_dof::AutopilotLimitsAndSettings limits = mSimInterface->GetLimitsAndSettings(mSelectedPlatformId);

      stream << "    limits_and_settings" << endl;
      stream << "      include " << WriteG_LimitsFile(aFilename, limits) << endl;
      stream << "      afterburner_threshold       " << limits.afterburnerThreshold << endl;
      stream << "      speedbrake_threshold        " << limits.speedBrakeThreshold << endl;
      stream << "      alpha_min                   " << limits.alpha_Min << endl;
      stream << "      alpha_max                   " << limits.alpha_Max << endl;
      stream << "      pitch_rate_min              " << limits.pitchRate_Min << endl;
      stream << "      pitch_rate_max              " << limits.pitchRate_Max << endl;
      stream << "      vert_speed_min              " << limits.vertSpd_Min << endl;
      stream << "      vert_speed_max              " << limits.vertSpd_Max << endl;
      stream << "      beta_max                    " << limits.beta_Max << endl;
      stream << "      yaw_rate_max                " << limits.yawRate_Max << endl;
      stream << "      roll_rate_max               " << limits.rollRate_Max << endl;
      stream << "      bank_angle_max              " << limits.bankAngle_Max << endl;
      stream << "      forward_accel_min           " << limits.forwardAccel_Min << endl;
      stream << "      forward_accel_max           " << limits.forwardAccel_Max << endl;
      stream << "      taxi_speed_max_fps          " << limits.taxiSpeed_Max << endl;
      stream << "      taxi_yaw_rate_max           " << limits.taxiYawRate_Max << endl;
      stream << "      turn_roll_in_multiplier     " << limits.turnRollInMultiplier
             << "         # This can be manually 'tuned' for improved performance" << endl;
      stream << "      route_allowable_angle_error " << limits.routeAllowableAngleError_rad * UtMath::cDEG_PER_RAD
             << " deg     # This can be manually 'tuned' for improved performance" << endl;
      stream << "    end_limits_and_settings" << endl;
      stream << endl;
      stream << "  end_pid_group" << endl;
      stream << endl;
      stream << "end_autopilot_config" << endl;
      stream << endl;

      file.close();
   }
}

QString WkSixDOF_Tuner::MainWidget::WriteG_LimitsFile(const QString&                                  aFilename,
                                                      const wsf::six_dof::AutopilotLimitsAndSettings& aLimits)
{
   UtPath      filePath{aFilename.toStdString()};
   std::string baseName     = filePath.GetFileName(false);
   std::string extension    = filePath.GetFileExtension();
   std::string modifiedName = baseName + "_g_limits" + extension;
   QString     auxFileName  = QString::fromStdString(modifiedName);
   filePath.Up();
   filePath += modifiedName;

   QFile file(QString::fromStdString(filePath.GetNormalizedPath()));
   if (file.open(QIODevice::WriteOnly))
   {
      QTextStream stream(&file);

      QDate today = QDate::currentDate();
      stream << "# ##################################################################################################"
             << endl;
      stream << "# " << endl;
      stream << "# This file was auto-generated by the Vehicle Tuner Plug-In for AFSIM Warlock." << endl;
      stream << "# " << endl;
      stream << "# Creation Date:  " << today.toString(Qt::RFC2822Date) << endl;
      stream << "# " << endl;
      stream << "# ##################################################################################################"
             << endl;
      stream << endl;
      stream << "      pitch_gload_min             " << aLimits.pitchGLoad_Min << endl;
      stream << "      pitch_gload_max             " << aLimits.pitchGLoad_Max << endl;
      stream << "      yaw_gload_max               " << aLimits.yawGLoad_Max << endl;

      file.close();
   }

   return auxFileName;
}

void WkSixDOF_Tuner::MainWidget::WritePIDBlock(QTextStream&                                 aStream,
                                               const QString&                               aBlockName,
                                               const QString&                               aBlockWhiteSpace,
                                               std::map<double, wsf::six_dof::PidGainData>& aGainData,
                                               double                                       aUpdateInterval)
{
   if (aGainData.size() == 0)
   {
      return;
   }

   aStream << aBlockWhiteSpace << aBlockName << endl;
   QString localWhiteSpace = aBlockWhiteSpace + "  ";

   aStream << aBlockWhiteSpace << "  update_interval " << aUpdateInterval << " s" << endl;

   bool useTable = (aGainData.size() != 1);
   if (useTable)
   {
      aStream << aBlockWhiteSpace << "  gain_table" << endl;
      localWhiteSpace = localWhiteSpace + "  ";
   }

   for (std::map<double, wsf::six_dof::PidGainData>::const_iterator iter = aGainData.begin(); iter != aGainData.end();
        ++iter)
   {
      if (useTable)
      {
         aStream << localWhiteSpace << "control_value " << iter->second.ControllingValue << endl;
      }
      aStream << localWhiteSpace << "kp " << iter->second.KpGain << endl;
      aStream << localWhiteSpace << "ki " << iter->second.KiGain << endl;
      aStream << localWhiteSpace << "kd " << iter->second.KdGain << endl;
      aStream << localWhiteSpace << "low_pass_alpha " << iter->second.LowpassAlpha << endl;

      constexpr float maxFloat = std::numeric_limits<float>::max();
      constexpr float minFloat = std::numeric_limits<float>::min();
      if (iter->second.MaxAccum < maxFloat)
      {
         aStream << localWhiteSpace << "max_error_accum " << iter->second.MaxAccum << endl;
      }
      if (iter->second.MaxErrorZero < maxFloat)
      {
         aStream << localWhiteSpace << "ignore_large_error_accum " << iter->second.MaxErrorZero << endl;
      }
      if (iter->second.MinErrorZero > minFloat)
      {
         aStream << localWhiteSpace << "ignore_small_error_accum " << iter->second.MinErrorZero << endl;
      }
      if (!UtMath::NearlyZero(iter->second.KtAntiWindup))
      {
         aStream << localWhiteSpace << "kt_anti_windup_gain " << iter->second.KtAntiWindup << endl;
      }
   }

   if (useTable)
   {
      aStream << aBlockWhiteSpace << "  end_gain_table" << endl;
   }

   aStream << aBlockWhiteSpace << "end_" << aBlockName << endl << endl;
}
