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

#include "PositionUtils.hpp"

#include <cmath>

#include "UtAngle.hpp"
#include "UtEntity.hpp"
#include "UtMath.hpp"
#include "UtSphericalEarth.hpp"
#include "UtUnitTypes.hpp"

namespace
{
double RoundUp(double aValue, int aDecimalPlaces)
{
   const double multiplier = std::pow(10.0, aDecimalPlaces);
   return std::ceil(aValue * multiplier) / multiplier;
}
} // namespace

namespace usmtf
{
const UtLLPos FindPositionOnGreaterCircle(const UtLLPos& aPosition, double aRadialBearing, double aRadius) noexcept
{
   double   lat = aPosition.GetLat();
   double   lon = aPosition.GetLon();
   double   alt = 0.0;
   double   locationNed[3];
   UtEntity nedEntity;

   nedEntity.SetLocationLLA(lat, lon, alt);

   // some trig to find position on a circles circumference
   locationNed[0] = aRadius * cos(aRadialBearing * UtMath::cRAD_PER_DEG);
   locationNed[1] = aRadius * sin(aRadialBearing * UtMath::cRAD_PER_DEG);
   locationNed[2] = 0.0;

   nedEntity.ConvertNEDToLLA(locationNed, lat, lon, alt);

   return UtLLPos(lat, lon);
}

double FindVectorTrueNorthHeading(const UtLLPos& aVectorOrigin, const UtLLPos& aVectorEnd) noexcept
{
   double heading  = 0.0;
   double distance = 0.0;

   UtSphericalEarth::GreatCircleHeadingAndDistance(aVectorOrigin.GetLat(),
                                                   aVectorOrigin.GetLon(),
                                                   aVectorEnd.GetLat(),
                                                   aVectorEnd.GetLon(),
                                                   heading,
                                                   distance);

   return heading;
}

std::vector<UtLLPos> FindSquareOfRefrencePositions(const UtLLPos& aOrigin,
                                                   const UtLLPos& aDestination,
                                                   double         aGreaterCircleDistance) noexcept
{
   return FindPositionsLeftAndRightOfOriginToDestRef(aOrigin, aDestination, aGreaterCircleDistance, aGreaterCircleDistance);
}

std::vector<UtLLPos> FindPositionsLeftAndRightOfOriginToDestRef(const UtLLPos& aOrigin,
                                                                const UtLLPos& aDestination,
                                                                double         aLeftMeters,
                                                                double         aRightMeters) noexcept
{
   double  bearing       = FindVectorTrueNorthHeading(aOrigin, aDestination);
   UtLLPos leftOfOrigin  = FindPositionOnGreaterCircle(aOrigin, bearing - 90, aLeftMeters);
   UtLLPos rightOfOrigin = FindPositionOnGreaterCircle(aOrigin, bearing + 90, aRightMeters);
   UtLLPos leftOfDest    = FindPositionOnGreaterCircle(aDestination, bearing - 90, aLeftMeters);
   UtLLPos rightOfDest   = FindPositionOnGreaterCircle(aDestination, bearing + 90, aRightMeters);
   return std::vector<UtLLPos>{leftOfOrigin, leftOfDest, rightOfDest, rightOfOrigin};
}

// I am more then open to suggestions from some one with more mathematical intuition that might
// point out a better solution. This does work though.
UtLLPos LineSegment::Intersect(const LineSegment& aOther) noexcept
{
   UtLLPos pointOfIntersection;

   double ax = mA.mX;
   double ay = mA.mY;

   double bx = mB.mX;
   double by = mB.mY;

   double cx = aOther.mA.mX;
   double cy = aOther.mA.mY;

   double dx = aOther.mB.mX;
   double dy = aOther.mB.mY;

   double roundedAx     = RoundUp(ax, 12);
   double roundedBx     = RoundUp(bx, 12);
   double abDenominator = ax - bx;

   // Avoids conditions where denominators are 0 (positions are the same lat or lon)
   // and conditions where denominator are so extremely close to zero (they are extremely close to one another)
   // where calculations cant calculate with double precision because the numbers are just to large
   // and result in a completely off position. A denominator of 0.0000000001 is so extremely negligible
   // that the resulting position is near identical to what it should be, but calculations can still be conducted.
   if ((roundedAx - roundedBx) == 0)
   {
      abDenominator = 0.0000000001;
   }
   double roundedCx     = RoundUp(cx, 12);
   double roundedDx     = RoundUp(dx, 12);
   double cdDenominator = cx - dx;

   // Avoids conditions where denominators are 0 (positions are the same lat or lon)
   // and conditions where denominator are so extremely close to zero (they are extremely close to one another)
   // where calculations cant calculate with double precision because the numbers are just to large
   // and result in a completely off position. A denominator of 0.0000000001 is so extremely negligible
   // that the resulting position is near identical to what it should be, but calculations can still be conducted.
   if ((roundedCx - roundedDx) == 0)
   {
      cdDenominator = 0.0000000001;
   }

   double slopeAB;
   double slopeCD;
   slopeAB = (ay - by) / abDenominator;
   slopeCD = (cy - dy) / cdDenominator;

   if ((slopeAB - slopeCD) != 0)
   {
      double x;
      double y; // determinant;
      double interceptAB = (ay - (slopeAB * ax));
      double interceptCD = (cy - (slopeCD * cx));

      x = ((interceptCD - interceptAB) / (slopeAB - slopeCD));
      y = ((slopeCD * x) + interceptCD);

      pointOfIntersection.Set(x, y);
      pointOfIntersection.SetLatFormat(UtAngle::FmtDMS);
      pointOfIntersection.SetLonFormat(UtAngle::FmtDMS);
   }
   return pointOfIntersection;
}
} // namespace usmtf
