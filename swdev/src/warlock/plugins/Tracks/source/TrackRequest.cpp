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

#include "TrackRequest.hpp"

#include "xio/WsfXIO_PacketRegistry.hpp"

void WkTracks::TrackRequest::Handle(const WsfXIO_LocalTrackPkt& aPkt)
{
   if (aPkt.mTrack)
   {
      // Push the track to the platform's track manager's list of tracks
      mSimInterface->AddTrackToPlatform(*aPkt.mTrack);
   }
}

void WkTracks::TrackRequest::Handle(const WsfXIO_TrackDropPkt& aPkt)
{
   // Remove the track from the platform's track manager's list of tracks
   mSimInterface->RemoveTrackFromPlatform(aPkt.mIsRawTrack, aPkt.mTrackId);
}
