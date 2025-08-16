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
#ifndef PlatformUpdater_HPP
#define PlatformUpdater_HPP

#include "WkfUnitUpdaters.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace wizard
{
template<class BASE_UPDATER>
class PlatformUpdaterT : public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   PlatformUpdaterT(const QString& aUpdaterName, const std::string& aPlatformName)
      : BaseUpdater(aUpdaterName, aPlatformName)
      , mLastUpdateTime(-1.0)
   {
   }

   ~PlatformUpdaterT() override = default;

   void SetValue(double aValue, bool aValid = true)
   {
      BaseUpdater::mValue     = aValue;
      BaseUpdater::mDataValid = aValid;
   }

protected:
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
} // namespace wizard
#endif
