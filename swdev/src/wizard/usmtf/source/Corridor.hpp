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

#ifndef CORRIDOR_HPP
#define CORRIDOR_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Radius.hpp"
#include "Set.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{
//! The Corridor Set is used to outline the positional details of a volume space that is shaped like a "rectangle
//! tunnel". It consists of a polygon.
//!    _________________              _____________
//!   |                  \           /             |
//!   |_____________      \_________/      ________|
//!                 \                     /
//!                  \___________________/
//!
//! Note on corridor complexity:
//! The points define the center mid point of each polygon volume, with the width we calculate the corners. Then,
//! based on the heading of the next leg, we interpolate elbow and intersecting corners.

//! USMTF Example:
//! CORRIDOR/5NM/LATM:3500N07900W/LATM:3530N07000W/LATM:3622N06957W/LATM:3619N06841W/LATM:3547N06715W/LATM:3505N06853W/

//! Standard Excerpt:
//! CORRIDOR/                                                 /          R         //
//!          1 WIDTH IN NAUTICAL MILES, METERS, OR KILOMETERS  2 POSITION OR POINT

//! Detailed Document:
//! CORRIDOR.UF.RTF
class USMTF_EXPORT Corridor : public Set
{
public:
   Corridor(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const Radius&              GetWidth() const noexcept;
   const std::vector<LatLon>& GetPoints() const noexcept;

private:
   bool                AreAllPointsValid();
   Radius              mWidth;
   std::vector<LatLon> mPoints;
};
} // namespace usmtf

#endif
