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

#include "FuelLoad.hpp"

namespace usmtf
{
FuelLoad::FuelLoad()
{
   SetValidRange(0.1F, 999.9F);
}

bool FuelLoad::ParseAndValidate(const Field& aField)
{
   if (!RangeValidator::ParseAndValidate(aField))
   {
      return false;
   }
   mFuelAmount = aField.GetContent();
   return true;
}

float FuelLoad::GetNumericalFuelAmount() const noexcept
{
   return GetCastedValue();
}

const std::string& FuelLoad::GetFuelAmount() const noexcept
{
   return mFuelAmount;
}

} // namespace usmtf
