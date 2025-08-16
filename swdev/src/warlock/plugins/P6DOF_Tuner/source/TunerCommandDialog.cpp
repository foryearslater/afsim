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
#include "TunerCommandDialog.hpp"

#include "P6DofVehicleData.hpp"
#include "WkfPlatform.hpp"

WkTuner::CommandDialog::CommandDialog(SimInterface* aSimInterface, QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
   , mSelectedPlatformId(-1)
   , mSimInterface(aSimInterface)
{
   mUi.setupUi(this);

   auto* validator = new QDoubleValidator(this);
   // Page 1, Altitude/Pitch
   mUi.altitude1LineEdit->setValidator(validator);
   mUi.altitude2LineEdit->setValidator(validator);
   mUi.verticalSpeed1LineEdit->setValidator(validator);
   mUi.verticalSpeed2LineEdit->setValidator(validator);
   mUi.alpha1LineEdit->setValidator(validator);
   mUi.alpha2LineEdit->setValidator(validator);
   mUi.pitchAngle1LineEdit->setValidator(validator);
   mUi.pitchAngle2LineEdit->setValidator(validator);
   mUi.pitchRate1LineEdit->setValidator(validator);
   mUi.pitchRate2LineEdit->setValidator(validator);
   mUi.flightPathAngle1LineEdit->setValidator(validator);
   mUi.flightPathAngle2LineEdit->setValidator(validator);
   mUi.deltaPitchAngle1LineEdit->setValidator(validator);
   mUi.deltaPitchAngle2LineEdit->setValidator(validator);
   mUi.pitchG1LineEdit->setValidator(validator);
   mUi.pitchG2LineEdit->setValidator(validator);

   // Page 2, Roll
   mUi.rollHeading1LineEdit->setValidator(validator);
   mUi.rollHeading2LineEdit->setValidator(validator);
   mUi.bankAngle1LineEdit->setValidator(validator);
   mUi.bankAngle2LineEdit->setValidator(validator);
   mUi.rollRate1LineEdit->setValidator(validator);
   mUi.rollRate2LineEdit->setValidator(validator);
   mUi.deltaRoll1LineEdit->setValidator(validator);
   mUi.deltaRoll2LineEdit->setValidator(validator);

   // Page 3, Yaw
   mUi.yawHeading1LineEdit->setValidator(validator);
   mUi.yawHeading2LineEdit->setValidator(validator);
   mUi.yawRate1LineEdit->setValidator(validator);
   mUi.yawRate2LineEdit->setValidator(validator);
   mUi.beta1LineEdit->setValidator(validator);
   mUi.beta2LineEdit->setValidator(validator);
   mUi.yawG1LineEdit->setValidator(validator);
   mUi.yawG2LineEdit->setValidator(validator);

   // Page 4, Speed
   mUi.CAS1LineEdit->setValidator(validator);
   mUi.CAS2LineEdit->setValidator(validator);
   mUi.TAS1LineEdit->setValidator(validator);
   mUi.TAS2LineEdit->setValidator(validator);
   mUi.mach1LineEdit->setValidator(validator);
   mUi.mach2LineEdit->setValidator(validator);

   // Page 5, Taxi
   mUi.taxiSpeed1LineEdit->setValidator(validator);
   mUi.taxiSpeed2LineEdit->setValidator(validator);
   mUi.taxiHeading1LineEdit->setValidator(validator);
   mUi.taxiHeading2LineEdit->setValidator(validator);
   mUi.taxiYawRate1LineEdit->setValidator(validator);
   mUi.taxiYawRate2LineEdit->setValidator(validator);

   // Page 1
   connect(mUi.altitude1PushButton, &QPushButton::pressed, this, &CommandDialog::AltitudeSendButton1Pressed);
   connect(mUi.altitude2PushButton, &QPushButton::pressed, this, &CommandDialog::AltitudeSendButton2Pressed);
   connect(mUi.verticalSpeed1PushButton, &QPushButton::pressed, this, &CommandDialog::VerticalSpeedSendButton1Pressed);
   connect(mUi.verticalSpeed2PushButton, &QPushButton::pressed, this, &CommandDialog::VerticalSpeedSendButton2Pressed);
   connect(mUi.alpha1PushButton, &QPushButton::pressed, this, &CommandDialog::AlphaSendButton1Pressed);
   connect(mUi.alpha2PushButton, &QPushButton::pressed, this, &CommandDialog::AlphaSendButton2Pressed);
   connect(mUi.pitchAngle1PushButton, &QPushButton::pressed, this, &CommandDialog::PitchAngleSendButton1Pressed);
   connect(mUi.pitchAngle2PushButton, &QPushButton::pressed, this, &CommandDialog::PitchAngleSendButton2Pressed);
   connect(mUi.pitchRate1PushButton, &QPushButton::pressed, this, &CommandDialog::PitchRateSendButton1Pressed);
   connect(mUi.pitchRate2PushButton, &QPushButton::pressed, this, &CommandDialog::PitchRateSendButton2Pressed);
   connect(mUi.flightPathAngle1PushButton, &QPushButton::pressed, this, &CommandDialog::FlightPathAngleSendButton1Pressed);
   connect(mUi.flightPathAngle2PushButton, &QPushButton::pressed, this, &CommandDialog::FlightPathAngleSendButton2Pressed);
   connect(mUi.deltaPitchAngle1PushButton, &QPushButton::pressed, this, &CommandDialog::DeltaPitchAngleSendButton1Pressed);
   connect(mUi.deltaPitchAngle2PushButton, &QPushButton::pressed, this, &CommandDialog::DeltaPitchAngleSendButton2Pressed);
   connect(mUi.pitchG1PushButton, &QPushButton::pressed, this, &CommandDialog::PitchGSendButton1Pressed);
   connect(mUi.pitchG2PushButton, &QPushButton::pressed, this, &CommandDialog::PitchGSendButton2Pressed);

   // Page 2
   connect(mUi.rollHeading1PushButton, &QPushButton::pressed, this, &CommandDialog::RollHeadingSendButton1Pressed);
   connect(mUi.rollHeading2PushButton, &QPushButton::pressed, this, &CommandDialog::RollHeadingSendButton2Pressed);
   connect(mUi.bankAngle1PushButton, &QPushButton::pressed, this, &CommandDialog::BankAngleSendButton1Pressed);
   connect(mUi.bankAngle2PushButton, &QPushButton::pressed, this, &CommandDialog::BankAngleSendButton2Pressed);
   connect(mUi.rollRate1PushButton, &QPushButton::pressed, this, &CommandDialog::RollRateSendButton1Pressed);
   connect(mUi.rollRate2PushButton, &QPushButton::pressed, this, &CommandDialog::RollRateSendButton2Pressed);
   connect(mUi.deltaRoll1PushButton, &QPushButton::pressed, this, &CommandDialog::DeltaRollSendButton1Pressed);
   connect(mUi.deltaRoll2PushButton, &QPushButton::pressed, this, &CommandDialog::DeltaRollSendButton2Pressed);

   // Page 3
   connect(mUi.yawHeading1PushButton, &QPushButton::pressed, this, &CommandDialog::YawHeadingSendButton1Pressed);
   connect(mUi.yawHeading2Pushbutton, &QPushButton::pressed, this, &CommandDialog::YawHeadingSendButton2Pressed);
   connect(mUi.yawRate1PushButton, &QPushButton::pressed, this, &CommandDialog::YawRateSendButton1Pressed);
   connect(mUi.yawRate2PushButton, &QPushButton::pressed, this, &CommandDialog::YawRateSendButton2Pressed);
   connect(mUi.beta1PushButton, &QPushButton::pressed, this, &CommandDialog::BetaSendButton1Pressed);
   connect(mUi.beta2PushButton, &QPushButton::pressed, this, &CommandDialog::BetaSendButton2Pressed);
   connect(mUi.yawG1PushButton, &QPushButton::pressed, this, &CommandDialog::YawGSendButton1Pressed);
   connect(mUi.yawG2PushButton, &QPushButton::pressed, this, &CommandDialog::YawGSendButton2Pressed);

   // Page 4
   connect(mUi.CAS1PushButton, &QPushButton::pressed, this, &CommandDialog::CASSendButton1Pressed);
   connect(mUi.CAS2PushButton, &QPushButton::pressed, this, &CommandDialog::CASSendButton2Pressed);
   connect(mUi.TAS1PushButton, &QPushButton::pressed, this, &CommandDialog::TASSendButton1Pressed);
   connect(mUi.TAS2PushButton, &QPushButton::pressed, this, &CommandDialog::TASSendButton2Pressed);
   connect(mUi.mach1PushButton, &QPushButton::pressed, this, &CommandDialog::MachSendButton1Pressed);
   connect(mUi.mach2PushButton, &QPushButton::pressed, this, &CommandDialog::MachSendButton2Pressed);

   // Page 5
   connect(mUi.taxiSpeed1PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiSpeedSendButton1Pressed);
   connect(mUi.taxiSpeed2PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiSpeedSendButton2Pressed);
   connect(mUi.taxiHeading1PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiHeadingSendButton1Pressed);
   connect(mUi.taxiHeading2PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiHeadingSendButton2Pressed);
   connect(mUi.taxiYawRate1PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiYawRateSendButton1Pressed);
   connect(mUi.taxiYawRate2PushButton, &QPushButton::pressed, this, &CommandDialog::TaxiYawRateSendButton2Pressed);

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &CommandDialog::PlatformOfInterestChanged);

   connect(mSimInterface, &SimInterface::NewP6DofPlatform, this, &CommandDialog::HandleNewP6DofVehicle);
   connect(mSimInterface, &SimInterface::DeletedP6DofPlatform, this, &CommandDialog::HandleDeletedP6DofVehicle);

   mUi.prevCmdTypeLabel->setText("None");
   mUi.prevCmdValueLabel->setText(" ");
   mUi.prevCmdUnitsLabel->setText(" ");
}

void WkTuner::CommandDialog::showEvent(QShowEvent* aEvent)
{
   // The first time the user opens the command dialog, override vertical speed to fpm units.
   if (!mInitialized)
   {
      mInitialized = true;
      mUi.verticalSpeedUnitsComboBox->setCurrentIndex(
         mUi.verticalSpeedUnitsComboBox->findData(UtUnitSpeed::cFEET_PER_MINUTE));
   }
}

void WkTuner::CommandDialog::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform)
   {
      QString platformName = QString::fromStdString(aPlatform->GetName());
      auto    entry        = mP6DofVehicleMap.find(platformName);
      if (entry != mP6DofVehicleMap.end())
      {
         mUi.selectedPlatformLabel->setText(platformName);
      }
      else
      {
         mUi.selectedPlatformLabel->setText("None");
      }
   }
   else
   {
      mUi.selectedPlatformLabel->setText("None");
   }
}

// Page 1 Button Functions:
void WkTuner::CommandDialog::AltitudeSendButton1Pressed()
{
   // Convert to standard units which are taken by the sim interface. The sim interface handles conversion to
   // the units that P6DOF prefers.
   double altitude = mUi.altitudeUnitsComboBox->ConvertToStandard(mUi.altitude1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Altitude, altitude);
   UpdatePreviousCommand(mUi.altitudeLabel->text(), mUi.altitude1LineEdit, mUi.altitudeUnitsComboBox);
}

void WkTuner::CommandDialog::AltitudeSendButton2Pressed()
{
   double altitude = mUi.altitudeUnitsComboBox->ConvertToStandard(mUi.altitude2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Altitude, altitude);
   UpdatePreviousCommand(mUi.altitudeLabel->text(), mUi.altitude2LineEdit, mUi.altitudeUnitsComboBox);
}

void WkTuner::CommandDialog::VerticalSpeedSendButton1Pressed()
{
   double vertSpeed = mUi.verticalSpeedUnitsComboBox->ConvertToStandard(mUi.verticalSpeed1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::VertSpeed, vertSpeed);
   UpdatePreviousCommand(mUi.verticalSpeedLabel->text(), mUi.verticalSpeed1LineEdit, mUi.verticalSpeedUnitsComboBox);
}

void WkTuner::CommandDialog::VerticalSpeedSendButton2Pressed()
{
   double vertSpeed = mUi.verticalSpeedUnitsComboBox->ConvertToStandard(mUi.verticalSpeed2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::VertSpeed, vertSpeed);
   UpdatePreviousCommand(mUi.verticalSpeedLabel->text(), mUi.verticalSpeed2LineEdit, mUi.verticalSpeedUnitsComboBox);
}

void WkTuner::CommandDialog::AlphaSendButton1Pressed()
{
   double alpha = mUi.alphaUnitsComboBox->ConvertToStandard(mUi.alpha1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Alpha, alpha);
   UpdatePreviousCommand(mUi.alphaLabel->text(), mUi.alpha1LineEdit, mUi.alphaUnitsComboBox);
}

void WkTuner::CommandDialog::AlphaSendButton2Pressed()
{
   double alpha = mUi.alphaUnitsComboBox->ConvertToStandard(mUi.alpha2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Alpha, alpha);
   UpdatePreviousCommand(mUi.alphaLabel->text(), mUi.alpha2LineEdit, mUi.alphaUnitsComboBox);
}

void WkTuner::CommandDialog::PitchAngleSendButton1Pressed()
{
   double pitchAng = mUi.pitchAngleUnitsComboBox->ConvertToStandard(mUi.pitchAngle1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::PitchAngle, pitchAng);
   UpdatePreviousCommand(mUi.pitchAngleLabel->text(), mUi.pitchAngle1LineEdit, mUi.pitchAngleUnitsComboBox);
}

void WkTuner::CommandDialog::PitchAngleSendButton2Pressed()
{
   double pitchAng = mUi.pitchAngleUnitsComboBox->ConvertToStandard(mUi.pitchAngle2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::PitchAngle, pitchAng);
   UpdatePreviousCommand(mUi.pitchAngleLabel->text(), mUi.pitchAngle2LineEdit, mUi.pitchAngleUnitsComboBox);
}

void WkTuner::CommandDialog::PitchRateSendButton1Pressed()
{
   double pitchRate = mUi.pitchRateUnitsComboBox->ConvertToStandard(mUi.pitchRate1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::PitchRate, pitchRate);
   UpdatePreviousCommand(mUi.pitchRateLabel->text(), mUi.pitchRate1LineEdit, mUi.pitchRateUnitsComboBox);
}

void WkTuner::CommandDialog::PitchRateSendButton2Pressed()
{
   double pitchRate = mUi.pitchRateUnitsComboBox->ConvertToStandard(mUi.pitchRate2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::PitchRate, pitchRate);
   UpdatePreviousCommand(mUi.pitchRateLabel->text(), mUi.pitchRate2LineEdit, mUi.pitchRateUnitsComboBox);
}

void WkTuner::CommandDialog::FlightPathAngleSendButton1Pressed()
{
   double fltPthAng = mUi.flightPathAngleUnitsComboBox->ConvertToStandard(mUi.flightPathAngle1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::FltPathAngle, fltPthAng);
   UpdatePreviousCommand(mUi.flightPathAngleLabel->text(), mUi.flightPathAngle1LineEdit, mUi.flightPathAngleUnitsComboBox);
}

void WkTuner::CommandDialog::FlightPathAngleSendButton2Pressed()
{
   double fltPthAng = mUi.flightPathAngleUnitsComboBox->ConvertToStandard(mUi.flightPathAngle2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::FltPathAngle, fltPthAng);
   UpdatePreviousCommand(mUi.flightPathAngleLabel->text(), mUi.flightPathAngle2LineEdit, mUi.flightPathAngleUnitsComboBox);
}

void WkTuner::CommandDialog::DeltaPitchAngleSendButton1Pressed()
{
   double dltPitchAng =
      mUi.deltaPitchAngleUnitsComboBox->ConvertToStandard(mUi.deltaPitchAngle1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::DeltaPitch, dltPitchAng);
   UpdatePreviousCommand(mUi.deltaPitchAngleLabel->text(), mUi.deltaPitchAngle1LineEdit, mUi.deltaPitchAngleUnitsComboBox);
}

void WkTuner::CommandDialog::DeltaPitchAngleSendButton2Pressed()
{
   double dltPitchAng =
      mUi.deltaPitchAngleUnitsComboBox->ConvertToStandard(mUi.deltaPitchAngle2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::DeltaPitch, dltPitchAng);
   UpdatePreviousCommand(mUi.deltaPitchAngleLabel->text(), mUi.deltaPitchAngle2LineEdit, mUi.deltaPitchAngleUnitsComboBox);
}

void WkTuner::CommandDialog::PitchGSendButton1Pressed()
{
   mSimInterface->SendCustomCommand(Data::CustomCommand::PitchG, mUi.pitchG1LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.pitchGLabel->text(), mUi.pitchG1LineEdit, mUi.pitchGUnitsComboBox);
}

void WkTuner::CommandDialog::PitchGSendButton2Pressed()
{
   mSimInterface->SendCustomCommand(Data::CustomCommand::PitchG, mUi.pitchG2LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.pitchGLabel->text(), mUi.pitchG2LineEdit, mUi.pitchGUnitsComboBox);
}

// Page 2 Button Functions:
void WkTuner::CommandDialog::RollHeadingSendButton1Pressed()
{
   double rollHdg = mUi.rollHeadingUnitsComboBox->ConvertToStandard(mUi.rollHeading1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::RollHeading, rollHdg);
   UpdatePreviousCommand(mUi.rollHeadingLabel->text(), mUi.rollHeading1LineEdit, mUi.rollHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::RollHeadingSendButton2Pressed()
{
   double rollHdg = mUi.rollHeadingUnitsComboBox->ConvertToStandard(mUi.rollHeading2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::RollHeading, rollHdg);
   UpdatePreviousCommand(mUi.rollHeadingLabel->text(), mUi.rollHeading2LineEdit, mUi.rollHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::BankAngleSendButton1Pressed()
{
   double bankAng = mUi.bankAngleUnitsComboBox->ConvertToStandard(mUi.bankAngle1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::BankAngle, bankAng);
   UpdatePreviousCommand(mUi.bankAngleLabel->text(), mUi.bankAngle1LineEdit, mUi.bankAngleUnitsComboBox);
}

void WkTuner::CommandDialog::BankAngleSendButton2Pressed()
{
   double bankAng = mUi.bankAngleUnitsComboBox->ConvertToStandard(mUi.bankAngle2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::BankAngle, bankAng);
   UpdatePreviousCommand(mUi.bankAngleLabel->text(), mUi.bankAngle2LineEdit, mUi.bankAngleUnitsComboBox);
}

void WkTuner::CommandDialog::RollRateSendButton1Pressed()
{
   double rollRate = mUi.rollRateUnitsComboBox->ConvertToStandard(mUi.rollRate1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::RollRate, rollRate);
   UpdatePreviousCommand(mUi.rollRateLabel->text(), mUi.rollRate1LineEdit, mUi.rollRateUnitsComboBox);
}

void WkTuner::CommandDialog::RollRateSendButton2Pressed()
{
   double rollRate = mUi.rollRateUnitsComboBox->ConvertToStandard(mUi.rollRate2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::RollRate, rollRate);
   UpdatePreviousCommand(mUi.rollRateLabel->text(), mUi.rollRate2LineEdit, mUi.rollRateUnitsComboBox);
}

void WkTuner::CommandDialog::DeltaRollSendButton1Pressed()
{
   double dltRoll = mUi.deltaRollUnitsComboBox->ConvertToStandard(mUi.deltaRoll1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::DeltaRoll, dltRoll);
   UpdatePreviousCommand(mUi.deltaRollLabel->text(), mUi.deltaRoll1LineEdit, mUi.deltaRollUnitsComboBox);
}

void WkTuner::CommandDialog::DeltaRollSendButton2Pressed()
{
   double dltRoll = mUi.deltaRollUnitsComboBox->ConvertToStandard(mUi.deltaRoll2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::DeltaRoll, dltRoll);
   UpdatePreviousCommand(mUi.deltaRollLabel->text(), mUi.deltaRoll2LineEdit, mUi.deltaRollUnitsComboBox);
}

// Page 3 Button Functions:
void WkTuner::CommandDialog::YawHeadingSendButton1Pressed()
{
   double yawHdg = mUi.yawHeadingUnitsComboBox->ConvertToStandard(mUi.yawHeading1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::YawHeading, yawHdg);
   UpdatePreviousCommand(mUi.yawHeadingLabel->text(), mUi.yawHeading1LineEdit, mUi.yawHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::YawHeadingSendButton2Pressed()
{
   double yawHdg = mUi.yawHeadingUnitsComboBox->ConvertToStandard(mUi.yawHeading2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::YawHeading, yawHdg);
   UpdatePreviousCommand(mUi.yawHeadingLabel->text(), mUi.yawHeading2LineEdit, mUi.yawHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::YawRateSendButton1Pressed()
{
   double yawRate = mUi.yawRateUnitsComboBox->ConvertToStandard(mUi.yawRate1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::YawRate, yawRate);
   UpdatePreviousCommand(mUi.yawRateLabel->text(), mUi.yawRate1LineEdit, mUi.yawRateUnitsComboBox);
}

void WkTuner::CommandDialog::YawRateSendButton2Pressed()
{
   double yawRate = mUi.yawRateUnitsComboBox->ConvertToStandard(mUi.yawRate2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::YawRate, yawRate);
   UpdatePreviousCommand(mUi.yawRateLabel->text(), mUi.yawRate2LineEdit, mUi.yawRateUnitsComboBox);
}

void WkTuner::CommandDialog::BetaSendButton1Pressed()
{
   double beta = mUi.betaUnitsComboBox->ConvertToStandard(mUi.beta1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Beta, beta);
   UpdatePreviousCommand(mUi.betaLabel->text(), mUi.beta1LineEdit, mUi.betaUnitsComboBox);
}

void WkTuner::CommandDialog::BetaSendButton2Pressed()
{
   double beta = mUi.betaUnitsComboBox->ConvertToStandard(mUi.beta2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::Beta, beta);
   UpdatePreviousCommand(mUi.betaLabel->text(), mUi.beta2LineEdit, mUi.betaUnitsComboBox);
}

void WkTuner::CommandDialog::YawGSendButton1Pressed()
{
   mSimInterface->SendCustomCommand(Data::CustomCommand::YawG, mUi.yawG1LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.yawGLabel->text(), mUi.yawG1LineEdit, mUi.yawGUnitsComboBox);
}

void WkTuner::CommandDialog::YawGSendButton2Pressed()
{
   mSimInterface->SendCustomCommand(Data::CustomCommand::YawG, mUi.yawG2LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.yawGLabel->text(), mUi.yawG2LineEdit, mUi.yawGUnitsComboBox);
}

// Page 4 Button Functions:
void WkTuner::CommandDialog::CASSendButton1Pressed()
{
   double cas = mUi.CASUnitsComboBox->ConvertToStandard(mUi.CAS1LineEdit->text().toDouble());
   mSimInterface->SendCustomCommand(Data::CustomCommand::KCAS, cas);
   UpdatePreviousCommand(mUi.CASLabel->text(), mUi.CAS1LineEdit, mUi.CASUnitsComboBox);
}

void WkTuner::CommandDialog::CASSendButton2Pressed()
{
   double cas = mUi.CASUnitsComboBox->ConvertToStandard(mUi.CAS2LineEdit->text().toDouble());
   mSimInterface->SendCustomCommand(Data::CustomCommand::KCAS, cas);
   UpdatePreviousCommand(mUi.CASLabel->text(), mUi.CAS2LineEdit, mUi.CASUnitsComboBox);
}

void WkTuner::CommandDialog::TASSendButton1Pressed()
{
   double tas = mUi.TASUnitsComboBox->ConvertToStandard(mUi.TAS1LineEdit->text().toDouble());
   mSimInterface->SendCustomCommand(Data::CustomCommand::KTAS, tas);
   UpdatePreviousCommand(mUi.TASLabel->text(), mUi.TAS1LineEdit, mUi.TASUnitsComboBox);
}

void WkTuner::CommandDialog::TASSendButton2Pressed()
{
   double tas = mUi.TASUnitsComboBox->ConvertToStandard(mUi.TAS2LineEdit->text().toDouble());
   mSimInterface->SendCustomCommand(Data::CustomCommand::KTAS, tas);
   UpdatePreviousCommand(mUi.TASLabel->text(), mUi.TAS2LineEdit, mUi.TASUnitsComboBox);
}

void WkTuner::CommandDialog::MachSendButton1Pressed()
{
   // unitless, no conversion necessary
   mSimInterface->SendCustomCommand(Data::CustomCommand::Mach, mUi.mach1LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.machLabel->text(), mUi.mach1LineEdit, mUi.machUnitsComboBox);
}

void WkTuner::CommandDialog::MachSendButton2Pressed()
{
   // unitless, no conversion necessary
   mSimInterface->SendCustomCommand(Data::CustomCommand::Mach, mUi.mach2LineEdit->text().toDouble());
   UpdatePreviousCommand(mUi.machLabel->text(), mUi.mach2LineEdit, mUi.machUnitsComboBox);
}

// Page 5 Button Functions:
void WkTuner::CommandDialog::TaxiSpeedSendButton1Pressed()
{
   double taxiSpeed = mUi.taxiSpeedUnitsComboBox->ConvertToStandard(mUi.taxiSpeed1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiSpeed, taxiSpeed);
   UpdatePreviousCommand(mUi.taxiSpeedLabel->text(), mUi.taxiSpeed1LineEdit, mUi.taxiSpeedUnitsComboBox);
}

void WkTuner::CommandDialog::TaxiSpeedSendButton2Pressed()
{
   double taxiSpeed = mUi.taxiSpeedUnitsComboBox->ConvertToStandard(mUi.taxiSpeed2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiSpeed, taxiSpeed);
   UpdatePreviousCommand(mUi.taxiSpeedLabel->text(), mUi.taxiSpeed2LineEdit, mUi.taxiSpeedUnitsComboBox);
}

void WkTuner::CommandDialog::TaxiHeadingSendButton1Pressed()
{
   double taxiHdg = mUi.taxiHeadingUnitsComboBox->ConvertToStandard(mUi.taxiHeading1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiHeading, taxiHdg);
   UpdatePreviousCommand(mUi.taxiHeadingLabel->text(), mUi.taxiHeading1LineEdit, mUi.taxiHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::TaxiHeadingSendButton2Pressed()
{
   double taxiHdg = mUi.taxiHeadingUnitsComboBox->ConvertToStandard(mUi.taxiHeading2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiHeading, taxiHdg);
   UpdatePreviousCommand(mUi.taxiHeadingLabel->text(), mUi.taxiHeading1LineEdit, mUi.taxiHeadingUnitsComboBox);
}

void WkTuner::CommandDialog::TaxiYawRateSendButton1Pressed()
{
   double taxiYawRate = mUi.taxiYawRateUnitsComboBox->ConvertToStandard(mUi.taxiYawRate1LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiSpeed, taxiYawRate);
   UpdatePreviousCommand(mUi.taxiYawRateLabel->text(), mUi.taxiYawRate1LineEdit, mUi.taxiYawRateUnitsComboBox);
}

void WkTuner::CommandDialog::TaxiYawRateSendButton2Pressed()
{
   double taxiYawRate = mUi.taxiYawRateUnitsComboBox->ConvertToStandard(mUi.taxiYawRate2LineEdit->text().toDouble());
   mSimInterface->SendCommandBasedOnPID(P6DOF::Pid::TaxiSpeed, taxiYawRate);
   UpdatePreviousCommand(mUi.taxiYawRateLabel->text(), mUi.taxiYawRate2LineEdit, mUi.taxiYawRateUnitsComboBox);
}

void WkTuner::CommandDialog::HandleNewP6DofVehicle(const std::string& aName, int aId)
{
   QString name           = QString::fromStdString(aName);
   mP6DofVehicleMap[name] = aId;

   // If the newly discovered P6DOF Mover belongs to the Platform Of Interest, go ahead and update the displays.
   auto* platformOfInterest = wkfEnv.GetPlatformOfInterest();
   if (platformOfInterest && platformOfInterest->GetName() == aName)
   {
      mUi.selectedPlatformLabel->setText(name);
   }
}

void WkTuner::CommandDialog::HandleDeletedP6DofVehicle(const std::string& aName)
{
   QString name = QString::fromStdString(aName);
   if (mUi.selectedPlatformLabel->text() == name)
   {
      mUi.selectedPlatformLabel->setText("None");
   }
   mP6DofVehicleMap.erase(name);
}

void WkTuner::CommandDialog::UpdatePreviousCommand(const QString& aCommandType,
                                                   QLineEdit*     aValueInputPtr,
                                                   QComboBox*     aUnitComboBoxPtr)
{
   mUi.prevCmdTypeLabel->setText(aCommandType);
   mUi.prevCmdValueLabel->setText(aValueInputPtr->text());
   mUi.prevCmdUnitsLabel->setText(aUnitComboBoxPtr->currentText());
}
