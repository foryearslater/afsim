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

#include "TracksSimCommands.hpp"

#include "TrackRequest.hpp"
#include "UtCast.hpp"
#include "WkSimEnvironment.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"
#include "xio/WsfXIO_ObjectInfo.hpp"

WkTracks::TrackRequest* WkTracks::RequestCommand::CreateRequest(int aRemoteIndex, WsfXIO_Connection* aConnectionPtr)
{
   // The current implementation of WsfXIO_TrackRequestManager::AddRequest is such that multiple matching TrackRequests
   // could be created if IsGlobalRequest() returns false.  This occurs when sensor name or update interval are not set
   // to their default values.  Do not call SetSensorName() or SetUpdateInterval() on the TrackRequest without first
   // fixing the implementation within the TrackManager.
   TrackRequest* trackRequest = new TrackRequest(mSimInterface, ut::safe_cast<int, int32_t>(aRemoteIndex), aConnectionPtr);
   trackRequest->EnableSendInitialTracks();
   return trackRequest;
}

// Request Tracks for all remote platforms that belong to a visible team
void WkTracks::AllTracksRequestCommand::Process(WsfSimulation& aSimulation)
{
   WsfXIO_TrackRequestManager* trackManager = mSimInterface->GetTrackRequestManager();
   if (trackManager != nullptr)
   {
      const WsfXIO_RemotePlatformList* platformList = simEnv.GetXIO_Info().GetPlatformList();
      if (platformList != nullptr)
      {
         auto platforms = platformList->GetAllPlatforms();
         for (const auto& p : platforms)
         {
            const WsfXIO_PlatformInfo* info = p.second;
            if (info != nullptr)
            {
               auto mapEntry = mTeamVisabilityMap.find(QString::fromStdString(info->GetSide()));
               if (mapEntry != mTeamVisabilityMap.end())
               {
                  if (mapEntry->second)
                  {
                     trackManager->AddRequest(CreateRequest(info->GetIndex(), info->GetHostConnection()));
                  }
               }
            }
         }
      }
   }
}

// Request Tracks for all remote platforms that belong to a visible team
void WkTracks::TeamTracksRequestCommand::Process(WsfSimulation& aSimulation)
{
   WsfXIO_TrackRequestManager* trackManager = mSimInterface->GetTrackRequestManager();
   if (trackManager != nullptr)
   {
      const WsfXIO_RemotePlatformList* platformList = simEnv.GetXIO_Info().GetPlatformList();
      if (platformList != nullptr)
      {
         auto platforms = platformList->GetAllPlatforms();
         for (const auto& p : platforms)
         {
            const WsfXIO_PlatformInfo* info = p.second;
            if (info != nullptr && info->GetSide() == mTeamName.toStdString())
            {
               trackManager->AddRequest(CreateRequest(info->GetIndex(), info->GetHostConnection()));
            }
         }
      }
   }
}

// Request Tracks for one particular remote platform
void WkTracks::PlatformTracksRequestCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByIndex(mPlatformIndex);
   if (platform != nullptr)
   {
      WsfXIO_TrackRequestManager* trackManager = mSimInterface->GetTrackRequestManager();
      if (trackManager != nullptr)
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            trackManager->AddRequest(CreateRequest(info->GetIndex(), info->GetHostConnection()));
         }
      }
   }
}

void WkTracks::RemoveTrackRequestCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByIndex(mPlatformIndex);
   if (platform != nullptr)
   {
      WsfXIO_TrackRequestManager* trackManager = mSimInterface->GetTrackRequestManager();
      if (trackManager != nullptr)
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            GenUniqueId id = info->GetHostConnection()->GetApplicationId();
            trackManager->RemoveRequest(id, ut::safe_cast<int, int32_t>(info->GetIndex()));
         }
      }
   }
}
