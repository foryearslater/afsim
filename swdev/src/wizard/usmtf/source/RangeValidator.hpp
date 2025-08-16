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


#ifndef RANGEDVALIDATOR_HPP
#define RANGEDVALIDATOR_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

#include "FFIRN_Validator.hpp"


namespace usmtf
{
template<typename T>
class USMTF_EXPORT RangeValidator : public FFIRN_Validator
{
   static_assert(std::is_arithmetic<T>::value, "Type must pass is_arithmetic<T>");

public:
   bool ParseAndValidate(const usmtf::Field& aField) override
   {
      SetField(aField);
      return IsInvalidRange(aField.GetContent());
   }

   void SetValidRange(T aLowerBound, T aUpperBound)
   {
      mLowerBound = aLowerBound;
      mUpperBound = aUpperBound;
   }

   T GetCastedValue() const noexcept { return mCastedRangeValue; }

protected:
   bool IsInvalidRange(T aValue) noexcept
   {
      if (FFIRN_Validator::IsInRange<T>(aValue, mLowerBound, mUpperBound))
      {
         return true;
      }
      std::string suggestedAction =
         "Value must fall in the range [" + std::to_string(mLowerBound) + ", " + std::to_string(mUpperBound) + "]";
      AddError("Value failed range check.", std::to_string(aValue), suggestedAction);
      mCastedRangeValue = 0;
      return false;
   }

   // For use on sub values in a field
   bool IsInvalidRange(const std::string& aValue) noexcept
   {
      T casted;
      std::stringstream(aValue) >> casted;
      if (!casted && aValue != "0")
      {
         AddError("Error Casting value", aValue, "Please make sure value is valid int/double/float");
         mCastedRangeValue = 0;
         return false;
      }
      mCastedRangeValue = casted;
      return IsInvalidRange(casted);
   }

   T mCastedRangeValue;
   T mLowerBound;
   T mUpperBound;
};
} // namespace usmtf

#endif
