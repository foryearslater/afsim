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


#ifndef RADIALBEARING_HPP
#define RADIALBEARING_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a RadialBearing.
//! An RadialBearing is a field that indicates a degree offset measured clockwise from True North.

//! USMTF Excerpt:
//!
//!        ENTER THE ENDING LIMIT RADIAL LINE MEASURED CLOCKWISE FROM TRUE NORTH.
//!        EXPRESSED AS FOLLOWS:
//!        BEARING IN DEGREES
//!        ALLOWABLE ENTRIES:  (000 THROUGH 359).

//!        ANGULAR MEASUREMENT REFERENCE, TRUE
//!        ALLOWABLE ENTRIES:  T.

//!        EXAMPLE: /030T*/
class USMTF_EXPORT RadialBearing : public FFIRN_Validator
{
public:
   RadialBearing() = default;
   const std::string& GetBearing() const noexcept;
   bool               ParseAndValidate(const Field& aField) override;

protected:
   bool        IsValidRadialBearing() noexcept;
   std::string mBearing;
};
} // namespace usmtf
#endif
