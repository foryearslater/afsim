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

#ifndef APERIOD_HPP
#define APERIOD_HPP

#include "usmtf_export.h"

#include <string>

#include "AirspaceTimeMode.hpp"
#include "Field.hpp"
#include "IntervalDuration.hpp"
#include "IntervalFrequency.hpp"
#include "Set.hpp"
#include "TimePoint.hpp"


namespace usmtf
{
//! The APeriod Set is used to provide details in relation to effective time of an ACM.
//! Example:
//! APERIOD/DISCRETE/271400ZNOV/271000ZNOV//

//! USMTF Standard Excerpt:
//! APERIOD/                     /                 /             /                     /                    //
//!        1 AIRSPACE TIME MODE  2 BEGINNING TIME  3 ENDING TIME 4 INTERVAL FREQUENCY  5 INTERVAL DURATION
//! It has optional fields based on whether the time mode is discrete or interval.

//! Detailed Document:
//! APERIOD.UF.RTF
class USMTF_EXPORT APeriod : public Set
{
public:
   APeriod(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const AirspaceTimeMode&  GetAirspaceTimeMode() const noexcept;
   const TimePoint&         GetBeginningTime() const noexcept;
   const TimePoint&         GetEndingTime() const noexcept;
   const IntervalFrequency& GetIntervalFrequency() const noexcept;
   const IntervalDuration&  GetIntervalDuration() const noexcept;

private:
   bool              IsIntervalFrequencyValid();
   bool              IsIntervalDurationValid();
   AirspaceTimeMode  mAirSpaceTimeMode;
   TimePoint         mBeginningTime;
   TimePoint         mEndingTime = TimePoint{true};
   IntervalDuration  mIntervalDuration;
   IntervalFrequency mIntervalFrequency;
};
} // namespace usmtf

#endif
