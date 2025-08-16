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

#include "TracksSimEvents.hpp"

void WkTracks::TrackDropEvent::Process(TrackContainer& aTrackContainer)
{
   aTrackContainer.DropTrack(mTrackId);
}

void WkTracks::TrackInitiatedEvent::Process(TrackContainer& aTrackContainer)
{
   aTrackContainer.AddTrack(mTrackData);
}

void WkTracks::TrackUpdatedEvent::Process(TrackContainer& aTrackContainer)
{
   aTrackContainer.UpdateTrack(mTrackData);
}

void WkTracks::PlatformDeletedEvent::Process(TrackContainer& aTrackContainer)
{
   aTrackContainer.DropAllPlatformTracks(mName);
}

void WkTracks::SimulationCompleteEvent::Process(TrackContainer& aTrackContainer)
{
   aTrackContainer.Clear();
}
