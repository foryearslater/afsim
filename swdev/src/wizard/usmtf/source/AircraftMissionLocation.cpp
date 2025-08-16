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

#include "AircraftMissionLocation.hpp"

#include "UtMemory.hpp"

namespace usmtf
{
AircraftMissionLocation::AircraftMissionLocation(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mMissionLocationName = GetFieldContent(3);
      if (!mAltittude.ParseAndValidate(GetField(4)))
      {
         AddErrors(mAltittude.GetErrors());
      }
      if (!mAreaGeometry.ParseAndValidate(GetField(6)))
      {
         AddErrors(mAreaGeometry.GetErrors());
      }
      // locations repeating
      size_t length = aFields.size();
      // 7 is start of locations fields, they will repeat until the end.
      // This loop serves only to add locations to the vector, the reality
      // is that locations will never be invalid because free text is supported.
      // Now, this is still actionable downstream as VariantValidation will match
      // on FreeText, making logic programmable for templating user input.
      for (size_t i = 7; i < length; i++)
      {
         std::unique_ptr<Location> loc = ut::make_unique<Location>();
         loc->ParseAndValidate(GetField(i));
         mLocations.push_back(std::move(loc));
      }
   }
   catch (const SetError&)
   {
      // All useful fields to AFSIM are optional in the spec at this time.
   }
}

const std::string& AircraftMissionLocation::GetMissionLocation() const
{
   return mMissionLocationName;
}

const Altitude& AircraftMissionLocation::GetAltitude() const noexcept
{
   return mAltittude;
}

const AircraftMissionLocation::AreaGeometry& AircraftMissionLocation::GetAreaGeometry() const noexcept
{
   return mAreaGeometry;
}

const AircraftMissionLocation::Locations& AircraftMissionLocation::GetLocations() const noexcept
{
   return mLocations;
}

} // namespace usmtf
