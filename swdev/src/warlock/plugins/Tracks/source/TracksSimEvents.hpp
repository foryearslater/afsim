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

#ifndef TRACKSSIMEVENTS_HPP
#define TRACKSSIMEVENTS_HPP

#include <string>

#include "TracksData.hpp"
#include "WkSimInterface.hpp"
#include "WsfTrackId.hpp"

namespace WkTracks
{
class TrackEvent : public warlock::SimEvent
{
public:
   virtual void Process(TrackContainer& aTrackContainer) = 0;
};

class SimulationCompleteEvent : public TrackEvent
{
public:
   SimulationCompleteEvent() = default;

   void Process(TrackContainer& aTrackContainer) override;
};

class TrackDropEvent : public TrackEvent
{
public:
   TrackDropEvent(const wkf::TrackId& aTrackId)
      : mTrackId(aTrackId)
   {
   }

   void Process(TrackContainer& aTrackContainer) override;

private:
   wkf::TrackId mTrackId;
};

class TrackInitiatedEvent : public TrackEvent
{
public:
   TrackInitiatedEvent(TrackContainer::TrackData& aTrackData)
      : mTrackData(aTrackData){};

   void Process(TrackContainer& aTrackContainer) override;

private:
   TrackContainer::TrackData mTrackData;
};

class TrackUpdatedEvent : public TrackEvent
{
public:
   TrackUpdatedEvent(TrackContainer::TrackData& aTrackData)
      : mTrackData(aTrackData){};

   void Process(TrackContainer& aTrackContainer) override;

private:
   TrackContainer::TrackData mTrackData;
};

class PlatformDeletedEvent : public TrackEvent
{
public:
   PlatformDeletedEvent(const std::string& aPlatformName)
      : mName(aPlatformName)
   {
   }

   void Process(TrackContainer& aTrackContainer) override;

private:
   std::string mName;
};
} // namespace WkTracks
#endif