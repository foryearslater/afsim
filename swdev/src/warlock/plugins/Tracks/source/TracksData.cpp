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

#include "TracksData.hpp"

#include "WsfLocalTrack.hpp"

void WkTracks::TrackContainer::TrackData::Populate(const WsfLocalTrack* aLocalTrackPtr)
{
   mTrackId        = wkf::TrackId(aLocalTrackPtr->GetTrackId().GetOwningPlatformId(),
                           aLocalTrackPtr->GetTrackId().GetLocalTrackNumber());
   mLocationValid  = aLocalTrackPtr->LocationValid();
   mVelocityValid  = aLocalTrackPtr->VelocityValid();
   mBearingValid   = aLocalTrackPtr->BearingValid();
   mRangeValid     = aLocalTrackPtr->RangeValid();
   mElevationValid = aLocalTrackPtr->ElevationValid();

   mElevation_rad = aLocalTrackPtr->GetElevation();
   mBearing_rad   = aLocalTrackPtr->GetBearing();
   mRange_m       = aLocalTrackPtr->GetRange();

   aLocalTrackPtr->GetLocationWCS(mPositionWCS);
   aLocalTrackPtr->GetVelocityWCS(mVelocityWCS);

   mSide           = aLocalTrackPtr->GetSideId();
   mQuality        = aLocalTrackPtr->GetQuality();
   mType           = aLocalTrackPtr->GetTypeId();
   mTargetPlatform = aLocalTrackPtr->GetTargetName();

   switch (aLocalTrackPtr->GetSpatialDomain())
   {
   case WSF_SPATIAL_DOMAIN_LAND:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_LAND;
      break;
   case WSF_SPATIAL_DOMAIN_AIR:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_AIR;
      break;
   case WSF_SPATIAL_DOMAIN_SURFACE:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_SURFACE;
      break;
   case WSF_SPATIAL_DOMAIN_SUBSURFACE:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_SUBSURFACE;
      break;
   case WSF_SPATIAL_DOMAIN_SPACE:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_SPACE;
      break;
   default:
      mSpatialDomain = wkf::SPATIAL_DOMAIN_UNKNOWN;
   }

   if (aLocalTrackPtr->IFF_StatusValid())
   {
      switch (aLocalTrackPtr->GetIFF_Status())
      {
      case WsfTrack::cIFF_FRIEND:
         mIFFStatus = wkf::cIFF_FRIEND;
         break;
      case WsfTrack::cIFF_NEUTRAL:
         mIFFStatus = wkf::cIFF_NEUTRAL;
         break;
      case WsfTrack::cIFF_FOE:
         mIFFStatus = wkf::cIFF_FOE;
         break;
      case WsfTrack::cIFF_AMBIGUOUS:
         mIFFStatus = wkf::cIFF_AMBIGUOUS;
         break;
      default:
         mIFFStatus = wkf::cIFF_UNKNOWN;
      }
   }
   else
   {
      mIFFStatus = wkf::cIFF_UNKNOWN;
   }

   // Should mRemote also be set here?
   // aLocalTrackPtr->GetSimulation() == nullptr on LocalTracks received over XIO (sometimes?) which makes setting mRemote difficult
}

std::map<int, WkTracks::TrackContainer::TrackData> WkTracks::TrackContainer::GetTrackData(const std::string& aPlatformName) const
{
   std::map<int, TrackData> retval;

   auto it = mTrackList.lower_bound({aPlatformName, 0});

   while (it != mTrackList.end() && it->first.GetPlatformName() == aPlatformName)
   {
      retval.emplace(it->first.GetLocalTrackId(), it->second);
      it++;
   }

   return retval;
}

const WkTracks::TrackContainer::TrackDataList& WkTracks::TrackContainer::GetAllTrackData() const
{
   return mTrackList;
}

void WkTracks::TrackContainer::AddTrack(const TrackData& aTrackData)
{
   // Should we check to make sure the track does not exist?
   mTrackList[aTrackData.mTrackId] = aTrackData;
   emit TrackInitiated(aTrackData);
}

void WkTracks::TrackContainer::Clear()
{
   mTrackList.clear();
}

void WkTracks::TrackContainer::DropTrack(const wkf::TrackId& aTrackId)
{
   auto it = mTrackList.find(aTrackId);
   if (it != mTrackList.end())
   {
      emit TrackDropped(aTrackId);
      mTrackList.erase(it);
   }
}

void WkTracks::TrackContainer::DropAllPlatformTracks(const std::string& aPlatformName)
{
   auto it = mTrackList.lower_bound({aPlatformName, 0});

   while (it != mTrackList.end() && it->first.GetPlatformName() == aPlatformName)
   {
      emit TrackDropped(it->first);
      it = mTrackList.erase(it);
   }
}

void WkTracks::TrackContainer::UpdateTrack(const TrackData& aTrackData)
{
   mTrackList[aTrackData.mTrackId] = aTrackData;
   emit TrackUpdated(aTrackData);
}
