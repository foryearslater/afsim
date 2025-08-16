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

#include "RelativeGeometryUpdater.hpp"

#include "UtRelativeGeometry.hpp"

bool RelativeGeometry::LinearRangeUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   // should we extrapolate either platform
   mValue = UtRelativeGeometry::CalculateLinearRange(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::RadialUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateRadial(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::InTrackUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateInTrack(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::CrossTrackUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateCrossTrack(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::BearingUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateBearing(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::ElevationUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateElevation(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::DownRangeUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateDownRange(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::CrossRangeUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateCrossRange(*aFromPlatform, *aToPlatform);
   return true;
}

bool RelativeGeometry::RangeRateUpdater::ReadData(WsfPlatform* aFromPlatform, WsfPlatform* aToPlatform)
{
   mValue = UtRelativeGeometry::CalculateRangeRate(*aFromPlatform, *aToPlatform);
   return true;
}
