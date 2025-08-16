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

#ifndef PERIOD_HPP
#define PERIOD_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "Set.hpp"
#include "VerifiedTimePoint.hpp"

namespace usmtf
{
//! The Period Set is used to convey the effective time of an entire ACO, unlike APERIOD that is time for the ACM (Or
//! zone in our interpretation).

//! USMTF Example:
//!   PERIOD / 141325ZFEB / 142359ZFEB//
//!   PERIOD / 141325ZFEB / NLT / 141600ZFEB//
//!   PERIOD / 141325ZFEB / UFN//

//! Standard Excerpt:
//! PERIOD/             /             /                     //
//!       1 START TIME  2 STOP TIME   3 STOP TIME MODIFIER

//! Detailed Document:
//! PERIOD.UF.RTF
class USMTF_EXPORT Period : public Set
{
public:
   Period(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const VerifiedTimePoint& GetStartTime() const noexcept;
   const VerifiedTimePoint& GetStopTime() const noexcept;
   const VerifiedTimePoint& GetStopTimeModifier() const noexcept;

private:
   bool              IsValidStopTimeModifier();
   bool              DoesStopTimeRequireModifier(const std::string& stopTime) const noexcept;
   VerifiedTimePoint mStartTime;
   VerifiedTimePoint mStopTime = VerifiedTimePoint{true};
   VerifiedTimePoint mStopTimeModifier;
};
} // namespace usmtf
#endif
