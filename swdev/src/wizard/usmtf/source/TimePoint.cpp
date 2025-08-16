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

#include "TimePoint.hpp"

#include <stdexcept>

namespace usmtf
{
TimePoint::TimePoint(bool aTimeQualifersAllowed) noexcept
   : mTimeQualifersAllowed(aTimeQualifersAllowed)
{
}

bool TimePoint::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   if (mTimeQualifersAllowed)
   {
      bool result = IsValidDateTimeGroup() || IsValidDayTimeMonth() || IsValidStopTimeQualifier();
      ClearErrorsIfValid(result);
      return result;
   }
   else
   {
      bool result = IsValidDateTimeGroup() || IsValidDayTimeMonth();
      ClearErrorsIfValid(result);
      return result;
   }
}

const std::string& TimePoint::GetDay() const noexcept
{
   return mDay;
}

const std::string& TimePoint::GetMonth() const noexcept
{
   return mMonth;
}

const std::string& TimePoint::GetYear() const noexcept
{
   return mYear;
}

const std::string& TimePoint::GetTimeZone() const noexcept
{
   return mTimeZone;
}

const std::string& TimePoint::GetMinute() const noexcept
{
   return mMinute;
}

const std::string& TimePoint::GetHour() const noexcept
{
   return mHour;
}

const std::string& TimePoint::GetStopTimeQualifier() const noexcept
{
   return mStopTimeQualifier;
}

bool TimePoint::IsValidStopTimeQualifier() noexcept
{
   const std::string& aValue = mField.GetContent();

   if (IsInSelection(
          aValue,
          {"AFTER", "ASOF", "ASAP", "ASAPAFT", "ASAPNLT", "BEFORE", "INDEF", "NET", "NLT", "ONCALL", "TBD", "UNK", "UFN"}))
   {
      mStopTimeQualifier = aValue;
      return true;
   }
   AddError("Stop Time Qualifier is Invalid",
            aValue,
            "Stop Time Qualifier must be one of the following:\n "
            "AFTER, ASOF, ASAP, ASAPAFT, \n"
            "ASAPNLT, BEFORE, INDEF, NET \n"
            "NLT, ONCALL, TBD, UNK, UFN \n");
   return false;
}

bool TimePoint::IsValidDateTimeGroup() noexcept
{
   const std::string& aValue = mField.GetContent();
   if (!IsStringLength(aValue, 14))
   {
      AddError("Date Time Group Failed Length Check", aValue, "Date Time Group must be 14 chars.");
      return false;
   }

   std::string day = aValue.substr(0, 2);
   if (!IsValidDay(day))
   {
      return false;
   }

   std::string hour = aValue.substr(2, 2);
   if (!IsValidHour(hour))
   {
      return false;
   }

   std::string minute = aValue.substr(4, 2);
   if (!IsValidMinuteOrSecond(minute))
   {
      return false;
   }

   std::string timeZone = aValue.substr(6, 1);
   if (!IsValidTimeZone(timeZone))
   {
      return false;
   }

   std::string month = aValue.substr(7, 3);
   if (!IsValidMonthName(month))
   {
      return false;
   }

   std::string year = aValue.substr(10, 4);
   if (!IsValidYear(year))
   {
      return false;
   }
   mDay      = day;
   mHour     = hour;
   mMinute   = minute;
   mTimeZone = timeZone;
   mMonth    = month;
   mYear     = year;
   return true;
}

bool TimePoint::IsValidDayTimeMonth() noexcept
{
   const std::string& aValue = mField.GetContent();
   if (!IsStringLength(aValue, 10))
   {
      AddError("Day Time Month Failed Length check", aValue, "Day Time Month must be length of 10 chars.");
      return false;
   }

   std::string day = aValue.substr(0, 2);
   if (!IsValidDay(day))
   {
      return false;
   }

   std::string hour = aValue.substr(2, 2);
   if (!IsValidHour(hour))
   {
      return false;
   }

   std::string minute = aValue.substr(4, 2);
   if (!IsValidMinuteOrSecond(minute))
   {
      return false;
   }

   std::string timeZone = aValue.substr(6, 1);
   if (!IsValidTimeZone(timeZone))
   {
      return false;
   }

   std::string month = aValue.substr(7, 3);
   if (!IsValidMonthName(month))
   {
      return false;
   }
   mDay      = day;
   mHour     = hour;
   mMinute   = minute;
   mTimeZone = timeZone;
   mMonth    = month;
   return true;
}

bool TimePoint::IsValidDayTime() noexcept
{
   const std::string& aDayTime = mField.GetContent();
   return ValidateDayTime(aDayTime);
}

bool TimePoint::IsValidDay(const std::string& aDay) noexcept
{
   try
   {
      auto day = std::stoi(aDay);
      if (IsInRange<int>(day, 0, 31))
      {
         return true;
      }
      AddError("Day Is Invalid", aDay, "Day must be between [0, 31]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Day Failed Conversion", aDay, "Day must be convertible to int.");
      return false;
   }
}

bool TimePoint::IsValidHour(const std::string& aHour) noexcept
{
   try
   {
      auto hour = std::stoi(aHour);
      if (IsInRange<int>(hour, 0, 23))
      {
         return true;
      }
      AddError("Hour Is Invalid.", aHour, "Hour must be between [0, 23]");
      return false;
   }
   catch (std::invalid_argument&)
   {
      AddError("Hour Failed Conversion", aHour, "Hour must be convertible to int.");
      return false;
   }
   catch (std::out_of_range&)
   {
      AddError("Hour Failed Conversion", aHour, "Hour must be fit into an int.");
      return false;
   }
}

bool TimePoint::IsValidMinuteOrSecond(const std::string& aUnit) noexcept
{
   try
   {
      if (IsInRange<int>(std::stoi(aUnit), 0, 59))
      {
         return true;
      }
      AddError("Unit Failed Range Check", aUnit, "Unit must fall in range [0,59]");
      return false;
   }
   catch (std::exception&) // stoi conversion failed
   {
      AddError("Unit Failed Conversion", aUnit, "Unit must be convertible to int.");
      return false;
   }
}

bool TimePoint::IsValidTimeZone(const std::string& aTimeZone) noexcept
{
   if (IsInSelection(aTimeZone,
                     {"A", "B", "C", "0", "D", "1", "E", "2", "F", "3", "G", "H", "I", "4", "K", "5", "L", "6",
                      "M", "N", "O", "P", "7", "Q", "R", "S", "T", "U", "8", "V", "9", "W", "X", "Y", "Z"}))
   {
      return true;
   }
   AddError("TimeZone Is Invalid.",
            aTimeZone,
            "Time zone must be one of the following:\n "
            "A, B, C, 0, D, \n"
            "1, E, 2, F, 3, \n"
            "G, H, I, 4, K, \n"
            "5, L, 6, M, N, \n"
            "O, P, 7, Q, R, \n"
            "S, T, U, 8, V, \n"
            "9, W, X, Y, Z \n");
   return false;
}

bool TimePoint::IsValidMonthName(const std::string& aMonth) noexcept
{
   if (IsInSelection(aMonth, {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}))
   {
      return true;
   }

   AddError("Month Is Invalid.",
            aMonth,
            "Month must be one of the following:\n "
            "JAN, FEB, MAR, APR \n"
            "MAY, JUN, JUL, AUG \n"
            "SEP, OCT, NOV, DEC \n");
   return false;
}

bool TimePoint::IsValidYear(const std::string& aYear) noexcept
{
   try
   {
      auto year = std::stoi(aYear);
      if (IsInRange<int>(year, 0, 9999))
      {
         return true;
      }
      AddError("Year Is Invalid", aYear, "Year must fall into range [0,9999]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Year Failed Conversion", aYear, "Year must be convertible to int.");
      return false;
   }
}

bool TimePoint::ValidateDayTime(const std::string& aDayTime) noexcept
{
   if (!IsStringLength(aDayTime, 7))
   {
      AddError("Day Time Failed Length Check", aDayTime, "Day Time must be 7 chars long");
      return false;
   }

   std::string day = aDayTime.substr(0, 2);
   if (!IsValidDay(day))
   {
      return false;
   }

   std::string hour = aDayTime.substr(2, 2);
   if (!IsValidHour(hour))
   {
      return false;
   }

   std::string minute = aDayTime.substr(4, 2);
   if (!IsValidMinuteOrSecond(minute))
   {
      return false;
   }

   std::string timeZone = aDayTime.substr(6, 1);
   if (!IsValidTimeZone(timeZone))
   {
      return false;
   }

   mDay      = day;
   mHour     = hour;
   mMinute   = minute;
   mTimeZone = timeZone;
   return true;
}
}; // namespace usmtf
