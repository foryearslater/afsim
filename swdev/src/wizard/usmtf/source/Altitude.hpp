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

#ifndef ALTITUDE_HPP
#define ALTITUDE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RangeValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a Altitude.
//! An Altitude is somewhat self explanatory given the mathematical definition. The class simply
//! Captures a height with a Unit for understanding the height number.

//! USMTF Excerpt:
/*!
    VERTICAL DISTANCE IN HUNDREDS OF FEET ABOVE MEAN SEA LEVEL (MSL)
    This definition is subject to change based on individual needs.
    This can be expanded to be arbitrary units and reference points.
    EXAMPLE: /210
*/
class USMTF_EXPORT Altitude : public RangeValidator<float>
{
public:
   //! Default Range for valid Altitude is [0,999];
   //! this can be configured with SetValidRange
   Altitude() noexcept;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetVerticalDistance() const noexcept;

protected:
   std::string mVerticalDistance;
};
} // namespace usmtf
#endif
