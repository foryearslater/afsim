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


#include "VerifiedLatLonS.hpp"

#include <sstream>

namespace usmtf
{
VerifiedLatLonS::VerifiedLatLonS()
{
   std::string expr = cStart + c00to90 + c00to59 + c00to59 + cNorS + c0to9 + cHyphenDelimeter + c000to180 + c00to59 +
                      c00to59 + cEorW + c0to9 + cEnd;
   SetRegexExpression(expr);
   auto err = R"(

            FORMAT EXPRESSED AS FOLLOWS :

            LATITUDE, DEGREES : 2 N
            ALLOWABLE ENTRIES : (00 THROUGH 90).

            LATITUDE MINUTE(ANGULAR): 2 N
            ALLOWABLE ENTRIES : (00 THROUGH 59).

            LATITUDE SECOND(ANGULAR): 2 N
            ALLOWABLE ENTRIES : (00 THROUGH 59).

            LATITUDINAL HEMISPHERE: 1 A
            ALLOWABLE ENTRIES : N or S.

            CONTEXT QUANTITY, 0 - 9: 1 N
            ALLOWABLE ENTRIES: (0 THROUGH 9).

            HYPHEN : 1 S
            ALLOWABLE ENTRIES : -.
            A CHARACTER THAT SEPARATES DATA ELEMENTS.

            LONGITUDE, DEGREES : 3 N
            ALLOWABLE ENTRIES : (000 THROUGH 180).

            LONGITUDE MINUTE(ANGULAR): 2 N
            ALLOWABLE ENTRIES : (00 THROUGH 59).

            LONGITUDE SECOND(ANGULAR): 2 N
            ALLOWABLE ENTRIES : (00 THROUGH 59).

            LONGITUDINAL HEMISPHERE: 1 A
            ALLOWABLE ENTRIES : E or W.

            CONTEXT QUANTITY, 0 - 9: 1 N
            ALLOWABLE ENTRIES: (0 THROUGH 9).

            EXAMPLE : /351025N6 - 0790125W4 )";
   SetErrorMessage(err);
}

bool VerifiedLatLonS::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   // Parse knowing its valid. Rules express in error template.
   mLattitudinalDegree = mMatchResults.captured(1).toStdString();
   mLatMinutes         = mMatchResults.captured(2).toStdString();
   mLatSeconds         = mMatchResults.captured(3).toStdString();
   mLatHemisphere      = mMatchResults.captured(4).toStdString();
   mLongitudinalDegree = mMatchResults.captured(7).toStdString();
   mLonMinutes         = mMatchResults.captured(8).toStdString();
   mLonSeconds         = mMatchResults.captured(9).toStdString();
   mLonHemisphere      = mMatchResults.captured(10).toStdString();
   SetUtLLPos();
   return true;
}

const std::string& VerifiedLatLonS::GetLatDegree() const noexcept
{
   return mLattitudinalDegree;
}

const std::string& VerifiedLatLonS::GetLonDegree() const noexcept
{
   return mLongitudinalDegree;
}

const std::string& VerifiedLatLonS::GetLatSeconds() const noexcept
{
   return mLatSeconds;
}

const std::string& VerifiedLatLonS::GetLonSeconds() const noexcept
{
   return mLonSeconds;
}

const std::string& VerifiedLatLonS::GetLatMinutes() const noexcept
{
   return mLatMinutes;
}

const std::string& VerifiedLatLonS::GetLonMinutes() const noexcept
{
   return mLonMinutes;
}

const std::string& VerifiedLatLonS::GetLatHemisphere() const noexcept
{
   return mLatHemisphere;
}

const std::string& VerifiedLatLonS::GetLonHemisphere() const noexcept
{
   return mLonHemisphere;
}

const UtLLPos& VerifiedLatLonS::GetUtLLPos() const noexcept
{
   return mUtLLPos;
}

std::string VerifiedLatLonS::GetFormattedLatLon() const noexcept
{
   std::stringstream ss{};
   ss << GetUtLLPos();
   return ss.str();
}

std::string VerifiedLatLonS::GetFormattedLatLon(const UtLLPos& position) noexcept
{
   std::stringstream ss{};
   ss << position;
   return ss.str();
}

void VerifiedLatLonS::SetUtLLPos() noexcept
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
} // namespace usmtf
