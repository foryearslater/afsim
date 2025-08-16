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

#ifndef AORBITZONE_HPP
#define AORBITZONE_HPP

#include "AOrbit.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! Transforms a Segment identifying an AOrbit Zone into an AFSIM zone_set script block
//! Please see AOrbit.hpp for a description of the shape this creates.

//! Segment Example:
//! ACMID/ACM:ATC/NAME:BBR-22/ORBIT/USE:CLSB//
//! AORBIT/LATS:251538N0342044E/LATS:305942N0343122E/40NM/C//
//! EFFLEVEL/RAFL:070AMSL-FL280//
class AOrbitZone final : public Zone
{
public:
   AOrbitZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;

private:
   const Acmid*    mAcmid;
   const APeriod*  mAperiod;
   const Efflevel* mEfflevel;
   const AOrbit*   mAOrbit;
};
} // namespace usmtf

#endif
