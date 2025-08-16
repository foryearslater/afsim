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

#include "AirspaceTimeMode.hpp"

namespace usmtf
{
bool AirspaceTimeMode::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidAirspaceTimeMode();
}

const std::string& AirspaceTimeMode::GetTimeMode() const noexcept
{
   return mTimeMode;
}

bool AirspaceTimeMode::IsValidAirspaceTimeMode() noexcept
{
   const std::string fieldContents = mField.GetContent();
   if (!IsStringLength(fieldContents, 8))
   {
      AddError("Airspace Time Mode Failed Length Check", fieldContents, "Airspace Time Mode must be a length of 8 chars.");
      return false;
   }

   if (!IsInSelection(fieldContents, {"INTERVAL", "DISCRETE"}))
   {
      AddError("Airspace Time Mode Is Invalid", fieldContents, "Airspace Time Mode must be INTERVAL or DISCRETE.");
      return false;
   }
   mTimeMode = fieldContents;
   return true;
}
}; // namespace usmtf
