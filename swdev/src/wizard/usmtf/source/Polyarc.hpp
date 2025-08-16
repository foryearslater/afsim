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

#ifndef POLYARC_HPP
#define POLYARC_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Polygon.hpp"
#include "Radarc.hpp"
#include "RadialBearing.hpp"
#include "Radius.hpp"
#include "Set.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{
//! A POLYARC is a complex shape, consisting of the idea of a RADARC and polygon nested in the Set.
//!   ____
//!  /     |_________
//! /      |         |
//! \      |_________|
//!  \_____|
//!
//!  ^^^^^   ^^^^^^^^^
//!  |||||   |||||||||
//!  RADARC   POLYGON
//! (Best I could do with Ascii art)

//! USMTF Example:
//! POLYARC/LATM:1510N05901E/330T/30NM/160T  <-- RADARC Sections
//!        /LATS:203632N0594256E / LATS:155000N0594815E / LATS : 155000N0591343E// <-- POLYGON section
/*!
 Standard Excerpt:
 POLYARC/                    /                                 .
       1 ORIGIN OF BEARING  2 BEGINNING RADIAL BEARING, TRUE
       /                                                  /                              .
       3 RADIUS IN NAUTICAL MILES, METERS, OR KILOMETERS  4 ENDING RADIAL BEARING, TRUE
       /          R         //
       5 POSITION OR POINT

 Detailed Document:
 POLYARC.UF.RTF
*/
class USMTF_EXPORT Polyarc : public Set
{
public:
   Polyarc(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const Radarc*  GetRadarc() const noexcept;
   const Polygon* GetPolygon() const noexcept;

private:
   bool IsRadarcAndPolygonValid();
   // We are using unique ptr here to defer instantiation .
   std::unique_ptr<Radarc>  mRadarc;
   std::unique_ptr<Polygon> mPolygon;
};
} // namespace usmtf
#endif
