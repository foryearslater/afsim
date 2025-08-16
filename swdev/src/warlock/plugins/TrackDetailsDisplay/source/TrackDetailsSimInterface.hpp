// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TRACKDETAILSSIMINTERFACE_HPP
#define TRACKDETAILSSIMINTERFACE_HPP

#include "TrackDetailsSimEvents.hpp"
#include "WkSimInterface.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackList.hpp"
#include "WsfTrackListFwd.hpp"

namespace WkTrackDetailsDisplay
{
class TrackDataExtractor
{
public:
   void                      Extract(WsfPlatform* aPlatform, const WsfTrackId& aTrackOfInterest);
   const PlatformsTrackData& GetExtractedData() const { return mData; }

private:
   void        ExtractLocalTrackTableData(WsfLocalTrackList& aLocalTrackList);
   void        ExtractLocalTrackData(WsfLocalTrack* aTrack);
   void        SetCovarianceEllipsoid(WsfLocalTrack* aTrack);
   double      TranslateRelativeHeading(const WsfLocalTrack* aLocalTrack);
   std::string TranslateTrackType(const WsfTrack::TrackType& aTrackType);

   WsfTrackId         mTrackId;
   PlatformsTrackData mData;
};

class TrackOfInterests
{
public:
   TrackOfInterests(QMutex& aSimMutex);
   void        SetTrackOfInterest(const int aDialogsTrack, const std::string& aPlatformName);
   void        SetTrackOfInterestId(WsfPlatform& aPlatform);
   std::string GetPlatform() const
   {
      QMutexLocker locker(&mSimMutex);
      return mPlatformOfInterest;
   }
   WsfTrackId GetTrackId() const
   {
      QMutexLocker locker(&mSimMutex);
      return mTrackOfInterestId;
   }

private:
   std::string mPlatformOfInterest;
   int         mTrackOfInterest;
   WsfTrackId  mTrackOfInterestId;
   QMutex&     mSimMutex;
};

class SimInterface : public warlock::SimInterfaceT<EventBase>, public TrackOfInterests, private TrackDataExtractor
{
public:
   SimInterface(const QString& aPluginName);
   int GetTrackName() { return GetTrackId().GetLocalTrackNumber(); }

private:
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   void WallClockRead(const WsfSimulation& aSimulation) override;
};
} // namespace WkTrackDetailsDisplay
#endif
