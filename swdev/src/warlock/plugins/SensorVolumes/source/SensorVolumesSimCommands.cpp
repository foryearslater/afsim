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

#include "SensorVolumesSimCommands.hpp"

#include "SensorVolumesSimEvents.hpp"
#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "WsfSensor.hpp"
#include "WsfSimulation.hpp"

void WkSensorVolumes::SensorListRequest::Process(WsfSimulation& aSimulation)
{
   const WsfPlatform* platform = aSimulation.GetPlatformByIndex(mPlatformIndex);
   if (platform)
   {
      unsigned int sensorCount = platform->GetComponentCount<WsfSensor>();
      QStringList  sensorList;
      for (unsigned int sc = 0; sc < sensorCount; ++sc)
      {
         const WsfSensor* sensor = platform->GetComponentEntry<WsfSensor>(sc);
         if (sensor)
         {
            sensorList.append(sensor->GetName().c_str());
         }
      }
      AddSimEvent(ut::make_unique<ActiveSensorListEvent>(mPlatformIndex, sensorList));
   }
}

void WkSensorVolumes::WeaponListRequest::Process(WsfSimulation& aSimulation)
{
   const WsfPlatform* platform = aSimulation.GetPlatformByIndex(mPlatformIndex);
   if (platform)
   {
      unsigned int weaponCount = platform->GetComponentCount<WsfWeapon>();
      QStringList  weaponList;
      for (unsigned int wc = 0; wc < weaponCount; ++wc)
      {
         const WsfWeapon* weapon = platform->GetComponentEntry<WsfWeapon>(wc);
         if ((weapon) && (weapon->IsA_TypeOf("WSF_RF_JAMMER")))
         {
            weaponList.append(weapon->GetName().c_str());
         }
      }
      AddSimEvent(ut::make_unique<ActiveWeaponListEvent>(mPlatformIndex, weaponList));
   }
}
