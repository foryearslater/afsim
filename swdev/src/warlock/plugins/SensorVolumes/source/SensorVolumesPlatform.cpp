// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SensorVolumesPlatform.hpp"

#include <QColor>

#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "WsfAntennaPattern.hpp"
#include "WsfCircularFieldOfView.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfPolygonalFieldOfView.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorBeam.hpp"
#include "WsfSensorMode.hpp"
#include "sensor_volume/WkfAttachmentSensorVolume.hpp"
#include "sensor_volume/WkfSensorVolumesPrefObject.hpp"

WkSensorVolumes::Platform::FOV::FOV(const WsfArticulatedPart* aPart,
                                    const WsfEM_Antenna*      aAntenna,
                                    const WsfAntennaPattern*  aAntennaPattern,
                                    double                    aFrequency)
{
   if (!aAntenna->DefaultFieldOfView())
   {
      const WsfFieldOfView* fovPtr  = aAntenna->GetFieldOfView();
      WsfStringId           fovType = fovPtr->GetType();
      if (fovType == "circular")
      {
         mShape                               = cCircular;
         const WsfCircularFieldOfView* cirFOV = dynamic_cast<const WsfCircularFieldOfView*>(fovPtr);
         mPoints.push_back(qMakePair(cirFOV->GetHalfAngle(), 0.0f));
      }
      else if (fovType == "polygonal")
      {
         mShape                                = cPolygonal;
         const WsfPolygonalFieldOfView* polFOV = dynamic_cast<const WsfPolygonalFieldOfView*>(fovPtr);
         const WsfZoneDefinition&       zd     = polFOV->GetPoints();

         if (zd.GetShapeType() == WsfZoneDefinition::cPOLYGONAL)
         {
            const std::vector<WsfZone::Point>&          points = zd.GetPoints();
            std::vector<WsfZone::Point>::const_iterator it;
            for (it = points.begin(); it != points.end(); ++it)
            {
               mPoints.push_back(qMakePair(it->mX, it->mY));
            }
         }
      }
      else
      {
         mShape = cRectangular;

         double minAzFOV;
         double maxAzFOV;
         aAntenna->GetAzimuthFieldOfView(minAzFOV, maxAzFOV);
         double minElFOV;
         double maxElFOV;
         aAntenna->GetElevationFieldOfView(minElFOV, maxElFOV);
         mPoints.push_back(qMakePair(minAzFOV, minElFOV));
         mPoints.push_back(qMakePair(maxAzFOV, maxElFOV));
      }
   }
}

WkSensorVolumes::Platform::Mode::Mode(WsfSensorMode& aMode)
   : mName(aMode.GetName().c_str())
{
   size_t             beamCount  = aMode.GetBeamCount();
   WsfSensor*         sensor     = aMode.GetSensor();
   WsfEM_Antenna*     antennaPtr = sensor->GetEM_Rcvr(0).GetAntenna();
   double             freq       = sensor->GetEM_Rcvr(0).GetFrequency();
   WsfAntennaPattern* pattern = sensor->GetEM_Rcvr(0).GetAntennaPattern(sensor->GetEM_Rcvr(0).GetPolarization(), freq);
   if (beamCount > 0)
   {
      for (size_t i = 0; i < beamCount; ++i)
      {
         WsfSensorBeam* beam = aMode.GetBeamEntry(i);
         antennaPtr          = beam->GetEM_Rcvr()->GetAntenna();
         mBeamList.push_back(Beam(sensor, antennaPtr, pattern, freq));
      }
   }
   else
   {
      mBeamList.push_back(Beam(sensor, antennaPtr, pattern, freq));
   }
}

WkSensorVolumes::Platform::Mode::Mode(WsfWeapon::WsfWeaponMode& aMode)
   : mName(aMode.GetName().c_str())
{
   size_t count = aMode.GetWeapon()->GetEM_XmtrCount();
   for (size_t i = 0; i < count; ++i)
   {
      const WsfEM_Xmtr&  xmtr       = aMode.GetWeapon()->GetEM_Xmtr(i);
      double             freq       = xmtr.GetFrequency();
      WsfAntennaPattern* pattern    = xmtr.GetAntennaPattern(xmtr.GetPolarization(), freq);
      WsfEM_Antenna*     antennaPtr = xmtr.GetAntenna();
      mBeamList.push_back(Beam(aMode.GetWeapon(), antennaPtr, pattern, freq));
   }
}

WkSensorVolumes::Platform::Beam::Beam(const WsfArticulatedPart* aPart,
                                      const WsfEM_Antenna*      aAntenna,
                                      const WsfAntennaPattern*  aAntennaPattern,
                                      double                    aFrequency)
   : mFOV(aPart, aAntenna, aAntennaPattern, aFrequency)
   , mMinRange(aAntenna->GetMinimumRange())
   , mMaxRange(aAntenna->GetMaximumRange())
   , mTilt(aAntenna->GetPitch())
   , mSlewMode(aPart->GetSlewMode())
   , mSlewLimits(aPart->GetMinAzSlew(), aPart->GetMaxAzSlew(), aPart->GetMinElSlew(), aPart->GetMaxElSlew())
   , mCueLimits(aPart->GetMinAzCue(), aPart->GetMaxAzCue(), aPart->GetMinElCue(), aPart->GetMaxElCue())
   , mScanStabilization(aAntenna->GetScanStabilization())
   , mScanMode(aAntenna->GetScanMode())
   , mScanLimits(aAntenna->GetMinimumAzimuthScanLimit(),
                 aAntenna->GetMaximumAzimuthScanLimit(),
                 aAntenna->GetMinimumElevationScanLimit(),
                 aAntenna->GetMaximumElevationScanLimit())
{
   if (aAntennaPattern)
   {
      mBeamWidth = std::make_pair(aAntennaPattern->GetAzimuthBeamwidth(aFrequency, 0.0, 0.0),
                                  aAntennaPattern->GetElevationBeamwidth(aFrequency, 0.0, 0.0));
   }
   else
   {
      mBeamWidth = std::make_pair(0.0, 0.0);
   }
}

WkSensorVolumes::Platform::Platform(const wkf::SensorVolumesPrefObject* aPrefObject, bool aType)
   : mVisible(true)
   , mVolumeVisible(false)
   , mPrefObject(aPrefObject)
   , mType(aType)
{
}

void WkSensorVolumes::Platform::RemoveAttachments()
{
   for (auto& it : mSensorMap)
   {
      for (auto& jt : it.second.mModes)
      {
         vespa::VaEntity* ent = &(jt.second->GetParent());
         ent->RemoveAttachment(jt.second->GetUniqueId());
      }
   }
   mSensorMap.clear();
}

void WkSensorVolumes::Platform::SetPlatformVisibility(bool aVisible)
{
   mVisible = aVisible;
}

void WkSensorVolumes::Platform::SetVolumeVisibility(bool aState)
{
   for (auto& it : mSensorMap)
   {
      it.second.mState = aState;
      for (auto& jt : it.second.mModes)
      {
         int nb = jt.second->NumberBeams();
         for (int i = 0; i < nb; i++)
         {
            auto& beam = jt.second->GetBeam(i);
            beam.SetVisible(aState);
            beam.SetMarked(!aState);
         }
      }
   }
   mVolumeVisible = aState;
}

bool WkSensorVolumes::Platform::IsDefaultVisible() const
{
   return mVolumeVisible;
}

bool WkSensorVolumes::Platform::IsAtAllVisible() const
{
   bool all  = true;
   bool some = true;
   CheckSensorVisibility(all, some);
   return some;
}

void WkSensorVolumes::Platform::MarkAll(Source aSource)
{
   if (IsAtAllVisible())
   {
      for (auto& it : mSensorMap)
      {
         if (it.second.mSource == aSource || it.second.mSource == Source::Default || aSource == Source::Default)
         {
            for (auto& jt : it.second.mModes)
            {
               int numBeams = jt.second->NumberBeams();
               for (int i = 0; i < numBeams; ++i)
               {
                  jt.second->GetBeam(i).SetMarked(true);
               }
            }
         }
      }
   }
}

void WkSensorVolumes::Platform::HideMarked()
{
   if (IsAtAllVisible())
   {
      for (auto& it : mSensorMap)
      {
         for (auto jt : it.second.mModes)
         {
            int numBeams = jt.second->NumberBeams();
            for (int i = 0; i < numBeams; ++i)
            {
               wkf::SensorBeam& beam = jt.second->GetBeam(i);
               if (beam.IsMarked())
               {
                  beam.SetVisible(false);
               }
            }
         }
      }
   }
}

void WkSensorVolumes::Platform::UpdateAndUnmark(const std::string& aName,
                                                const double       aSlew[3],
                                                const double       aCue[3],
                                                const double       aTranslation[3],
                                                std::vector<Mode>  aModes,
                                                vespa::VaEntity*   aEntityPtr,
                                                vespa::VaViewer*   aViewerPtr,
                                                Source             aSource)
{
   if (aEntityPtr != nullptr)
   {
      SensorInfo& sensor     = mSensorMap[aName];
      auto        now        = Clock::now();
      sensor.mLastUpdateTime = now;
      sensor.mSource         = aSource;
      for (const Mode& m : aModes)
      {
         if (sensor.mModes.find(m.mName.toStdString()) == sensor.mModes.end()) // if we don't have the mode
         {
            auto* attachment = vespa::make_attachment<wkf::AttachmentSensorVolume>(*aEntityPtr,
                                                                                   aViewerPtr,
                                                                                   aName + "_" + m.mName.toStdString());
            sensor.mModes.emplace(m.mName.toStdString(), attachment);
         }
      }

      for (auto& a : sensor.mModes)
      {
         UpdateAttachment(aName, a.first, aSlew, aCue, aTranslation, a.second, aModes);
      }
   }
}

void WkSensorVolumes::Platform::ModesReset()
{
   for (auto& it : mSensorMap)
   {
      bool   showFaces;
      bool   showEdges;
      bool   showProjections;
      QColor fColor;
      QColor eColor;
      QColor pColor;
      for (auto& jt : it.second.mModes)
      {
         mPrefObject->GetModeDefinition(jt.second->ModeName(), mType, showFaces, fColor, showEdges, eColor, showProjections, pColor);
         jt.second->SetDisplayData(showFaces, fColor, showEdges, eColor, showProjections, pColor);
      }
   }
}

void WkSensorVolumes::Platform::SetSensorVisible(const std::string& aName, bool aState)
{
   auto& sensor  = mSensorMap[aName];
   sensor.mState = aState;

   for (auto& jt : sensor.mModes)
   {
      int nb = jt.second->NumberBeams();
      for (int i = 0; i < nb; i++)
      {
         auto& beam = jt.second->GetBeam(i);
         beam.SetVisible(aState);
         beam.SetMarked(!aState);
      }
   }
}

void WkSensorVolumes::Platform::CheckSensorVisibility(bool& aAll, bool& aSome) const
{
   if (mSensorMap.empty())
   {
      aAll  = mVolumeVisible;
      aSome = mVolumeVisible;
   }
   else
   {
      aSome = false;
      aAll  = true;
      for (const auto& it : mSensorMap)
      {
         const auto state = it.second.mState;
         if (!state.has_value())
         {
            // do nothing
         }
         else if (*state)
         {
            aSome = true;
         }
         else
         {
            aAll = false;
         }
      }
   }
}

bool WkSensorVolumes::Platform::SensorVisible(const std::string& aName, bool aUnsureReturn) const
{
   const auto& it = mSensorMap.find(aName);
   if (it != mSensorMap.end() && it->second.mState.has_value())
   {
      return *it->second.mState;
   }
   return aUnsureReturn;
}

std::vector<std::string> WkSensorVolumes::Platform::GetSensorList() const
{
   std::vector<std::string> retval;
   retval.reserve(mSensorMap.size());
   for (const auto& sensor : mSensorMap)
   {
      retval.push_back(sensor.first);
   }
   return retval;
}

bool WkSensorVolumes::Platform::LastPacketBefore(const std::string& aSensorName,
                                                 Clock::duration    aDuration,
                                                 Clock::time_point  aNow) const
{
   auto it = mSensorMap.find(aSensorName);
   if (it != mSensorMap.end())
   {
      const auto updated      = it->second.mLastPacketTime;
      const auto sinceUpdated = aNow - updated;
      return sinceUpdated > aDuration;
   }
   return false;
}

void WkSensorVolumes::Platform::SetLastPacketTime(const std::string& aSensorName, Clock::time_point aNow)
{
   auto it = mSensorMap.find(aSensorName);
   if (it != mSensorMap.end())
   {
      it->second.mLastPacketTime = aNow;
   }
}

void WkSensorVolumes::Platform::RemoveAttachmentsPastTimeout(Clock::duration aTimeout, Clock::time_point aNow)
{
   for (auto it = mSensorMap.begin(); it != mSensorMap.end(); /*no increment*/)
   {
      auto&      sensor       = it->second;
      const auto updated      = sensor.mLastUpdateTime;
      const auto sinceUpdated = aNow - updated;
      if (sinceUpdated > aTimeout && sensor.mSource == Source::Network)
      {
         for (auto& jt : sensor.mModes)
         {
            vespa::VaEntity* ent = &(jt.second->GetParent());
            ent->RemoveAttachment(jt.second->GetUniqueId());
         }
         it = mSensorMap.erase(it);
      }
      else
      {
         ++it;
      }
   }
}

void WkSensorVolumes::Platform::UpdateAttachment(const std::string&           aSensor,
                                                 const std::string&           aMode,
                                                 const double                 aSlew[3],
                                                 const double                 aCue[3],
                                                 const double                 aTranslation[3],
                                                 wkf::AttachmentSensorVolume* aAttachment,
                                                 std::vector<Mode>&           aModes)
{
   bool nonCueing = false;
   auto mode      = find(aModes.begin(), aModes.end(), aMode.c_str());
   if (mode != aModes.end())
   {
      unsigned int numBeams   = mode->mBeamList.size();
      bool         modeChange = aAttachment->ModeName().c_str() != mode->mName;
      bool         visible    = false;
      // If the platform is visible, then check to see if the sensor should be shown
      if (mVisible)
      {
         // Check the sensor state map to see if the sensor is "on"/visible
         const auto& it = mSensorMap.find(aSensor);
         if (it != mSensorMap.end() && it->second.mState.has_value())
         {
            visible = *it->second.mState;
         }
         else
         {
            // If the sensor does not exist in the map, default to an invisible sensor
            mSensorMap[aSensor].mState = mVolumeVisible;
            visible                    = mVolumeVisible;
         }
      }
      for (unsigned int i = 0; i < numBeams; ++i)
      {
         if (aAttachment->NumberBeams() <= i) // if the beams don't exist, add them
         {
            aAttachment->AddBeam();
         }
         wkf::SensorBeam& sensorBeam = aAttachment->GetBeam(i);
         sensorBeam.SetVisible(visible);
         sensorBeam.SetMarked(!visible);

         if (modeChange) // else if the mode is different, update
         {
            BuildBeam(mode->mBeamList[i], sensorBeam, nonCueing);

            bool   showFaces;
            bool   showEdges;
            bool   showProjections;
            QColor fColor;
            QColor eColor;
            QColor pColor;
            mPrefObject->GetModeDefinition(mode->mName.toStdString(),
                                           mType,
                                           showFaces,
                                           fColor,
                                           showEdges,
                                           eColor,
                                           showProjections,
                                           pColor);
            aAttachment->SetDisplayData(showFaces, fColor, showEdges, eColor, showProjections, pColor);
         }
      }
      aAttachment->SetModeName(mode->mName.toStdString());
   }
   wkf::SensorVolumesPrefData::DrawMode drawMode = mPrefObject->GetDrawMode();

   if (drawMode == wkf::SensorVolumesPrefData::cSLEW_VOLUME) // these are should not be rotated by the part
   {
      const double zero[3] = {0.0, 0.0, 0.0};
      aAttachment->Articulate(zero, zero, aTranslation); // articulate
   }
   else if ((drawMode == wkf::SensorVolumesPrefData::cCUE_VOLUME) || nonCueing)
   {
      const double zero[3] = {0.0, 0.0, 0.0};
      aAttachment->Articulate(aSlew, zero, aTranslation); // articulate
   }
   else
   {
      aAttachment->Articulate(aSlew, aCue, aTranslation); // articulate
   }
}

void WkSensorVolumes::Platform::BuildBeam(Beam& aBeam, wkf::SensorBeam& aSensorBeam, bool& aNonCueing)
{
   float minR = aBeam.mMinRange;
   float maxR = aBeam.mMaxRange;
   if (std::isinf(maxR))
   {
      maxR = 63710000.0; // radius of the earth times 10
   }
   wkf::SensorVolumesPrefData::DrawMode drawMode = mPrefObject->GetDrawMode();
   if (drawMode == wkf::SensorVolumesPrefData::cSLEW_VOLUME)
   {
      aSensorBeam.RebuildRectangular(0.0,
                                     minR,
                                     maxR,
                                     aBeam.mSlewLimits.mAzimuthLimits.first,
                                     aBeam.mSlewLimits.mElevationLimits.first,
                                     aBeam.mSlewLimits.mAzimuthLimits.second,
                                     aBeam.mSlewLimits.mElevationLimits.second);
   }
   else if (drawMode == wkf::SensorVolumesPrefData::cCUE_VOLUME)
   {
      aSensorBeam.RebuildRectangular(0.0,
                                     minR,
                                     maxR,
                                     aBeam.mCueLimits.mAzimuthLimits.first,
                                     aBeam.mCueLimits.mElevationLimits.first,
                                     aBeam.mCueLimits.mAzimuthLimits.second,
                                     aBeam.mCueLimits.mElevationLimits.second);
   }
   else if (drawMode == wkf::SensorVolumesPrefData::cSCAN_VOLUME)
   {
      aSensorBeam.RebuildRectangular(0.0,
                                     minR,
                                     maxR,
                                     aBeam.mScanLimits.mAzimuthLimits.first,
                                     aBeam.mScanLimits.mElevationLimits.first,
                                     aBeam.mScanLimits.mAzimuthLimits.second,
                                     aBeam.mScanLimits.mElevationLimits.second);
   }
   else if (drawMode == wkf::SensorVolumesPrefData::cBEAM_WIDTH)
   {
      double azMax = 0.0;
      double azMin = 0.0;
      double elMax = 0.0;
      double elMin = 0.0;
      if (aBeam.mScanMode & WsfEM_Antenna::cSCAN_AZ)
      {
         azMin = aBeam.mScanLimits.mAzimuthLimits.first;
         azMax = aBeam.mScanLimits.mAzimuthLimits.second;
      }
      else if (aBeam.mBeamWidth.first != 0.0)
      {
         azMax = 0.5 * aBeam.mBeamWidth.first;
         azMin = -azMax;
      }
      if (aBeam.mScanMode & WsfEM_Antenna::cSCAN_EL)
      {
         elMin = aBeam.mScanLimits.mElevationLimits.first;
         elMax = aBeam.mScanLimits.mElevationLimits.second;
      }
      else if (aBeam.mBeamWidth.second)
      {
         elMax = 0.5 * aBeam.mBeamWidth.second;
         elMin = -elMax;
      }
      aSensorBeam.RebuildRectangular(0.0, minR, maxR, azMin, elMin, azMax, elMax);
   }
   else if ((drawMode == wkf::SensorVolumesPrefData::cFIELD_OF_VIEW) ||
            (!aBeam.mFOV.mPoints.empty())) // we use FOV for calculated, when provided
   {
      int   shape = aBeam.mFOV.mShape;
      float tilt  = aBeam.mTilt;
      if (shape == FOV::cRectangular) // rectangular
      {
         float minAz = aBeam.mFOV.mPoints[0].first;
         float minEl = aBeam.mFOV.mPoints[0].second;
         float maxAz = aBeam.mFOV.mPoints[1].first;
         float maxEl = aBeam.mFOV.mPoints[1].second;

         aSensorBeam.RebuildRectangular(tilt, minR, maxR, minAz, minEl, maxAz, maxEl);
      }
      else if (shape == FOV::cCircular)
      {
         float halfAngle = aBeam.mFOV.mPoints[0].first;
         aSensorBeam.RebuildCircular(tilt, minR, maxR, halfAngle);
      }
      else if (shape == FOV::cPolygonal)
      {
         std::vector<std::pair<float, float>> points;

         int numPoints = aBeam.mFOV.mPoints.size();
         for (int j = 0; j < numPoints; ++j)
         {
            points.emplace_back(aBeam.mFOV.mPoints[j].first, aBeam.mFOV.mPoints[j].second);
         }
         aSensorBeam.RebuildPolygonal(tilt, minR, maxR, points);
      }
   }
   else // calculated
   {
      // this will return shapes mostly like those prior to 2.8
      double azMax = 0.0;
      double azMin = 0.0;
      double elMax = 0.0;
      double elMin = 0.0;
      // if scanning, that volume will be shown instead of the beam
      if (aBeam.mScanMode & WsfEM_Antenna::cSCAN_AZ)
      {
         azMin = aBeam.mScanLimits.mAzimuthLimits.first;
         azMax = aBeam.mScanLimits.mAzimuthLimits.second;
      }
      else if (aBeam.mBeamWidth.first != 0.0)
      {
         azMax = 0.5 * aBeam.mBeamWidth.first;
         azMin = -azMax;
      }
      else if (aBeam.mSlewMode & WsfArticulatedPart::cSLEW_AZ)
      {
         azMin      = aBeam.mSlewLimits.mAzimuthLimits.first;
         azMax      = aBeam.mSlewLimits.mAzimuthLimits.second;
         aNonCueing = true;
      }
      else
      {
         azMin = -UtMath::cPI;
         azMax = UtMath::cPI;
      }
      if (aBeam.mScanMode & WsfEM_Antenna::cSCAN_EL)
      {
         elMin = aBeam.mScanLimits.mElevationLimits.first;
         elMax = aBeam.mScanLimits.mElevationLimits.second;
      }
      else if (aBeam.mBeamWidth.second != 0.0)
      {
         elMax = 0.5 * aBeam.mBeamWidth.second;
         elMin = -elMax;
      }
      else if (aBeam.mSlewMode & WsfArticulatedPart::cSLEW_EL)
      {
         elMin      = aBeam.mSlewLimits.mElevationLimits.first;
         elMax      = aBeam.mSlewLimits.mElevationLimits.second;
         aNonCueing = true;
      }
      else
      {
         elMin = -UtMath::cPI_OVER_2;
         elMax = UtMath::cPI_OVER_2;
      }

      aSensorBeam.RebuildRectangular(0.0, minR, maxR, azMin, elMin, azMax, elMax);
   }
}
