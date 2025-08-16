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

#ifndef CIRCULARZONE_HPP
#define CIRCULARZONE_HPP

#include "APoint.hpp"
#include "Circle.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! Transforms a Segment identifying an Circle Zone, Point Zone, or Radarc Zone into an AFSIM zone script block

//! Circle Segment Example:
//! ACMID/ACM:ROZ/NAME:CIRCLE TEST/CIRCLE/USE:AEW//
//! CIRCLE/LATS:203632N0594256E/15NM//
//! EFFLEVEL/BRRA:GL-005AGL//
//! AMPN/TEST FOR CIRCLE//

//! Radarc Segment Example:
//! ACMID/ACM:ROZ/NAME:RADARC TEST/RADARC/USE:AEW//
//! RADARC/1500N05900E/300T/060T/0NM/50NM//
//! EFFLEVEL/RAFL:100AGL-FL600//
//! APERIOD/DISCRETE/270001ZJAN/UFN//
//! AMPN/TEST FOR RADARC//

//! Point Zone Segment Example:
//! ACMID/ACM:ROZ/NAME:POINT TEST/POINT/USE:AEW//
//! APOINT/1500N05900E//
//! EFFLEVEL/BRFL:MSL-FL230//
//! AMPN/TEST FOR POINT//
class CircularZone final : public Zone
{
public:
   CircularZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;

private:
   //! Sets Required to make a Circular zone.
   //! Circular Zone as opposed to others are a special case
   //! They can be made out of a segment of different types.
   //! So we have to handle that
   InputBlock GetRadarcZoneInputBlock() const;
   InputBlock GetPointZoneInputBlock() const;
   InputBlock GetCircleZoneInputBlock() const;

   const Acmid*    mAcmid;
   const APeriod*  mAperiod;
   const Efflevel* mEfflevel;
   const Radarc*   mRadarc;
   const Circle*   mCircle;
   const APoint*   mPoint;
   std::string     mCirculareZoneType;
};
} // namespace usmtf

#endif
