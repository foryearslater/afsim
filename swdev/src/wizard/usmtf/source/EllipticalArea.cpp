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


#include "EllipticalArea.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace usmtf
{

bool EllipticalArea::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return IsValidEllipsesArea();
}
const std::string& EllipticalArea::GetHorizontalAxisDistance() const noexcept
{
   return mHorizontalAxisDistance;
}
const std::string& EllipticalArea::GetHorizontalAxisUOM() const noexcept
{
   return mHorizontalAxisUOM;
}
const std::string& EllipticalArea::GetVerticalAxisDistance() const noexcept
{
   return mVerticalAxisDistance;
}
const std::string& EllipticalArea::GetVerticalAxisUOM() const noexcept
{
   return mVerticalAxisUOM;
}
float EllipticalArea::GetAngleFromTrueNorth() const noexcept
{
   return mAngleInDegrees;
}

bool EllipticalArea::IsValidEllipsesArea()
{
   const std::string        fieldContent = mField.GetContent();
   std::vector<std::string> split        = SplitOnDelimeter(fieldContent, "-");
   if (split.empty())
   {
      AddError("Elliptical Area has failed parsing.", fieldContent, "Must include a - for delineation of parts.");
      return false;
   }
   try
   {
      std::string horizontalAxis = split.at(0);
      std::string verticalAxis   = split.at(1);
      std::string degree         = split.at(2);

      bool haIsValid     = IsValidAxis(horizontalAxis, Axis::Horizontal);
      bool vaIsValid     = IsValidAxis(verticalAxis, Axis::Vertical);
      bool degreeIsValid = IsValidDegree(degree);
      if (!haIsValid || !vaIsValid || !degreeIsValid)
      {
         mAngleInDegrees         = 0;
         mHorizontalAxisDistance = std::string();
         mHorizontalAxisUOM      = std::string();
         mVerticalAxisDistance   = std::string();
         mVerticalAxisUOM        = std::string();
         return false;
      }
      return true;
   }
   catch (const std::out_of_range&)
   {
      AddError("Elliptical Area has failed parsing.",
               fieldContent,
               "Must have 3 parts HorizontalAxis - VerticalAxis - Degree");
      return false;
   }
}

bool EllipticalArea::IsValidAxis(const std::string& aAxis, Axis axis)
{
   std::vector<std::string> acceptableUnitsOfMesure{"CM", "FT", "HM", "HF", "IN", "KF", "KM", "MM", "NM", "SM", "YD", "M"};
   std::size_t              position = CheckForAnyDelimeter(aAxis, acceptableUnitsOfMesure);
   if (position == std::string::npos)
   {
      AddError("Elliptical Area has failed parsing.",
               aAxis,
               "Axis Must contain a valid unit of measurement: CM, FT, HM,"
               "HF, IN, KF, M, MM, NM, SM, YD, M");
      return false;
   }
   std::string distance = aAxis.substr(0, position);
   std::string uom      = aAxis.substr(position);
   try
   {
      float distancef = std::stof(distance);
      if (IsInRange<float>(distancef, 0, 99999))
      {
         switch (axis)
         {
         case Axis::Horizontal:
            mHorizontalAxisDistance = distance;
            mHorizontalAxisUOM      = uom;
            break;
         case Axis::Vertical:
            mVerticalAxisDistance = distance;
            mVerticalAxisUOM      = uom;
            break;
         }
         return true;
      }
      AddError("Axis Distance is Invalid", distance, "Axis Distance must fall in range of [0, 99999]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Axis Distance Failed Conversion", distance, "Axis Distance must be convertible to float.");
      return false;
   }
}

bool EllipticalArea::IsValidDegree(const std::string& aDegree)
{
   try
   {
      auto degree = std::stof(aDegree);
      if (IsInRange<float>(degree, 0, 360))
      {
         mAngleInDegrees = degree;
         return true;
      }
      AddError("Degree Offset is Invalid", aDegree, "Degree Offset must fall in range of [0, 360.0]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Degree Offset Failed Conversion", aDegree, "Degree Offset must be convertible to float.");
      return false;
   }
}
} // namespace usmtf
