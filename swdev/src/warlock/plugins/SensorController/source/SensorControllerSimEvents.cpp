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

#include "SensorControllerSimEvents.hpp"

#include "SensorControllerDataContainer.hpp"

//////////////////////////////////////////////////////////////
//! Process the SensorEvent by updating sensor data to the new values.
//!
//! @param aData The data holding all current sensor data
void WkSensorController::SensorEvent::Process(DataContainer& aData)
{
   aData.SetData(mPlatform, mSensor, mSensorId, mCanBeTurnedOn, mCanBeTurnedOff, mIsSlewable, mAzMin, mAzMax, mElMin, mElMax);
}

//////////////////////////////////////////////////////////////
//! Notify the GUI that the platform/sensor pair has completed cuing/slewing.
//!
//! @param aData The data holding all current sensor data
void WkSensorController::SlewCompleteEvent::Process(DataContainer& aData)
{
   aData.SlewComplete(mPlatform, mSensor);
}
