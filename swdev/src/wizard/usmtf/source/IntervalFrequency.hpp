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

#ifndef INTERVALFREQUENCY_HPP
#define INTERVALFREQUENCY_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an IntervalFrequency.
//! An IntervalFrequency is a field that indicates an interval time frames frequency. It comes in
//! several forms.

//! USMTF Excerpt:
//! INTERVAL FREQUENCY
//!   BIWEEKLY
//!   DAILY
//!   MON WED FRI
//!   MONTHLY BY DATE
//!   MONTHLY BY WEEKDAY
//!   TUE THUR
//!   WEEKDAYS
//!   WEEKENDS
//!   WEEKLY
//!   YEARLY
class USMTF_EXPORT IntervalFrequency : public FFIRN_Validator
{
public:
   IntervalFrequency() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetIntervalFrequency() const noexcept;

protected:
   bool        IsValidIntervalFrequency() noexcept;
   std::string mIntervalFrequency;
};
} // namespace usmtf
#endif
