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

#include "Acmid.hpp"

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
Acmid::Acmid(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      GetField(2);
      mShape.ParseAndValidate(GetField(3));
      if (!mShape.IsValid())
      {
         AddError("ACMID Required Field Validations Failed", "ACMID", "See Following Errors for details.");
         AddErrors(mShape.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("ACMID Is Missing Required Fields", "ACMID", "Please make sure ACMID has fields in positions 2,3");
   }
}


const std::string& Acmid::GetAirspaceControlMeansIdentifier() const noexcept
{
   return GetFieldContent(2);
}

const std::string& Acmid::GetTypeOfAirspaceShape() const noexcept
{
   return mShape.GetShape();
}
} // namespace usmtf
