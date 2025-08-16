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

#include "RefuelingTasking.hpp"

namespace usmtf
{
RefuelingTasking::RefuelingTasking(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      if (!mAerialRefuelingSystem.ParseAndValidate(GetField(1)))
      {
         AddErrors(mAerialRefuelingSystem.GetErrors());
      }
      if (!mOffloadFuel.ParseAndValidate(GetField(2)))
      {
         AddErrors(mOffloadFuel.GetErrors());
      }
      if (!mContingencyOffloadFuel.ParseAndValidate(GetField(3)))
      {
         AddErrors(mContingencyOffloadFuel.GetErrors());
      }
      if (!mTacanChannel.ParseAndValidate(GetField(6)))
      {
         AddErrors(mTacanChannel.GetErrors());
      }
      if (!mBeacon.ParseAndValidate(GetField(7)))
      {
         AddErrors(mBeacon.GetErrors());
      }
      // Frequencies technically cant be invalid as they have FreeText options.
      mPrimaryFrequency.ParseAndValidate(GetField(4));
      mSecondaryFrequency.ParseAndValidate(GetField(5));
   }
   catch (const SetError& err)
   {
      if (err.GetPosition() == 3 || err.GetPosition() == 5 || err.GetPosition() == 6 || err.GetPosition() == 7)
      {
         return;
      }
      AddError("REFTSK Is Missing Required Fields", "REFTSK", "Please make sure REFTSK has a field in positions 1, 2 and 4.");
   }
}

const AerialRefuelingSystem& RefuelingTasking::GetRefuelingSystem() const noexcept
{
   return mAerialRefuelingSystem;
}

const FuelLoad& RefuelingTasking::GetOffloadFuel() const noexcept
{
   return mOffloadFuel;
}

const FuelLoad& RefuelingTasking::GetContingencyOffloadFuel() const noexcept
{
   return mContingencyOffloadFuel;
}

const TacanChannel& RefuelingTasking::GetTacanChannel() const noexcept
{
   return mTacanChannel;
}

const Beacon& RefuelingTasking::GetBeacon() const noexcept
{
   return mBeacon;
}
} // namespace usmtf
