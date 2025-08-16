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

#ifndef FUELTYPE_HPP
#define FUELTYPE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of FuelType.
//! USMTF Excerpt:
/*!
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
(U)	GASOLINE, AVIATION, GRADE 100/130	(U)	F18	(U)	U.S. EQUIVALENT: 100/130 MIL SPEC, GRADE 100 GASOLINE LOW
LEAD (BLUE) (U)	TURBINE FUEL, AVIATION, HIGH FLASH TYPE WITH S748	(U)	F44	(U)	GRADE JP-5. S748 IS FUEL SYSTEM
ICING INHIBITOR. U.S. EQUIVALENT: JP-5 MIL SPEC, FREEZE POINT MINUS 46 DEGREES CENTIGRADE. (U)	TURBINE FUEL, AVIATION,
KEROSENE TYPE	(U)	F35	(U)	U.S. EQUIVALENT: JET A-1, KEROSENE, WITHOUT FS-II FUEL SYSTEM ICING INHIBITOR, FREEZE
POINT MINUS 47 DEGREES CENTIGRADE. (U)	TURBINE FUEL, AVIATION, KEROSENE TYPE WITH S748	(U)	F34	(U)	S748 IS FUEL
SYSTEM ICING INHIBITOR. U.S. EQUIVALENT: JET A-1, KEROSENE WITH FS-II FUEL SYSTEM ICING INHIBITOR, FREEZE POINT MINUS 47
DEGREES CENTIGRADE, AND JP-8 MIL SPEC. (U)	TURBINE FUEL, AVIATION, WIDE OUT TYPE, WITH S748	(U)	F40	(U)	GRADE
JP-4. S748 IS FUEL SYSTEM ICING INHIBITOR. U.S. EQUIVALENT: JP-4 MIL SPEC, FREEZE POINT MINUS 58 DEGREES CENTIGRADE.

EXAMPLE: /B:F34
*/
class USMTF_EXPORT FuelType : public RegexValidator
{
public:
   FuelType();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetType() const noexcept;

protected:
   std::string mType;
};
} // namespace usmtf
#endif
