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

#include "AirAssetControl.hpp"

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
AirAssetControl::AirAssetControl(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      // Only Agency can be validated as all others have freetext fields
      if (!mAgency.ParseAndValidate(GetField(1)))
      {
         AddErrors(mAgency.GetErrors());
      }
      mCallSign.ParseAndValidate(GetField(2));
      mPrimaryFrequency.ParseAndValidate(GetField(3));
      mSecondaryFrequency.ParseAndValidate(GetField(4));
      mLink16CallSign.ParseAndValidate(GetField(5));
      mReportInPoint.ParseAndValidate(GetField(6));
   }
   catch (const SetError& err)
   {
      if (err.GetPosition() >= 4)
      {
         return;
      }
      // Only these fields are mandatory
      AddError("CONTROLA Is Missing Required Fields",
               "CONTROLA",
               "Please make sure CONTROLA has a field in positions 1, 2 and 3.");
   }

   // optional and support free text, just getting the values if we can;
}

const AircraftControlAgency& AirAssetControl::GetAgency() const noexcept
{
   return mAgency;
}

const FreeText& AirAssetControl::GetCallsign() const noexcept
{
   return mCallSign;
}

const FreeText& AirAssetControl::GetPrimaryFrequency() const noexcept
{
   return mPrimaryFrequency;
}

const FreeText& AirAssetControl::GetSecondaryFrequency() const noexcept
{
   return mSecondaryFrequency;
}

const FreeText& AirAssetControl::GetLink16Callsign() const noexcept
{
   return mLink16CallSign;
}
const VariantValidator<LatLon, UTM_Location, VerifiedLatLonTM, VerifiedLatLonDS, FreeText>&
AirAssetControl::GetReportInPoint() const noexcept
{
   return mReportInPoint;
}
} // namespace usmtf
