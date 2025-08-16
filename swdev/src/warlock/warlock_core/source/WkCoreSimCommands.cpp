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

#include "WkCoreSimCommands.hpp"

#include "UtEllipsoidalEarth.hpp"
#include "WkObserver.hpp"
#include "WkSimEnvironment.hpp"
#include "WsfPlatform.hpp"
#include "WsfRouteMover.hpp"
#include "WsfSimulation.hpp"
#include "xio/WsfXIO_ObjectInfo.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

void warlock::PlatformMovementCommand::Process(WsfSimulation& aSimulation)
{
   //*******************
   // This is nearly the same as the PlatformMovementCommand with instant = true;
   // When this is refactored consider moving the event to a common location

   WsfPlatform* platform = aSimulation.GetPlatformByName(mName);
   if (platform != nullptr)
   {
      if (platform->IsExternallyControlled())
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            double locationWCS[3];
            UtEllipsoidalEarth::ConvertLLAToECEF(mLatitude, mLongitude, mAltitude, locationWCS);

            WsfXIO_ReRoutePlatformPkt pkt;
            pkt.mPlatformIndex  = info->GetIndex();
            pkt.mLocationWCS[0] = locationWCS[0];
            pkt.mLocationWCS[1] = locationWCS[1];
            pkt.mLocationWCS[2] = locationWCS[2];

            pkt.mType = WsfXIO_ReRoutePlatformPkt::cSET_LOCATION;

            info->GetHostConnection()->Send(pkt);
         }
      }
      else
      {
         // Set the location for the platform
         platform->SetLocationLLA(mLatitude, mLongitude, mAltitude);

         // If the platform has a mover, it may need to be told that the position has changed.
         //  Note: This does not support all movers
         //  Currently, this supports WsfRouteMover and WsfP6DOF_Mover
         WsfMover* mover = platform->GetMover();
         if (mover != nullptr)
         {
            mover->ResetPosition(aSimulation.GetSimTime());
         }
         // Update the position of the platform
         platform->Update(aSimulation.GetSimTime());
      }
      WkObserver::SimulationUserAction(aSimulation)("Platform Moved", platform);
   }
}
