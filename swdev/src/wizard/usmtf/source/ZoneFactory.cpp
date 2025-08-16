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

#include "ZoneFactory.hpp"

#include "AOrbitZone.hpp"
#include "CircularZone.hpp"
#include "OneTrackZone.hpp"
#include "PolyarcZone.hpp"
#include "PolygonalZone.hpp"
#include "UtMemory.hpp"
#include "Zone.hpp"

namespace usmtf
{
std::unique_ptr<Zone> CreateZone(const std::string& zoneType, const Segment& aSegment) noexcept
{
   if (zoneType == ZoneType::cPOLYARC)
   {
      return ut::make_unique<PolyarcZone>(zoneType, aSegment);
   }
   if (zoneType == ZoneType::cPOLYGON || zoneType == ZoneType::cCORRIDOR || zoneType == ZoneType::cLINE)
   {
      return ut::make_unique<PolygonalZone>(zoneType, aSegment);
   }
   if (zoneType == ZoneType::cTRACK)
   {
      return ut::make_unique<OneTrackZone>(zoneType, aSegment);
   }
   if (zoneType == ZoneType::cORBIT)
   {
      return ut::make_unique<AOrbitZone>(zoneType, aSegment);
   }
   if (zoneType == ZoneType::cPOINT || zoneType == ZoneType::cCIRCLE || zoneType == ZoneType::cRADARC)
   {
      return ut::make_unique<CircularZone>(zoneType, aSegment);
   }
   return nullptr;
}
} // namespace usmtf
