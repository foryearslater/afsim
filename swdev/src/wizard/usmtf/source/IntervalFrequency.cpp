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

#include "IntervalFrequency.hpp"

namespace usmtf
{
bool IntervalFrequency::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidIntervalFrequency();
}

const std::string& IntervalFrequency::GetIntervalFrequency() const noexcept
{
   return mIntervalFrequency;
}

bool IntervalFrequency::IsValidIntervalFrequency() noexcept
{
   const std::string& aValue = mField.GetContent();
   if (IsInSelection(aValue,
                     {"BIWEEKLY",
                      "DAILY",
                      "MON WED FRI",
                      "MONTHLY BY DATE",
                      "MONTHLY BY WEEKDAY",
                      "TUE THUR",
                      "WEEKDAYS",
                      "WEEKENDS",
                      "WEEKLY",
                      "YEARLY"}))
   {
      mIntervalFrequency = aValue;
      return true;
   }
   AddError("Interval Frequency Is Invalid.",
            aValue,
            "Interval Frequency must be one of the following:\n "
            "BIWEEKLY,           DAILY,    MON WED FRI, MONTHLY BY DATE \n"
            "MONTHLY BY WEEKDAY, TUE THUR, WEEKDAYS,    WEEKENDS \n"
            "WEEKLY, YEARLY");
   return false;
}

}; // namespace usmtf
