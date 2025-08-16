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

#ifndef TRACKSDATA_HPP
#define TRACKSDATA_HPP

#include <string>

#include <QObject>

class WsfLocalTrack;

#include "WkfTrack.hpp"

namespace WkTracks
{
class TrackContainer : public QObject
{
   Q_OBJECT

public:
   struct TrackData
   {
      wkf::TrackId mTrackId;
      bool         mLocationValid  = false;
      double       mPositionWCS[3] = {0, 0, 0};
      bool         mVelocityValid  = false;
      double       mVelocityWCS[3] = {0, 0, 0};
      bool         mBearingValid   = false;
      double       mBearing_rad    = 0.0;
      bool         mElevationValid = false;
      double       mElevation_rad  = 0.0;
      bool         mRangeValid     = false;
      double       mRange_m        = 0.0;
      std::string  mSide;
      double       mQuality = 0.0;
      std::string  mType;
      std::string  mTargetPlatform;
      std::string  mMilStdIcon;
      bool         mRemote = false;

      wkf::SpatialDomain mSpatialDomain;
      wkf::IFF_Status    mIFFStatus;

      void Populate(const WsfLocalTrack* aLocalTrackPtr);
   };
   std::map<int, TrackData> GetTrackData(const std::string& aPlatformName) const;

   // map < platform name, map < local track index, data >>
   using TrackDataList = std::map<wkf::TrackId, TrackData>;

   const TrackDataList& GetAllTrackData() const;

   void AddTrack(const TrackData& aTrackData);
   void Clear();
   void UpdateTrack(const TrackData& aTrackData);
   void DropTrack(const wkf::TrackId& aTrackId);
   void DropAllPlatformTracks(const std::string& aPlatformName);

signals:
   void TrackInitiated(const TrackData& aTrackData);
   void TrackDropped(const wkf::TrackId& aTrackId);
   void TrackUpdated(const TrackData& aTrackData);

private:
   TrackDataList mTrackList;
};
} // namespace WkTracks
#endif
