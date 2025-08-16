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

#ifndef TRACKSSIMCOMMANDS_HPP
#define TRACKSSIMCOMMANDS_HPP

#include "WkSimInterface.hpp"
#include "WsfTrack.hpp"

class WsfXIO_Connection;

namespace WkTracks
{
class SimInterface;
class TrackRequest;

class RequestCommand : public warlock::SimCommand
{
public:
   RequestCommand(SimInterface* aSimInterface)
      : warlock::SimCommand()
      , mSimInterface(aSimInterface){};

protected:
   TrackRequest* CreateRequest(int aRemoteIndex, WsfXIO_Connection* aConnectionPtr);

   SimInterface* mSimInterface;
};

// Request Tracks for all remote platforms that belong to any visible team
class AllTracksRequestCommand : public RequestCommand
{
public:
   AllTracksRequestCommand(const std::map<QString, bool>& aTeamVisabilityMap, SimInterface* aSimInterface)
      : RequestCommand(aSimInterface)
      , mTeamVisabilityMap(aTeamVisabilityMap)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::map<QString, bool> mTeamVisabilityMap;
};

// Request Tracks for all remote platforms that belong to a particular team
class TeamTracksRequestCommand : public RequestCommand
{
public:
   TeamTracksRequestCommand(const QString& aTeamName, SimInterface* aSimInterface)
      : RequestCommand(aSimInterface)
      , mTeamName(aTeamName)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   QString mTeamName;
};

// Request Tracks for one particular remote platform
class PlatformTracksRequestCommand : public RequestCommand
{
public:
   PlatformTracksRequestCommand(size_t aPlatformIndex, SimInterface* aSimInterface)
      : RequestCommand(aSimInterface)
      , mPlatformIndex(aPlatformIndex)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   size_t mPlatformIndex;
};

// Remove track request for one particular remote platform
class RemoveTrackRequestCommand : public warlock::SimCommand
{
public:
   RemoveTrackRequestCommand(size_t aPlatformIndex, SimInterface* aSimInterface)
      : warlock::SimCommand()
      , mPlatformIndex(aPlatformIndex)
      , mSimInterface(aSimInterface)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   size_t        mPlatformIndex;
   SimInterface* mSimInterface;
};
} // namespace WkTracks
#endif