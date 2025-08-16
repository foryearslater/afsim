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

#include "Altitude.hpp"

namespace usmtf
{

Altitude::Altitude() noexcept
{
   SetValidRange(0, 999);
}

bool Altitude::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   if (RangeValidator::ParseAndValidate(aField))
   {
      mVerticalDistance = std::to_string(mCastedRangeValue);
      ClearErrors(); // Has to be set because states can change with SetValidRange
      return true;
   }
   return false;
}


const std::string& Altitude::GetVerticalDistance() const noexcept
{
   return mVerticalDistance;
}
} // namespace usmtf
