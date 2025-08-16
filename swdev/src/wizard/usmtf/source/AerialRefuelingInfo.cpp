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

#include "AerialRefuelingInfo.hpp"

namespace usmtf
{
AerialRefuelingInfo::AerialRefuelingInfo(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mAirCallSign.ParseAndValidate(GetField(1));
      mMissionNumber.ParseAndValidate(GetField(2));
      mIffSifMode.ParseAndValidate(GetField(3));
      mControlPoint.ParseAndValidate(
         GetField(4)); // given FreeText is the last validator in the Variant, its not possible to fail.
      if (!mAltitude.ParseAndValidate(GetField(5)))
      {
         AddErrors(mAltitude.GetErrors());
      }
      mRefuelTime.ParseAndValidate(GetField(6));
      mRefuelTimeEnd.ParseAndValidate(GetField(7));
      if (!mOffloadFuel.ParseAndValidate(GetField(8)))
      {
         AddErrors(mOffloadFuel.GetErrors());
      }
      mPrimaryFrequency.ParseAndValidate(GetField(9));
      mSecondaryFrequency.ParseAndValidate(GetField(10));
      mLink16Callsign.ParseAndValidate(GetField(11));
      mAircraftType.ParseAndValidate(GetField(12));
      if (!mRefuelSystem.ParseAndValidate(GetField(13)))
      {
         if (GetFieldContent(13) != NULL_FIELD)
         {
            AddErrors(mRefuelSystem.GetErrors());
         }
      }
      RangeValidator<int> numberOfAircraft{};
      numberOfAircraft.SetValidRange(1, 99);
      if (!numberOfAircraft.ParseAndValidate(GetField(14)))
      {
         if (GetFieldContent(14) != NULL_FIELD)
         {
            AddErrors(numberOfAircraft.GetErrors());
         }
         else
         {
            mAircraftInCell = "0";
         }
      }
      else
      {
         mAircraftInCell = numberOfAircraft.GetRawContent();
      }
      mCellSequence.ParseAndValidate(GetField(15));
      if (!mTacanChannel.ParseAndValidate(GetField(16)))
      {
         if (GetFieldContent(16) != NULL_FIELD)
         {
            AddErrors(mTacanChannel.GetErrors());
         }
      }
      if (!mBeacon.ParseAndValidate(GetField(17)))
      {
         if (GetFieldContent(17) != NULL_FIELD)
         {
            AddErrors(mBeacon.GetErrors());
         }
      }
   }

   catch (const SetError& err)
   {
      if (err.GetPosition() == 7 || err.GetPosition() >= 10)
      {
         return;
      }
      AddError("ARINFO Is Missing Required Fields",
               "ARINFO",
               "Please make sure ARINFO has fields in mandatory positions 1,2,3,4,5,6, 8 and 9.");
   }
}

const FreeText& AerialRefuelingInfo::GetCallsign() const noexcept
{
   return mAirCallSign;
}

const FreeText& AerialRefuelingInfo::GetMissionNumber() const noexcept
{
   return mMissionNumber;
}

const FreeText& AerialRefuelingInfo::GetIffSifMode() const noexcept
{
   return mIffSifMode;
}

const VariantValidator<LatLon, RelativeBearingRange, FreeText>& AerialRefuelingInfo::GetControlPoint() const noexcept
{
   return mControlPoint;
}

const Altitude& AerialRefuelingInfo::GetAltitude() const noexcept
{
   return mAltitude;
}

const FreeText& AerialRefuelingInfo::GetRefuelTime() const noexcept
{
   return mRefuelTime;
}

const FreeText& AerialRefuelingInfo::GetRefuelEndTime() const noexcept
{
   return mRefuelTimeEnd;
}

const FuelLoad& AerialRefuelingInfo::GetOffloadFuel() const noexcept
{
   return mOffloadFuel;
}

const FreeText& AerialRefuelingInfo::GetLink16Callsign() const noexcept
{
   return mLink16Callsign;
}

const FreeText& AerialRefuelingInfo::GetAircraftType() const noexcept
{
   return mAircraftType;
}

const AerialRefuelingSystem& AerialRefuelingInfo::GetRefuelSystem() const noexcept
{
   return mRefuelSystem;
}

const std::string& AerialRefuelingInfo::GetNumberOfAircraftInCell() const noexcept
{
   return mAircraftInCell;
}

const FreeText& AerialRefuelingInfo::GetCellSequence() const noexcept
{
   return mCellSequence;
}

const TacanChannel& AerialRefuelingInfo::GetTacanChannel() const noexcept
{
   return mTacanChannel;
}

const Beacon& AerialRefuelingInfo::GetBeacon() const noexcept
{
   return mBeacon;
}
} // namespace usmtf
