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

#include "PackageData.hpp"

#include "RangeValidator.hpp"

namespace usmtf
{
PackageData::PackageData(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : TabularSet(aSTF, aFields, 7)
{
   try
   {
      bool validFieldCount = IsValidFieldCount();
      if (!validFieldCount)
      {
         AddError("Field count is invalid",
                  std::to_string(GetFieldCount()),
                  "Number of Columns must be equal to number defined by header row.");
      }
      size_t rows = GetRowCount();
      for (size_t currentRow = 1; currentRow <= rows; currentRow++)
      {
         Series row = GetRow(currentRow);

         // FreeText fields cant be invalid, But we need to parse them.
         FreeText packageId{};
         packageId.ParseAndValidate(*row[0]);

         FreeText taskedUnit{};
         taskedUnit.ParseAndValidate(*row[1]);

         FreeText missionNumber{};
         missionNumber.ParseAndValidate(*row[2]);

         FreeText missionType{};
         missionType.ParseAndValidate(*row[3]);

         RangeValidator<int> airplanes{};
         airplanes.SetValidRange(1, 99);
         airplanes.ParseAndValidate(*row[4]);
         if (!airplanes.IsValid())
         {
            AddErrors(airplanes.GetErrors());
         }

         FreeText typeOfAircraft{};
         typeOfAircraft.ParseAndValidate(*row[5]);

         FreeText aircraftCallsign{};
         aircraftCallsign.ParseAndValidate(*row[6]);


         Unit unit{packageId, taskedUnit, missionNumber, missionType, airplanes.GetCastedValue(), typeOfAircraft, aircraftCallsign};
         mUnits.push_back(std::move(unit));
      }
   }
   catch (SetError&)
   {
      AddError("PackageData Is Missing Required Fields",
               "PackageData",
               "Please make sure PackageData has fields in positions mandatory positions of every row.");
   }
}

const PackageData::Units& PackageData::GetUnits() const noexcept
{
   return mUnits;
}
} // namespace usmtf
