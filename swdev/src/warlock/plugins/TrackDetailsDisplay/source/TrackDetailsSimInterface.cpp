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


#include "TrackDetailsSimInterface.hpp"

#include "DataTypes.hpp"
#include "UtCovariance.hpp"
#include "UtCovarianceEllipsoid.hpp"
#include "UtMemory.hpp"
#include "WsfCorrelationStrategyTypes.hpp"
#include "WsfFusionStrategy.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackList.hpp"
#include "WsfTrackListFwd.hpp"
#include "WsfTrackManager.hpp"

void WkTrackDetailsDisplay::TrackDataExtractor::Extract(WsfPlatform* aPlatform, const WsfTrackId& aTrackOfInterest)
{
   mTrackId = aTrackOfInterest;
   mData.mTableData.clear();
   mData.mTrackData = LocalTrackData();
   ExtractLocalTrackTableData(aPlatform->GetMasterTrackList());

   WsfLocalTrack* localTrack = aPlatform->GetTrackManager().FindTrack(aTrackOfInterest);
   if (localTrack)
   {
      int numContributeTracks = localTrack->GetRawTrackIds().GetCount();
      if (numContributeTracks > 0)
      {
         const WsfTrackId* rawTrackId =
            aPlatform->GetTrackManager().FindTrack(aTrackOfInterest)->GetRawTrackIds().GetEntry(0);
         WsfTrack* rawTrack = aPlatform->GetTrackManager().FindRawTrack(*rawTrackId);

         if (rawTrack)
         {
            mData.mTrackData.mMultipleContributors = numContributeTracks > 1;

            if (rawTrack->GetTrackType() == WsfTrack::cFILTERED_SENSOR || WsfTrack::cUNFILTERED_SENSOR)
            {
               mData.mTrackData.mSensorName = rawTrack->GetSensorNameId().GetString();
               mData.mTrackData.mSensorType = rawTrack->GetSensorTypeId().GetString();
               mData.mTrackData.mSensorMode = rawTrack->GetSensorModeId().GetString();
            }
            else
            {
               mData.mTrackData.mSensorName = rawTrack->GetOriginatorNameId().GetString();
               mData.mTrackData.mSensorType = rawTrack->GetOriginatorTypeId().GetString();
               mData.mTrackData.mSensorMode = rawTrack->GetSensorModeId().GetString();
            }

            if (rawTrack->SensorElevationValid())
            {
               mData.mTrackData.mElevation      = rawTrack->GetElevation();
               mData.mTrackData.mElevationError = rawTrack->GetElevationError();
            }
            else
            {
               mData.mTrackData.mElevation = {};
            }

            rawTrack->SignalToNoiseValid() ? mData.mTrackData.mSignalToNoise = rawTrack->GetSignalToNoise() :
                                             mData.mTrackData.mSignalToNoise = {};

            rawTrack->FrequencyValid() ? mData.mTrackData.mFrequency = rawTrack->GetFrequency() :
                                         mData.mTrackData.mFrequency = {};

            WsfPlatform* sensorPlatform = aPlatform->GetSimulation()->GetPlatformByName(rawTrack->GetOriginatorNameId());
            if (sensorPlatform)
            {
               double lla[3];
               sensorPlatform->GetLocationLLA(lla[0], lla[1], lla[2]);
               mData.mTrackData.mSensorLLA.Set(lla[0], lla[1], lla[2]);
            }

            mData.mTrackData.mSensorAzimuth      = rawTrack->GetSensorAzimuth();
            mData.mTrackData.mSensorAzimuthError = rawTrack->GetSensorAzimuthError();
         }
      }
   }
}

void WkTrackDetailsDisplay::TrackDataExtractor::ExtractLocalTrackTableData(WsfLocalTrackList& aLocalTrackList)
{
   for (unsigned int i = 0; i < aLocalTrackList.GetTrackCount(); i++)
   {
      WsfLocalTrack* track = aLocalTrackList.GetTrackEntry(i);
      if (track)
      {
         TrackId        id;
         TrackTableData tableData;

         id.mId = track->GetTrackId().GetLocalTrackNumber();

         tableData.mIFF = track->GetIFF_Status();

         track->RangeValid() ? tableData.mRange = track->GetRange() : tableData.mRange = {};

         track->RangeRateValid() ? tableData.mRangeRate = track->GetRangeRate() : tableData.mRangeRate = {};

         track->VelocityValid() ? tableData.mSpeed = track->GetSpeed() : tableData.mSpeed = {};

         track->LocationValid() && track->BearingValid() ? tableData.mRelativeHeading = TranslateRelativeHeading(track) :
                                                           tableData.mRelativeHeading = {};

         track->TypeIdsValid() ? tableData.mPlatformType = track->GetTypeId().GetString() : tableData.mPlatformType = {};

         track->SideIdValid() ? tableData.mSide = track->GetSideId() : tableData.mSide = {};

         std::pair<TrackId, TrackTableData> idDataPair{id, tableData};

         mData.mTableData.push_back(idDataPair);

         if (mTrackId == track->GetTrackId())
         {
            ExtractLocalTrackData(track);
         }
      }
   }
}

void WkTrackDetailsDisplay::TrackDataExtractor::ExtractLocalTrackData(WsfLocalTrack* aTrack)
{
   mData.mTrackData.mSpatialDomain = WsfTypes::EnumToString(aTrack->GetSpatialDomain());

   mData.mTrackData.mPrebriefed = aTrack->IsPredefinedTrack();

   mData.mTrackData.mRawTrackCount = aTrack->GetTrackManager()->GetRawTrackCount();

   mData.mTrackData.mCorrelationMethod = aTrack->GetTrackManager()->GetCorrelationStrategy().GetTypeId().GetString();

   mData.mTrackData.mFusionMethod = aTrack->GetTrackManager()->GetFusionStrategy().GetTypeId().GetString();

   mData.mTrackData.mTrackPurgeable = aTrack->IsPurgeable();

   mData.mTrackData.mQuality = aTrack->GetTrackQuality();

   mData.mTrackData.mUpdateCount = aTrack->GetUpdateCount();

   mData.mTrackData.mTrackStart = aTrack->GetStartTime();

   mData.mTrackData.mTrackUpdate = aTrack->GetUpdateTime();

   // might include later
   // mData.mTrackData.mTrackAge = aTrack->GetSimulation()->GetSimTime() - aTrack->GetStartTime();

   SetCovarianceEllipsoid(aTrack);

   mData.mTrackData.mRangeError     = aTrack->GetRangeError();
   mData.mTrackData.mRangeRateError = aTrack->GetRangeRateError();

   aTrack->BearingValid() ? mData.mTrackData.mBearing = aTrack->GetBearing() : mData.mTrackData.mBearing = {};
   mData.mTrackData.mBearingError = aTrack->GetBearingError();

   aTrack->ElevationValid() ? mData.mTrackData.mElevation = aTrack->GetElevation() : mData.mTrackData.mElevation = {};
   mData.mTrackData.mElevationError = aTrack->GetElevationError();

   if (aTrack->LocationValid())
   {
      double lla[3];
      aTrack->GetLocationLLA(lla[0], lla[1], lla[2]);
      mData.mTrackData.mLLA = UtLLAPos{lla[0], lla[1], lla[2]};
   }
   else
   {
      mData.mTrackData.mLLA = {};
   }

   mData.mTrackData.mTrackType = TranslateTrackType(aTrack->GetTrackType());

   if (!aTrack->GetOriginatorNameId().IsNull())
   {
      mData.mTrackData.mOriginatorName = aTrack->GetOriginatorNameId().GetString();
   }
   else
   {
      mData.mTrackData.mOriginatorName = "";
   };
}

void WkTrackDetailsDisplay::TrackDataExtractor::SetCovarianceEllipsoid(WsfLocalTrack* aTrack)
{
   if (aTrack->StateCovarianceMatrixValid())
   {
      UtMatrixd             matrix      = *aTrack->GetStateCovarianceMatrix();
      UtCovarianceEllipsoid ellipsoid   = UtCovarianceEllipsoid::CovarianceNED2Ellipsoid(matrix);
      mData.mTrackData.mSemiAxisForward = ellipsoid.GetSemiAxisForward();
      mData.mTrackData.mSemiAxisSide    = ellipsoid.GetSemiAxisSide();
      mData.mTrackData.mSemiAxisUp      = ellipsoid.GetSemiAxisUp();

      // Root-Mean Square of the data above.
      double rms = std::pow(mData.mTrackData.mSemiAxisForward, 2.0) + std::pow(mData.mTrackData.mSemiAxisSide, 2.0) +
                   std::pow(mData.mTrackData.mSemiAxisUp, 2.0);
      rms = rms / 3;
      rms = std::sqrt(rms);

      mData.mTrackData.mRmsCovariance = rms;
   }
   else
   {
      mData.mTrackData.mRmsCovariance = {};
   }
}

double WkTrackDetailsDisplay::TrackDataExtractor::TranslateRelativeHeading(const WsfLocalTrack* aLocalTrack)
{
   double   xyz[3];
   UtEntity originator;
   UtVec3d  originatorPosWCS = aLocalTrack->GetOriginatorLocationWCS();
   originatorPosWCS.Get(xyz[0], xyz[1], xyz[2]);
   originator.SetLocationWCS(xyz);

   UtVec3d targetPositionWCS = aLocalTrack->GetLocationWCS();
   targetPositionWCS.Get(xyz[0], xyz[1], xyz[2]);

   return UtMath::NormalizeAngle0_TwoPi(originator.RelativeBearing(xyz));
}

std::string WkTrackDetailsDisplay::TrackDataExtractor::TranslateTrackType(const WsfTrack::TrackType& aTrackType)
{
   switch (aTrackType)
   {
   case WsfTrack::cFILTERED_SENSOR:
      return "FILTERED_SENSOR";
   case WsfTrack::cUNFILTERED_SENSOR:
      return "UNFILTERED_SENSOR";
   case WsfTrack::cPREDEFINED:
      return "PREDEFINED";
   case WsfTrack::cPROCESSED:
      return "PROCESSED(Candidate)";
   case WsfTrack::cSTATIC_IMAGE:
      return "STATIC_IMAGE";
   case WsfTrack::cPSEUDO_SENSOR:
      return "PSEUDO_SENSOR";
   default:
      return "UNKNOWN";
   }
}

WkTrackDetailsDisplay::TrackOfInterests::TrackOfInterests(QMutex& aSimMutex)
   : mSimMutex(aSimMutex)
{
}

void WkTrackDetailsDisplay::TrackOfInterests::SetTrackOfInterest(const int aDialogsTrack, const std::string& aPlatformName)
{
   QMutexLocker locker(&mSimMutex);
   if (mTrackOfInterest != aDialogsTrack)
   {
      mTrackOfInterest = aDialogsTrack;
   }
   if (mPlatformOfInterest != aPlatformName)
   {
      mPlatformOfInterest = aPlatformName;
   }
}

void WkTrackDetailsDisplay::TrackOfInterests::SetTrackOfInterestId(WsfPlatform& aPlatform)
{
   QMutexLocker       locker(&mSimMutex);
   WsfLocalTrackList& list = aPlatform.GetMasterTrackList();

   if (mTrackOfInterest >= 0 && (unsigned int)mTrackOfInterest < list.GetTrackCount())
   {
      mTrackOfInterestId = list.GetTrackEntry(mTrackOfInterest)->GetTrackId();
   }
}

WkTrackDetailsDisplay::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<EventBase>(aPluginName)
   , TrackOfInterests(this->mMutex)
{
}

void WkTrackDetailsDisplay::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<AddPlatformEvent>(aPlatform.GetName()));
}

void WkTrackDetailsDisplay::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   SetTrackOfInterest(-1, "");
}

void WkTrackDetailsDisplay::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<DeletePlatformEvent>(aPlatform.GetName()));
}

void WkTrackDetailsDisplay::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   WsfPlatform* simPlatform = aSimulation.GetPlatformByName(GetPlatform());
   if (simPlatform)
   {
      SetTrackOfInterestId(*simPlatform);
      Extract(simPlatform, GetTrackId());
      AddSimEvent(ut::make_unique<UpdateEvent>(GetExtractedData()));
   }
}
