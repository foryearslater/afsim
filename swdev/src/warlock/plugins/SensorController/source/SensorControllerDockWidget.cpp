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
#include "SensorControllerDockWidget.hpp"

#include "SensorControllerSimCommand.hpp"
#include "SensorControllerSimInterface.hpp"
#include "UtMath.hpp"
#include "UtQtUnitValueEdit.hpp"
#include "UtUnitTypes.hpp"
#include "UtUnits.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfUnitsObject.hpp"

// Constructor
WkSensorController::DockWidget::DockWidget(SimInterface*   aInterfacePtr,
                                           DataContainer&  aDataContainer,
                                           QWidget*        aParent /*= nullptr*/,
                                           Qt::WindowFlags aFlags /*= Qt::WindowFlags()*/)
   : QDockWidget(aParent, aFlags)
   , mInterfacePtr(aInterfacePtr)
   , mDataContainer(aDataContainer)
{
   mUI.setupUi(this);

   auto validatorPtr = new QDoubleValidator(this);
   mUI.azimuthLineEdit->setValidator(validatorPtr);
   mUI.elevationLineEdit->setValidator(validatorPtr);

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
   connect(mUI.sensorComboBox, &QComboBox::currentTextChanged, this, &DockWidget::SensorComboBoxChanged);
   connect(mUI.cueToAnglePushButton, &QPushButton::clicked, this, &DockWidget::CueToAngle);
   connect(mUI.cueToTrackPushButton, &QPushButton::clicked, this, &DockWidget::CueToTrack);
   connect(mUI.turnOffPushButton, &QPushButton::clicked, this, &DockWidget::TurnOff);
   connect(mUI.turnOnPushButton, &QPushButton::clicked, this, &DockWidget::TurnOn);
   connect(&mDataContainer, &DataContainer::SensorUpdated, this, &DockWidget::UpdateCurrentSensor);
   connect(&mDataContainer, &DataContainer::SlewComplete, this, &DockWidget::SlewComplete);

   Clear();
}

// When the platform selection changes, update the GUI to the first selected platform and add
// its sensors (if any)
void WkSensorController::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatformPtr)
{
   Clear();
   auto* platformPtr = dynamic_cast<warlock::Platform*>(aPlatformPtr);
   if (platformPtr != nullptr)
   {
      auto platformName = QString::fromStdString(platformPtr->GetName());
      mUI.platformLabel->setText(platformName);
      mUI.platformLabel->setVisible(true);
      auto sensors = platformPtr->GetSensorNames();
      if (sensors.empty())
      {
         mUI.sensorComboBox->addItem("NO SENSOR");
         mUI.sensorComboBox->setEnabled(false);
      }
      else
      {
         mUI.sensorComboBox->setEnabled(true);
         for (const auto& sensor : sensors)
         {
            mUI.sensorComboBox->addItem(QString::fromStdString(sensor));
         }
      }
   }
}

// Update the gui to the new sensor
void WkSensorController::DockWidget::SensorComboBoxChanged(const QString& aSensor)
{
   auto platform   = mUI.platformLabel->text().toStdString();
   auto sensorName = aSensor.toStdString();
   UpdateGui(mDataContainer.GetData(platform, sensorName));
}

// If the current sensor has been modified outside of the GUI, update the GUI to show the changes.
void WkSensorController::DockWidget::UpdateCurrentSensor(const std::string& aPlatform, const std::string& aSensor)
{
   auto currPlatform = mUI.platformLabel->text().toStdString();
   auto currSensor   = mUI.sensorComboBox->currentText().toStdString();
   if (aPlatform == currPlatform && aSensor == currSensor)
   {
      UpdateGui(mDataContainer.GetData(aPlatform, aSensor));
   }
}

// Update the GUI to the given sensor data
void WkSensorController::DockWidget::UpdateGui(const DataContainer::Sensor& aSensor)
{
   if (aSensor.mId != -1)
   {
      bool canSlewAndTurnOff = aSensor.mIsSlewable && aSensor.mCanBeTurnedOff;
      mSensorId              = aSensor.mId;
      mUI.angleGroupBox->setEnabled(canSlewAndTurnOff);
      mUI.trackGroupBox->setEnabled(canSlewAndTurnOff);
      mUI.turnOffPushButton->setEnabled(aSensor.mCanBeTurnedOff);
      mUI.turnOnPushButton->setEnabled(aSensor.mCanBeTurnedOn);

      auto    unitObj = wkfEnv.GetPreferenceObject<wkf::UnitsObject>();
      auto    azMin   = unitObj->GetFormattedValue(wkf::ValueType::cANGLE, aSensor.mAzMin);
      auto    azMax   = unitObj->GetFormattedValue(wkf::ValueType::cANGLE, aSensor.mAzMax);
      auto    elMin   = unitObj->GetFormattedValue(wkf::ValueType::cANGLE, aSensor.mElMin);
      auto    elMax   = unitObj->GetFormattedValue(wkf::ValueType::cANGLE, aSensor.mElMax);
      QString toolTip = "Cue Limits\n   Min: " + azMin + "\n   Max: " + azMax;
      mUI.azimuthLineEdit->setToolTip(toolTip);
      toolTip = "Cue Limits\n   Min: " + elMin + "\n   Max: " + elMax;
      mUI.elevationLineEdit->setToolTip(toolTip);
   }
}

// Clear the GUI to the default state of no platform or sensor selected
void WkSensorController::DockWidget::Clear()
{
   mUI.azimuthLineEdit->clear();
   mUI.elevationLineEdit->clear();
   mUI.sensorComboBox->clear();
   mUI.trackSelector->Clear();
   mUI.platformLabel->setVisible(false);
   mUI.turnOffPushButton->setEnabled(false);
   mUI.turnOnPushButton->setEnabled(false);
   mUI.angleGroupBox->setEnabled(false);
   mUI.trackGroupBox->setEnabled(false);
}

// Cue to the selected track
void WkSensorController::DockWidget::CueToTrack()
{
   if (!mUI.trackSelector->GetTrackId().IsNull())
   {
      auto platform = mUI.platformLabel->text().toStdString();
      auto trackId  = mUI.trackSelector->GetTrackId();
      mInterfacePtr->AddSimCommand(ut::make_unique<SlewToTrackCommand>(platform, mSensorId, trackId));

      // Clear previous "Slew Complete" message if exists
      wkfEnv.GetMainWindow()->ShowStatusMessage("");
   }
   else
   {
      QString warning = "The Track provided is invalid.";
      QMessageBox::warning(this, "Invaild value", warning);
   }
}

// Cue to the selected angle
void WkSensorController::DockWidget::CueToAngle()
{
   auto platform = mUI.platformLabel->text().toStdString();
   auto az       = mUI.azUnitsComboBox->ConvertToStandard(mUI.azimuthLineEdit->text().toDouble());
   auto el       = mUI.elUnitsComboBox->ConvertToStandard(mUI.elevationLineEdit->text().toDouble());
   mInterfacePtr->AddSimCommand(ut::make_unique<SlewToAzElCommand>(platform, mSensorId, az, el));
   // Clear previous "Slew Complete" message if exists
   wkfEnv.GetMainWindow()->ShowStatusMessage("");
}

// Turn off the current sensor
void WkSensorController::DockWidget::TurnOff()
{
   mInterfacePtr->AddSimCommand(ut::make_unique<TurnOffCommand>(mUI.platformLabel->text().toStdString(), mSensorId));
}

// Turn on the current sensor
void WkSensorController::DockWidget::TurnOn()
{
   mInterfacePtr->AddSimCommand(ut::make_unique<TurnOnCommand>(mUI.platformLabel->text().toStdString(), mSensorId));
}

// When the sensor is done cueing/slewing, alert the user that it has been complete
void WkSensorController::DockWidget::SlewComplete(const std::string& aPlatform, const std::string& aSensor)
{
   auto currPlatform = mUI.platformLabel->text().toStdString();
   auto currSensor   = mUI.sensorComboBox->currentText().toStdString();
   if (aPlatform == currPlatform && aSensor == currSensor)
   {
      wkfEnv.GetMainWindow()->ShowStatusMessage("Slew Complete");
   }
}
