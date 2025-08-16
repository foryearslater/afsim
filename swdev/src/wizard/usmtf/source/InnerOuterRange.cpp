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


#include "InnerOuterRange.hpp"

namespace usmtf
{
bool InnerOuterRange::ParseAndValidate(const Field& aField)
{
   std::string fieldContent = aField.GetContent();
   auto        parts        = SplitOnDelimeter(fieldContent, "-");
   if (parts.size() != 2)
   {
      AddError("InnerOuterRange Failed Parsing",
               fieldContent,
               "InnerOuterRange must contain a delineation string of - and two parts for inner and outer radius.");
      return false;
   }

   try
   {
      std::string innerRadius = parts.at(0);
      std::string outerRadius = parts.at(1);

      // delegate outer to the radius, this cover uom and outer range
      Radius::ParseAndValidate(Field{outerRadius});

      // check inner range
      IsInvalidRange(innerRadius);

      // checks the validity of the results of both & inner outer.
      if (!IsValid())
      {
         // Set all to defaults
         mInnerRadius       = std::string();
         mInnerRadiusDouble = 0;
         mRadiusDouble      = 0;
         mRadiusValue       = std::string();
         mUnitOfMeasure     = std::string();
         return false;
      }
      mInnerRadiusDouble = mCastedRangeValue;
      mInnerRadius       = std::to_string(mInnerRadiusDouble);
      return true;
   }
   catch (const std::out_of_range&)
   {
      AddError("InnerOuterRange Failed Parsing", fieldContent, "InnerOuterRange must contain both an inner and outer radius.");
      return false;
   }
}

const std::string& InnerOuterRange::GetInnerRadiusValue() const noexcept
{
   return mInnerRadius;
}

double InnerOuterRange::GetInnerRadius() const noexcept
{
   return mInnerRadiusDouble;
}

double InnerOuterRange::GetInnerRadiusInMeters() const noexcept
{
   return GetRadiusInMeters(mInnerRadiusDouble);
}


} // namespace usmtf
