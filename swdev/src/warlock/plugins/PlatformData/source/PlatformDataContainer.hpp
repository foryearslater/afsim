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
#ifndef PLATFORMDATACONTAINER_HPP
#define PLATFORMDATACONTAINER_HPP

#include <map>
#include <string>

#include "DisEntityId.hpp"
#include "DisEntityType.hpp"

namespace WkPlatformData
{
struct PlatformData
{
   std::string name          = "";
   std::string side          = "";
   std::string platformType  = "";
   std::string spatialDomain = "";
   std::string locality      = "";

   size_t index = 0;

   DisEntityId   disId;
   DisEntityType disType;
   std::string   marking = "";

   double latitude          = 0.0;
   double longitude         = 0.0;
   double altitude_m        = 0.0;
   double speed_m           = 0.0;
   double mach              = 0.0;
   double heading_rad       = 0.0;
   double pitch_rad         = 0.0;
   double roll_rad          = 0.0;
   double damageFactor      = 0.0;
   double fuelCapacity      = 0.0;
   bool   fuelCapacityValid = false;
   double fuelCurrent       = 0.0;

   std::map<std::string, std::string> auxData;
};
} // namespace WkPlatformData
#endif
