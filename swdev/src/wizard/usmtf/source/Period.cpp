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

#include "Period.hpp"

#include "FFIRN_Validator.hpp"
#include "Field.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Period::Period(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mStartTime.ParseAndValidate(GetField(1));
      mStopTime.ParseAndValidate(GetField(2));
      bool validationResult = mStartTime.IsValid() && mStopTime.IsValid() && IsValidStopTimeModifier();
      if (!validationResult)
      {
         AddErrors(mStartTime.GetErrors());
         AddErrors(mStopTime.GetErrors());
         AddError("PERIOD Required Field Validations Failed", "PERIOD", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("PERIOD Is Missing Required Fields",
               "PERIOD",
               "Please make sure PERIOD has fields in positions 1,2 and 3 if stop time modifer required.");
   }
}

const VerifiedTimePoint& Period::GetStartTime() const noexcept
{
   return mStartTime;
}

const VerifiedTimePoint& Period::GetStopTime() const noexcept
{
   return mStopTime;
}

const VerifiedTimePoint& Period::GetStopTimeModifier() const noexcept
{
   return mStopTimeModifier;
}

bool Period::IsValidStopTimeModifier()
{
   if (!DoesStopTimeRequireModifier(GetField(2).GetContent()))
   {
      // Sets optional field to nullptr.
      return true;
   }

   mStopTimeModifier.ParseAndValidate(GetField(3));
   if (!mStopTimeModifier.IsValid())
   {
      AddErrors(mStopTimeModifier.GetErrors());
      return false;
   }
   return true;
}

bool Period::DoesStopTimeRequireModifier(const std::string& stopTime) const noexcept
{
   return FFIRN_Validator::IsInSelection(stopTime, {"AFTER", "ASOF", "ASAPFT", "ASAPNLT", "BEFORE", "NLT", "NET"});
}
} // namespace usmtf
