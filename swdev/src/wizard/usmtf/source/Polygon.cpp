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

#include "Polygon.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Polygon::Polygon(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      if (!AreAllPointsValid())
      {
         AddError("POLYGON Required Field Validations Failed", "POLYGON", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("POLYGON Is Missing Required Fields", "POLYGON", "Please make sure POLYGON has fields for points.");
   }
}


const std::vector<LatLon>& Polygon::GetPoints() const noexcept
{
   return mPoints;
}

bool Polygon::AreAllPointsValid()
{
   for (size_t i = 1; i < mFields.size(); i++) // starts at one to skip set name
   {
      LatLon point{};
      point.ParseAndValidate(GetField(i));

      if (!point.IsValid())
      {
         AddErrors(point.GetErrors());
         return false;
      }
      mPoints.push_back(point);
   }
   return true;
}
} // namespace usmtf
