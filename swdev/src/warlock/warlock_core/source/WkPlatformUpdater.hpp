// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WkPlatformUpdater_HPP
#define WkPlatformUpdater_HPP

#include "WkSimEnvironment.hpp"
#include "WkSimInterface.hpp"
#include "WkfUnitUpdaters.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace warlock
{
template<class BASE_UPDATER>
class PlatformUpdaterT : public SimInterfaceBase, public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   PlatformUpdaterT(const QString& aUpdaterName, const std::string& aPlatformName)
      : SimInterfaceBase(aUpdaterName)
      , BaseUpdater(aUpdaterName, aPlatformName)
      , mLastUpdateTime(-1.0)
   {
   }

   ~PlatformUpdaterT() override = default;

   void PendingDelete() override
   {
      // Disconnect all of the (direct) connections to signals emitted from the simulation thread
      // so that the object isn't deleted from main thread while being accessed on other thread.
      if (warlock::SimEnvironment::Exists())
      {
         disconnect(&simEnv, nullptr, this, nullptr);
      }

      // Wait for any code that may be in a protected section to finish before exiting this function.
      // This lock will NOT be released.
      mMutex.lock();
   }

   void WallClockRead(const WsfSimulation& aSimulation) override
   {
      if (mMutex.tryLock())
      {
         WsfPlatform* platform = aSimulation.GetPlatformByName(BaseUpdater::GetPlatformOfInterest());
         if (platform != nullptr)
         {
            double updateTime = platform->GetLastUpdateTime();
            if (updateTime > mLastUpdateTime)
            {
               BaseUpdater::mDataValid = ReadData(platform);
               mLastUpdateTime         = updateTime;
            }
         }
         mMutex.unlock();
      }
   }

protected:
   // This should be const WsfPlatform* aPlatform, but some accessors in AFSIM are not const!
   virtual bool ReadData(WsfPlatform* aPlatform) = 0;

   double mLastUpdateTime;
};

using PlatformAccelerationUpdater        = PlatformUpdaterT<wkf::AccelerationValueUpdater>;
using PlatformAltitudeUpdater            = PlatformUpdaterT<wkf::AltitudeValueUpdater>;
using PlatformHighAltitudeUpdater        = PlatformUpdaterT<wkf::HighAltitudeValueUpdater>;
using PlatformAngleUpdater               = PlatformUpdaterT<wkf::AngleValueUpdater>;
using PlatformAngularAccelerationUpdater = PlatformUpdaterT<wkf::AngularAccelerationValueUpdater>;
using PlatformAngularRateUpdater         = PlatformUpdaterT<wkf::AngularRateValueUpdater>;
using PlatformForceUpdater               = PlatformUpdaterT<wkf::ForceValueUpdater>;
using PlatformFrequencyUpdater           = PlatformUpdaterT<wkf::FrequencyValueUpdater>;
using PlatformLengthUpdater              = PlatformUpdaterT<wkf::LengthValueUpdater>;
using PlatformMassUpdater                = PlatformUpdaterT<wkf::MassValueUpdater>;
using PlatformPowerUpdater               = PlatformUpdaterT<wkf::PowerValueUpdater>;
using PlatformPowerDBUpdater             = PlatformUpdaterT<wkf::PowerDBValueUpdater>;
using PlatformPressureUpdater            = PlatformUpdaterT<wkf::PressureValueUpdater>;
using PlatformRatioUpdater               = PlatformUpdaterT<wkf::RatioValueUpdater>;
using PlatformSpeedUpdater               = PlatformUpdaterT<wkf::SpeedValueUpdater>;
using PlatformTimeUpdater                = PlatformUpdaterT<wkf::TimeValueUpdater>;
using PlatformUnitlessUpdater            = PlatformUpdaterT<wkf::UnitlessValueUpdater>;
using PlatformUpdater                    = PlatformUpdaterT<wkf::Updater>;
using PlatformLatitudeUpdater            = PlatformUpdaterT<wkf::LatitudeValueUpdater>;
using PlatformLongitudeUpdater           = PlatformUpdaterT<wkf::LongitudeValueUpdater>;
} // namespace warlock
#endif
