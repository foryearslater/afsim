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

#include "PlatformOptionsWidget.hpp"

#include <QMessageBox>

SPLAT::PlatformOptionsWidget::PlatformOptionsWidget(PlotType aPlotType, PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
   , mTargetTypeValidator(new PlatformTypeValidator)
   , mPlotType(aPlotType)
{
   mUi.setupUi(this);
   mUi.mSensorPlatformToDetectEdit->setValidator(mTargetTypeValidator);
   mUi.mJammerNameEdit->setValidator(mTargetTypeValidator);

   Initialize();
   Populate();
   Connect();
}

SPLAT::PlatformOptionsWidget::~PlatformOptionsWidget()
{
   delete mTargetTypeValidator;
}

void SPLAT::PlatformOptionsWidget::WriteData(std::ostream& aStream)
{
   if (mPlotType == PlotOptionsWidget::PlotType::cHORIZONTAL_MAP)
   {
      aStream << "   sensor_platform_yaw " << mSensorControl.mYaw << "\n";
      aStream << "   sensor_platform_pitch " << mSensorControl.mPitch << "\n";
      aStream << "   sensor_platform_roll " << mSensorControl.mRoll << "\n";
      aStream << "   automatic_target_cueing " << (mSensorControl.mAutoCue ? "true" : "false") << "\n";

      if (!mSensorControl.mModeName.isEmpty())
      {
         aStream << "   mode_name " << mSensorControl.mModeName.toStdString() << "\n";
      }

      if (!mSensorControl.mPlatformToDetect.isEmpty())
      {
         aStream << "   platform_to_detect_name " << mSensorControl.mPlatformToDetect.toStdString() << "\n";
      }

      if (mUi.mJammerControlGroupBox->isEnabled())
      {
         if (!mJammerControl.mPlatformName.isEmpty())
         {
            aStream << "   jamming_platform_name " << mJammerControl.mPlatformName.toStdString() << "\n";
         }
         aStream << "   jammer_to_signal_reference " << mJammerControl.mSigRef << "\n";
      }

      aStream << "   target_altitude " << mTargetOptions.mAltitude << "\n";
      aStream << "   target_speed " << mTargetOptions.mSpeed << "\n";
      aStream << "   target_yaw " << mTargetOptions.mYaw << "\n";
      aStream << "   target_pitch " << mTargetOptions.mPitch << "\n";
      aStream << "   target_roll " << mTargetOptions.mRoll << "\n";

      if (mUi.mTargetHeadingCheckBox->isChecked())
      {
         aStream << "   target_heading " << mTargetOptions.mHeading << "\n";
      }
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cSPHERICAL_MAP)
   {
      aStream << "   sensor_platform_altitude " << mSensorControl.mAltitude << "\n";
      aStream << "   sensor_platform_yaw " << mSensorControl.mYaw << "\n";
      aStream << "   sensor_platform_pitch " << mSensorControl.mPitch << "\n";
      aStream << "   sensor_platform_roll " << mSensorControl.mRoll << "\n";
      aStream << "   automatic_target_cueing " << (mSensorControl.mAutoCue ? "true" : "false") << "\n";

      if (!mSensorControl.mModeName.isEmpty())
      {
         aStream << "   mode_name " << mSensorControl.mModeName.toStdString() << "\n";
      }

      if (mUi.mJammerControlGroupBox->isEnabled())
      {
         if (!mJammerControl.mPlatformName.isEmpty())
         {
            aStream << "   jamming_platform_name " << mJammerControl.mPlatformName.toStdString() << "\n";
         }
         aStream << "   jammer_to_signal_reference " << mJammerControl.mSigRef << "\n";
      }

      if (!mUi.mFixedSensorCheckBox->isChecked())
      {
         aStream << "fixed_target_position\n";
      }

      aStream << "   target_speed " << mTargetOptions.mSpeed << "\n";
      aStream << "   target_yaw " << mTargetOptions.mYaw << "\n";
      aStream << "   target_pitch " << mTargetOptions.mPitch << "\n";
      aStream << "   target_roll " << mTargetOptions.mRoll << "\n";
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cVERTICAL_MAP)
   {
      if (!mSensorControl.mModeName.isEmpty())
      {
         aStream << "   mode_name " << mSensorControl.mModeName.toStdString() << "\n";
      }

      if (mUi.mJammerControlGroupBox->isEnabled())
      {
         if (!mJammerControl.mPlatformName.isEmpty())
         {
            aStream << "   jamming_platform_name " << mJammerControl.mPlatformName.toStdString() << "\n";
         }
         aStream << "   jammer_to_signal_reference " << mJammerControl.mSigRef << "\n";
      }

      aStream << "   target_speed " << mTargetOptions.mSpeed << "\n";
      aStream << "   target_yaw " << mTargetOptions.mYaw << "\n";
      aStream << "   target_pitch " << mTargetOptions.mPitch << "\n";
      aStream << "   target_roll " << mTargetOptions.mRoll << "\n";
      if (mUi.mTargetHeadingCheckBox->isChecked())
      {
         aStream << "   target_heading " << mTargetOptions.mHeading << "\n";
      }
   }
}

bool SPLAT::PlatformOptionsWidget::ApplyChanges()
{
   bool isValid = PlatformOptionsValid();
   if (isValid)
   {
      SetPlatformOptions();
   }
   else
   {
      QMessageBox::critical(this, "Error", "Platform Options Invalid");
   }
   return isValid;
}

void SPLAT::PlatformOptionsWidget::RestoreDefaults()
{
   mTargetOptions = TargetOptions();
   mSensorControl = SensorControl();
   mJammerControl = JammerControl();

   Initialize();
   Populate();
}

void SPLAT::PlatformOptionsWidget::RequiredJammingPowerPressed(bool aChecked)
{
   mUi.mJammerControlGroupBox->setEnabled(aChecked);
}

void SPLAT::PlatformOptionsWidget::UpdateModes(const QStringList& aModeNames)
{
   mModeNames = aModeNames;
   Populate();
}

void SPLAT::PlatformOptionsWidget::UpdatePlatformTypes(const QStringList& aPlatformTypes)
{
   mTargetTypeValidator->setValidList(aPlatformTypes);
}

void SPLAT::PlatformOptionsWidget::CheckValidity()
{
   UtQtText::SetValid(mUi.mSensorPlatformToDetectEdit->hasAcceptableInput(), *mUi.mSensorPlatformToDetectEdit);
   UtQtText::SetValid(mUi.mJammerNameEdit->hasAcceptableInput(), *mUi.mJammerNameEdit);
}

void SPLAT::PlatformOptionsWidget::Initialize()
{
   mUi.mSensorModeComboBox->clear();
   mUi.mJammerControlGroupBox->setEnabled(false);

   mUi.mTargetHeadingCheckBox->setChecked(false);
   mUi.mTargetHeadingEdit->setEnabled(false);

   if (mPlotType == PlotOptionsWidget::PlotType::cHORIZONTAL_MAP)
   {
      mUi.mSensorAltitudeEdit->setEnabled(false);
      mUi.mFixedSensorCheckBox->setEnabled(false);
      mUi.mAutoTargetCueingCheckBox->setChecked(true);
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cHORIZONTAL_COVERAGE)
   {
      mUi.mSensorAltitudeEdit->setEnabled(false);
      mUi.mFixedSensorCheckBox->setEnabled(false);
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cSPHERICAL_MAP)
   {
      mUi.mFixedSensorCheckBox->setChecked(true);
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cVERTICAL_COVERAGE)
   {
      mUi.mSensorAltitudeEdit->setEnabled(false);
      mUi.mFixedSensorCheckBox->setEnabled(false);
   }
   else if (mPlotType == PlotOptionsWidget::PlotType::cVERTICAL_MAP)
   {
      mUi.mSensorAltitudeEdit->setEnabled(false);
      mUi.mFixedSensorCheckBox->setEnabled(false);
      mUi.mSensorYawEdit->setEnabled(false);
      mUi.mSensorPitchEdit->setEnabled(false);
      mUi.mSensorRollEdit->setEnabled(false);
      mUi.mAutoTargetCueingCheckBox->setEnabled(false);
      mUi.mSensorPlatformToDetectEdit->setEnabled(false);

      mUi.mTargetAltitudeEdit->setEnabled(false);
   }
}

void SPLAT::PlatformOptionsWidget::Populate()
{
   mUi.mSensorModeComboBox->clear();
   mUi.mSensorModeComboBox->insertItems(0, mModeNames);

   mUi.mTargetAltitudeEdit->SetValue(mTargetOptions.mAltitude);
   mUi.mTargetSpeedEdit->SetValue(mTargetOptions.mSpeed);
   mUi.mTargetYawEdit->SetValue(mTargetOptions.mYaw);
   mUi.mTargetPitchEdit->SetValue(mTargetOptions.mPitch);
   mUi.mTargetRollEdit->SetValue(mTargetOptions.mRoll);
   mUi.mTargetHeadingEdit->SetValue(mTargetOptions.mHeading);

   mUi.mSensorPDSpinBox->setValue(mSensorControl.mRequiredPD);
   mUi.mSensorAltitudeEdit->SetValue(mSensorControl.mAltitude);
   mUi.mSensorYawEdit->SetValue(mSensorControl.mYaw);
   mUi.mSensorPitchEdit->SetValue(mSensorControl.mPitch);
   mUi.mSensorRollEdit->SetValue(mSensorControl.mRoll);

   mUi.mJammerToSignalEdit->SetValue(mJammerControl.mSigRef);
}

void SPLAT::PlatformOptionsWidget::Connect()
{
   connect(mUi.mTargetHeadingCheckBox,
           &QCheckBox::stateChanged,
           this,
           [this](bool aState) { mUi.mTargetHeadingEdit->setEnabled(aState); });

   connect(mUi.mSensorPlatformToDetectEdit, &QLineEdit::textChanged, this, &PlatformOptionsWidget::TextChangedHandler);
   connect(mUi.mJammerNameEdit, &QLineEdit::textChanged, this, &PlatformOptionsWidget::TextChangedHandler);
}

void SPLAT::PlatformOptionsWidget::SetPlatformOptions()
{
   mTargetOptions.mAltitude = mUi.mTargetAltitudeEdit->GetValue();
   mTargetOptions.mSpeed    = mUi.mTargetSpeedEdit->GetValue();
   mTargetOptions.mYaw      = mUi.mTargetYawEdit->GetValue();
   mTargetOptions.mPitch    = mUi.mTargetPitchEdit->GetValue();
   mTargetOptions.mRoll     = mUi.mTargetRollEdit->GetValue();
   mTargetOptions.mHeading  = mUi.mTargetHeadingEdit->GetValue();

   mSensorControl.mModeName         = mUi.mSensorModeComboBox->currentText();
   mSensorControl.mPlatformToDetect = mUi.mSensorPlatformToDetectEdit->text();
   mSensorControl.mAltitude         = mUi.mSensorAltitudeEdit->GetValue();
   mSensorControl.mYaw              = mUi.mSensorYawEdit->GetValue();
   mSensorControl.mPitch            = mUi.mSensorPitchEdit->GetValue();
   mSensorControl.mRoll             = mUi.mSensorRollEdit->GetValue();
   mSensorControl.mRequiredPD       = mUi.mSensorPDSpinBox->value();
   mSensorControl.mAutoCue          = mUi.mAutoTargetCueingCheckBox->isChecked();

   mJammerControl.mPlatformName = mUi.mJammerNameEdit->text();
   mJammerControl.mSigRef       = mUi.mJammerToSignalEdit->GetValue();
}

bool SPLAT::PlatformOptionsWidget::PlatformOptionsValid()
{
   return mUi.mTargetAltitudeEdit->IsValid() && mUi.mTargetSpeedEdit->IsValid() && mUi.mTargetYawEdit->IsValid() &&
          mUi.mTargetPitchEdit->IsValid() && mUi.mTargetRollEdit->IsValid() && mUi.mTargetHeadingEdit->IsValid() &&
          mUi.mSensorAltitudeEdit->IsValid() && mUi.mSensorYawEdit->IsValid() && mUi.mSensorPitchEdit->IsValid() &&
          mUi.mSensorRollEdit->IsValid() && mUi.mSensorPlatformToDetectEdit->hasAcceptableInput() &&
          mUi.mJammerNameEdit->hasAcceptableInput();
}

void SPLAT::PlatformOptionsWidget::TextChangedHandler()
{
   QLineEdit* edit = (QLineEdit*)sender();
   UtQtText::SetValid(edit->hasAcceptableInput(), *edit);
}
