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

#include "OneTrackZone.hpp"

#include <tuple>

#include "PositionUtils.hpp"

namespace usmtf
{
OneTrackZone::OneTrackZone(const std::string& aOneTrackZoneType, const Segment& aSegment) noexcept
   : Zone(aSegment)
{
   mAcmid    = ExtractSet<Acmid>("ACMID", "Track Zone");
   mOneTrack = ExtractSet<OneTrack>("1TRACK", "Track zone");
   mAperiod  = ExtractSet<APeriod>("APERIOD", "", true);
}

InputBlock OneTrackZone::GetInputBlock() const
{
   if (!IsValid())
   {
      return Zone::GetInputBlock();
   }

   InputBlock            block{"zone_set", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   const OneTrack::Legs& legs = mOneTrack->GetLegs();
   for (const auto& leg : legs)
   {
      InputBlock                zone{"zone", ""};
      usmtf::InputBlockProperty type{"polygonal", "", ""};
      usmtf::InputBlockProperty pointFormat{"lat_lon", "", ""};
      const auto                begin         = std::get<1>(leg).GetUtLLPos();
      const auto                end           = std::get<2>(leg).GetUtLLPos();
      const auto                leftOffset    = std::get<3>(leg).GetLeftRadius();
      const auto                rightOffset   = std::get<3>(leg).GetRightRadius();
      const double              leftDistance  = leftOffset->GetRadiusInMeters();
      const double              rightDistance = rightOffset->GetRadiusInMeters();
      std::vector<UtLLPos>      allPoints =
         FindPositionsLeftAndRightOfOriginToDestRef(begin, end, leftDistance, rightDistance); // begin offsets,
      usmtf::InputBlockProperty minAlt{"minimum_altitude",
                                       std::to_string(std::get<4>(leg).GetNumericalMinAltitude()) + " FT",
                                       " "};
      usmtf::InputBlockProperty maxAlt{"maximum_altitude",
                                       std::to_string(std::get<4>(leg).GetNumericalMaxAltitude()) + " FT",
                                       " "};
      zone.AddProperty(type);
      zone.AddProperty(pointFormat);
      for (const UtLLPos& ll : allPoints)
      {
         usmtf::InputBlockProperty point{"point", LatLon::GetFormattedLatLon(ll), " "};
         zone.AddProperty(point);
      }
      zone.AddProperty(minAlt);
      zone.AddProperty(maxAlt);
      AddAmplificationBlockIfPresent(zone);
      block.AddBlock(zone);
   }

   return block;
}
} // namespace usmtf
