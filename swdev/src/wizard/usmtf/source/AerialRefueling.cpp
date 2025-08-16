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

#include "AerialRefueling.hpp"

#include "UtMemory.hpp"

namespace usmtf
{
AerialRefueling::AerialRefueling(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : TabularSet(aSTF, aFields, 9)
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
         FreeText missionNumber{};
         missionNumber.ParseAndValidate(*row[0]);

         FreeText recieverCallsign{};
         recieverCallsign.ParseAndValidate(*row[1]);

         RangeValidator<int> allocatedAircraft{};
         allocatedAircraft.SetValidRange(1, 99);
         allocatedAircraft.ParseAndValidate(*row[2]);
         if (!allocatedAircraft.IsValid())
         {
            AddErrors(allocatedAircraft.GetErrors());
         }

         FreeText typeOfAircraft{};
         typeOfAircraft.ParseAndValidate(*row[3]);

         FuelLoad fuelLoad{};
         if (!fuelLoad.ParseAndValidate(*row[4]))
         {
            AddErrors(fuelLoad.GetErrors());
         }

         FreeText controlTime{};
         controlTime.ParseAndValidate(*row[5]);

         FreeText cellSequence{};
         cellSequence.ParseAndValidate(*row[6]);

         FuelType refuelType{};
         if (!refuelType.ParseAndValidate(*row[7]))
         {
            if (row[7]->GetContent() != NULL_FIELD)
            {
               AddErrors(refuelType.GetErrors());
            }
         }

         AerialRefuelingSystem system{};
         if (!system.ParseAndValidate(*row[8]))
         {
            if (row[8]->GetContent() != NULL_FIELD)
            {
               AddErrors(system.GetErrors());
            }
         }

         RefuelInstruction instruction{missionNumber,
                                       recieverCallsign,
                                       allocatedAircraft.GetCastedValue(),
                                       typeOfAircraft,
                                       fuelLoad,
                                       controlTime,
                                       cellSequence,
                                       refuelType,
                                       system};

         mInstructions.push_back(std::move(instruction));
      }
   }
   catch (const SetError& err)
   {
      if (err.GetPosition() == 6 || err.GetPosition() == 7 || err.GetPosition() == 8)
      {
         return;
      }
      AddError("5REFUEL Is Missing Required Fields",
               "5REFUEL",
               "Please make sure 5REFUEL has fields in mandatory positions 1,2,3,4,5 and 6 of every row.");
   }
}

const AerialRefueling::RefuelInstructions& AerialRefueling::GetRefuelInstructions() const noexcept
{
   return mInstructions;
}
} // namespace usmtf
