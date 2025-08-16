// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SensorVolumesNetwork.hpp"

WkSensorVolumes::Platform::FOV WkSensorVolumes::FOV_Data::GetData() const
{
   Platform::FOV retval;
   retval.mShape = static_cast<Platform::FOV::Shape>(mShape->Get());
   retval.mPoints.clear();

   const int pointCount = mPoints->Size();
   for (int i = 0; i < pointCount; i++)
   {
      const auto& point = mPoints->Get(i);
      retval.mPoints.append({point.mX->Get(), point.mY->Get()});
   }
   return retval;
}

void WkSensorVolumes::FOV_Data::SetData(const Platform::FOV& aFOV)
{
   mShape->Set(aFOV.mShape);
   mPoints->Clear();
   for (const auto& pt : aFOV.mPoints)
   {
      auto& point = mPoints->Append();
      point.mX->Set(pt.first);
      point.mY->Set(pt.second);
   }
}

WkSensorVolumes::Platform::AngleLimits WkSensorVolumes::AngleLimitsData::GetData() const
{
   Platform::AngleLimits retval;
   retval.mAzimuthLimits.first    = mMinAzimuth->Get();
   retval.mAzimuthLimits.second   = mMaxAzimuth->Get();
   retval.mElevationLimits.first  = mMinElevation->Get();
   retval.mElevationLimits.second = mMaxElevation->Get();
   return retval;
}

void WkSensorVolumes::AngleLimitsData::SetData(const Platform::AngleLimits& aLimits)
{
   mMinAzimuth->Set(aLimits.mAzimuthLimits.first);
   mMaxAzimuth->Set(aLimits.mAzimuthLimits.second);
   mMinElevation->Set(aLimits.mElevationLimits.first);
   mMaxElevation->Set(aLimits.mElevationLimits.second);
}

WkSensorVolumes::Platform::Beam WkSensorVolumes::BeamData::GetData() const
{
   Platform::Beam retval;
   retval.mFOV      = mFov->GetData();
   retval.mMinRange = mMinRange->Get();
   retval.mMaxRange = mMaxRange->Get();
   retval.mTilt     = mTilt->Get();

   retval.mSlewLimits = mSlewLimits->GetData();
   retval.mCueLimits  = mCueLimits->GetData();
   retval.mScanLimits = mScanLimits->GetData();

   retval.mBeamWidth.first  = mAzimuthBeamWidth->Get();
   retval.mBeamWidth.second = mElevationBeamWidth->Get();

   retval.mSlewMode          = static_cast<WsfArticulatedPart::SlewMode>(mSlewMode->Get());
   retval.mScanStabilization = static_cast<WsfEM_Antenna::ScanStabilization>(mScanStabilization->Get());
   retval.mScanMode          = static_cast<WsfEM_Antenna::ScanMode>(mScanMode->Get());

   return retval;
}

void WkSensorVolumes::BeamData::SetData(const Platform::Beam& aBeam)
{
   mFov->SetData(aBeam.mFOV);
   mMinRange->Set(aBeam.mMinRange);
   mMaxRange->Set(aBeam.mMaxRange);
   mTilt->Set(aBeam.mTilt);

   mSlewLimits->SetData(aBeam.mSlewLimits);
   mCueLimits->SetData(aBeam.mCueLimits);
   mScanLimits->SetData(aBeam.mScanLimits);

   mAzimuthBeamWidth->Set(aBeam.mBeamWidth.first);
   mElevationBeamWidth->Set(aBeam.mBeamWidth.second);

   mSlewMode->Set(aBeam.mSlewMode);
   mScanStabilization->Set(aBeam.mScanStabilization);
   mScanMode->Set(aBeam.mScanMode);
}

WkSensorVolumes::Platform::Mode WkSensorVolumes::ModeData::GetData() const
{
   Platform::Mode retval;
   retval.mName = mMode->Get();

   const int beamCount = mBeamList->Size();
   for (int i = 0; i < beamCount; i++)
   {
      const auto& beam = mBeamList->Get(i);
      retval.mBeamList.append(beam.GetData());
   }
   return retval;
}

void WkSensorVolumes::ModeData::SetData(const Platform::Mode& aMode)
{
   mMode->Set(aMode.mName);
   mBeamList->Clear();
   for (const auto& beam : aMode.mBeamList)
   {
      mBeamList->Append().SetData(beam);
   }
}

std::vector<WkSensorVolumes::Platform::Mode> WkSensorVolumes::SensorVolumePacket::GetModeList() const
{
   std::vector<Platform::Mode> retval;
   const int                   modeCount = mModeList->Size();
   for (int i = 0; i < modeCount; i++)
   {
      const auto& modeData = mModeList->Get(i);
      retval.push_back(modeData.GetData());
   }
   return retval;
}
