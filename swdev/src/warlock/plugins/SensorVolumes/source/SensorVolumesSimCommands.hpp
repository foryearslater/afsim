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

#ifndef SENSORVOLUMESSIMCOMMANDS_HPP
#define SENSORVOLUMESSIMCOMMANDS_HPP

#include "SensorVolumesSimInterface.hpp"
#include "WkSimInterface.hpp"

namespace WkSensorVolumes
{
class SensorListRequest : public SimInterface::Command
{
public:
   SensorListRequest(SimInterface* aSimInterface, unsigned int aPlatformIndex)
      : Command(aSimInterface, true)
      , mPlatformIndex(aPlatformIndex)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   unsigned int mPlatformIndex;
};
class WeaponListRequest : public SimInterface::Command
{
public:
   WeaponListRequest(SimInterface* aSimInterface, unsigned int aPlatformIndex)
      : Command(aSimInterface, true)
      , mPlatformIndex(aPlatformIndex)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   unsigned int mPlatformIndex;
};
} // namespace WkSensorVolumes

#endif
