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

#include "Frequency.hpp"

namespace usmtf
{
Frequency::Frequency()
{
   SetValidRange(0.0000001F, 999999.9F);
}

bool Frequency::ParseAndValidate(const Field& aField)
{
   if (!RangeValidator::ParseAndValidate(aField))
   {
      return false;
   }
   mFrequency = aField.GetContent();
   return true;
}

float Frequency::GetNumericalFrequency() const noexcept
{
   return GetCastedValue();
}

const std::string& Frequency::GetFrequency() const noexcept
{
   return mFrequency;
}

} // namespace usmtf
