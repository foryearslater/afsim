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

#ifndef APOINT_HPP
#define APOINT_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The APoint Set is used to provide a single position.

//! Example:
//! APOINT/LATM:2037N05943E//

//! USMTF Standard Excerpt:
//! APOINT/                 //
//!        1 AIRSPACE POINT

//! Detailed Document:
//! APOINT.UF.RTF
class USMTF_EXPORT APoint : public Set
{
public:
   APoint(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const LatLon& GetAirSpacePoint() const noexcept;

private:
   LatLon mAirspacePoint;
};
} // namespace usmtf

#endif
