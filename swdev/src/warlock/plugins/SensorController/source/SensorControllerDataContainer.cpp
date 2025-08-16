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

#include "SensorControllerDataContainer.hpp"

//////////////////////////////////////////////////////////////
//! Update the map to include the given sensor data. If a platform or sensor has not been
//! previously found, add it.
//!
//! @param aPlatform The parent platform containing the sensor
//! @param aSensor The name of the sensor
//! @param aSensorId The sensor ID
//! @param aCanBeTurnedOn The flag describing if a sensor can be turned on (See WsfPlatformPart::CanBeTurnedOn)
//! @param aCanBeTurnedOff The flag describing if a sensor can be turned off (See WsfPlatformPart::CanBeTurnedOff)
//! @param aIsSlewable The flag describing if a sensor can be cued/slewed in the current mode
//! @param aMinAz The minimum azimuth angle the sensor can be cued/slewed
//! @param aMaxAz The maximum azimuth angle the sensor can be cued/slewed
//! @param aMinEl The minimum elevation angle the sensor can be cued/slewed
//! @param aMaxEl The maximum elevation angle the sensor can be cued/slewed
void WkSensorController::DataContainer::SetData(const std::string& aPlatform,
                                                const std::string& aSensor,
                                                int                aSensorId,
                                                bool               aCanBeTurnedOn,
                                                bool               aCanBeTurnedOff,
                                                bool               aIsSlewable,
                                                double             aMinAz,
                                                double             aMaxAz,
                                                double             aMinEl,
                                                double             aMaxEl)
{
   Sensor sensor(aSensorId, aCanBeTurnedOn, aCanBeTurnedOff, aIsSlewable, aMinAz, aMaxAz, aMinEl, aMaxEl);
   auto   platIt = mPlatformToSensorMap.find(aPlatform);
   if (platIt != mPlatformToSensorMap.end())
   {
      auto sensorIt = platIt->second.find(aSensor);
      if (sensorIt != platIt->second.end())
      {
         sensorIt->second = sensor;
         emit SensorUpdated(aPlatform, aSensor);
      }
      else
      {
         platIt->second.emplace(aSensor, sensor);
      }
   }
   else
   {
      std::map<std::string, Sensor> map;
      map.emplace(aSensor, sensor);
      mPlatformToSensorMap.emplace(aPlatform, map);
   }
}

//////////////////////////////////////////////////////////////
//! Get the data for the platform/sensor pair.
//!
//! @param aPlatform The desired platform
//! @param aSensor The desired sensor
//! @return The Sensor data container for aSensor on aPlatform, if the platform/sensor pair exists,
//!         returns empty otherwise.
WkSensorController::DataContainer::Sensor WkSensorController::DataContainer::GetData(const std::string& aPlatform,
                                                                                     const std::string& aSensor) const
{
   WkSensorController::DataContainer::Sensor sensor;
   auto                                      platIt = mPlatformToSensorMap.find(aPlatform);
   if (platIt != mPlatformToSensorMap.end())
   {
      auto sensorIt = platIt->second.find(aSensor);
      if (sensorIt != platIt->second.end())
      {
         sensor = sensorIt->second;
      }
   }
   return sensor;
}
