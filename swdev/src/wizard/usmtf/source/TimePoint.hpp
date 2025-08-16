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

#ifndef TIMEPOINT_HPP
#define TIMEPOINT_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a TimePoint.
//! A TimePoint is a field that indicates a point in time. It has many forms, they are fairly
//! self explanatory, but the sheer number of options can be confusing.

//!                  TimePoint
//!                   | | | |
//!                   v v v v
//! APERIOD/DISCRETE/141325ZFEB/142359ZFEB//
class USMTF_EXPORT TimePoint : public FFIRN_Validator
{
public:
   TimePoint(bool aTimeQualifersAllowed = false) noexcept;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetDay() const noexcept;
   const std::string& GetMonth() const noexcept;
   const std::string& GetYear() const noexcept;
   const std::string& GetTimeZone() const noexcept;
   const std::string& GetMinute() const noexcept;
   const std::string& GetHour() const noexcept;
   // Probably the only one that needs explanations, the stop
   // time qualifier is a reference string for things like
   // Until further Notice or To Be Determined
   const std::string& GetStopTimeQualifier() const noexcept;

protected:
   bool IsValidStopTimeQualifier() noexcept;
   bool IsValidDateTimeGroup() noexcept;
   bool IsValidDayTimeMonth() noexcept;
   bool IsValidDayTime() noexcept;
   bool IsValidDay(const std::string& aDay) noexcept;
   bool IsValidHour(const std::string& aHour) noexcept;
   bool IsValidMinuteOrSecond(const std::string& aUnit) noexcept;
   bool IsValidTimeZone(const std::string& aTimeZone) noexcept;
   bool IsValidMonthName(const std::string& aMonth) noexcept;
   bool IsValidYear(const std::string& aYear) noexcept;
   bool ValidateDayTime(const std::string& aDayTime) noexcept; // Support two uses, Cant override signatures because one
                                                               // is used in bind.
   bool        mTimeQualifersAllowed;
   std::string mStopTimeQualifier;
   std::string mDay;
   std::string mHour;
   std::string mMinute;
   std::string mTimeZone;
   std::string mMonth;
   std::string mYear;
};
} // namespace usmtf
#endif
