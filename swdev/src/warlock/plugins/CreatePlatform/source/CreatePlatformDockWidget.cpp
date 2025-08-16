// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CreatePlatformDockWidget.hpp"

#include <QDoubleValidator>

#include "CreatePlatformSimCommand.hpp"
#include "CreatePlatformSimInterface.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WkfSelectorWidgets.hpp"

WkCreatePlatform::DockWidget::DockWidget(SimInterface*   aInterfacePtr,
                                         DataContainer&  aCreatePlatformData,
                                         QWidget*        parent,
                                         Qt::WindowFlags f)
   : QDockWidget(parent, f)
   , mUI()
   , mCreatePlatformData(aCreatePlatformData)
   , mSimInterfacePtr(aInterfacePtr)
{
   mUI.setupUi(this);
   connect(mUI.mLocationButton, &wkf::LocationSelector::Selected, this, &DockWidget::LocationSelected);
   connect(mUI.mCreatePlatformButton, &QPushButton::clicked, this, &DockWidget::CreatePlatform);
   connect(&aCreatePlatformData, &DataContainer::TypeListChanged, this, &DockWidget::UpdateTypeComboBox);

   mUI.mAltitudeLineEdit->setValidator(new QDoubleValidator(0.0, 1000000.0, 6, mUI.mAltitudeLineEdit));
   mUI.mHeadingLineEdit->setValidator(new QDoubleValidator(-180.0, 360.0, 6, mUI.mHeadingLineEdit));
   mUI.mSpeedLineEdit->setValidator(new QDoubleValidator(0.0, 1000000.0, 6, mUI.mSpeedLineEdit));
}

void WkCreatePlatform::DockWidget::CreatePlatform()
{
   if (!warlock::HasPermissionToCreateOrRemovePlatforms())
   {
      QMessageBox::warning(this, "Platform not created", "User does not have permissions to create a platform.");
   }
   else
   {
      std::string name = mUI.mNameLineEdit->text().toStdString();

      if (mUI.mLatitudeLineEdit->IsValid() && mUI.mLatitudeLineEdit->IsValid())
      {
         std::string type    = mUI.mTypeComboBox->currentText().toStdString();
         std::string side    = mUI.mSideLineEdit->text().toStdString();
         double      lat     = mUI.mLatitudeLineEdit->GetValue();
         double      lon     = mUI.mLongitudeLineEdit->GetValue();
         double      alt     = mUI.mAltitudeUnitsBox->ConvertToStandard(mUI.mAltitudeLineEdit->text().toDouble());
         double      heading = mUI.mHeadingUnitsBox->ConvertToStandard(mUI.mHeadingLineEdit->text().toDouble());
         double      speed   = mUI.mSpeedUnitsBox->ConvertToStandard(mUI.mSpeedLineEdit->text().toDouble());

         mSimInterfacePtr->AddSimCommand(
            ut::make_unique<CreatePlatformCommand>(name, side, type, lat, lon, alt, heading, speed));
      }
      else
      {
         QMessageBox::warning(this,
                              "Platform not created",
                              "The platform \"" + QString::fromStdString(name) +
                                 "\" could not be created. (Invalid Lat/Lon)");
      }
   }
}

void WkCreatePlatform::DockWidget::LocationSelected(double aLatitude, double aLongitude, double aAltitude, bool aAltitudeValid)
{
   mUI.mAltitudeLineEdit->setText(QString::number(aAltitude));
   mUI.mLatitudeLineEdit->SetValue(aLatitude);
   mUI.mLongitudeLineEdit->SetValue(aLongitude);
}

void WkCreatePlatform::DockWidget::LocationChanged(double aLatitude, double aLongitude)
{
   mUI.mLatitudeLineEdit->SetValue(aLatitude);
   mUI.mLongitudeLineEdit->SetValue(aLongitude);
}

void WkCreatePlatform::DockWidget::UpdateTypeComboBox(const std::vector<std::string>& aTypeList)
{
   mUI.mTypeComboBox->clear();
   for (const auto& type : aTypeList)
   {
      mUI.mTypeComboBox->addItem(QString::fromStdString(type));
   }
}
