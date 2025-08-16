// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef RELATIVEGEOMETRYUPDATER_HPP
#define RELATIVEGEOMETRYUPDATER_HPP

#include <QString>

#include "WkSimEnvironment.hpp"
#include "WkSimInterface.hpp"
#include "WkfUnitUpdaters.hpp"
#include "WkfUpdater.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace RelativeGeometry
{
template<class BASE_UPDATER>
class DualPlatformUpdaterT : public warlock::SimInterfaceBase, public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   DualPlatformUpdaterT(const QString& aUpdaterName, const QString& aFromPlatformName, const QString& aToPlatformName)
      : SimInterfaceBase(aUpdaterName)
      , BaseUpdater(aUpdaterName, aFromPlatformName.toStdString())
      , mLastUpdateTime(-1.0)
      , mToPlatformName(aToPlatformName.toStdString())
   {
   }

   ~DualPlatformUpdaterT() override
   {
      // Disconnect all of the (direct) connections to signals emitted from the simulation thread
      // so that the object isn't deleted from main thread while being accessed on other thread.
      if (warlock::SimEnvironment::Exists())
      {
         disconnect(&simEnv, nullptr, this, nullptr);
      }
      // Make sure the mutex is not locked prior to delete.
      QMutexLocker locker(&mMutex);
   }

   void WallClockRead(const WsfSimulation& aSimulation) override
   {
      if (mMutex.tryLock())
      {
         WsfPlatform* fromplatform = aSimulation.GetPlatformByName(BaseUpdater::GetPlatformOfInterest());
         WsfPlatform* toplatform   = aSimulation.GetPlatformByName(mToPlatformName);
         if (fromplatform && toplatform)
         {
            double updateTime = fromplatform->GetLastUpdateTime();
            if (updateTime > mLastUpdateTime)
            {
               if (fromplatform == toplatform)
               {
                  BaseUpdater::mDataValid = false;
               }
               else
               {
                  BaseUpdater::mDataValid = ReadData(fromplatform, toplatform);
                  mLastUpdateTime         = updateTime;
               }
            }
         }
         mMutex.unlock();
      }
   }

   QString GetValueString() const override
   {
      if (BaseUpdater::mDataValid)
      {
         return BaseUpdater::GetValueString();
      }
      return "INVALID";
   }

protected:
   // This should be const WsfPlatform* aPlatform, but some accessors in AFSIM are not const!
   virtual bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) = 0;

   double      mLastUpdateTime;
   std::string mToPlatformName;
};

using DualPlatformLengthUpdater = DualPlatformUpdaterT<wkf::LengthValueUpdater>;
using DualPlatformAngleUpdater  = DualPlatformUpdaterT<wkf::AngleValueUpdater>;
using DualPlatformSpeedUpdater  = DualPlatformUpdaterT<wkf::SpeedValueUpdater>;

class LinearRangeUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Linear Range";
   LinearRangeUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~LinearRangeUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class RadialUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Radial";
   RadialUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~RadialUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class InTrackUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "In Track";
   InTrackUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~InTrackUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class CrossTrackUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Cross Track";
   CrossTrackUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~CrossTrackUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class BearingUpdater : public DualPlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Bearing";
   BearingUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformAngleUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~BearingUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class ElevationUpdater : public DualPlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Elevation";
   ElevationUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformAngleUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~ElevationUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class DownRangeUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Down Range";
   DownRangeUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~DownRangeUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class CrossRangeUpdater : public DualPlatformLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Cross Range";
   CrossRangeUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformLengthUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~CrossRangeUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
class RangeRateUpdater : public DualPlatformSpeedUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Range Rate";
   RangeRateUpdater(const QString& aFromPlatformName = "", const QString& aToPlatformName = "")
      : DualPlatformSpeedUpdater(cUPDATER_NAME, aFromPlatformName, aToPlatformName)
   {
   }
   ~RangeRateUpdater() override = default;

protected:
   bool ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform) override;
};
} // namespace RelativeGeometry

#endif
