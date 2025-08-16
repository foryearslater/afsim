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

#include "ProjectorSimCommands.hpp"

#include "WsfSimulation.hpp"

void Projector::InstallProjectorUpdaterCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platformPtr = aSimulation.GetPlatformByName(mPlatformName);
   if (platformPtr)
   {
      WsfSensor* sensorPtr = platformPtr->GetComponent<WsfSensor>(mSensorName);
      if (sensorPtr)
      {
         mLocalSimInterface->InstallUpdater(*sensorPtr);
      }
   }
}

void Projector::UninstallProjectorUpdaterCommand::Process(WsfSimulation& aSimulation)
{
   mLocalSimInterface->UninstallUpdater(mPlatformName, mSensorName);
}
