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

#include "NamedLocation.hpp"

namespace usmtf
{
bool NamedLocation::ParseAndValidate(const Field& aField)
{
   bool isLatLon = LatLon::ParseAndValidate(aField);
   if (isLatLon)
   {
      mLocation   = GetFormattedLatLon();
      mIsFreeForm = false;
      return true;
   }
   std::string formattedLatLon = mICAOService.LookUpCode(aField.GetContent());
   if (!formattedLatLon.empty())
   {
      mLocation   = formattedLatLon;
      mIsFreeForm = false;
      return true;
   }
   mLocation   = aField.GetContent(); // Arbitrary name
   mIsFreeForm = true;
   return true;
   // There is no false case because we accept arbitrary string
}

const std::string& NamedLocation::GetLocation() const noexcept
{
   return mLocation;
}

bool NamedLocation::IsLocationFreeForm() const noexcept
{
   return mIsFreeForm;
}
} // namespace usmtf
