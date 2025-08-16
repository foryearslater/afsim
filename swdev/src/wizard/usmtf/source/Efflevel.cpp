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

#include "Efflevel.hpp"

#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Efflevel::Efflevel(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mVerticalDimension.ParseAndValidate(GetField(1));
      if (!mVerticalDimension.IsValid())
      {
         AddError("Efflevel Required Field Validations Failed", "Efflevel", "See Following Errors for details.");
         AddErrors(mVerticalDimension.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("Efflevel Is Missing Required Fields", "Efflevel", "Please make sure Efflevel has fields in positions 1");
   }
}

const VerticalDimension& Efflevel::GetVerticalDimension() const noexcept
{
   return mVerticalDimension;
}
} // namespace usmtf
