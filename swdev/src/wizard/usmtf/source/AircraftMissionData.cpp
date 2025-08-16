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

#include "AircraftMissionData.hpp"

namespace usmtf
{
AircraftMissionData::AircraftMissionData(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      // Given every field has a free text component, this set literally cant be invalid unless missing mandatory field.
      // This means there is no need to extract errors.
      mRMI.ParseAndValidate(GetField(1));
      mMissionNumber.ParseAndValidate(GetField(2));
      mAmc.ParseAndValidate(GetField(3));
      mPackageId.ParseAndValidate(GetField(4));
      mMissionCommander.ParseAndValidate(GetField(5));
      mPrimaryMissionType.ParseAndValidate(GetField(6));
      mSecondaryMissionType.ParseAndValidate(GetField(7));
      mAlertStatus.ParseAndValidate(GetField(8));
      mDepartureLocation.ParseAndValidate(GetField(9));
      mTimeOfDeparture.ParseAndValidate(GetField(10));
      for (size_t i = 11; i < mFields.size(); i += 2)
      {
         std::unique_ptr<Location> loc = ut::make_unique<Location>();
         loc->ParseAndValidate(GetField(i));
         mRecoveryLocations.push_back(std::move(loc));
         FreeText recoverTime;
         recoverTime.ParseAndValidate(GetField(i + 1));
         mTimeOfRecovery.push_back(recoverTime);
      }
   }
   catch (const SetError& err)
   {
      if (err.GetPosition() == 3 || err.GetPosition() == 5 || err.GetPosition() >= 7)
      {
         return;
      }
      AddError("AMSNDAT Is Missing Required Fields",
               "AMSNDAT",
               "Please make sure AMSNDAT has fields in positions mandatory positions 1,2,4 and 6.");
   }
}

const FreeText& AircraftMissionData::GetResidualMissionIndicator() const noexcept
{
   return mRMI;
}

const FreeText& AircraftMissionData::GetMissionNumber() const noexcept
{
   return mMissionNumber;
}

const VariantValidator<RangeValidator<int>, FreeText>& AircraftMissionData::GetEventNumber() const noexcept
{
   return mAmc;
}

const FreeText& AircraftMissionData::GetPackageIdentification() const noexcept
{
   return mPackageId;
}

const FreeText& AircraftMissionData::GetMissionCommander() const noexcept
{
   return mMissionCommander;
}

const FreeText& AircraftMissionData::GetPrimaryMissionType() const noexcept
{
   return mPrimaryMissionType;
}

const FreeText& AircraftMissionData::GetSecondaryMissionType() const noexcept
{
   return mSecondaryMissionType;
}

const FreeText& AircraftMissionData::GetAlertStatus() const noexcept
{
   return mAlertStatus;
}

const AircraftMissionData::Location& AircraftMissionData::GetDepartureLocation() const noexcept
{
   return mDepartureLocation;
}

const FreeText& AircraftMissionData::GetTimeOfDeparture() const noexcept
{
   return mTimeOfDeparture;
}

const AircraftMissionData::Locations& AircraftMissionData::GetRecoveryLocations() const noexcept
{
   return mRecoveryLocations;
}

const std::vector<FreeText>& AircraftMissionData::GetRecoverTimes() const noexcept
{
   return mTimeOfRecovery;
}
} // namespace usmtf
