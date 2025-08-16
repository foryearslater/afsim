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

#include "Circle.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Circle::Circle(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mCenterOfCircle.ParseAndValidate(GetField(1));
      mRadius.ParseAndValidate(GetField(2));

      bool validationResult = mCenterOfCircle.IsValid() && mRadius.IsValid();
      if (!validationResult)
      {
         AddError("CIRCLE Required Field Validations Failed", "CIRCLE", "See Following Errors for details.");
         AddErrors(mCenterOfCircle.GetErrors());
         AddErrors(mRadius.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("CIRCLE Is Missing Required Fields", "CIRCLE", "Please make sure APERIOD has fields in positions 1,2");
   }
}

const LatLon& Circle::GetCenterOfCircle() const noexcept
{
   return mCenterOfCircle;
}

const Radius& Circle::GetRadius() const noexcept
{
   return mRadius;
}
} // namespace usmtf
