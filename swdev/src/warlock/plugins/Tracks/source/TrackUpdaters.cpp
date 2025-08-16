// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TrackUpdaters.hpp"

#include "UtAirspeed.hpp"
#include "UtUnitTypes.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackId.hpp"
#include "WsfTrackManager.hpp"

void WkTracks::DataUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformId);
   if (platform)
   {
      WsfLocalTrack* track = platform->GetTrackManager().FindTrack(WsfTrackId(mPlatformId, mLocalTrackId));
      if (track)
      {
         double updateTime = track->GetUpdateTime();
         if (mLastUpdate < updateTime)
         {
            mData.emplace_back(std::pair<double, double>(updateTime, ReadDataP(track)));
            mLastUpdate = updateTime;
         }
      }
   }
}

WkTracks::AltitudeUpdater::AltitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAltitudeUnit(0);
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::LowAltitudeUnitChanged,
           this,
           [=](UtUnitLength::BaseUnit aUnit)
           { SetUnit(aUnit, QString::fromStdString(UtUnitLength::FindUnitName(aUnit))); });
}

double WkTracks::AltitudeUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double alt_m = 0;
   if (aTrack->LocationValid())
   {
      double lat_deg = 0;
      double lon_deg = 0;
      aTrack->GetLocationLLA(lat_deg, lon_deg, alt_m);
   }
   return UtUnitLength::ConvertFromStandard(alt_m, mUnitType);
}

WkTracks::BearingUpdater::BearingUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::AngleUnitChanged,
           this,
           [=](UtUnitAngle::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitAngle::FindUnitName(aUnit))); });
}

double WkTracks::BearingUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double bearing_rad = 0;
   if (aTrack->BearingValid())
   {
      bearing_rad = aTrack->GetBearing();
   }
   return UtUnitAngle::ConvertFromStandard(bearing_rad, mUnitType);
}

WkTracks::ElevationUpdater::ElevationUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::AngleUnitChanged,
           this,
           [=](UtUnitAngle::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitAngle::FindUnitName(aUnit))); });
}

double WkTracks::ElevationUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double elevation_rad = 0;
   if (aTrack->ElevationValid())
   {
      elevation_rad = aTrack->GetElevation();
   }
   return UtUnitAngle::ConvertFromStandard(elevation_rad, mUnitType);
}

WkTracks::HeadingUpdater::HeadingUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::AngleUnitChanged,
           this,
           [=](UtUnitAngle::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitAngle::FindUnitName(aUnit))); });
}

double WkTracks::HeadingUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double heading_rad = 0;
   if (aTrack->VelocityValid())
   {
      double loc[3], vel[3];
      ;
      aTrack->GetLocationWCS(loc);
      aTrack->GetVelocityWCS(vel);
      UtEntity entity;
      entity.SetLocationWCS(loc);
      entity.SetVelocityWCS(vel);
      entity.GetVelocityNED(vel);
      heading_rad = atan2(vel[1], vel[0]);
      // For display purposes put the valid heading range in [0, 2*PI]
      heading_rad = UtMath::NormalizeAngle0_TwoPi(heading_rad);
   }
   return UtUnitAngle::ConvertFromStandard(heading_rad, mUnitType);
}

WkTracks::LatitudeUpdater::LatitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::AngleUnitChanged,
           this,
           [=](UtUnitAngle::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitAngle::FindUnitName(aUnit))); });
}

double WkTracks::LatitudeUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double lat_deg = 0;
   if (aTrack->LocationValid())
   {
      double lon_deg = 0;
      double alt_m   = 0;
      aTrack->GetLocationLLA(lat_deg, lon_deg, alt_m);
   }
   lat_deg = UtUnitAngle::ConvertToStandard(lat_deg, UtUnitAngle::cDEGREES);
   return UtUnitAngle::ConvertFromStandard(lat_deg, mUnitType);
}

WkTracks::LongitudeUpdater::LongitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::AngleUnitChanged,
           this,
           [=](UtUnitAngle::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitAngle::FindUnitName(aUnit))); });
}

double WkTracks::LongitudeUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double lon_deg = 0;
   if (aTrack->LocationValid())
   {
      double lat_deg = 0;
      double alt_m   = 0;
      aTrack->GetLocationLLA(lat_deg, lon_deg, alt_m);
   }
   lon_deg = UtUnitAngle::ConvertToStandard(lon_deg, UtUnitAngle::cDEGREES);
   return UtUnitAngle::ConvertFromStandard(lon_deg, mUnitType);
}

WkTracks::RangeUpdater::RangeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::LengthUnitChanged,
           this,
           [=](UtUnitLength::BaseUnit aUnit)
           { SetUnit(aUnit, QString::fromStdString(UtUnitLength::FindUnitName(aUnit))); });
}

double WkTracks::RangeUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double range_m = 0;
   if (aTrack->RangeValid())
   {
      range_m = aTrack->GetRange();
   }
   return UtUnitLength::ConvertFromStandard(range_m, mUnitType);
}

WkTracks::SpeedUpdater::SpeedUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetSpeedUnit();
   SetUnit(unit, QString::fromStdString(UtUnitSpeed::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::SpeedUnitChanged,
           this,
           [=](UtUnitSpeed::BaseUnit aUnit) { SetUnit(aUnit, QString::fromStdString(UtUnitSpeed::FindUnitName(aUnit))); });
}

double WkTracks::SpeedUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   return UtUnitSpeed::ConvertFromStandard(aTrack->GetSpeed(), mUnitType);
}

WkTracks::MachUpdater::MachUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
   : DataUpdater(aPlatformName, aTrackId, aSeriesNum)
{
}

double WkTracks::MachUpdater::ReadDataP(const WsfLocalTrack* aTrack)
{
   double alt_m = 0;
   if (aTrack->LocationValid())
   {
      double lat_deg = 0;
      double lon_deg = 0;
      aTrack->GetLocationLLA(lat_deg, lon_deg, alt_m);
   }
   double speed_m = aTrack->GetSpeed();

   return UtAirspeed::CalculateMachAtAltitude_m(alt_m, speed_m);
}
