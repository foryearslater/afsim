// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "PlatformDataSimInterface.hpp"

#include "PlatformDataContainer.hpp"
#include "UtAirspeed.hpp"
#include "UtAttributeContainer.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WsfAttributeContainer.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"
#include "xio/WsfXIO_ObjectInfo.hpp"

WkPlatformData::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<PlatformDataEvent>(aPluginName)
   , mPlatformOfInterest("")
{
}

void WkPlatformData::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   mPlatformOfInterest = aPlatformName;
}

void WkPlatformData::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   std::string platformName = "";
   if (mMutex.tryLock())
   {
      platformName = mPlatformOfInterest;
      mMutex.unlock();
   }

   WsfPlatform* platform = aSimulation.GetPlatformByName(platformName);
   if (platform != nullptr)
   {
      PlatformData data;
      data.name = platformName;
      // default unit is radians for angles but GetLocationLLA() returns it in degree, so convert.
      platform->GetLocationLLA(data.latitude, data.longitude, data.altitude_m);
      data.platformType = platform->GetType();
      data.speed_m      = platform->GetSpeed();
      data.mach         = UtAirspeed::CalculateMachAtAltitude_m(data.altitude_m, data.speed_m);
      platform->GetOrientationNED(data.heading_rad, data.pitch_rad, data.roll_rad);
      data.side          = platform->GetSide();
      data.index         = platform->GetIndex();
      data.spatialDomain = WsfTypes::EnumToString(platform->GetSpatialDomain());
      data.damageFactor  = platform->GetDamageFactor();

      // If platform has a fuel object, retrieve fuel capacity.
      if (platform->GetFuel() != nullptr)
      {
         data.fuelCapacity      = platform->GetFuel()->GetMaximumQuantity();
         data.fuelCapacityValid = true;
      }
      else // Otherwise set fuel capacity to zero.
      {
         data.fuelCapacity      = 0;
         data.fuelCapacityValid = false;
      }
      data.fuelCurrent = platform->GetFuelMass();

      WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
      if (info != nullptr)
      {
         data.locality = "external";
      }
      else if (platform->IsExternallyControlled())
      {
         data.locality = "external (DIS only)";
      }
      else if (platform->IsExternallyMoved())
      {
         data.locality = "externally moved";
      }
      else
      {
         data.locality = "local";
      }

      data.disId           = DisEntityId();
      data.disType         = DisEntityType();
      data.marking         = "";
      WsfDisInterface* dis = WsfDisInterface::Find(aSimulation);
      if (dis != nullptr)
      {
         WsfDisPlatform* disPlatformPtr = dis->GetDisPlatform(platform->GetIndex());
         if (disPlatformPtr != nullptr)
         {
            data.disId   = disPlatformPtr->GetEntityId();
            data.disType = disPlatformPtr->GetEntityType();
            data.marking = platform->GetMarking();
         }
      }

      //***********************************************************************
      // Handle Aux Data
      for (const auto& iter : platform->GetAuxDataConst().GetAttributeMap())
      {
         std::string auxName  = iter.first;
         std::string auxValue = "unknown_type";

         const UtAttributeBase* attributePtr = iter.second.get();
         if (!attributePtr->IsContainerType())
         {
            // What kind?
            if (dynamic_cast<const UtIntAttribute*>(attributePtr) != nullptr)
            {
               auxValue = std::to_string(dynamic_cast<const UtIntAttribute*>(attributePtr)->Get());
            }
            else if (dynamic_cast<const UtDoubleAttribute*>(attributePtr) != nullptr)
            {
               auxValue = std::to_string(dynamic_cast<const UtDoubleAttribute*>(attributePtr)->Get());
            }
            else if (dynamic_cast<const UtStringAttribute*>(attributePtr) != nullptr)
            {
               auxValue = dynamic_cast<const UtStringAttribute*>(attributePtr)->Get();
            }
            else if (dynamic_cast<const UtBoolAttribute*>(attributePtr) != nullptr)
            {
               dynamic_cast<const UtBoolAttribute*>(attributePtr)->Get() ? auxValue = "true" : auxValue = "false";
            }
         }
         data.auxData[auxName] = auxValue;
      }
      AddSimEvent(ut::make_unique<UpdateEvent>(data));
   }
}
