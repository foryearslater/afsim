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

#include "OrbitAlignment.hpp"

namespace usmtf
{
bool OrbitAlignment::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidOrbitAlignment();
}

const std::string& OrbitAlignment::GetOrbitAlignment() const noexcept
{
   return mOrbitAlignment;
}

bool OrbitAlignment::IsValidOrbitAlignment() noexcept
{
   const std::string& val = mField.GetContent();
   if (IsInSelection(val, {"C", "L", "R"}))
   {
      mOrbitAlignment = val;
      return true;
   }

   AddError("Orbit Alignment Is Invalid", val, "Orbit Alignment must be C, L, or R");
   return false;
}
}; // namespace usmtf
