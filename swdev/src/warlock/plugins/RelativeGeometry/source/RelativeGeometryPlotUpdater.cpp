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

#include "RelativeGeometryPlotUpdater.hpp"

#include "UtRelativeGeometry.hpp"
#include "UtUnitTypes.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

RelativeGeometry::PlotUpdater::PlotUpdater(const QString& aId, const QString& aFromPlatform, const QString& aToPlatform)
   : warlock::PlotUpdater(aFromPlatform.toStdString(), 0)
   , mLastUpdateTime(-1.0)
   , mDatum(aId)
   , mToPlatformName(aToPlatform.toStdString())
{
}

void RelativeGeometry::PlotUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* from = aSimulation.GetPlatformByName(mPlatformId);
   WsfPlatform* to   = aSimulation.GetPlatformByName(mToPlatformName);
   if (from && to)
   {
      double updateTime = from->GetLastUpdateTime();
      if (updateTime > mLastUpdateTime)
      {
         mLastUpdateTime = updateTime;
         mData.emplace_back(updateTime, ReadDataP(*from, *to));
      }
   }
}

double RelativeGeometry::PlotUpdater::ReadDataP(const WsfPlatform& aFrom, const WsfPlatform& aTo)
{
   if (mDatum == "Range (linear)")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateLinearRange(aFrom, aTo), u);
   }
   else if (mDatum == "Radial")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateRadial(aFrom, aTo), u);
   }
   else if (mDatum == "In-Track")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateInTrack(aFrom, aTo), u);
   }
   else if (mDatum == "Cross-Track")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateCrossTrack(aFrom, aTo), u);
   }
   else if (mDatum == "Bearing")
   {
      UtUnitAngle::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
      return UtUnitAngle::ConvertFromStandard(UtRelativeGeometry::CalculateBearing(aFrom, aTo), u);
   }
   else if (mDatum == "Elevation")
   {
      UtUnitAngle::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
      return UtUnitAngle::ConvertFromStandard(UtRelativeGeometry::CalculateElevation(aFrom, aTo), u);
   }
   else if (mDatum == "Down Range")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateDownRange(aFrom, aTo), u);
   }
   else if (mDatum == "Cross Range")
   {
      UtUnitLength::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
      return UtUnitLength::ConvertFromStandard(UtRelativeGeometry::CalculateCrossRange(aFrom, aTo), u);
   }
   else if (mDatum == "Range Rate")
   {
      UtUnitSpeed::BaseUnit u = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetSpeedUnit();
      return UtUnitSpeed::ConvertFromStandard(UtRelativeGeometry::CalculateRangeRate(aFrom, aTo), u);
   }
   else
   {
      return 0.0;
   }
}
