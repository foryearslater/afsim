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

#ifndef ACMID_HPP
#define ACMID_HPP

#include "usmtf_export.h"

#include <string>

#include "AirspaceShape.hpp"
#include "Field.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The Air Control Message ID Set Represents the name and types
//! of Air Control Message that it identifies. In Zones this is the name and shape of the zone/zone set.
//! There are other fields but have not been identified as useful for AFSIM.
//! Example:
//!                  Identifier  Shape
//!                  | | | | |  | | | |
//!                  v v v v v  v v v v
//! ACMID/OTR:RFHHLL/DESIG:C34/CORRIDOR/OTR:SPAA//

//! USMTF Standard Excerpt:
//! ACMSTAT/                                /                  R                 //
//!          1 AIRSPACE CONTROL MEANS STATUS  2 AIRSPACE CONTROL MEANS IDENTIFIER

//! Detailed Document:
//! ACMIND.UF.RTF
class USMTF_EXPORT Acmid : public Set
{
public:
   Acmid(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const std::string& GetAirspaceControlMeansIdentifier() const noexcept;
   const std::string& GetTypeOfAirspaceShape() const noexcept;

private:
   AirspaceShape mShape;
};
} // namespace usmtf
#endif
