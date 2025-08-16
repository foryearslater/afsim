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

#ifndef TRACKREQUEST_HPP
#define TRACKREQUEST_HPP

#include "TracksSimInterface.hpp"
#include "xio/WsfXIO_TrackRequest.hpp"

namespace WkTracks
{
class TrackRequest : public WsfXIO_TrackRequest
{
public:
   TrackRequest(SimInterface* aSimInterface, int aRemotePlatformIndex, WsfXIO_Connection* aConnectionPtr, bool aIsReliable = true)
      : WsfXIO_TrackRequest(aRemotePlatformIndex, aConnectionPtr, aIsReliable)
      , mSimInterface(aSimInterface)
   {
   }

   void Handle(const WsfXIO_LocalTrackPkt& aPkt) override;
   void Handle(const WsfXIO_TrackDropPkt& aPkt) override;

private:
   // Store the SimInterface so that Handle() may get access to the WsfSimulation object when a track is recieved.
   SimInterface* mSimInterface;
};
} // namespace WkTracks

#endif // !TRACKREQUEST_HPP
