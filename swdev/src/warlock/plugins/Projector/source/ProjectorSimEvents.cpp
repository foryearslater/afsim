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

#include "ProjectorSimEvents.hpp"

#include "ProjectorPlugin.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

void Projector::UpdateEvent::Process(Plugin* aPluginPtr)
{
   aPluginPtr->UpdateSensor(mPlatformName, mSensorName, mSensorState);
}

void Projector::SensorProjectableEvent::Process(Plugin* aPluginPtr)
{
   aPluginPtr->SensorProjectableChanged(mPlatformName, mSensorName, mGood);
}
