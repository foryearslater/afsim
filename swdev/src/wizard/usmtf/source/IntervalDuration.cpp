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

#include "IntervalDuration.hpp"

#include <stdexcept>

namespace usmtf
{
bool IntervalDuration::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   bool result = IsValidIntervalDuration() || IsUntilFurtherNotice() || IsValidDateTimeGroup();
   ClearErrorsIfValid(result);
   return result;
}

const std::string& IntervalDuration::GetIntervalDuration() const noexcept
{
   return mIntervalDuration;
}

const std::string& IntervalDuration::GetContextQuantity() const noexcept
{
   return mContextQuantity;
}

const std::string& IntervalDuration::GetUOM() const noexcept
{
   return mUOM;
}

bool IntervalDuration::IsValidIntervalDuration() noexcept
{
   const std::string& aValue = mField.GetContent();
   if (!IsInRange<size_t>(aValue.length(), 3, 7))
   {
      AddError("Interval Duration Failed Length Check", aValue, "Interval Duration must be between [3,7] chars.");
      return false;
   }
   size_t splitPos = CheckForAnyDelimeter(aValue, {"BIWK", "DAY", "MON", "WK", "YR"});

   if (splitPos == std::string::npos)
   {
      AddError("Interval Duration Failed Parsing",
               aValue,
               "Interval Duration requires the presence of BIWK, DAY, MON, WK, or YR for delineation.");
      return false;
   }
   size_t      lengthOfTimeMeasure   = aValue.length() - splitPos;
   size_t      lengthOfContextNumber = aValue.length() - lengthOfTimeMeasure;
   std::string contextNumber         = aValue.substr(0, lengthOfContextNumber);
   try
   {
      auto contextNum = std::stoi(contextNumber);
      if (!IsInRange<int>(contextNum, 0, 999))
      {
         AddError("Interval Duration Context Number Failed Range Check",
                  contextNumber,
                  "Interval Duration Context Number must fall within [0, 999]");
         return false;
      }
   }
   catch (std::exception&) // failed stoi
   {
      AddError("Interval Duration Failed Conversion",
               aValue,
               "Interval Duration must be data that can be converted to an integer.");
      return false;
   }

   std::string timeMeasure = aValue.substr(splitPos);
   if (!IsValidUnitOfTimeMeasurement(timeMeasure))
   {
      return false;
   }
   mContextQuantity  = contextNumber;
   mUOM              = timeMeasure;
   mIntervalDuration = contextNumber + timeMeasure;
   return true;
}

bool IntervalDuration::IsValidUnitOfTimeMeasurement(const std::string& aValue) noexcept
{
   if (IsInSelection(aValue, {"BIWK", "DAY", "MON", "WK", "YR"}))
   {
      return true;
   }
   AddError("Time Measure is Invalid.", aValue, "Allowed Time Measure value are BIWK, DAY, MON, WK, or YR");
   return false;
}

bool IntervalDuration::IsUntilFurtherNotice() noexcept
{
   std::string content{mField.GetContent()};
   if (content == "UFN")
   {
      mIntervalDuration = content;
      return true;
   }
   AddError("Value is Not Until Further Notice.", content, "Value must be UFN");
   return false;
}

}; // namespace usmtf
