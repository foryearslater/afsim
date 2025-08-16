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

#ifndef UTMLOCATION_HPP
#define UTMLOCATION_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "Field.hpp"
#include "RegexValidator.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a UTM Location.


//! Table TABLE 330,12,542  LOCATION, MGRS 1-METER, 10-METER, 100-METER
//! 330.UF.html
/*!
   UTM (Universal Transverse Mercator) coordinate system is basically geographical latitude longitude
   system that is expressed in two-dimensional projection of the surface of earth where the earth map
   is divided into 60 zones, with each of them separated by 6 degrees in longitude and the locations
   are expressed in terms of so called easting and northing, i.e. Easting 380749.6, Northing 4928286.8
   This system is most commonly used for military purposes.

   Easting: the x coordinate
   Northing: the y coordinate


   EXPRESSED AS FOLLOWS:

   UTM GRID ZONE COLUMN
   2 N

   ALLOWABLE ENTRIES: (01 THROUGH 60).


   UTM GRID ZONE ROW
   1 A

   ALLOWABLE ENTRIES: (C, D THROUGH X) [OMIT I, O]. NEITHER I NOR O OCCURS.

   100,000-METER SQUARE COLUMN
   1 A

   ALLOWABLE ENTRIES: (A, B THROUGH Z) [OMIT I, O].

   100,000-METER SQUARE ROW
   1 A

   ALLOWABLE ENTRIES: (A, B THROUGH V) [OMIT I, O].

   UTM 1-METER EASTING
   5 N


   ALLOWABLE ENTRIES: (00000 THROUGH 99999).

   UTM 1-METER NORTHING
   5 N

   ALLOWABLE ENTRIES: (00000 THROUGH 99999).

   EXAMPLE: /18RTV2345678921

*/
//! USMTF Excerpt:
//! /UTMT:32WDL12341234
//! /UTMO:32WDL123123
//! /UTM:32WDL1234512345

class USMTF_EXPORT UTM_Location : public RegexValidator
{
public:
   UTM_Location();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetZoneColumn() const noexcept;
   const std::string& GetZoneRow() const noexcept;
   const std::string& GetSquareColumn() const noexcept;
   const std::string& GetSquareRow() const noexcept;
   const std::string& GetEasting() const noexcept;
   const std::string& GetNorthing() const noexcept;
   int                GetEastingInMeters() const noexcept;
   int                GetNorthingInMeters() const noexcept;
   const std::string  GetFormattedLocation() const noexcept;

protected:
   UtLLPos     ConvertToLLPos() const noexcept;
   std::string mZoneColumn;
   std::string mZoneRow;
   std::string mSquareColumn;
   std::string mSquareRow;
   std::string mEasting;
   std::string mNorthing;
};
} // namespace usmtf
#endif
