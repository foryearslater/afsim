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
#ifndef POSITIONUTILS_HPP
#define POSITIONUTILS_HPP

#include "LatLon.hpp"
#include "RadialBearing.hpp"
#include "Radius.hpp"
#include "UtEntity.hpp"
#include "UtLLAPos.hpp"

namespace usmtf
{
/*!
   Position Utils is sort of a hodge podge collection of code related to
   using, converting and calculating positions from other positions.

   used to find a position on a circle circumference
*/
const UtLLPos FindPositionOnGreaterCircle(const UtLLPos& aPosition, double aRadialBearing, double aRadius) noexcept;

//! Used to calculate corner positions given a start point/end point with just a single equal distance
std::vector<UtLLPos> FindSquareOfRefrencePositions(const UtLLPos& aOrigin,
                                                   const UtLLPos& aDestination,
                                                   double         aOriginsGreaterCircleDistance) noexcept;

//! Used to calculate corner positions given a start point/end point and a width left/right.
std::vector<UtLLPos> FindPositionsLeftAndRightOfOriginToDestRef(const UtLLPos& aOrigin,
                                                                const UtLLPos& aDestination,
                                                                double         aLeftMeters,
                                                                double         aRightMeters) noexcept;

//! Used to extract heading out of UtLLPos::HeadingAndDistance calculation, simple convenience wrapper
double FindVectorTrueNorthHeading(const UtLLPos& aVectorOrigin, const UtLLPos& aVectorEnd) noexcept;

//! Represents a point
struct Point
{
   double mX;
   double mY;
};

//! Represents a Line segment so that intersection can be calculated.
struct LineSegment
{
   UtLLPos Intersect(const LineSegment& aOther) noexcept;
   Point   mA;
   Point   mB;
};
} // namespace usmtf
#endif
