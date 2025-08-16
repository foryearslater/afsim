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


#include "VerifiedLatLonDS.hpp"

#include <math.h>
#include <sstream>

namespace usmtf
{
VerifiedLatLonDS::VerifiedLatLonDS()
{
   std::string expr = cStart + c00to90 + c00to59 + c00to59dec0to9 + cNorS + c0to9 + cHyphenDelimeter + c000to180 +
                      c00to59 + c00to59dec0to9 + cEorW + c0to9 + cEnd;
   SetRegexExpression(expr);
   auto err = R"(

            EXPRESSED AS FOLLOWS:

            LATITUDE, DEGREES
            2 N
            ALLOWABLE ENTRIES: (00 THROUGH 90).

            LATITUDE MINUTE (ANGULAR)
            2 N
            ALLOWABLE ENTRIES: (00 THROUGH 59).

            LATITUDE SECOND (ANGULAR)
            2 N
            ALLOWABLE ENTRIES: (00 THROUGH 59).

            DECIMAL
            1 S
            ALLOWABLE ENTRIES: . A CHARACTER THAT IS USED TO DENOTE A DECIMAL NUMERIC VALUE.

            TENTH OF SECOND OF LATITUDE
            1 N
            ALLOWABLE ENTRIES: (0 THROUGH 9).

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

            LONGITUDE SECOND (ANGULAR)
            2 N
            ALLOWABLE ENTRIES: (00 THROUGH 59).

            DECIMAL
            1 S
            ALLOWABLE ENTRIES: . A CHARACTER THAT IS USED TO DENOTE A DECIMAL NUMERIC VALUE.

            TENTH OF SECOND OF LONGITUDE
            1 N
            ALLOWABLE ENTRIES: (0 THROUGH 9).

            LONGITUDINAL HEMISPHERE
            1 A
            USE TABLE B FOR ALLOWABLE ENTRIES

            CONTEXT QUANTITY, 0-9
            1 N
            ALLOWABLE ENTRIES: (0 THROUGH 9).

            EXAMPLE: /351025.3N9-0790125.7W1)";
   SetErrorMessage(err);
}

bool VerifiedLatLonDS::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false;
   }

   // Parse knowing its valid. Rules express in error template.
   mLattitudinalDegree = mMatchResults.captured(1).toStdString();
   mLatMinutes         = mMatchResults.captured(2).toStdString();
   mLatSeconds         = mMatchResults.captured(3).toStdString() + mMatchResults.captured(4).toStdString() +
                 mMatchResults.captured(5).toStdString();
   double latSeconds = 0;
   std::stringstream{mLatSeconds} >> latSeconds;
   double latSecondsWhole = 0;
   mLatSecondsFraction    = std::modf(latSeconds, &latSecondsWhole);
   mLatHemisphere         = mMatchResults.captured(6).toStdString();
   mLongitudinalDegree    = mMatchResults.captured(9).toStdString();
   mLonMinutes            = mMatchResults.captured(10).toStdString();
   mLonSeconds            = mMatchResults.captured(11).toStdString() + mMatchResults.captured(12).toStdString() +
                 mMatchResults.captured(13).toStdString();
   double lonSeconds = 0;
   std::stringstream{mLonSeconds} >> lonSeconds;
   double lonSecondsWhole = 0;
   mLonSecondsFraction    = std::modf(lonSeconds, &lonSecondsWhole);
   mLonHemisphere         = mMatchResults.captured(14).toStdString();
   SetUtLLPos();
   return true;
}
} // namespace usmtf
