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
#include "PlatformDataInterface.hpp"

#include "Platform.hpp"
#include "ProxyWatcher.hpp"
#include "UtAirspeed.hpp"
#include "UtEntity.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPM_Root.hpp"

PlatformData::Interface::Interface(QObject* parent)
   : QObject(parent)
   , mIndex(SCENARIO)
{
}

void PlatformData::Interface::SetPlatformOfInterest(std::string aPlatformName)
{
   mPlatformData.name = aPlatformName;
   mIndex             = LOOKUP_REQUIRED;

   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wizard::Platform* curPlat = dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(aPlatformName));
      if (curPlat && WsfPM_Root(curPlat->GetPlatform().GetProxy()).platforms().Find(aPlatformName).IsValid())
      {
         UtEntity           tmpEntity = curPlat->ToUtEntity();
         WsfPM_Root         proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
         WsfPM_Platform     proxyPlatform = curPlat->GetPlatform();
         WsfPM_Route        proxyRoute    = proxyPlatform.Mover().InitialRoute();
         WsfProxy::Position pos           = proxyPlatform.InitialLocation();

         mPlatformData.latitude  = pos.GetLatitude();
         mPlatformData.longitude = pos.GetLongitude();
         // If a route is defined on the platform via a mover, the mover is responsible for the altitude reference
         // instead of the platform itself
         mPlatformData.altitude_reference = static_cast<int>(proxyPlatform.EffectiveAGL());
         mPlatformData.altitude_m         = UtLengthValue(proxyPlatform.EffectiveAltitude());
         mPlatformData.yaw                = UtAngleValue(proxyPlatform.Heading());
         mPlatformData.pitch              = UtAngleValue(proxyPlatform.Pitch());
         mPlatformData.roll               = UtAngleValue(proxyPlatform.Roll());
         mPlatformData.speed_m            = tmpEntity.GetSpeed();
         mPlatformData.mach = UtAirspeed::CalculateMachAtAltitude_m(mPlatformData.altitude_m, mPlatformData.speed_m);
         mPlatformData.state_valid = true;

         mPlatformData.icon = proxyPlatform.Icon();
         mPlatformData.side = proxyPlatform.Side();
         mPlatformData.type = proxyPlatform.GetTypeName();
      }
   }
}

PlatformData::Interface::PlatformData PlatformData::Interface::GetPlatformData()
{
   return mPlatformData;
}
