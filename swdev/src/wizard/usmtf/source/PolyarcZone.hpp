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

#ifndef POLYARCZONE_HPP
#define POLYARCZONE_HPP

#include "Polyarc.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! Transforms a Segment identifying an Polyarc Zone into an AFSIM zone_set script block
//! Please see Polyarc.hpp for a description of the shape this creates.

//!   Example Segment:
//!   ACMID/ACM:ATC/DESIG:POLYARC TEST/POLYARC/USE:WARN//
//!   POLYARC / LATM:2006N05912E / 330T / 30NM / 160T / LATS : 203632N0594256E
//!   / LATS : 155000N0594815E / LATS : 155000N0591343E//
//!   EFFLEVEL / FLFL : FL210 - FL250//
//!   APERIOD / DISCRETE / 270001ZJAN / 280001ZJAN//
//!   CNTRLPT / EP / ORANGE / LATM : 2036N05943E / FLFL : FL220 - FL220//
//!   CNTRLPT / EP / APPLE / LATM : 1550N05948E / FLFL : FL230 - FL230//
//!   AMPN / TEST FOR POLYARC//
class PolyarcZone final : public Zone
{
public:
   PolyarcZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;

private:
   const Acmid*    mAcmid;
   const APeriod*  mAperiod;
   const Efflevel* mEfflevel;
   const Polyarc*  mPolyarc;
};
} // namespace usmtf

#endif
