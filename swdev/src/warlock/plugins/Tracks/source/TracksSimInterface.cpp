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
#include "TracksSimInterface.hpp"

#include "TracksData.hpp"
#include "TracksSimCommands.hpp"
#include "UtMemory.hpp"
#include "WkSimEnvironment.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackId.hpp"
#include "WsfTrackManager.hpp"
#include "visibility/WkfTrackVisibilityPrefObject.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_ObjectInfo.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

WkTracks::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<TrackEvent>(aPluginName)
   , mTrackRequestManager(nullptr)
   , mTrackPrefs()
   , mSimulation(nullptr)
{
}

void WkTracks::SimInterface::SetTrackPrefs(const wkf::TrackVisibilityPrefData& aTrackVisibilityData,
                                           const std::map<QString, bool>&      aTeamVisibility)
{
   QMutexLocker locker(&mMutex);

   mTrackPrefs.mTrackVisibilityData = aTrackVisibilityData;
   mTrackPrefs.mTeamVisibilityMap   = aTeamVisibility;
}

// Request Tracks for one particular remote platform
void WkTracks::SimInterface::AddPlatformTrackRequestCommand(size_t aIndex)
{
   AddSimCommand(ut::make_unique<PlatformTracksRequestCommand>(aIndex, this));
}

// Request Tracks for all remote platforms that belong to any visible team
void WkTracks::SimInterface::AddAllTracksRequestCommand()
{
   QMutexLocker locker(&mMutex);
   AddSimCommand(ut::make_unique<AllTracksRequestCommand>(mTrackPrefs.mTeamVisibilityMap, this));
}

// Request Tracks for all remote platforms that belong to a particular team
void WkTracks::SimInterface::AddTeamTracksRequestCommand()
{
   QMutexLocker locker(&mMutex);
   AddSimCommand(ut::make_unique<TeamTracksRequestCommand>(mTrackPrefs.mTrackVisibilityData.mSelectedTeam, this));
}

// Remove track request for one particular remote platform
void WkTracks::SimInterface::RemovePlatformTrackRequestCommand(size_t aIndex)
{
   AddSimCommand(ut::make_unique<RemoveTrackRequestCommand>(aIndex, this));
}

// Add a track to its platform's track manager
void WkTracks::SimInterface::AddTrackToPlatform(const WsfLocalTrack& aTrack)
{
   WsfPlatform* platform = mSimulation->GetPlatformByName(aTrack.GetTrackId().GetOwningPlatformId());
   if (platform != nullptr)
   {
      WsfTrackManager& trackManager = platform->GetTrackManager();
      WsfLocalTrack*   track        = trackManager.FindTrack(aTrack.GetTrackId());
      if (track != nullptr)
      {
         // Update the track located within the TrackManager
         (*track) = aTrack;
         track->SetSimulation(trackManager.GetSimulation());
         // This call will cause the track manager's LocalTrackUpdated to be emitted
         trackManager.NotifyOfLocalTrackUpdated(mSimulation->GetSimTime(), track, track);
      }
      else
      {
         // This call will cause the track manager's LocalTrackInitiated to be emitted
         trackManager.AddTrack(mSimulation->GetSimTime(), aTrack);
      }
   }
}

void WkTracks::SimInterface::RemoveTrackFromPlatform(bool aIsRawTrack, const WsfTrackId& aTrackId)
{
   WsfPlatform* platform = mSimulation->GetPlatformByName(aTrackId.GetOwningPlatformId());
   if (platform != nullptr)
   {
      WsfTrackManager& trackManager = platform->GetTrackManager();
      if (aIsRawTrack)
      {
         trackManager.DropTrack(mSimulation->GetSimTime(), aTrackId);
      }
      else // Local Track
      {
         trackManager.RemoveTrack(aTrackId);
      }
   }
}

void WkTracks::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   using namespace std::placeholders;

   const bool        remote   = aPlatform.IsExternallyControlled();
   UtCallbackHolder& cbHolder = mCallbacksMap[aPlatform.GetIndex()];

   // Track Dropped
   cbHolder += aPlatform.GetTrackManager().LocalTrackDropped.Connect(&SimInterface::LocalTrackDroppedCB, this);

   // Track Initiated
   cbHolder += aPlatform.GetTrackManager().LocalTrackInitiated.Connect(
      std::bind(&SimInterface::LocalTrackInitiatedCB, this, _1, _2, _3, remote));

   // Track Updated
   cbHolder += aPlatform.GetTrackManager().LocalTrackUpdated.Connect(
      std::bind(&SimInterface::LocalTrackUpdatedCB, this, _1, _2, _3, remote));

   // When a platform is added, we may need its tracks
   // if the track visibility is ALL_VISIBLE_TEAMS or SELECTED_TEAM
   mMutex.lock();
   if (mTrackPrefs.mTrackVisibilityData.mTrackVisibility == wkf::tracks::Visibility::cALL_VISIBLE_TEAMS)
   {
      mMutex.unlock();
      AddAllTracksRequestCommand();
   }
   else if (mTrackPrefs.mTrackVisibilityData.mTrackVisibility == wkf::tracks::Visibility::cSELECTED_TEAM)
   {
      mMutex.unlock();
      AddTeamTracksRequestCommand();
   }
   else
   {
      mMutex.unlock();
   }
}

void WkTracks::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   mCallbacksMap.erase(aPlatform.GetIndex());
   AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatform.GetName()));
}

void WkTracks::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   WsfXIO_Interface* xioInterface = WsfXIO_Extension::Find(aSimulation);
   if (xioInterface != nullptr)
   {
      mTrackRequestManager = new WsfXIO_TrackRequestManager(xioInterface);
   }
   mSimulation = &aSimulation;
}

void WkTracks::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   mSimulation = nullptr;
   mCallbacksMap.clear();
   AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
}

void WkTracks::SimInterface::LocalTrackDroppedCB(double aSimTime, const WsfLocalTrack* aLocalTrackPtr)
{
   AddSimEvent(ut::make_unique<TrackDropEvent>(wkf::TrackId(aLocalTrackPtr->GetTrackId().GetOwningPlatformId(),
                                                            aLocalTrackPtr->GetTrackId().GetLocalTrackNumber())));
}

void WkTracks::SimInterface::LocalTrackInitiatedCB(double               aSimTime,
                                                   const WsfLocalTrack* aLocalTrackPtr,
                                                   const WsfTrack*      aRawTrackPtr,
                                                   bool                 aRemote)
{
   TrackContainer::TrackData data;
   data.Populate(aLocalTrackPtr);
   if (mSimulation)
   {
      auto* platform = mSimulation->GetPlatformByIndex(aLocalTrackPtr->GetTargetIndex());
      if (platform)
      {
         data.mMilStdIcon = platform->GetIcon();
      }
   }
   data.mRemote = aRemote;
   AddSimEvent(ut::make_unique<TrackInitiatedEvent>(data));
}

void WkTracks::SimInterface::LocalTrackUpdatedCB(double               aSimTime,
                                                 const WsfLocalTrack* aLocalTrackPtr,
                                                 const WsfTrack*      aRawTrackPtr,
                                                 bool                 aRemote)
{
   TrackContainer::TrackData data;
   data.Populate(aLocalTrackPtr);
   if (mSimulation)
   {
      auto* platform = mSimulation->GetPlatformByIndex(aLocalTrackPtr->GetTargetIndex());
      if (platform)
      {
         data.mMilStdIcon = platform->GetIcon();
      }
   }
   data.mRemote = aRemote;
   AddSimEvent(ut::make_unique<TrackUpdatedEvent>(data));
}
