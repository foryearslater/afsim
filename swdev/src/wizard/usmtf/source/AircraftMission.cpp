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

#include "AircraftMission.hpp"

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
AircraftMission::AircraftMission(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mNumberOfAircraft = std::stoi(GetFieldContent(1));
      mAircraftType     = GetFieldContent(2);
      mAircraftCallsign = GetFieldContent(3);
   }
   catch (SetError&)
   {
      AddError("MSNACFT Is Missing Required Fields",
               "MSNACFT",
               "Please make sure MSNACFT has a field in positions 1, 2 and 3.");
   }
   catch (std::invalid_argument&)
   {
      AddError("MSNACFT has invalid number of aircraft",
               "Number of aircraft",
               "Please make sure field 1 has a valid integer value.");
   }
   catch (std::out_of_range&)
   {
      AddError("MSNACFT has invalid number of aircraft",
               "Number of aircraft",
               "Please make sure field 1 has a valid integer value.");
   }
}

int AircraftMission::GetNumberOfAircraft() const noexcept
{
   return mNumberOfAircraft;
}

const std::string& AircraftMission::GetAircraftType() const noexcept
{
   return mAircraftType;
}

const std::string& AircraftMission::GetAircraftCallsign() const noexcept
{
   return mAircraftCallsign;
}

} // namespace usmtf
