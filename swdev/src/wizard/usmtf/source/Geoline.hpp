// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef GEOLINE_HPP
#define GEOLINE_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The Geoline Set is used to outline the positional details of a volume space that is shaped like a "line".
//! The volumetric aspects are implied to be 1 meter width, height will be determined by efflevel association.
//!    _________________              _____________
//!                     \           /
//!                      \_________/

//! Note on line complexity:
//! The points define the center mid point of each polygon volume, with the implied width we calculate the corners.

//! USMTF Example:
//! GEOLINE/LATM:1510N05901E/LATS:153632N0594256E/LATS:155000N0594815E/LATS:155000N0591343E//

//! Standard Excerpt:
//! GEOLINE  /         R         //
//!          1  POSITION OR POINT
//!

//! Detailed Document:
//! GEOLINE.UF.HTML
class USMTF_EXPORT Geoline : public Set
{
public:
   Geoline(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const std::vector<LatLon>& GetPoints() const noexcept;

private:
   bool                AreAllPointsValid();
   std::vector<LatLon> mPoints;
};
} // namespace usmtf

#endif
