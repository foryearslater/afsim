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


#include "Corridor.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Corridor::Corridor(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mWidth.ParseAndValidate(GetField(1));
      bool validationResult = mWidth.IsValid() && AreAllPointsValid();
      if (!validationResult)
      {
         AddErrors(mWidth.GetErrors());
         AddError("CORRIDOR Required Field Validations Failed", "CORRIDOR", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("CORRIDOR Is Missing Required Fields",
               "CORRIDOR",
               "Please make sure CORRIDOR has fields in positions 1 and n repeating");
   }
}

const Radius& Corridor::GetWidth() const noexcept
{
   return mWidth;
}

const std::vector<LatLon>& Corridor::GetPoints() const noexcept
{
   return mPoints;
}

bool Corridor::AreAllPointsValid()
{
   for (size_t i = 2; i < mFields.size(); i++)
   {
      LatLon point{};
      point.ParseAndValidate(GetField(i));
      if (!point.IsValid())
      {
         AddErrors(point.GetErrors());
         return false;
      }
      mPoints.push_back(std::move(point));
   }
   return true;
}
} // namespace usmtf
