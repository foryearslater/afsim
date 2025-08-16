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

#ifndef POLYGONALZONE_HPP
#define POLYGONALZONE_HPP

#include "Corridor.hpp"
#include "Geoline.hpp"
#include "Polygon.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! Transforms a Segment identifying an Polygon Zone into an AFSIM zone_set script block
//! Please see Polygon.hpp for a description of the shape this creates.

//! Example Segment:
//! ACMID/ACM:ROZ/NAME:POLYGON TEST/POLYGON/USE:AEW//
//! POLYGON/LATM:2037N05943E/LATS:203632N0594256E/LATS:203932N0594256E/LATS:203932N0604256E/LATM:2037N05943E//
//! EFFLEVEL/RAFL:100AGL-FL600//
//! APERIOD/DISCRETE/270001ZJAN/UFN//
//! AMPN/TEST FOR POLYGON//
class PolygonalZone final : public Zone
{
public:
   PolygonalZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;

private:
   InputBlock        GetPolygonInputBlock() const;
   InputBlock        GetCorridorInputBlock() const;
   InputBlock        GetLineInputBlock() const;
   InputBlock        BuildWall(const std::vector<LatLon>& positions, double widthInMeters) const;
   const Acmid*      mAcmid;
   const APeriod*    mAperiod;
   const Efflevel*   mEfflevel;
   const Corridor*   mCorridor;
   const Polygon*    mPolygon;
   const Geoline*    mGeoline;
   const std::string mPolygonalZoneType;
};
} // namespace usmtf

#endif
