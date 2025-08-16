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


#include "RadialBearing.hpp"

#include <stdexcept>

namespace usmtf
{
bool RadialBearing::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidRadialBearing();
}

const std::string& RadialBearing::GetBearing() const noexcept
{
   return mBearing;
}

bool RadialBearing::IsValidRadialBearing() noexcept
{
   const std::string& aValue = mField.GetContent();
   try
   {
      if (!IsStringLength(aValue, 4))
      {
         AddError("Radial Bearing Failed Length Check", aValue, "Radial Bearing Must be a length of 4 chars.");
         return false;
      }

      if (aValue.back() != 'T')
      {
         AddError("Radial Bearing is Invalid", aValue, "Radial Bearing must have a suffix of T");
         return false;
      }
      std::string sdegree = aValue.substr(0, 3);
      if (!IsInRange<int>(std::stoi(sdegree), 0, 359))
      {
         AddError("Radial Bearing is Invalid", sdegree, "Radial Bearing must have a degree between [0. 359]");
         return false;
      }
      mBearing = sdegree;
      return true;
   }
   catch (std::exception&)
   {
      AddError("Radial Bearing Failed Conversion", aValue, "Radial Bearing must be convertible to int");
      return false;
   }
}
}; // namespace usmtf
