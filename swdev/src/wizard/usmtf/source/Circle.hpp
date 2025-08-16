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

#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Radius.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The Circle Set is used to provide a single position with a radius specifying width.

//! Example:
//! CIRCLE/LATS:203632N0594256E/15NM//

//! USMTF Standard Excerpt:
//! CIRCLE/                   /                                                  //
//!        1 CENTER OF CIRCLE  2 RADIUS IN NAUTICAL MILES, METERS, OR KILOMETERS

//! Detailed Document:
//! CIRCLE.UF.RTF
class USMTF_EXPORT Circle : public Set
{
public:
   Circle(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const LatLon& GetCenterOfCircle() const noexcept;
   const Radius& GetRadius() const noexcept;

private:
   LatLon mCenterOfCircle;
   Radius mRadius;
};
} // namespace usmtf

#endif
