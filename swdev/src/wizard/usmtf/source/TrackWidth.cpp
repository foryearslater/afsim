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

#include "TrackWidth.hpp"

#include "Radius.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
bool TrackWidth::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidTrackWidth();
}

const Radius* TrackWidth::GetLeftRadius() const noexcept
{
   return mLeftRadius.get();
}

const Radius* TrackWidth::GetRightRadius() const noexcept
{
   return mRightRadius.get();
}

bool TrackWidth::IsValidTrackWidth() noexcept
{
   const std::string& aValue   = mField.GetContent();
   size_t             splitPos = CheckForAnyDelimeter(aValue, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Track Width Failed Parsing", aValue, "Track Width must contain delineation in the form of a - ");
      return false;
   }
   size_t      split_size = aValue.length() - splitPos;
   size_t      leftLength = aValue.length() - split_size;
   std::string left       = aValue.substr(0, leftLength);
   std::string right      = aValue.substr(splitPos + 1);    // skip dash
   std::string leftRadius = left.substr(0, leftLength - 1); // strip identifier
   Radius      lr{};
   lr.ParseAndValidate(Field{leftRadius});
   char        leftIdentifier = left.back();
   std::string rightRadius    = right.substr(0, right.length() - 1); // strip identifier
   Radius      rr{};
   rr.ParseAndValidate(Field{rightRadius});
   char rightIdentifier = right.back();
   if (!lr.IsValid() || (leftIdentifier != 'L'))
   {
      AddError(
         "Track Width is Invalid.",
         std::to_string(leftIdentifier),
         "Track Width Must contain Left/Right Identification to determine which radius is which. L was expected.");
      return false;
   }
   if (!rr.IsValid() || (rightIdentifier != 'R'))
   {
      AddError(
         "Track Width is Invalid.",
         std::to_string(rightIdentifier),
         "Track Width Must contain Left/Right Identification to determine which radius is which. R was expected.");
      return false;
   }

   mRightRadius = ut::make_unique<Radius>(rr);
   mLeftRadius  = ut::make_unique<Radius>(lr);
   return true;
}

}; // namespace usmtf
