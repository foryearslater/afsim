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
#ifndef SENSORVOLUMES_NETWORK_HPP
#define SENSORVOLUMES_NETWORK_HPP

#include "SensorVolumesPlatform.hpp"
#include "WkNetwork.hpp"

namespace WkSensorVolumes
{
using namespace warlock::net;

struct PointData final : DeriveObject<PointData>
{
   explicit PointData(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   Field<Float32> mX{this, "x"};
   Field<Float32> mY{this, "y"};
};

//! Mirrors WkSensorVolumes::Platform::FOV
struct FOV_Data final : DeriveObject<FOV_Data>
{
   explicit FOV_Data(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   Platform::FOV GetData() const;
   void          SetData(const Platform::FOV& aFOV);

   enum Shape
   {
      cRECTANGULAR = 0,
      cCIRCULAR    = 1,
      cPOLYGONAL   = 2
   };
   Field<Int32>         mShape{this, "shape"};
   ListField<PointData> mPoints{this, "points"};
};

//! Mirrors WkSensorVolumes::Platform::AngleLimits
struct AngleLimitsData final : DeriveObject<AngleLimitsData>
{
   explicit AngleLimitsData(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   Platform::AngleLimits GetData() const;
   void                  SetData(const Platform::AngleLimits& aLimits);

   Field<Float64> mMinAzimuth{this, "min_azimuth"};
   Field<Float64> mMaxAzimuth{this, "max_azimuth"};
   Field<Float64> mMinElevation{this, "min_elevation"};
   Field<Float64> mMaxElevation{this, "max_elevation"};
};

//! Mirrors WkSensorVolumes::Platform::Beam
struct BeamData final : DeriveObject<BeamData>
{
   explicit BeamData(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   Platform::Beam GetData() const;
   void           SetData(const Platform::Beam& aBeam);

   Field<FOV_Data> mFov{this, "fov"};
   Field<Float32>  mMinRange{this, "min_range"};
   Field<Float32>  mMaxRange{this, "max_range"};
   Field<Float32>  mTilt{this, "tilt"};

   Field<AngleLimitsData> mSlewLimits{this, "slew_limits"};
   Field<AngleLimitsData> mCueLimits{this, "cue_limits"};
   Field<AngleLimitsData> mScanLimits{this, "scan_limits"};

   Field<Float64> mAzimuthBeamWidth{this, "azimuth_beam_width"};
   Field<Float64> mElevationBeamWidth{this, "elevation_beam_width"};

   enum SlewMode
   {
      cSLEW_FIXED = 0,
      cSLEW_AZ    = 1,
      cSLEW_EL    = 2,
      cSLEW_AZ_EL = 3
   };
   Field<Int32> mSlewMode{this, "slew_mode"};

   enum ScanStabilization
   {
      cSS_NONE  = 0,
      cSS_PITCH = 1,
      cSS_ROLL  = 2,
      cSS_BOTH  = 3
   };
   Field<Int32> mScanStabilization{this, "scan_stabilization"};

   enum ScanMode
   {
      cSCAN_FIXED = 0,
      cSCAN_AZ    = 1,
      cSCAN_EL    = 2,
      cSCAN_AZ_EL = 3
   };
   Field<Int32> mScanMode{this, "scan_mode"};
};

//! Mirrors WkSensorVolumes::Platform::Mode
struct ModeData final : DeriveObject<ModeData>
{
   explicit ModeData(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   Platform::Mode GetData() const;
   void           SetData(const Platform::Mode& aMode);

   Field<String>       mMode{this, "mode"};
   ListField<BeamData> mBeamList{this, "beam_list"};
};

//! Contains all the passive sensor volume data for a single platform.
struct SensorVolumePacket final : DerivePacket<SensorVolumePacket>
{
   QString PacketType() const override { return "SensorVolumePacket"; }

   //! Returns the contents of mModeList.
   std::vector<Platform::Mode> GetModeList() const;

   Field<PlatformId>   mPlatform{this, "platform"};
   Field<String>       mSensor{this, "sensor"};
   ListField<ModeData> mModeList{this, "mode_list"};
};
} // namespace WkSensorVolumes

#endif
