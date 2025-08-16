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

#include "Onetrack.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"

namespace usmtf
{
OneTrack::OneTrack(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : TabularSet(aSTF, aFields, 5)
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
      bool validationResult = validFieldCount && AreLegsValid();
      if (!validationResult)
      {
         AddError("OneTrack Required Field Validations Failed", "OneTrack", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("ONETRACK Is Missing Required Fields",
               "ONETRACK",
               "Please make sure ONETRACK has fields in positions 1,2,3,4 and 5 of every LEG");
   }
}

const OneTrack::Legs& OneTrack::GetLegs() const noexcept
{
   return mLegs;
}

bool OneTrack::AreLegsValid()
{
   size_t rows = GetRowCount();
   for (size_t currentRow = 1; currentRow <= rows; currentRow++)
   {
      Series            row = GetRow(currentRow);
      const std::string seq = row[0]->GetContent();
      AreLegsSequential(seq, currentRow);

      LatLon legBegin{};
      legBegin.ParseAndValidate(*row[1]);
      if (!legBegin.IsValid())
      {
         AddErrors(legBegin.GetErrors());
      }

      LatLon legEnd{};
      legEnd.ParseAndValidate(*row[2]);
      if (!legEnd.IsValid())
      {
         AddErrors(legEnd.GetErrors());
      }

      TrackWidth width{};
      width.ParseAndValidate(*row[3]);
      if (!width.IsValid())
      {
         AddErrors(width.GetErrors());
      }

      VerticalDimension depth{};
      depth.ParseAndValidate(*row[4]);
      if (!depth.IsValid())
      {
         AddErrors(depth.GetErrors());
      }

      Leg leg{currentRow, legBegin, legEnd, std::move(width), depth};
      mLegs.push_back(std::move(leg));
   }

   return IsValid();
}

bool OneTrack::AreLegsSequential(const std::string& aLegSequence, size_t aCurrentIteration)
{
   std::string rs = std::to_string(aCurrentIteration);
   // pad it to match format from usmtf
   while (rs.length() < 2)
   {
      rs = "0" + rs;
   }

   if (rs != aLegSequence)
   {
      AddError("Leg Sequence was not in order.", rs, "Please make sure you legs are in sequential order");
      return false;
   }
   return true;
}
} // namespace usmtf
