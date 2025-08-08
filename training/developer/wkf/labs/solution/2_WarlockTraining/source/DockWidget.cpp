// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "DockWidget.hpp"

#include "WkfEnvironment.hpp"

#include "SimCommands.hpp"

WarlockTraining::DockWidget::DockWidget(SimInterface&   aSimInterface,
                                        DataContainer&  aDataContainer,
                                        PrefObject*     aPrefObject,
                                        Qt::WindowFlags aWindowFlags)
   : QDockWidget(nullptr, aWindowFlags)
   , mSimInterface(aSimInterface)
   , mDataContainer(aDataContainer)
{
   //! Without this line, nothing will show up in the dock widget.
   mUI.setupUi(this);

   // EXERCISE 1 TASK 2a
   // Connect the DataContainer's DataChanged signal to UpdateDisplay()
   // Connect the WkfEnvironment's PlatformOfInterestChanged signal to PlatformOfInterestChanged()
   connect(&mDataContainer, &DataContainer::DataChanged, this, &DockWidget::UpdateDisplay);
   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);

   // EXERCISE 2 TASK 3a
   // Connect the PrefObject's Changed() signal to PreferencesChanged()
   connect(aPrefObject, &WarlockTraining::PrefObject::Changed, this, &DockWidget::PreferencesChanged);

   // EXERCISE 1 TASK 3a
   // connect the clicked signal for northPushButton, eastPushButton, southPushButton, and westPushButton 
   // to TurnHeading with appropriate heading passed in as an argument
   connect(mUI.northPushButton, &QPushButton::clicked, this, [this]() { TurnToHeading(0); });
   connect(mUI.eastPushButton, &QPushButton::clicked, this, [this]() { TurnToHeading(90); });
   connect(mUI.southPushButton, &QPushButton::clicked, this, [this]() { TurnToHeading(180); });
   connect(mUI.westPushButton, &QPushButton::clicked, this, [this]() { TurnToHeading(270); });
}

void WarlockTraining::DockWidget::TurnToHeading(double aHeading)
{
   // EXERCISE 1 TASK 3b
   // Create the TurnCommand (arguments should the platform of interest and the desired heading)
   //  and then add it to the SimInterface
   // note: std::make_unique can't be used until c++14, 
   //       since AFSIM is c++11 we have to use ut::make_unique instead.
   mSimInterface.AddSimCommand(ut::make_unique<TurnCommand>(mPlatformOfInterest, aHeading));
}

void WarlockTraining::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   //Store the platform of interest 
   //note: if aPlatform == nullptr, that means there is no platform of interest,
   //      so clear mPlatformOfInterest
   if (aPlatform)
   {
      mPlatformOfInterest = aPlatform->GetName();
   }
   else
   {
      mPlatformOfInterest.clear();
   }
}

void WarlockTraining::DockWidget::PreferencesChanged(const PrefData& aPrefData)
{
   // EXERCISE 2 TASK 3b
   // Hide/Show the altitude/heading labels & lineEdits based on the user selection in Preferences.
   mUI.altitudeLabel->setVisible(aPrefData.mDisplayAltitude);
   mUI.altitudeLineEdit->setVisible(aPrefData.mDisplayAltitude);
   mUI.headingLabel->setVisible(aPrefData.mDisplayHeading);
   mUI.headingLineEdit->setVisible(aPrefData.mDisplayHeading);
}

void WarlockTraining::DockWidget::UpdateDisplay()
{
   //Get the data for the platform of interest from the DataContainer
   PlatformData data = mDataContainer.GetPlatformData(mPlatformOfInterest);
   mUI.nameOutput->setText(QString::fromStdString(mPlatformOfInterest));

   // EXERCISE 1 TASK 2b
   // Using the PlatformData, set the value to display in the latitude, longitude, altitude, and heading LineEdits
   mUI.latitudeLineEdit->SetValue(data.mLatitude);
   mUI.longitudeLineEdit->SetValue(data.mLongitude);
   mUI.altitudeLineEdit->SetValue(data.mAltitude);
   mUI.headingLineEdit->SetValue(data.mHeading);
}
