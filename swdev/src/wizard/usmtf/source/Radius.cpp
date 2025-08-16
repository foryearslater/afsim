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

#include "Radius.hpp"

#include <stdexcept>

#include "UtStringUtil.hpp"
#include "UtUnitTypes.hpp"

namespace usmtf
{
Radius::Radius()
   : mRadiusDouble{0}
{
   SetValidRange(0, 99999);
}

bool Radius::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidRadius();
}

const std::string& Radius::GetRadiusValue() const noexcept
{
   return mRadiusValue;
}

const std::string& Radius::GetUnitOfMeasure() const noexcept
{
   return mUnitOfMeasure;
}

const double Radius::GetRadiusInMeters(double aRadius) const noexcept
{
   if (!IsValid())
   {
      return 0;
   }
   if (mUnitOfMeasure == "NM")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cNAUTICAL_MILES);
   }
   else if (mUnitOfMeasure == "KM")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cKILOMETERS);
   }
   else if (mUnitOfMeasure == "CM")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cCENTIMETERS);
   }
   else if (mUnitOfMeasure == "FT")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cFEET);
   }
   else if (mUnitOfMeasure == "HM")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cMETERS) * 100;
   }
   else if (mUnitOfMeasure == "HF")
   {
      // hundreds of feet
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cFEET) * 100;
   }
   else if (mUnitOfMeasure == "IN")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cINCHES);
   }
   else if (mUnitOfMeasure == "KF")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cKILOFEET);
   }
   else if (mUnitOfMeasure == "MM")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cMILLIMETERS);
   }
   else if (mUnitOfMeasure == "SM")
   {
      // Statute miles
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cMILES);
   }
   else if (mUnitOfMeasure == "YD")
   {
      return UtUnitLength::ConvertToStandard(aRadius, UtUnitLength::cFEET) * 3;
   }
   return mRadiusDouble; // meters
}

const double Radius::GetRadiusInMeters() const noexcept
{
   return GetRadiusInMeters(mRadiusDouble);
}

const double Radius::GetRadius() const noexcept
{
   return mRadiusDouble;
}

bool Radius::IsValidRadius() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsInRange<size_t>(aFieldContent.length(), 1, 7))
   {
      AddError("Radius Failed Length Check", aFieldContent, "Radius must be between [1,7] chars.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent,
                                          {
                                             "NM",
                                             "KM",
                                             "YD",
                                             "CM",
                                             "FT",
                                             "HM",
                                             "HF",
                                             "IN",
                                             "KF",
                                             "MM",
                                             "SM",
                                             "M",
                                          });
   if (splitPos == std::string::npos)
   {
      AddError("Radius Failed Parsing",
               aFieldContent,
               "Radius must contain a delineation string of NM, KM, CM, FT, HM, HF, IN, KF, MM, SM, YD or M ");
      return false;
   }

   std::string radius = aFieldContent.substr(0, splitPos);
   if (!RangeValidator::IsInvalidRange(radius))
   {
      return false;
   }

   std::string uom = aFieldContent.substr(splitPos);
   if (!IsValidUnitOfMeasureForRadius(uom))
   {
      return false;
   }

   mRadiusDouble  = mCastedRangeValue;
   mRadiusValue   = std::to_string(mRadiusDouble);
   mUnitOfMeasure = uom;

   return true;
}

bool Radius::IsValidUnitOfMeasureForRadius(const std::string& aUom) noexcept
{
   if (IsInSelection(aUom, {"NM", "KM", "YD", "CM", "FT", "HM", "HF", "IN", "KF", "MM", "SM", "M"}))
   {
      return true;
   }
   AddError("Radius Unit of Measure is Invalid.",
            aUom,
            "Unit of Measure must be NM, KM, CM, FT, HM, HF, IN, KF, MM, SM, YD or M");
   return false;
}
}; // namespace usmtf
