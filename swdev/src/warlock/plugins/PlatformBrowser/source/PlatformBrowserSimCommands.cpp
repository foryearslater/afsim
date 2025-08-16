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

#include "PlatformBrowserSimCommands.hpp"

#include "WkObserver.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

void WkPlatformBrowser::DeletePlatformsCommand::Process(WsfSimulation& aSimulation)
{
   for (auto& name : mPlatformList)
   {
      WsfPlatform* platform = aSimulation.GetPlatformByName(name);
      if (platform != nullptr)
      {
         WkObserver::SimulationUserAction(aSimulation)("Platform Deleted", platform);
         if (platform->IsExternallyControlled())
         {
            WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
            if (info != nullptr)
            {
               WsfXIO_DeletePlatformPkt pkt;
               pkt.mPlatformIndices.push_back(info->GetIndex());
               info->GetHostConnection()->Send(pkt);
            }
         }
         else
         {
            aSimulation.DeletePlatform(aSimulation.GetSimTime(), platform);
         }
      }
   }
}
