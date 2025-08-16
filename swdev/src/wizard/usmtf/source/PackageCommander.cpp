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

#include "PackageCommander.hpp"

namespace usmtf
{
PackageCommander::PackageCommander(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      // All Free Text Fields, Set cant be invalid beyond missing fields.
      mPackageId.ParseAndValidate(GetField(1));
      mTaskedUnit.ParseAndValidate(GetField(2));
      mMissionNumber.ParseAndValidate(GetField(3));
      mAircraftCallsign.ParseAndValidate(GetField(4));
      mLink16Callsign.ParseAndValidate(GetField(5));
   }
   catch (const SetError& err)
   {
      if (err.GetPosition() == 5)
      {
         return;
      }
      // Only these fields are mandatory
      AddError("PKGCMD Is Missing Required Fields",
               "PKGCMD",
               "Please make sure PKGCMD has a field in positions 1, 2, 3 and 4.");
   }

   // optional and support free text, just getting the values if we can;
}

const FreeText& PackageCommander::GetPackageId() const noexcept
{
   return mPackageId;
}

const FreeText& PackageCommander::GetTaskedUnit() const noexcept
{
   return mTaskedUnit;
}

const FreeText& PackageCommander::GetMissionNumber() const noexcept
{
   return mMissionNumber;
}

const FreeText& PackageCommander::GetAircraftCallsign() const noexcept
{
   return mAircraftCallsign;
}

const FreeText& PackageCommander::GetLink16Callsign() const noexcept
{
   return mLink16Callsign;
}
} // namespace usmtf
