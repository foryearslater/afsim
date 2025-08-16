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

#ifndef SENSORVOLUMESPLATFORM_HPP
#define SENSORVOLUMESPLATFORM_HPP

#include <chrono>

#include <QObject>

#include "UtOptional.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Rcvr.hpp"

namespace vespa
{
class VaEntity;
class VaViewer;
} // namespace vespa

namespace wkf
{
class AttachmentSensorVolume;
class SensorBeam;
class SensorVolumesPrefObject;
} // namespace wkf

class WsfSensor;
class WsfSensorMode;
#include "WsfWeapon.hpp"

namespace WkSensorVolumes
{
using Clock = std::chrono::steady_clock;
class Platform
{
public:
   enum class Source
   {
      Default,
      SimInterface,
      Network
   };

   struct FOV
   {
      enum Shape
      {
         cNone        = -1,
         cRectangular = 0,
         cCircular    = 1,
         cPolygonal   = 2
      };
      QList<QPair<float, float>> mPoints;
      Shape                      mShape = Shape::cNone;

      FOV() = default;
      FOV(const WsfArticulatedPart* aPart,
          const WsfEM_Antenna*      aAntenna,
          const WsfAntennaPattern*  aAntennaPattern,
          double                    aFrequency);
   };

   struct AngleLimits
   {
      AngleLimits() = default;
      AngleLimits(double aAzMin, double aAzMax, double aElMin, double aElMax)
         : mAzimuthLimits(aAzMin, aAzMax)
         , mElevationLimits(aElMin, aElMax)
      {
      }
      std::pair<double, double> mAzimuthLimits;
      std::pair<double, double> mElevationLimits;
   };

   struct Beam
   {
      FOV                              mFOV;
      float                            mMinRange;
      float                            mMaxRange;
      float                            mTilt;
      WsfArticulatedPart::SlewMode     mSlewMode;
      AngleLimits                      mSlewLimits;
      AngleLimits                      mCueLimits;
      WsfEM_Antenna::ScanStabilization mScanStabilization;
      WsfEM_Antenna::ScanMode          mScanMode;
      AngleLimits                      mScanLimits;
      std::pair<double, double>        mBeamWidth;

      Beam() = default;
      Beam(const WsfArticulatedPart* aPart,
           const WsfEM_Antenna*      aAntenna,
           const WsfAntennaPattern*  aAntennaPattern,
           double                    aFrequency);
   };

   struct Mode
   {
      QString     mName;
      QList<Beam> mBeamList;

      Mode() = default;
      Mode(WsfSensorMode& aMode);
      Mode(WsfWeapon::WsfWeaponMode& aMode);

      bool operator==(const Mode& aMode) { return mName == aMode.mName; }

      bool operator==(const QString& aName) { return mName == aName; }
   };

   Platform(const wkf::SensorVolumesPrefObject* aPrefObject, bool aType);
   virtual ~Platform() = default;
   void RemoveAttachments();

   void SetPlatformVisibility(bool aVisible);
   void SetVolumeVisibility(bool aVisible);

   bool IsDefaultVisible() const;
   bool IsAtAllVisible() const;

   //! Marks all sensors with source aSource or 0 (default).
   //! If aSource == Source::Default, marks all.
   void MarkAll(Source aSource);
   void HideMarked();
   void UpdateAndUnmark(const std::string& aName,
                        const double       aSlew[3],
                        const double       aCue[3],
                        const double       aTranslation[3],
                        std::vector<Mode>  aModes,
                        vespa::VaEntity*   aEntityPtr,
                        vespa::VaViewer*   aViewerPtr,
                        Source             aSource);
   void ModesReset();
   void SetSensorVisible(const std::string& aName, bool aState);
   void CheckSensorVisibility(bool& aAll, bool& aSome) const;
   // aUnsureReturn is the value that will be returned if there isn't a conclusive answer
   bool SensorVisible(const std::string& aName, bool aUnsureReturn) const;

   //! Returns a list of the names of the sensors on the platform.
   std::vector<std::string> GetSensorList() const;

   //! Returns true if time aDuration has elapsed since sensor aSensorName was last updated over the network.
   //! Used to reduce over-sampling on the network.
   //! Returns false if sensor not found.
   bool LastPacketBefore(const std::string& aSensorName,
                         Clock::duration    aDuration,
                         Clock::time_point  aNow = Clock::now()) const;

   //! Sets the last packet-sent time for sensor aSensorName.
   //! Does nothing if sensor not found.
   void SetLastPacketTime(const std::string& aSensorName, Clock::time_point aNow);

   //! Removes all sensors last updated before the timeout.
   void RemoveAttachmentsPastTimeout(Clock::duration aTimeout, Clock::time_point aNow = Clock::now());

private:
   void UpdateAttachment(const std::string&           aSensor,
                         const std::string&           aMode,
                         const double                 aSlew[3],
                         const double                 aCue[3],
                         const double                 aTranslation[3],
                         wkf::AttachmentSensorVolume* aAttachment,
                         std::vector<Mode>&           aModes);
   void BuildBeam(Beam& aBeam, wkf::SensorBeam& aSensorBeam, bool& aNonCueing);

   struct SensorInfo
   {
      //! Map from mode to attachment.
      std::map<std::string, wkf::AttachmentSensorVolume*> mModes;
      Clock::time_point                                   mLastUpdateTime = Clock::now();
      Clock::time_point                                   mLastPacketTime = Clock::now();
      ut::optional<bool>                                  mState;
      Source                                              mSource = Source::Default;
   };

   std::map<std::string, SensorInfo>   mSensorMap;
   bool                                mVisible;
   bool                                mVolumeVisible;
   const wkf::SensorVolumesPrefObject* mPrefObject;
   bool                                mType;
};
} // namespace WkSensorVolumes

#endif // !SENSORVOLUMESPLATFORM_HPP
