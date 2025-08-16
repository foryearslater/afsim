// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformDataPlotUpdaters.hpp"

#include "UtAirspeed.hpp"
#include "UtUnitTypes.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

void WkPlatformData::PlotUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformId);
   if (platform != nullptr)
   {
      double updateTime = platform->GetLastUpdateTime();
      if (updateTime > mLastUpdateTime)
      {
         mLastUpdateTime = updateTime;
         mData.emplace_back(updateTime, ReadDataP(platform));
      }
   }
}

WkPlatformData::AltitudePlotUpdater::AltitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AltitudePlotUpdater::UnitChangedCB);
}

double WkPlatformData::AltitudePlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   return UtUnitLength::ConvertFromStandard(aPlatform->GetAltitude(), mUnitType);
}

void WkPlatformData::AltitudePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

WkPlatformData::LatitudePlotUpdater::LatitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &LatitudePlotUpdater::UnitChangedCB);
}

double WkPlatformData::LatitudePlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double lat, lon, alt;
   aPlatform->GetLocationLLA(lat, lon, alt);
   lat = UtUnitAngle::ConvertToStandard(lat, UtUnitAngle::cDEGREES);
   return UtUnitAngle::ConvertFromStandard(lat, mUnitType);
   ;
}

void WkPlatformData::LatitudePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkPlatformData::LongitudePlotUpdater::LongitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &LongitudePlotUpdater::UnitChangedCB);
}

double WkPlatformData::LongitudePlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double lat, lon, alt;
   aPlatform->GetLocationLLA(lat, lon, alt);
   lon = UtUnitAngle::ConvertToStandard(lon, UtUnitAngle::cDEGREES);
   return UtUnitAngle::ConvertFromStandard(lon, mUnitType);
   ;
}

void WkPlatformData::LongitudePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkPlatformData::HeadingPlotUpdater::HeadingPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &HeadingPlotUpdater::UnitChangedCB);
}

double WkPlatformData::HeadingPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double heading, pitch, roll;
   aPlatform->GetOrientationNED(heading, pitch, roll);
   heading = UtMath::NormalizeAngle0_TwoPi(heading);
   return UtUnitAngle::ConvertFromStandard(heading, mUnitType);
}

void WkPlatformData::HeadingPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkPlatformData::PitchPlotUpdater::PitchPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &PitchPlotUpdater::UnitChangedCB);
}

double WkPlatformData::PitchPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double heading, pitch, roll;
   aPlatform->GetOrientationNED(heading, pitch, roll);
   return UtUnitAngle::ConvertFromStandard(pitch, mUnitType);
}

void WkPlatformData::PitchPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkPlatformData::RollPlotUpdater::RollPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RollPlotUpdater::UnitChangedCB);
}

double WkPlatformData::RollPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double heading, pitch, roll;
   aPlatform->GetOrientationNED(heading, pitch, roll);
   return UtUnitAngle::ConvertFromStandard(roll, mUnitType);
}

void WkPlatformData::RollPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkPlatformData::SpeedPlotUpdater::SpeedPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetSpeedUnit();
   SetUnit(unit, QString::fromStdString(UtUnitSpeed::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &SpeedPlotUpdater::UnitChangedCB);
}

double WkPlatformData::SpeedPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   return UtUnitSpeed::ConvertFromStandard(aPlatform->GetSpeed(), mUnitType);
}

void WkPlatformData::SpeedPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetSpeedUnit();
   SetUnit(unit, QString::fromStdString(UtUnitSpeed::FindUnitName(unit)));
}

WkPlatformData::MachPlotUpdater::MachPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = UtUnitRatio::cABSOLUTE;
   SetUnit(unit, QString::fromStdString(UtUnitRatio::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &MachPlotUpdater::UnitChangedCB);
}

double WkPlatformData::MachPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   double alt_m   = aPlatform->GetAltitude();
   double speed_m = aPlatform->GetSpeed();
   double mach    = UtAirspeed::CalculateMachAtAltitude_m(alt_m, speed_m);
   return UtUnitRatio::ConvertFromStandard(mach, mUnitType);
}

void WkPlatformData::MachPlotUpdater::UnitChangedCB()
{
   int unit = UtUnitRatio::cABSOLUTE;
   SetUnit(unit, QString::fromStdString(UtUnitRatio::FindUnitName(unit)));
}

double WkPlatformData::DamageFactorPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   return aPlatform->GetDamageFactor();
}

double WkPlatformData::FuelPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   if (aPlatform->GetFuel() != nullptr)
   {
      return aPlatform->GetFuelMass();
   }
   return 0;
}

double WkPlatformData::AuxDataPlotUpdater::ReadDataP(WsfPlatform* aPlatform)
{
   if (aPlatform->GetAuxDataConst().AttributeExists(mDataField))
   {
      const UtAttributeBase& attribute = aPlatform->GetAuxDataConst().GetAttribute(mDataField);

      if (!attribute.IsContainerType())
      {
         // What kind?
         if (dynamic_cast<const UtIntAttribute*>(&attribute) != nullptr)
         {
            return dynamic_cast<const UtIntAttribute*>(&attribute)->Get();
         }
         else if (dynamic_cast<const UtDoubleAttribute*>(&attribute) != nullptr)
         {
            return dynamic_cast<const UtDoubleAttribute*>(&attribute)->Get();
         }
         else if (dynamic_cast<const UtBoolAttribute*>(&attribute) != nullptr)
         {
            return dynamic_cast<const UtBoolAttribute*>(&attribute)->Get();
         }
      }
   }

   return 0;
}
