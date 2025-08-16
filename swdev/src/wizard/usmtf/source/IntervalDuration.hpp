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

#ifndef INTERVALDURATION_HPP
#define INTERVALDURATION_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "TimePoint.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an IntervalDuration.
//! An IntervalDuration is a field that indicates an interval time frames duration. It comes in
//! several forms.

//! USMTF Excerpt:
//! A. INTERVAL DURATION

//!  EXPRESSED AS FOLLOWS:
//!  CONTEXT QUANTITY, 1-999
//!  ALLOWABLE ENTRIES:  (1 THROUGH 999).

//!  UNIT OF TIME MEASUREMENT, DAY/WK/BIWK/MON/YR
//!  EXAMPLE: /4WK

//!   B. TIME QUALIFIER, UNTIL FURTHER NOTICE
//!   ALLOWABLE ENTRIES:  UFN.
//!   EXAMPLE: /UFN

//!   C. ENDING DATE-TIME GROUP
//!   EXAMPLE: /141325ZFEB1997
class USMTF_EXPORT IntervalDuration : public TimePoint
{
public:
   IntervalDuration() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetIntervalDuration() const noexcept;
   const std::string& GetContextQuantity() const noexcept;
   const std::string& GetUOM() const noexcept;

protected:
   bool        IsValidIntervalDuration() noexcept;
   bool        IsValidUnitOfTimeMeasurement(const std::string& aValue) noexcept;
   bool        IsUntilFurtherNotice() noexcept;
   std::string mContextQuantity;
   std::string mUOM;
   std::string mIntervalDuration;
};
} // namespace usmtf
#endif
