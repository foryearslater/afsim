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

#ifndef DATATYPES_HPP
#define DATATYPES_HPP

#include <QString>

#include "UtLLAPos.hpp"
#include "UtOptional.hpp"
#include "WsfTrack.hpp"

namespace WkTrackDetailsDisplay
{
struct TrackId
{
   TrackId()
      : mId(0)
   {
   }
   bool operator<(const TrackId& aRhs) const { return this->mId < aRhs.mId; }
   bool operator!=(const TrackId& aRhs) const { return this->mId != aRhs.mId; }
   int  mId;
};

struct TrackTableData
{
   QString GetIFF() const;
   QString GetPlatformType() const;
   QString GetSide() const;
   QString GetRange() const;
   QString GetRangeRate() const;
   QString GetSpeed() const;
   QString GetRelativeHeading() const;

   bool operator!=(const TrackTableData& aRhs) const;

   WsfTrack::IFF_Status      mIFF;
   ut::optional<std::string> mPlatformType;
   ut::optional<std::string> mSide;
   ut::optional<double>      mRange;
   ut::optional<double>      mRangeRate;
   ut::optional<double>      mSpeed;
   ut::optional<double>      mRelativeHeading;
};

struct LocalTrackData
{
   std::string mSpatialDomain;
   std::string mCorrelationMethod;
   std::string mFusionMethod;
   std::string mSensorName;
   std::string mSensorType;
   std::string mSensorMode;
   std::string mTrackType;
   std::string mOriginatorName;

   ut::optional<double> mRmsCovariance;
   ut::optional<double> mBearing;
   ut::optional<double> mElevation;
   ut::optional<double> mFrequency;
   ut::optional<double> mSignalToNoise;
   ut::optional<double> mSensorElevation;

   UtLLAPos               mSensorLLA;
   ut::optional<UtLLAPos> mLLA;

   bool mPrebriefed           = false;
   bool mTrackPurgeable       = false;
   bool mMultipleContributors = false;

   unsigned int mRawTrackCount;

   double mQuality;
   double mSemiAxisForward;
   double mSemiAxisSide;
   double mSemiAxisUp;
   double mUpdateCount;
   double mTrackStart;
   // Might include later
   // double mTrackAge;
   double mRangeError;
   double mRangeRateError;
   double mBearingError;
   double mElevationError;
   double mSensorAzimuth;
   double mSensorAzimuthError;
   double mSensorElevationError;
   double mTrackUpdate;
};

struct PlatformsTrackData
{
   std::vector<std::pair<TrackId, TrackTableData>> mTableData;
   LocalTrackData                                  mTrackData;
};
} // namespace WkTrackDetailsDisplay
#endif
