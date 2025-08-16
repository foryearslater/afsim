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

#include "TacanChannel.hpp"

namespace usmtf
{
TacanChannel::TacanChannel()
{
   std::string expr = cStart + c1to999 + cHyphenDelimeter + c1to999 + cEnd;
   SetRegexExpression(expr);
   auto err = R"(
            THE TACAN CHANNEL OF THE RECEIVER AND TANKER AIRCRAFT INVOLVED IN AIR-TO-AIR REFUELING.

            EXPRESSED AS FOLLOWS:
            RECEIVER TACAN CHANNEL
            ALLOWABLE ENTRIES: (1 THROUGH 999).

            HYPHEN
            ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

            TANKER TACAN CHANNEL
            ALLOWABLE ENTRIES: (1 THROUGH 999).

            EXAMPLE: /16-72
)";
   SetErrorMessage(err);
}

bool TacanChannel::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mRecieverChannel = mMatchResults.captured(1).toStdString();
   mTankerChannel   = mMatchResults.captured(3).toStdString();
   return true;
}

const std::string& TacanChannel::GetRecieverChannel() const noexcept
{
   return mRecieverChannel;
}

const std::string& TacanChannel::GetTankerChannel() const noexcept
{
   return mTankerChannel;
}
} // namespace usmtf
