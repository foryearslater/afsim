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

#include "FuelType.hpp"

namespace usmtf
{
FuelType::FuelType()
{
   std::string expr = cStart + R"((JP4|JP5|JP5|JP7|JP8|F18|F44|F35|F34|F40))" + cEnd;
   SetRegexExpression(expr);
   auto err = R"(
   TABLE A

   REFUELING FUEL TYPE
   CODE

   EXPLANATION
   (U)	JP-4 FUEL	(U)	JP4
   (U)	JP-5 FUEL	(U)	JP5
   (U)	JP-7 FUEL	(U)	JP7
   (U)	JP-8 FUEL	(U)	JP8

   EXAMPLE: /A:JP8


   TABLE B

   NATO STANDARD AIRCRAFT FUEL TYPES

   CODE

   EXPLANATION
   (U)	GASOLINE, AVIATION, GRADE 100/130	(U)	               F18	
   (U)	TURBINE FUEL, AVIATION, HIGH FLASH TYPE WITH S748	(U)	F44	
   (U)	TURBINE FUEL, AVIATION, KEROSENE TYPE	(U)	            F35	
   (U)	TURBINE FUEL, AVIATION, KEROSENE TYPE WITH S748	(U)	   F34	
   (U)	TURBINE FUEL, AVIATION, WIDE OUT TYPE, WITH S748	(U)	F40	
)";
   SetErrorMessage(err);
}

bool FuelType::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mType = mMatchResults.captured(1).toStdString();
   return true;
}

const std::string& FuelType::GetType() const noexcept
{
   return mType;
}
} // namespace usmtf
