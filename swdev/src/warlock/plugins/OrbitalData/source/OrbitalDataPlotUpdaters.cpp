//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#include "OrbitalDataPlotUpdaters.hpp"

#include "OrbitalDataContainer.hpp"
#include "OrbitalDataSimInterface.hpp"
#include "UtOrbitalElements.hpp"
#include "UtUnitTypes.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"
#include "dis/WsfDisMover.hpp"

void WkOrbitalData::PlotUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformId);
   if (platform)
   {
      auto   mover      = platform->GetMover();
      double updateTime = mover->GetLastUpdateTime();
      if (mLastUpdate < updateTime)
      {
         auto disMover   = dynamic_cast<WsfDisMover*>(mover);
         auto spaceMover = dynamic_cast<WsfSpaceMoverBase*>(mover);
         if (spaceMover)
         {
            auto orbitalElements = spaceMover->GetOrbitalState().GetOrbitalElementsTOD();
            mData.emplace_back(std::pair<double, double>(updateTime, ReadDataP(orbitalElements)));
            mLastUpdate = updateTime;
         }
         else if (disMover && disMover->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
         {
            auto cal             = aSimulation.GetDateTime().GetCurrentTime(aSimulation.GetSimTime());
            auto orbitalElements = std::move(WkOrbitalData::SimInterface::GenerateOrbitalElements(platform, cal));
            mData.emplace_back(std::pair<double, double>(updateTime, ReadDataP(orbitalElements)));
            mLastUpdate = updateTime;
         }
      }
   }
}

// ===============================================================================================================

double WkOrbitalData::EccentricityPlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetEccentricity(), mUnitType);
}

void WkOrbitalData::EccentricityPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

// ===============================================================================================================

WkOrbitalData::SemiMajorAxisPlotUpdater::SemiMajorAxisPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &SemiMajorAxisPlotUpdater::UnitChangedCB);
}

double WkOrbitalData::SemiMajorAxisPlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetSemiMajorAxis(), mUnitType);
}

void WkOrbitalData::SemiMajorAxisPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

// ===============================================================================================================

WkOrbitalData::InclinationPlotUpdater::InclinationPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &InclinationPlotUpdater::UnitChangedCB);
}

double WkOrbitalData::InclinationPlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetInclination(), mUnitType);
}

void WkOrbitalData::InclinationPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

// ===============================================================================================================

WkOrbitalData::RAAN_PlotUpdater::RAAN_PlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RAAN_PlotUpdater::UnitChangedCB);
}

double WkOrbitalData::RAAN_PlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetRAAN(), mUnitType);
}

void WkOrbitalData::RAAN_PlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

// ===============================================================================================================

WkOrbitalData::ArgOfPeriapsisPlotUpdater::ArgOfPeriapsisPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &ArgOfPeriapsisPlotUpdater::UnitChangedCB);
}

double WkOrbitalData::ArgOfPeriapsisPlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetArgumentOfPeriapsis(), mUnitType);
}

void WkOrbitalData::ArgOfPeriapsisPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

// ===============================================================================================================

WkOrbitalData::TrueAnomalyPlotUpdater::TrueAnomalyPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &TrueAnomalyPlotUpdater::UnitChangedCB);
}

double WkOrbitalData::TrueAnomalyPlotUpdater::ReadDataP(const UtOrbitalElements& aElements)
{
   return UtUnitLength::ConvertFromStandard(aElements.GetTrueAnomaly(), mUnitType);
}

void WkOrbitalData::TrueAnomalyPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}
