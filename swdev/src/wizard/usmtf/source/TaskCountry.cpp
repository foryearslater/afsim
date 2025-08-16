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

#include "TaskCountry.hpp"

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
TaskCountry::TaskCountry(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mCountry = GetFieldContent(1);
   }
   catch (SetError&)
   {
      AddError("TSKCNTRY Is Missing Required Fields", "TSKCNTRY", "Please make sure TSKCNTRY has a country as field 1.");
   }
}

const std::string& TaskCountry::GetCountry() const noexcept
{
   return mCountry;
}

} // namespace usmtf
