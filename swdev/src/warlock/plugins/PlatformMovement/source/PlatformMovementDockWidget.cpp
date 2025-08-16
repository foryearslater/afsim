// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "PlatformMovementDockWidget.hpp"

#include <QComboBox>
#include <QString>

#include "PlatformMovementSimCommands.hpp"
#include "PlatformMovementSimInterface.hpp"
#include "UtCallback.hpp"
#include "UtMemory.hpp"
#include "VaEntity.hpp"
#include "VaObserver.hpp"
#include "VaPosition.hpp"
#include "VaViewer.hpp"
#include "WkPermissions.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfSelectorWidgets.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfVtkEnvironment.hpp"
#include "ui_PlatformMovementDockWidget.h"

WkPlatformMovement::DockWidget::DockWidget(SimInterface*          aInterfacePtr,
                                           RouteBrowserInterface* aRouteInterface,
                                           QWidget*               parent,
                                           Qt::WindowFlags        f)
   : QDockWidget(parent, f)
   , mEntityPtr(nullptr)
   , mLatitudeBeforeSelection(0)
   , mLongitudeBeforeSelection(0)
   , mRouteInterfacePtr(aRouteInterface)
   , mSimInterfacePtr(aInterfacePtr)
{
   mUI.setupUi(this);

   // Map Entity and Location Selection
   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);

   // Routes
   connect(&mRouteInterfacePtr->GetGlobalRoutes(),
           &RouteContainer::RoutesInitializing,
           this,
           [this]() { mUI.mRouteComboBox->clear(); });

   connect(&mRouteInterfacePtr->GetGlobalRoutes(),
           &RouteContainer::RouteCreated,
           this,
           [this](const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
           { mUI.mRouteComboBox->addItem(aRouteInfo.mName); });

   connect(&mRouteInterfacePtr->GetGlobalRoutes(),
           &RouteContainer::RouteDeleted,
           this,
           [this](const QString& aRouteName)
           { mUI.mRouteComboBox->removeItem(mUI.mRouteComboBox->findText(aRouteName)); });

   connect(&mRouteInterfacePtr->GetGlobalRoutes(),
           &RouteContainer::RouteEdited,
           this,
           [this](const QString& aRouteName, int aWaypointCount)
           {
              mUI.mRouteComboBox->setCurrentText(aRouteName);
              mUI.mWaypointComboBox->clear();
              for (int i = 0; i < aWaypointCount; ++i)
              {
                 mUI.mWaypointComboBox->addItem(QString::number(i));
              }
           });

   // Command Execution Buttons
   connect(mUI.mAltitudeButton, &QPushButton::clicked, this, &DockWidget::GoToAltitude);
   connect(mUI.mHeadingButton, &QPushButton::clicked, this, &DockWidget::TurnToHeading);
   connect(mUI.mLocationButton, &QPushButton::clicked, this, &DockWidget::GoToLocation);
   connect(mUI.mSpeedButton, &QPushButton::clicked, this, &DockWidget::GoToSpeed);
   connect(mUI.mRouteButton, &QPushButton::clicked, this, &DockWidget::FollowRoute);
   connect(mUI.mReturnButton, &QPushButton::clicked, this, &DockWidget::ReturnToRoute);
   connect(mUI.mDelayButton, &QPushButton::clicked, this, &DockWidget::DelayRoute);

   // Optional Argument Check Boxes
   connect(mUI.mAltitudeRateOfChangeCheckBox, &QCheckBox::toggled, this, &DockWidget::AltitudeRateOfChangeToggled);
   connect(mUI.mLocationAltitudeCheckBox, &QCheckBox::toggled, this, &DockWidget::LocationAltitudeToggled);
   connect(mUI.mSpeedLinearAccelerationCheckBox, &QCheckBox::toggled, this, &DockWidget::SpeedLinearAccelerationToggled);
   connect(mUI.mHeadingRadialAccelerationCheckBox, &QCheckBox::toggled, this, &DockWidget::HeadingRadialAccelerationToggled);

   connect(mUI.mRouteComboBox,
           static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),
           this,
           &DockWidget::RouteComboBoxActivatedHandler);

   connect(mUI.mLocationMapButton, &wkf::LocationSelector::SelectionStarted, this, &DockWidget::LocationStarted);
   connect(mUI.mLocationMapButton, &wkf::LocationSelector::SelectionTerminated, this, &DockWidget::LocationTerminated);
   connect(mUI.mLocationMapButton, &wkf::LocationSelector::LocationChanged, this, &DockWidget::LocationChanged);
   connect(mUI.mLocationMapButton, &wkf::LocationSelector::Selected, this, &DockWidget::LocationSelected);

   mUI.mPlatformMovementToolBox->setEnabled(false);
}

void WkPlatformMovement::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform)
   {
      auto platformPtr = dynamic_cast<warlock::Platform*>(aPlatform);
      if (platformPtr)
      {
         // Check to make sure I can control the selected entity.
         if ((!platformPtr->GetExternallyControlled() || platformPtr->GetXIO_Controlled()) &&
             warlock::HasPermissionToControlPlatform(*platformPtr))
         {
            mEntityPtr = platformPtr;
            mUI.mPlatformMovementToolBox->setEnabled(true);
            mUI.mSelectedEntityLabel->setText("Selected Platform: " + QString::fromStdString(mEntityPtr->GetName()));
            Update();
         }
         else
         {
            mEntityPtr = nullptr;
            mUI.mPlatformMovementToolBox->setEnabled(false);
            mUI.mSelectedEntityLabel->setText("Selected Platform can not be controlled.");
         }
      }
   }
   else
   {
      mEntityPtr = nullptr;
      mUI.mPlatformMovementToolBox->setEnabled(false);
      mUI.mSelectedEntityLabel->setText("No Platform Selected");
   }
}

void WkPlatformMovement::DockWidget::LocationStarted()
{
   mLatitudeBeforeSelection  = mUI.latitudeLineEdit->GetValue();
   mLongitudeBeforeSelection = mUI.longitudeLineEdit->GetValue();
}

void WkPlatformMovement::DockWidget::LocationTerminated()
{
   mUI.latitudeLineEdit->SetValue(mLatitudeBeforeSelection);
   mUI.longitudeLineEdit->SetValue(mLongitudeBeforeSelection);
}

void WkPlatformMovement::DockWidget::LocationChanged(double aLatitude, double aLongitude)
{
   mUI.latitudeLineEdit->SetValue(aLatitude);
   mUI.longitudeLineEdit->SetValue(aLongitude);
}

void WkPlatformMovement::DockWidget::LocationSelected(double aLatitude, double aLongitude, double aAltitude, bool aAltitudeValid)
{
   if (aAltitudeValid && mUI.mLocationAltitudeCheckBox->isChecked())
   {
      mUI.mLocationAltitudeSpinBox->setValue(aAltitude);
   }

   // Since we are connected to the Selection Terminate signal, update these values so that the
   //  correct value is put into the Lat/Lon line edit when the LocationTerminated signal is handled.
   mLatitudeBeforeSelection  = aLatitude;
   mLongitudeBeforeSelection = aLongitude;
}

void WkPlatformMovement::DockWidget::GoToAltitude()
{
   if (mEntityPtr != nullptr)
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<AltitudeCommand>(
         mEntityPtr->GetName(),
         mUI.mAltitudeUnitsBox->ConvertToStandard(mUI.mAltitudeSpinBox->value()),
         mUI.mAltitudeRateOfChangeUnitsBox->ConvertToStandard(mUI.mAltitudeRateOfChangeSpinBox->value()),
         mUI.mAltitudeKeepRouteCheckBox->isChecked()));
   }
}

void WkPlatformMovement::DockWidget::GoToLocation()
{
   if (mEntityPtr != nullptr)
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<LocationCommand>(mEntityPtr->GetName(),
                                                                       mUI.latitudeLineEdit->GetValue(),
                                                                       mUI.longitudeLineEdit->GetValue(),
                                                                       mUI.mLocationAltitudeCheckBox->isChecked(),
                                                                       mUI.mLocationAltitudeUnitsBox->ConvertToStandard(
                                                                          mUI.mLocationAltitudeSpinBox->value()),
                                                                       mUI.mLocationImmediateCheckBox->isChecked()));
   }
}

void WkPlatformMovement::DockWidget::GoToSpeed()
{
   if (mEntityPtr != nullptr)
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<SpeedCommand>(
         mEntityPtr->GetName(),
         mUI.mSpeedUnitsBox->ConvertToStandard(mUI.mSpeedSpinBox->value()),
         mUI.mSpeedLinearAccelerationUnitsBox->ConvertToStandard(mUI.mSpeedLinearAccelerationSpinBox->value()),
         mUI.mSpeedKeepRouteCheckBox->isChecked()));
   }
}

void WkPlatformMovement::DockWidget::TurnToHeading()
{
   if (mEntityPtr != nullptr)
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<HeadingCommand>(
         mEntityPtr->GetName(),
         mUI.mHeadingUnitsBox->ConvertToStandard(mUI.mHeadingSpinBox->value()),
         mUI.mHeadingRadialAccelerationUnitsBox->ConvertToStandard(mUI.mHeadingRadialAccelerationSpinBox->value()),
         mUI.mHeadingAbsoluteButton->isChecked()));
   }
}

void WkPlatformMovement::DockWidget::FollowRoute()
{
   if (mEntityPtr != nullptr)
   {
      std::string routeName = mUI.mRouteComboBox->currentText().toStdString();
      for (auto& r : mRouteInterfacePtr->GetGlobalRoutes().GetRoutes())
      {
         if (r.first.GetName() == routeName)
         {
            mSimInterfacePtr->AddSimCommand(
               ut::make_unique<FollowRouteCommand>(mEntityPtr->GetName(),
                                                   r.first,
                                                   mUI.mWaypointComboBox->currentText().toInt()));
            break;
         }
      }
   }
}

void WkPlatformMovement::DockWidget::DelayRoute()
{
   if (mEntityPtr != nullptr)
   {
      double standardTime = mUI.mDelayComboBox->ConvertToStandard(mUI.mDelaySpinBox->value());
      mSimInterfacePtr->AddSimCommand(ut::make_unique<DelayRouteCommand>(mEntityPtr->GetName(), standardTime));
   }
}

void WkPlatformMovement::DockWidget::ReturnToRoute()
{
   mSimInterfacePtr->AddSimCommand(ut::make_unique<ReturnToRouteCommand>(mEntityPtr->GetName()));
}

void WkPlatformMovement::DockWidget::Update()
{
   if (mEntityPtr != nullptr)
   {
      // We are using the altitude of the VaEntity here, not the altitude received from AFSIM.
      // Due to extrapolation of VaEntity as they are moving along the ground, a platform at 0
      // altitude may fluctuate between 0.0 and a very small value (for example 1e-13). We don't
      // want the displays to be toggling the value constantly, so clamp the value to 0.0 when
      // less than 6 digits of precision
      double currentAltitude = mEntityPtr->GetPosition().GetAlt();
      if (std::abs(currentAltitude) < 0.00001)
      {
         currentAltitude = 0;
      }

      // Format the Position string
      std::ostringstream oss;
      oss << UtLatPos(mEntityPtr->GetPosition().GetLat(),
                      wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting())
          << ", "
          << UtLonPos(mEntityPtr->GetPosition().GetLon(),
                      wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());

      mUI.mCurrentPositionValue->setText(QString::fromStdString(oss.str()));
      mUI.mCurrentAltitudeLocationValue->SetValue(currentAltitude);
      mUI.mCurrentAltitudeValue->SetValue(currentAltitude);
      mUI.mCurrentHeadingValue->SetValue(mEntityPtr->GetHeading());
      mUI.mCurrentSpeedValue->SetValue(mEntityPtr->GetVelocityWCS().Magnitude());
   }
}

void WkPlatformMovement::DockWidget::AltitudeRateOfChangeToggled(bool aChecked)
{
   if (!aChecked)
   {
      mUI.mAltitudeRateOfChangeSpinBox->setValue(0.0);
   }

   mUI.mAltitudeRateOfChangeSpinBox->setEnabled(aChecked);
   mUI.mAltitudeRateOfChangeUnitsBox->setEnabled(aChecked);
}

void WkPlatformMovement::DockWidget::LocationAltitudeToggled(bool aChecked)
{
   if (!aChecked)
   {
      mUI.mLocationAltitudeSpinBox->setValue(0.0);
   }

   mUI.mLocationAltitudeSpinBox->setEnabled(aChecked);
   mUI.mLocationAltitudeUnitsBox->setEnabled(aChecked);
}

void WkPlatformMovement::DockWidget::SpeedLinearAccelerationToggled(bool aChecked)
{
   if (!aChecked)
   {
      mUI.mSpeedLinearAccelerationSpinBox->setValue(0.0);
   }

   mUI.mSpeedLinearAccelerationSpinBox->setEnabled(aChecked);
   mUI.mSpeedLinearAccelerationUnitsBox->setEnabled(aChecked);
}

void WkPlatformMovement::DockWidget::HeadingRadialAccelerationToggled(bool aChecked)
{
   if (!aChecked)
   {
      mUI.mHeadingRadialAccelerationSpinBox->setValue(0.0);
   }

   mUI.mHeadingRadialAccelerationSpinBox->setEnabled(aChecked);
   mUI.mHeadingRadialAccelerationUnitsBox->setEnabled(aChecked);
}

void WkPlatformMovement::DockWidget::RouteComboBoxActivatedHandler(const QString& aRouteName)
{
   mUI.mWaypointComboBox->clear();
   int waypointCount = emit RouteComboBoxSelected(aRouteName);
   for (int i = 0; i < waypointCount; ++i)
   {
      mUI.mWaypointComboBox->addItem(QString::number(i));
   }
}
