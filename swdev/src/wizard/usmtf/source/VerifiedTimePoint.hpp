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


#ifndef VERIFIEDTIMEPOINT_HPP
#define VERIFIEDTIMEPOINT_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "TimePoint.hpp"

namespace usmtf
{
//! Same as TimePoint.hpp (see it) but with a few extra verification validations
class USMTF_EXPORT VerifiedTimePoint : public TimePoint
{
public:
   VerifiedTimePoint(bool aTimeQualifersAllowed = false) noexcept;
   bool ParseAndValidate(const Field& aField) override;

protected:
   bool IsValidVerifiedDateTime() noexcept;
   bool ValidateVerifiedDateTime(const std::string& aDateTime) noexcept; // support two ways to pass it in. Cant override
                                                                         // signatures because one is used in bind.
   bool IsValidVerifiedMonthDateTime() noexcept;
   bool IsValidContextQuantity(const std::string& aValue) noexcept;
};
} // namespace usmtf
#endif
