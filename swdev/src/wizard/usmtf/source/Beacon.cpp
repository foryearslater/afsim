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

#include "Beacon.hpp"

namespace usmtf
{
Beacon::Beacon()
{
   std::string expr = cStart + R"(([0-6]))" + cHyphenDelimeter + R"(([0-6]))" + cHyphenDelimeter + R"(([0-6]))" + cEnd;
   SetRegexExpression(expr);
   auto err = R"(
            THE CODED SIGNAL TRANSMITTED AIR TO AIR FOR THE PURPOSE OF TANKER IDENTIFICATION DURING RENDEZVOUS.

            EXPRESSED AS FOLLOWS:

            CONTEXT QUANTITY, 0-6
            ALLOWABLE ENTRIES: (0 THROUGH 6).

            HYPHEN
            ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

            CONTEXT QUANTITY, 0-6
            ALLOWABLE ENTRIES: (0 THROUGH 6).

            HYPHEN
            ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

            CONTEXT QUANTITY, 0-6
            ALLOWABLE ENTRIES: (0 THROUGH 6).

            EXAMPLE: /4-3-1
)";
   SetErrorMessage(err);
}

bool Beacon::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mFirst  = mMatchResults.captured(1).toStdString();
   mSecond = mMatchResults.captured(3).toStdString();
   mThird  = mMatchResults.captured(5).toStdString();
   return true;
}

const std::string& Beacon::GetFirst() const noexcept
{
   return mFirst;
}

const std::string& Beacon::GetSecond() const noexcept
{
   return mSecond;
}

const std::string& Beacon::GetThird() const noexcept
{
   return mThird;
}
} // namespace usmtf
