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

#include "LatLon.hpp"

#include <sstream>
#include <stdexcept>

#include "UtLatPos.hpp"
#include "UtLonPos.hpp"

namespace usmtf
{
bool LatLon::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   bool isValid = ValidateByDescriptor({{ffirn_descriptors::cLATM, [this]() -> bool { return IsValidLatLonMinutes(); }},
                                        {ffirn_descriptors::cLATS, [this]() -> bool { return IsValidLatLonSeconds(); }}});

   if (isValid)
   {
      SetUtLLPos();
   }
   return isValid;
}

const std::string& LatLon::GetLatDegree() const noexcept
{
   return mLattitudinalDegree;
}

const std::string& LatLon::GetLonDegree() const noexcept
{
   return mLongitudinalDegree;
}

const std::string& LatLon::GetLatSeconds() const noexcept
{
   return mLatSeconds;
}

const std::string& LatLon::GetLonSeconds() const noexcept
{
   return mLonSeconds;
}

const std::string& LatLon::GetLatMinutes() const noexcept
{
   return mLatMinutes;
}

const std::string& LatLon::GetLonMinutes() const noexcept
{
   return mLonMinutes;
}

const std::string& LatLon::GetLatHemisphere() const noexcept
{
   return mLatHemisphere;
}

const std::string& LatLon::GetLonHemisphere() const noexcept
{
   return mLonHemisphere;
}

const UtLLPos& LatLon::GetUtLLPos() const noexcept
{
   return mUtLLPos;
}

std::string LatLon::GetFormattedLatLon() const noexcept
{
   std::stringstream ss{};
   ss << GetUtLLPos();
   return ss.str();
}

std::string LatLon::GetFormattedLatLon(const UtLLPos& position) noexcept
{
   std::stringstream ss{};
   ss << position;
   return ss.str();
}

void LatLon::SetUtLLPos() noexcept
{
   int latDegree;
   int latMinute;
   int latSecond;

   latDegree = std::stoi(mLattitudinalDegree);
   latMinute = std::stoi(mLatMinutes);
   latSecond = std::stoi(mLatSeconds);
   if (mLatHemisphere == "S")
   {
      latDegree = latDegree * -1;
      latMinute = latMinute * -1;
      latSecond = latSecond * -1;
   }

   UtLatPos afsimLatPos(latDegree, latMinute, latSecond);

   int lonDegree;
   int lonMinute;
   int lonSecond;

   lonDegree = std::stoi(mLongitudinalDegree);
   lonMinute = std::stoi(mLonMinutes);
   lonSecond = std::stoi(mLonSeconds);
   if (mLonHemisphere == "W")
   {
      lonDegree = lonDegree * -1;
      lonMinute = lonMinute * -1;
      lonSecond = lonSecond * -1;
   }


   UtLonPos afsimLonPos(lonDegree, lonMinute, lonSecond);
   mUtLLPos.Set(afsimLatPos, afsimLonPos);
}

bool LatLon::IsValidLatLonMinutes() noexcept
{
   const std::string& aFieldContents = mField.GetContent();
   if (!IsStringLength(aFieldContents, 11))
   {
      AddError("Failed String Length Check",
               aFieldContents,
               "A Valid USMTF LATM is 11 chars long. Field was " + std::to_string(aFieldContents.length()));
      return false;
   }

   // break string into components specified by USMTF latM Format.
   std::string latDegrees = aFieldContents.substr(0, 2);
   if (!IsValidLattitudinalDegree(latDegrees))
   {
      return false;
   }

   std::string latMinutes = aFieldContents.substr(2, 2);
   if (!IsValidMinuteOrSecond(latMinutes))
   {
      return false;
   }

   std::string latHemisphere = aFieldContents.substr(4, 1);
   if (!IsValidLattitudinalHemisphere(latHemisphere))
   {
      return false;
   }

   std::string lonDegrees = aFieldContents.substr(5, 3);
   if (!IsValidLongitudinalDegree(lonDegrees))
   {
      return false;
   }

   std::string lonMinute = aFieldContents.substr(8, 2);
   if (!IsValidMinuteOrSecond(lonMinute))
   {
      return false;
   }

   std::string lonHemisphere = aFieldContents.substr(10, 1);
   if (!IsValidLongitudinalHemisphere(lonHemisphere))
   {
      return false;
   }

   // These are set at the end so that any invalid state returns no properties uniformly.
   mLattitudinalDegree = latDegrees;
   mLatMinutes         = latMinutes;
   mLatSeconds         = "0";
   mLatHemisphere      = latHemisphere;
   mLongitudinalDegree = lonDegrees;
   mLonMinutes         = lonMinute;
   mLonSeconds         = "0";
   mLonHemisphere      = lonHemisphere;

   return true;
}

bool LatLon::IsValidLatLonSeconds() noexcept
{
   const std::string& aFieldContents = mField.GetContent();

   if (!IsStringLength(aFieldContents, 15))
   {
      AddError("Failed String Length Check",
               aFieldContents,
               "A Valid USMTF LATS is 15 chars long. Field was " + std::to_string(aFieldContents.length()));
      return false;
   }

   // break string into components specified by USMTF latS Format.
   std::string latDegrees = aFieldContents.substr(0, 2);
   if (!IsValidLattitudinalDegree(latDegrees))
   {
      return false;
   }

   std::string latMinutes = aFieldContents.substr(2, 2);
   if (!IsValidMinuteOrSecond(latMinutes))
   {
      return false;
   }

   std::string latSeconds = aFieldContents.substr(4, 2);
   if (!IsValidMinuteOrSecond(latSeconds))
   {
      return false;
   }

   std::string latHemisphere = aFieldContents.substr(6, 1);
   if (!IsValidLattitudinalHemisphere(latHemisphere))
   {
      return false;
   }

   std::string lonDegrees = aFieldContents.substr(7, 3);
   if (!IsValidLongitudinalDegree(lonDegrees))
   {
      return false;
   }

   std::string lonMinutes = aFieldContents.substr(10, 2);
   if (!IsValidMinuteOrSecond(lonMinutes))
   {
      return false;
   }

   std::string lonSeconds = aFieldContents.substr(12, 2);
   if (!IsValidMinuteOrSecond(lonSeconds))
   {
      return false;
   }

   std::string lonHemisphere = aFieldContents.substr(14, 1);
   if (!IsValidLongitudinalHemisphere(lonHemisphere))
   {
      return false;
   }

   // These are set at the end so that any invalid state returns no properties uniformly.
   mLattitudinalDegree = latDegrees;
   mLatMinutes         = latMinutes;
   mLatSeconds         = latSeconds;
   mLatHemisphere      = latHemisphere;
   mLongitudinalDegree = lonDegrees;
   mLonMinutes         = lonMinutes;
   mLonSeconds         = lonSeconds;
   mLonHemisphere      = lonHemisphere;

   return true;
}

bool LatLon::IsValidLattitudinalDegree(const std::string& aFieldContent) noexcept
{
   try
   {
      if (IsInRange<int>(std::stoi(aFieldContent), 0, 90))
      {
         return true;
      }
      AddError("Latitudinal Degree Failed Range Check", aFieldContent, "Lattitudinal Degree must fall in [0, 90]");
      return false;
   }
   catch (std::exception&) // stoi conversion failed
   {
      AddError("Lattitudinal Degree failed conversion to integer.",
               aFieldContent,
               "Lattitudinal Degree must be an integer, please fix.");
      return false;
   }
}

bool LatLon::IsValidLongitudinalDegree(const std::string& aFieldContent) noexcept
{
   try
   {
      if (!IsInRange<int>(std::stoi(aFieldContent), 0, 180))
      {
         AddError("Longitudinal Degree Failed Range Check", aFieldContent, "Longitudinal Degree must fall in [0, 180]");
         return false;
      }

      return true;
   }
   catch (std::invalid_argument&) // stoi conversion failed
   {
      AddError("Longitudinal Degree failed conversion to integer.",
               aFieldContent,
               "Longitudinal Degree must be an integer, please fix.");
      return false;
   }
   catch (std::out_of_range&) // stoi conversion failed
   {
      AddError("Longitudinal Degree failed conversion to integer.",
               aFieldContent,
               "Longitudinal Degree must fit inside of an integer, please fix.");
      return false;
   }
}

bool LatLon::IsValidMinuteOrSecond(const std::string& aFieldContent) noexcept
{
   try
   {
      int atime = std::stoi(aFieldContent);
      if (!IsInRange<int>(atime, 0, 59))
      {
         AddError("Value Failed Range Check", aFieldContent, "Value must fall in [0, 59]");
         return false;
      }

      return true;
   }
   catch (std::exception&) // stoi conversion failed
   {
      AddError("Value failed conversion to integer.", aFieldContent, "Value must be an integer, please fix.");
      return false;
   }
}

bool LatLon::IsValidLattitudinalHemisphere(const std::string& aFieldContent) noexcept
{
   if (aFieldContent != "N" && aFieldContent != "S")
   {
      AddError("Value failed Lattitudinal Hemisphere Check", aFieldContent, "Value must be either N or S");
      return false;
   }

   return true;
}

bool LatLon::IsValidLongitudinalHemisphere(const std::string& aFieldContent) noexcept
{
   if (aFieldContent != "E" && aFieldContent != "W")
   {
      AddError("Value failed Longitudinal Hemisphere Check", aFieldContent, "Value must be either E or W");
      return false;
   }
   return true;
}
}; // namespace usmtf
