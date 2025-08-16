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

#ifndef TUNERCOMMANDSDIALOG_HPP
#define TUNERCOMMANDSDIALOG_HPP

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

#include "TunerSimInterface.hpp"
#include "ui_TunerCommandsDialog.h"

namespace wkf
{
class Platform;
}

namespace WkSixDOF_Tuner
{
class CommandDialog : public QDialog
{
   Q_OBJECT

public:
   CommandDialog(SimInterface* aSimInterface, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

   ~CommandDialog() override = default;

protected:
   void showEvent(QShowEvent* aEvent) override;

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatform);

   // Button Press Functions:
   // Page 1, Altitude/Pitch
   void AltitudeSendButton1Pressed();
   void AltitudeSendButton2Pressed();
   void VerticalSpeedSendButton1Pressed();
   void VerticalSpeedSendButton2Pressed();
   void AlphaSendButton1Pressed();
   void AlphaSendButton2Pressed();
   void PitchAngleSendButton1Pressed();
   void PitchAngleSendButton2Pressed();
   void PitchRateSendButton1Pressed();
   void PitchRateSendButton2Pressed();
   void FlightPathAngleSendButton1Pressed();
   void FlightPathAngleSendButton2Pressed();
   void DeltaPitchAngleSendButton1Pressed();
   void DeltaPitchAngleSendButton2Pressed();
   void PitchGSendButton1Pressed();
   void PitchGSendButton2Pressed();

   // Page 2, Roll
   void RollHeadingSendButton1Pressed();
   void RollHeadingSendButton2Pressed();
   void BankAngleSendButton1Pressed();
   void BankAngleSendButton2Pressed();
   void RollRateSendButton1Pressed();
   void RollRateSendButton2Pressed();
   void DeltaRollSendButton1Pressed();
   void DeltaRollSendButton2Pressed();

   // Page 3, Yaw
   void YawHeadingSendButton1Pressed();
   void YawHeadingSendButton2Pressed();
   void YawRateSendButton1Pressed();
   void YawRateSendButton2Pressed();
   void BetaSendButton1Pressed();
   void BetaSendButton2Pressed();
   void YawGSendButton1Pressed();
   void YawGSendButton2Pressed();

   // Page 4, Speed
   void CASSendButton1Pressed();
   void CASSendButton2Pressed();
   void TASSendButton1Pressed();
   void TASSendButton2Pressed();
   void MachSendButton1Pressed();
   void MachSendButton2Pressed();

   // Page 5, Taxi
   void TaxiSpeedSendButton1Pressed();
   void TaxiSpeedSendButton2Pressed();
   void TaxiHeadingSendButton1Pressed();
   void TaxiHeadingSendButton2Pressed();
   void TaxiYawRateSendButton1Pressed();
   void TaxiYawRateSendButton2Pressed();

   void HandleNewVehicle(const std::string& aName, int aId);
   void HandleDeletedVehicle(const std::string& aName);

   void UpdatePreviousCommand(const QString& aCommandType, QLineEdit* aValueInputPtr, QComboBox* aUnitComboBoxPtr);

   int mSelectedPlatformId = -1;

   std::map<QString, int> mVehicleMap;

   SimInterface* mSimInterface{nullptr};

   Ui::TunerCommandsDialog mUi;

   bool mInitialized = false;
};
} // namespace WkSixDOF_Tuner

#endif
