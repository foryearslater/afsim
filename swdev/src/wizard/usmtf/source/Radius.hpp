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

#ifndef RADIUS_HPP
#define RADIUS_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RangeValidator.hpp"
#include "UtCast.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a Radius.
//! An Radius is somewhat self explanatory given the mathematical definition. What might be
//! confusing is this classes use in situations that simple mean distance, like in widths of volumes.
//! It comes with a unit of measure.

//! USMTF Excerpt:
/*!
    EXPRESSED AS FOLLOWS:

    CONTEXT QUANTITY, DECIMAL PT PERMITTED, 5 CHARACTER MAX
    ALLOWABLE ENTRIES:  (0 THROUGH 99999) [0 TO 4 DECIMAL PLACES].
    DATA ITEM AND DATA CODE VALUES CONSIST OF THE INTEGERS 0 THROUGH 99999, AND DECIMAL POINT VALUES.
    DECIMAL POINT VALUES RANGE FROM .0001 TO 999.9, CONSISTING OF ALL POSSIBLE COMBINATIONS OF FOUR OR
    FEWER SIGNIFICANT DIGITS AND A SIGNIFICANT DECIMAL POINT. THERE IS A MAXIMUM OF FIVE CHARACTERS.

    UNIT OF LINEAR MEASUREMENT, M/KM/NM (Meters, Kilometers, Nautical Miles)
    EXAMPLE: /25NM
*/
class USMTF_EXPORT Radius : public RangeValidator<double>
{
public:
   Radius();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetRadiusValue() const noexcept;
   const std::string& GetUnitOfMeasure() const noexcept;
   //! Given Meters is the common UOM in use for calculations
   //! we opted to always have a implicit conversion between the other
   //! supported UOMS,  nm, km etc
   const double GetRadiusInMeters() const noexcept;
   const double GetRadiusInMeters(double aRadius) const noexcept;
   const double GetRadius() const noexcept;

protected:
   bool        IsValidRadius() noexcept;
   bool        IsValidUnitOfMeasureForRadius(const std::string& aUom) noexcept;
   std::string mRadiusValue;
   std::string mUnitOfMeasure;
   double      mRadiusDouble;
};
} // namespace usmtf
#endif
