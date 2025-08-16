// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include "Geoline.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Geoline::Geoline(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      AreAllPointsValid();
   }
   catch (SetError&)
   {
      AddError("GEOLINE Is Missing Required Fields",
               "GEOLINE",
               "Please make sure GEOLINE has fields in positions 1 and n repeating");
   }
}

const std::vector<LatLon>& Geoline::GetPoints() const noexcept
{
   return mPoints;
}

bool Geoline::AreAllPointsValid()
{
   for (size_t i = 1; i < mFields.size(); i++)
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
