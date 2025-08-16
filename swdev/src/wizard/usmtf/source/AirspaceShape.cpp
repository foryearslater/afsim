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

#include "AirspaceShape.hpp"

namespace usmtf
{
bool AirspaceShape::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidTypeOfAirspaceShape();
}

const std::string& AirspaceShape::GetShape() const noexcept
{
   return mAirspaceShape;
}

bool AirspaceShape::IsValidTypeOfAirspaceShape() noexcept
{
   const std::string value = mField.GetContent();
   if (IsInSelection(value, {"CIRCLE", "CORRIDOR", "LINE", "POINT", "POLYARC", "POLYGON", "RADARC", "TRACK", "ORBIT"}))
   {
      mAirspaceShape = value;
      return true;
   }
   AddError("Airspace Shape Is Invalid.",
            value,
            "Shape must be a CIRCLE, CORRIDOR, LINE, POINT, POLYARC, POLYGON, RADARC or TRACK.");
   return false;
}
}; // namespace usmtf
