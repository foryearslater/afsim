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

#include "APeriod.hpp"

#include "AirspaceTimeMode.hpp"
#include "FFIRN_Validator.hpp"
#include "Field.hpp"
#include "IntervalDuration.hpp"
#include "IntervalFrequency.hpp"
#include "TimePoint.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
APeriod::APeriod(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mAirSpaceTimeMode.ParseAndValidate(GetField(1));
      mBeginningTime.ParseAndValidate(GetField(2));
      mEndingTime.ParseAndValidate(GetField(3));

      bool validationResult = mAirSpaceTimeMode.IsValid() && mBeginningTime.IsValid() && mEndingTime.IsValid() &&
                              IsIntervalFrequencyValid() && IsIntervalDurationValid();
      if (!validationResult)
      {
         AddError("APERIOD Required Field Validations Failed", "APERIOD", "See Following Errors for details.");
         AddErrors(mAirSpaceTimeMode.GetErrors());
         AddErrors(mBeginningTime.GetErrors());
         AddErrors(mEndingTime.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("APERIOD Is Missing Required Fields",
               "APERIOD",
               "Please make sure APERIOD has fields in positions 1,2,3 and 4,5 if is INTERVAL");
   }
}

const AirspaceTimeMode& APeriod::GetAirspaceTimeMode() const noexcept
{
   return mAirSpaceTimeMode;
}

const TimePoint& APeriod::GetBeginningTime() const noexcept
{
   return mBeginningTime;
}

const TimePoint& APeriod::GetEndingTime() const noexcept
{
   return mEndingTime;
}

const IntervalFrequency& APeriod::GetIntervalFrequency() const noexcept
{
   return mIntervalFrequency;
}

const IntervalDuration& APeriod::GetIntervalDuration() const noexcept
{
   return mIntervalDuration;
}

bool APeriod::IsIntervalFrequencyValid()
{
   if (GetField(1).GetContent() != "INTERVAL")
   {
      return true;
   }
   mIntervalFrequency.ParseAndValidate(GetField(4));
   if (!mIntervalFrequency.IsValid())
   {
      AddErrors(mIntervalFrequency.GetErrors());
      return false;
   }
   return true;
}

bool APeriod::IsIntervalDurationValid()
{
   if (GetField(1).GetContent() != "INTERVAL")
   {
      return true;
   }
   mIntervalDuration.ParseAndValidate(GetField(5));

   if (!mIntervalDuration.IsValid())
   {
      AddErrors(mIntervalDuration.GetErrors());
      return false;
   }
   return true;
}

} // namespace usmtf
