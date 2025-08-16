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


#include "VerifiedTimePoint.hpp"

namespace usmtf
{
VerifiedTimePoint::VerifiedTimePoint(bool aTimeQualifersAllowed) noexcept
   : TimePoint(aTimeQualifersAllowed)
{
}

bool VerifiedTimePoint::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   if (mTimeQualifersAllowed)
   {
      bool result = IsValidVerifiedDateTime() || IsValidDayTime() || IsValidDayTimeMonth() ||
                    IsValidVerifiedMonthDateTime() || IsValidDateTimeGroup() || IsValidStopTimeQualifier();

      ClearErrorsIfValid(result);
      return result;
   }
   else
   {
      bool result = IsValidDayTime() || IsValidVerifiedDateTime() || IsValidDayTimeMonth() ||
                    IsValidVerifiedMonthDateTime() || IsValidDateTimeGroup();

      ClearErrorsIfValid(result);
      return result;
   }
}

bool VerifiedTimePoint::IsValidVerifiedDateTime() noexcept
{
   const std::string& aValue = mField.GetContent();
   return ValidateVerifiedDateTime(aValue);
}

bool VerifiedTimePoint::ValidateVerifiedDateTime(const std::string& aDateTime) noexcept
{
   if (!IsStringLength(aDateTime, 8))
   {
      AddError("Verified Date Time Failed Length Check", aDateTime, "Verified Date Time must be 8 chars.");
      return false;
   }

   if (!ValidateDayTime(aDateTime.substr(0, 7)))
   {
      return false;
   }

   if (!IsValidContextQuantity(aDateTime.substr(7, 1)))
   {
      return false;
   }

   return true;
}

bool VerifiedTimePoint::IsValidVerifiedMonthDateTime() noexcept
{
   const std::string& aValue = mField.GetContent();
   if (!IsStringLength(aValue, 11))
   {
      AddError("Verified Month Date Time Failed Length Check", aValue, "Verified Month Date Time must be 11 chars.");
      return false;
   }

   if (!ValidateVerifiedDateTime(aValue.substr(0, 8)))
   {
      return false;
   }

   if (!IsValidMonthName(aValue.substr(8, 3)))
   {
      return false;
   }

   return true;
}

bool VerifiedTimePoint::IsValidContextQuantity(const std::string& aValue) noexcept
{
   try
   {
      int contextQuantity = std::stoi(aValue);
      if (IsInRange<int>(contextQuantity, 0, 9))
      {
         return true;
      }
      AddError("Context Quantity Is Invalid ", aValue, "Context Quantity must be in range of [0,9]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Context Quantity Failed Conversion",
               aValue,
               "Context Quantity must be data that can be converted to an integer.");
      return false;
   }
}
}; // namespace usmtf
