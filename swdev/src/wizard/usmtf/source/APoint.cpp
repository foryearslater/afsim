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

#include "APoint.hpp"

#include "FFIRN_Validator.hpp"
#include "LatLon.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
APoint::APoint(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mAirspacePoint.ParseAndValidate(GetField(1));
      if (!mAirspacePoint.IsValid())
      {
         AddError("APOINT Required Field Validations Failed", "APOINT", "See Following Errors for details.");
         AddErrors(mAirspacePoint.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("APOINT Is Missing Required Fields", "APOINT", "Please make sure APOINT has fields in positions 1");
   }
}

const LatLon& APoint::GetAirSpacePoint() const noexcept
{
   return mAirspacePoint;
}
} // namespace usmtf
