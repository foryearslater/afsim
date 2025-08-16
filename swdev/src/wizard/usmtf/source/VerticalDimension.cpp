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

#include "VerticalDimension.hpp"

#include <stdexcept>

namespace usmtf
{
bool VerticalDimension::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   return ValidateByDescriptor(
      {{usmtf::ffirn_descriptors::cBRRA, [this]() -> bool { return IsValidVerticalDimensionBaseToRelativeAlt(); }},
       {usmtf::ffirn_descriptors::cBRFL, [this]() -> bool { return IsValidVerticalDimensionBaseToFlightLevel(); }},
       {usmtf::ffirn_descriptors::cRAFL, [this]() -> bool { return IsValidVerticalDimensionRelativeAltToFlightLevel(); }},
       {usmtf::ffirn_descriptors::cRARA, [this]() -> bool { return IsValidVerticalDimensionRelativeAltToRelativeAlt(); }},
       {usmtf::ffirn_descriptors::cFLFL, [this]() -> bool { return IsValidVerticalDimensionFlightLevelToFlightLevel(); }}});
}

const std::string& VerticalDimension::GetBaseReference() const noexcept
{
   return mBaseReference;
}

const std::string& VerticalDimension::GetMinAltitude() const noexcept
{
   return mMinAltitude;
}

const std::string& VerticalDimension::GetAltitudeUnitOfMeasure() const noexcept
{
   return mAltitudeMeasurement;
}

const std::string& VerticalDimension::GetFlightIndicator() const noexcept
{
   return mFlightIndicator;
}

const std::string& VerticalDimension::GetMaxAltitude() const noexcept
{
   return mMaxAltitude;
}

int VerticalDimension::GetNumericalMinAltitude() const noexcept
{
   try
   {
      auto numMinAlt = std::stoi(GetMinAltitude());
      return numMinAlt * 100;
   }
   catch (std::exception&)
   {
      return 0;
   }
}

int VerticalDimension::GetNumericalMaxAltitude() const noexcept
{
   try
   {
      auto numMaxAlt = std::stoi(GetMaxAltitude());
      return numMaxAlt * 100;
   }
   catch (std::exception&)
   {
      return 0;
   }
}

bool VerticalDimension::IsValidVerticalDimensionBaseToFlightLevel() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsInRange<size_t>(aFieldContent.length(), 8, 9))
   {
      AddError("Base to Flight Level Dimension Failed Length Check",
               aFieldContent,
               "Base to Flight Level Dimension must be either 8 or 9 chars.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Base to Flight Level Dimension Failed Parsing",
               aFieldContent,
               "Base to Flight Level Dimension have a - to delineate parts.");
      return false;
   }

   std::string baseRef = aFieldContent.substr(0, splitPos);
   if (!IsValidAltitudeBaseReferencePoint(baseRef))
   {
      return false;
   }
   mBaseReference = baseRef;

   std::string flightIndicator = aFieldContent.substr(splitPos + 1, 2); // skip dash
   if (!IsValidFlightLevelIndicator(flightIndicator))
   {
      return false;
   }
   mFlightIndicator = flightIndicator;

   std::string maxAltitude = aFieldContent.substr(splitPos + 3); // skip flight indicator
   if (!IsValidRelativeAltitude(maxAltitude))
   {
      return false;
   }
   mMaxAltitude = maxAltitude;

   return true;
}

bool VerticalDimension::IsValidVerticalDimensionBaseToRelativeAlt() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsInRange<size_t>(aFieldContent.length(), 9, 11))
   {
      AddError("Base to Relative Alt Vertical Dimension Failed Length Check",
               aFieldContent,
               "Must be 9, 10 or 11 chars long.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Base to Relative Alt Vertical Dimension Failed Parsing",
               aFieldContent,
               "Must include a - for delineation of parts.");
      return false;
   }

   std::string baseRef = aFieldContent.substr(0, splitPos);
   if (!IsValidAltitudeBaseReferencePoint(baseRef))
   {
      return false;
   }
   mBaseReference = baseRef;

   std::string maxAltitude = aFieldContent.substr(splitPos + 1, 3); // skip dash
   if (!IsValidRelativeAltitude(maxAltitude))
   {
      return false;
   }
   mMaxAltitude = maxAltitude;

   std::string altitudeMeasurement = aFieldContent.substr(splitPos + 4); // skip maxAlt
   if (!IsValidAltitudeUOM(altitudeMeasurement))
   {
      return false;
   }
   mAltitudeMeasurement = altitudeMeasurement;

   return true;
}

// /RAFL:100AGL-FL230
bool VerticalDimension::IsValidVerticalDimensionRelativeAltToFlightLevel() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsInRange<size_t>(aFieldContent.length(), 12, 13))
   {
      AddError("Relative Alt to Flight Vertical Dimension Failed Length Check", aFieldContent, "Must be 12 or 13 chars long.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Relative Alt to Flight Level Vertical Dimension Failed Parsing",
               aFieldContent,
               "Must include a - for delineation of parts.");
      return false;
   }

   std::string minAltitude = aFieldContent.substr(0, 3);
   if (!IsValidRelativeAltitude(minAltitude))
   {
      return false;
   }
   mMinAltitude = minAltitude;

   // difference from total first data point as entry for second portion is dynamically sized of 3-4.
   std::string altitudeMeasurement = aFieldContent.substr(3, splitPos - 3);
   if (!IsValidAltitudeUOM(altitudeMeasurement))
   {
      return false;
   }
   mAltitudeMeasurement = altitudeMeasurement;

   std::string flightIndicator = aFieldContent.substr(splitPos + 1, 2); // skip dash, get flight indicator
   if (!IsValidFlightLevelIndicator(flightIndicator))
   {
      return false;
   }
   mFlightIndicator = flightIndicator;

   std::string maxAltitude = aFieldContent.substr(splitPos + 3); // skip flight indicator to end
   if (!IsValidRelativeAltitude(maxAltitude))
   {
      return false;
   }
   mMaxAltitude = maxAltitude;

   return true;
}

// /RARA:000AGL-020AGL
bool VerticalDimension::IsValidVerticalDimensionRelativeAltToRelativeAlt() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsInRange<size_t>(aFieldContent.length(), 13, 15))
   {
      AddError("Relative Alt to Relative Alt Vertical Dimension Failed Length Check",
               aFieldContent,
               "Must be 13, 14 or 15 chars long.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Relative Alt to Relative Alt Vertical Dimension Failed Parsing",
               aFieldContent,
               "Must include a - for delineation of parts.");
      return false;
   }

   std::string minAltitude = aFieldContent.substr(0, 3);
   if (!IsValidRelativeAltitude(minAltitude))
   {
      return false;
   }
   mMinAltitude = minAltitude;

   // difference from total first data point as entry for second portion is dynamically sized of 3-4.
   std::string altitudeMeasurement = aFieldContent.substr(3, splitPos - 3);
   if (!IsValidAltitudeUOM(altitudeMeasurement))
   {
      return false;
   }
   mAltitudeMeasurement = altitudeMeasurement;

   std::string maxAltitude = aFieldContent.substr(splitPos + 1, 3); // skip dash, get max altitude
   if (!IsValidRelativeAltitude(maxAltitude))
   {
      return false;
   }
   mMaxAltitude = maxAltitude;

   std::string altitudeUom = aFieldContent.substr(splitPos + 4); // skip max alt, get to end
   if (!IsValidAltitudeUOM(altitudeUom))
   {
      return false;
   }
   mAltitudeMeasurement = altitudeUom;

   return true;
}

// /FLFL:FL250-FL290
bool VerticalDimension::IsValidVerticalDimensionFlightLevelToFlightLevel() noexcept
{
   const std::string& aFieldContent = mField.GetContent();
   if (!IsStringLength(aFieldContent, 11))
   {
      AddError("Flight Level to Flight Level Vertical Dimension Failed Length Check",
               aFieldContent,
               "FLFL must be 11 chars long.");
      return false;
   }

   size_t splitPos = CheckForAnyDelimeter(aFieldContent, {"-"});
   if (splitPos == std::string::npos)
   {
      AddError("Flight Level to Flight Level Vertical Dimension Failed Parsing",
               aFieldContent,
               "FLFL must include a - for delineation of parts.");
      return false;
   }

   std::string firstFlightIndicator = aFieldContent.substr(0, 2);
   if (!IsValidFlightLevelIndicator(firstFlightIndicator))
   {
      return false;
   }
   mFlightIndicator = firstFlightIndicator;

   // I am unsure why its called context Quantity in USMTF, going with it.
   std::string firstContextQuantity = aFieldContent.substr(2, 3);
   if (!IsValidRelativeAltitude(firstContextQuantity))
   {
      return false;
   }
   mMinAltitude = firstContextQuantity;

   std::string secondFlightIndicator = aFieldContent.substr(splitPos + 1, 2); // skip dash, get flight indicator
   if (!IsValidFlightLevelIndicator(secondFlightIndicator))
   {
      return false;
   }
   mFlightIndicator += secondFlightIndicator;

   std::string secondContextQuantity = aFieldContent.substr(splitPos + 3); // skip flight indicator to end
   if (!IsValidRelativeAltitude(secondContextQuantity))
   {
      return false;
   }
   mMaxAltitude = secondContextQuantity;

   return true;
}

bool VerticalDimension::IsValidAltitudeBaseReferencePoint(const std::string& aValue) noexcept
{
   if (IsInSelection(aValue, {"GL", "MSL"}))
   {
      return true;
   }
   AddError("Altitude Base Reference Point is Invalid", aValue, " Altitude Base Reference Point must be GL or MSL.");
   return false;
}

bool VerticalDimension::IsValidRelativeAltitude(const std::string& aValue) noexcept
{
   try
   {
      auto altitude = std::stoi(aValue);
      if (IsInRange<int>(altitude, 0, 999))
      {
         return true;
      }
      AddError("Relative Altitude is Invalid", aValue, "Relative Altitude must fall in range of [0, 999]");
      return false;
   }
   catch (std::exception&)
   {
      AddError("Relative Altitude Failed Conversion", aValue, "Relative Altitude must be convertible to integer.");
      return false;
   }
}

bool VerticalDimension::IsValidAltitudeUOM(const std::string& aValue) noexcept
{
   if (IsInSelection(aValue, {"AGL", "AMSL"}))
   {
      return true;
   }
   AddError("Altitude Unit of Measure is invalid", aValue, "Altitude UOM must be AGL or AMSL");
   return false;
}

bool VerticalDimension::IsValidFlightLevelIndicator(const std::string& aValue) noexcept
{
   if (aValue == "FL")
   {
      return true;
   }
   AddError("Invalid Flight Level indicator", aValue, "Flight level indicator must be FL");
   return false;
}

} // namespace usmtf
