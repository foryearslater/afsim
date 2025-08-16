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

#ifndef RADARC_HPP
#define RADARC_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "RadialBearing.hpp"
#include "Radius.hpp"
#include "Set.hpp"

namespace usmtf
{
/*!
The Radarc Set is used to represent a sort of "Radar shape" extending out from a central coordinate point.
      ___
     /   \
    /     \
  /        \
  \        /
    \     /
     \___/

USMTF Example:
      RADARC/1500N05900E/300T/060T/0NM/50NM//

Standard Excerpt:
  RADARC/                    /                                 /                              .
        1 ORIGIN OF BEARING  2 BEGINNING RADIAL BEARING, TRUE  3 ENDING RADIAL BEARING, TRUE
        /                                                        .
        4 INNER RADIUS IN NAUTICAL MILES, METERS, OR KILOMETERS
        /                                                        //
        5 OUTER RADIUS IN NAUTICAL MILES, METERS, OR KILOMETERS
*/
class USMTF_EXPORT Radarc : public Set
{
public:
   Radarc(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const LatLon&        GetOriginOfBearing() const noexcept;
   const RadialBearing& GetBeginningRadialBearingTrue() const noexcept;
   const RadialBearing& GetEndingRadialBearingTrue() const noexcept;
   const Radius&        GetInnerRadius() const noexcept;
   const Radius&        GetOuterRadius() const noexcept;

private:
   LatLon        mOriginOfBearing;
   RadialBearing mBeginningRadialBearing;
   RadialBearing mEndingRadialBearing;
   Radius        mInnerRadius;
   Radius        mOuterRadius;
};
} // namespace usmtf
#endif
