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

#include "AerialRefuelingSystem.hpp"

namespace usmtf
{
AerialRefuelingSystem::AerialRefuelingSystem()
{
   std::string expr = cStart + R"((BOM|CBD|CWT|BWD|CLD|CDT|WTD))" + cEnd;
   SetRegexExpression(expr);
   auto err = R"(
            Value Must from table below:

            TABLE 539A
            AERIAL REFUELING SYSTEM
            ORDER BY DINAME
            DATA ITEM
            CODE
            EXPLANATION
            BOOM		                                          BOM
            BOOM AND CENTERLINE DROGUE		                     CBD
            BOOM AND CENTERLINE DROGUE, AND WINGTIP DROGUE		CWT
            BOOM AND WINGTIP DROGUE		                        BWD
            CENTERLINE DROGUE	                                 CLD
            CENTERLINE DROGUE AND WINGTIP DROGUE		         CDT
            WINGTIP DROGUE	                                    WTD
)";
   SetErrorMessage(err);
}

bool AerialRefuelingSystem::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mSystem = mMatchResults.captured(1).toStdString();
   return true;
}

const std::string& AerialRefuelingSystem::GetSystem() const noexcept
{
   return mSystem;
}
} // namespace usmtf
