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

#include "Radarc.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Radarc::Radarc(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mOriginOfBearing.ParseAndValidate(GetField(1));
      mBeginningRadialBearing.ParseAndValidate(GetField(2));
      mEndingRadialBearing.ParseAndValidate(GetField(3));
      mInnerRadius.ParseAndValidate(GetField(4));
      mOuterRadius.ParseAndValidate(GetField(5));

      bool validationResult = mOriginOfBearing.IsValid() && mBeginningRadialBearing.IsValid() &&
                              mEndingRadialBearing.IsValid() && mInnerRadius.IsValid() && mOuterRadius.IsValid();

      if (!validationResult)
      {
         AddErrors(mOriginOfBearing.GetErrors());
         AddErrors(mBeginningRadialBearing.GetErrors());
         AddErrors(mEndingRadialBearing.GetErrors());
         AddErrors(mInnerRadius.GetErrors());
         AddErrors(mOuterRadius.GetErrors());
         AddError("RADARC Required Field Validations Failed", "RADARC", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("RADARC Is Missing Required Fields", "RADARC", "Please make sure RADARC has fields in positions 1,2,3,4,5");
   }
}

const LatLon& Radarc::GetOriginOfBearing() const noexcept
{
   return mOriginOfBearing;
}

const RadialBearing& Radarc::GetBeginningRadialBearingTrue() const noexcept
{
   return mBeginningRadialBearing;
}

const RadialBearing& Radarc::GetEndingRadialBearingTrue() const noexcept
{
   return mEndingRadialBearing;
}

const Radius& Radarc::GetInnerRadius() const noexcept
{
   return mInnerRadius;
}

const Radius& Radarc::GetOuterRadius() const noexcept
{
   return mOuterRadius;
}
} // namespace usmtf
