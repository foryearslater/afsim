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


#include "VerifiedLatLonTM.hpp"

#include <math.h>
#include <sstream>

namespace usmtf
{
VerifiedLatLonTM::VerifiedLatLonTM()
   : mLatSecondsFraction{0}
   , mLonSecondsFraction{0}
{
   std::string expr = cStart + c00to90 + c00to59dec000to999 + cNorS + c0to9 + cHyphenDelimeter + c000to180 +
                      c00to59dec000to999 + cEorW + c0to9 + cEnd;
   SetRegexExpression(expr);
   auto err = R"(

            LATITUDE, DEGREES
      2 N
      ALLOWABLE ENTRIES: (00 THROUGH 90).

      LATITUDE MINUTE (ANGULAR)
      2 N
      ALLOWABLE ENTRIES: (00 THROUGH 59).

      DECIMAL
      1 S
      ALLOWABLE ENTRIES: . A CHARACTER THAT IS USED TO DENOTE A DECIMAL NUMERIC VALUE.

      THOUSANDTH OF MINUTE OF LATITUDE
      3 N
      ALLOWABLE ENTRIES: (000 THROUGH 999).

      LATITUDINAL HEMISPHERE
      1 A
      USE TABLE A FOR ALLOWABLE ENTRIES

      CONTEXT QUANTITY, 0-9
      1 N
      ALLOWABLE ENTRIES: (0 THROUGH 9).

      HYPHEN
      1 S
      ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

      LONGITUDE, DEGREES
      3 N
      ALLOWABLE ENTRIES: (000 THROUGH 180).

      LONGITUDE MINUTE (ANGULAR)
      2 N
      ALLOWABLE ENTRIES: (00 THROUGH 59).

      DECIMAL
      1 S
      ALLOWABLE ENTRIES: . A CHARACTER THAT IS USED TO DENOTE A DECIMAL NUMERIC VALUE.

      THOUSANDTH OF MINUTE OF LONGITUDE
      3 N
      ALLOWABLE ENTRIES: (000 THROUGH 999).

      LONGITUDINAL HEMISPHERE
      1 A
      USE TABLE B FOR ALLOWABLE ENTRIES

      CONTEXT QUANTITY, 0-9
      1 N
      ALLOWABLE ENTRIES: (0 THROUGH 9).

      EXAMPLE: /3510.234N8-07901.123W3)";
   SetErrorMessage(err);
}

bool VerifiedLatLonTM::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false;
   }

   mLattitudinalDegree           = mMatchResults.captured(1).toStdString();
   mLatMinutes                   = mMatchResults.captured(2).toStdString();
   std::string latMinuteFraction = mMatchResults.captured(3).toStdString() + mMatchResults.captured(4).toStdString();
   mLatSeconds                   = CalculateSeconds(latMinuteFraction, mLatSecondsFraction);
   mLatHemisphere                = mMatchResults.captured(5).toStdString();
   mLongitudinalDegree           = mMatchResults.captured(8).toStdString();
   mLonMinutes                   = mMatchResults.captured(9).toStdString();
   std::string lonMinuteFraction = mMatchResults.captured(10).toStdString() + mMatchResults.captured(11).toStdString();
   mLonSeconds                   = CalculateSeconds(lonMinuteFraction, mLonSecondsFraction);
   mLonHemisphere                = mMatchResults.captured(12).toStdString();
   SetUtLLPos();
   return true;
}

void VerifiedLatLonTM::SetUtLLPos() noexcept
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

   UtLatPos afsimLatPos(latDegree, latMinute, latSecond, mLatSecondsFraction); // here is the reason for override.

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


   UtLonPos afsimLonPos(lonDegree, lonMinute, lonSecond, mLonSecondsFraction); // here is the reason for override.
   mUtLLPos.Set(afsimLatPos, afsimLonPos);
}

std::string VerifiedLatLonTM::CalculateSeconds(std::string& aMinuteFraction, double& secondFraction) noexcept
{
   float             castedFraction;
   std::stringstream ss{aMinuteFraction};
   ss >> castedFraction;
   float secondsTotal = castedFraction * 60;
   float secondsWhole;
   secondFraction = std::modf(secondsTotal, &secondsWhole);
   return std::to_string(secondsTotal);
}
} // namespace usmtf
