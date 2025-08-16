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


#include "TaskUnit.hpp"

namespace usmtf
{
TaskUnit::TaskUnit(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mUnitDesignator.ParseAndValidate(GetField(1));
      mLocation.ParseAndValidate(GetField(2));
      mComments = GetFieldContent(4);
   }
   catch (const SetError&)
   {
      // There really isn't an invalid state of Task Unit at this time. The fields
      // that are actionable are optional. If they are present great, lets use them
      // otherwise blanks data will come out of the methods.
   }
}

const FreeText& TaskUnit::GetUnitDesignator() const noexcept
{
   return mUnitDesignator;
}

const NamedLocation& TaskUnit::GetLocation() const noexcept
{
   return mLocation;
}

const std::string& TaskUnit::GetComments() const noexcept
{
   return mComments;
}
} // namespace usmtf
