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

#ifndef AIRSPACESHAPE_HPP
#define AIRSPACESHAPE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an Airspace shape
//! An Airspace shape is simple, is the shape of the volume identifying the space.
//! IE: Corridor, point, circle, polygon etc...

//! Example:
//!                          AirspaceShape
//!                            | | | |
//!                             v v v v
//! ACMID/OTR:RFHHLL/DESIG:C34/CORRIDOR/OTR:SPAA//

//! Supported Shapes:
//! CIRCLE
//! CORRIDOR
//! ORBIT
//! POINT
//! POLYARC
//! POLYGON
//! RADARC
//! TRACK
class USMTF_EXPORT AirspaceShape : public FFIRN_Validator
{
public:
   AirspaceShape() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetShape() const noexcept;

protected:
   bool        IsValidTypeOfAirspaceShape() noexcept;
   std::string mAirspaceShape;
};
} // namespace usmtf
#endif
