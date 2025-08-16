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


#include "RelativeBearingRange.hpp"

#include <sstream>

namespace usmtf
{
RelativeBearingRange::RelativeBearingRange()
{
   std::string expr = cStart + c000to359 + cHyphenDelimeter + cAny + cSpace + cAny + cHyphenDelimeter + c0to999 + cEnd;
   SetRegexExpression(expr);
   auto err = R"(

            EXPRESSED AS FOLLOWS:

            CONTEXT QUANTITY, 000-359
            3 N

            ALLOWABLE ENTRIES: (000 THROUGH 359).

            HYPHEN
            1 S

            ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

            REFERENCE
            1-12 ABNS

            ALLOWABLE ENTRIES: [LITERAL]. THE NAME OF A PERSON, PLACE OR THING. IF NAME IS MORE THAN THE ALLOWED CHARACTERS, USE A FREE-TEXT SET TO AMPLIFY THIS FIELD.

            HYPHEN
            1 S

            ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

            DISTANCE IN NAUTICAL MILES
            1-3 N

            ALLOWABLE ENTRIES: (0 THROUGH 999).

            EXAMPLE: /330-PT ALFA-50)";
   SetErrorMessage(err);
}

bool RelativeBearingRange::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mDegree   = mMatchResults.captured(1).toStdString();
   mFreeOne  = mMatchResults.captured(3).toStdString();
   mFreeTwo  = mMatchResults.captured(4).toStdString();
   mDistance = mMatchResults.captured(6).toStdString();
   return true;
}

const std::string& RelativeBearingRange::GetDegrees() const noexcept
{
   return mDegree;
}
const std::string& RelativeBearingRange::GetFreeTextOne() const noexcept
{
   return mFreeOne;
}
const std::string& RelativeBearingRange::GetFreeTextTwo() const noexcept
{
   return mFreeTwo;
}
const std::string& RelativeBearingRange::GetDistance() const noexcept
{
   return mDistance;
}
} // namespace usmtf
