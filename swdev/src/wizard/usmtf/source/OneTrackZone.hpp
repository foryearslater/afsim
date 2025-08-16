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

#ifndef ONETRACKZONE_HPP
#define ONETRACKZONE_HPP

#include "Onetrack.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! Transforms a Segment identifying an OneTrack Zone into an AFSIM zone_set script block
//! Please see Onetrack.hpp for a description of the shape this creates.

//!   Example Segment:
//!   ACMID / ACM:ATC / DESIG : ONETRACK TEST / TRACK / USE : CLSB//
//!   1TRACK
//!   / LEG / LEG - BEGIN / LEG - END / LEG - WIDTH / MINALT - MAXALT
//!   / 01 / 152345N0505657E / 192646N0531226E / 30.5NML - 60.9NMR / 050AMSL - 100AMSL
//!   / 02 / 192646N0531226E / 231031N0545323E / 60.5NML - 60.5NMR / 080AMSL - 120AMSL
//!   / 03 / 231031N0545325E / 280628N0562901E / 60NML - 80NMR / 100AMSL - 150AMSL
//!   / 04 / 280628N0562901E / 330938N0574603E / 25NML - 25NMR / 120AMSL - 180AMSL
//!   //
//!  APERIOD / DISCRETE / 272300ZNOV / 280100ZNOV//
//!   AMPN / TEST FOR ONETRACK//
class OneTrackZone final : public Zone
{
public:
   OneTrackZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;

private:
   const Acmid*    mAcmid;
   const APeriod*  mAperiod;
   const OneTrack* mOneTrack;
};
} // namespace usmtf

#endif
