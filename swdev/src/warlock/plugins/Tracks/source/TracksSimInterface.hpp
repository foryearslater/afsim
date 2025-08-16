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
#ifndef TRACKSSIMINTERFACE_HPP
#define TRACKSSIMINTERFACE_HPP

#include <unordered_map>

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WkfTrack.hpp"
#include "visibility/WkfTrackVisibilityPrefObject.hpp"
class WsfLocalTrack;
class WsfSimulation;
class WsfTrack;
#include "TracksSimEvents.hpp"
#include "xio/WsfXIO_TrackRequest.hpp"

namespace WkTracks
{
struct TrackPrefs
{
   TrackPrefs()
      : mTrackVisibilityData()
      , mTeamVisibilityMap()
   {
   }

   wkf::TrackVisibilityPrefData mTrackVisibilityData;
   std::map<QString, bool>      mTeamVisibilityMap;
};

class SimInterface : public warlock::SimInterfaceT<TrackEvent>
{
public:
   SimInterface(const QString& aPluginName);

   void SetTrackPrefs(const wkf::TrackVisibilityPrefData& aTrackVisibilityData,
                      const std::map<QString, bool>&      aTeamVisibility);

   //***************************************************
   // Wrappers for sending Commands to the Simulation

   // Request Tracks for one particular remote platform
   void AddPlatformTrackRequestCommand(size_t aIndex);

   // Request Tracks for all remote platforms that belong to any visible team
   void AddAllTracksRequestCommand();

   // Request Tracks for all remote platforms that belong to a particular team
   void AddTeamTracksRequestCommand();

   void RemovePlatformTrackRequestCommand(size_t aIndex);

   //***************************************************
   // Simulation Thread functions

   // This method is meant to be called on the Simulation thread only
   void AddTrackToPlatform(const WsfLocalTrack& aTrack);
   void RemoveTrackFromPlatform(bool aIsRawTrack, const WsfTrackId& aTrackId);

   // This method is meant to be called on the Simulation thread only
   WsfXIO_TrackRequestManager* GetTrackRequestManager() const { return mTrackRequestManager; }

   //***************************************************
private:
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;

   void LocalTrackDroppedCB(double aSimTime, const WsfLocalTrack* aLocalTrackPtr);

   void LocalTrackInitiatedCB(double aSimTime, const WsfLocalTrack* aLocalTrackPtr, const WsfTrack* aRawTrackPtr, bool aRemote);

   void LocalTrackUpdatedCB(double aSimTime, const WsfLocalTrack* aLocalTrackPtr, const WsfTrack* aRawTrackPtr, bool aRemote);

   // allows for deletion of callbacks when platform is deleted (size_t key represents platform index)
   std::unordered_map<size_t, UtCallbackHolder> mCallbacksMap;
   WsfXIO_TrackRequestManager*                  mTrackRequestManager;

   TrackPrefs mTrackPrefs; // Track Visibility, etc to use for filtering track requests

   // WARNING: Only use this mSimulation object on the simulation thread (not GUI).
   //  Use of this should be minimal, it is the work around for not receiving the
   //  simulation when processing XIO packets such as WsfXIO_LocalTrackPkt.
   const WsfSimulation* mSimulation;
};
} // namespace WkTracks

#endif // TRACKSSIMINTERFACE_HPP
