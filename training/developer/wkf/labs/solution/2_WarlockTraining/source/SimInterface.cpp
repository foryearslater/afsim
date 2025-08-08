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

#include "SimInterface.hpp"

#include "UtMemory.hpp"

#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

WarlockTraining::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<EventBase>(aPluginName)
{}

void WarlockTraining::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   std::map<std::string, PlatformData> platformData;

   // For each platform, add its information to dataForEvent.
   for(std::size_t i = 0; i < aSimulation.GetPlatformCount(); i++)
   {
      WsfPlatform* platform = aSimulation.GetPlatformEntry(i);
      if (platform)
      {
         PlatformData& dataBeingRead = platformData[platform->GetName()];

         //Get the position information for the platform
         platform->GetLocationLLA(dataBeingRead.mLatitude,
                                  dataBeingRead.mLongitude,
                                  dataBeingRead.mAltitude);

         //Dummy variables for in-out parameters.
         double pitch, roll;
         //Get the heading information for the platform
         platform->GetOrientationNED(dataBeingRead.mHeading, pitch, roll);
      }
   }

   // EXERCISE 1 TASK 1a
   // Create the UpdateEvent and then add it to the SimInterface
   // note: std::make_unique can't be used until c++14, 
   //       since AFSIM is c++11 we have to use ut::make_unique instead.
   AddSimEvent(ut::make_unique<UpdateEvent>(platformData));
}
