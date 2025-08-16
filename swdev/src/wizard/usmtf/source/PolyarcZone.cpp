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

#include "PolyarcZone.hpp"

#include <iterator>

namespace usmtf
{
PolyarcZone::PolyarcZone(const std::string& aPolyarcZoneType, const Segment& aSegment) noexcept
   : Zone(aSegment)
{
   // Setting for cpp check, use for a period is still being decided by customer use case.
   mAperiod  = nullptr;
   mAcmid    = ExtractSet<Acmid>("ACMID", "Polyarc Zone");
   mEfflevel = ExtractSet<Efflevel>("EFFLEVEL", "Polyarc zone");
   mPolyarc  = ExtractSet<Polyarc>("POLYARC", "Polyarc zone");
}

InputBlock PolyarcZone::GetInputBlock() const
{
   if (!IsValid())
   {
      return Zone::GetInputBlock();
   }

   // Zone 1, semi circle.
   InputBlock zoneSet{"zone_set", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};

   InputBlock                circularZone{"zone", ""};
   usmtf::InputBlockProperty cztype{"circular", "", ""};
   usmtf::InputBlockProperty czposition{"position", mPolyarc->GetRadarc()->GetOriginOfBearing().GetFormattedLatLon(), " "};
   usmtf::InputBlockProperty czmaxRadius{"maximum_radius",
                                         mPolyarc->GetRadarc()->GetOuterRadius().GetRadiusValue() + " " +
                                            mPolyarc->GetRadarc()->GetOuterRadius().GetUnitOfMeasure(),
                                         " "};
   usmtf::InputBlockProperty czminAlt{"minimum_altitude",
                                      std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) +
                                         " FT",
                                      " "};
   usmtf::InputBlockProperty czmaxAlt{"maximum_altitude",
                                      std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) +
                                         " FT",
                                      " "};
   usmtf::InputBlockProperty czstartAngle{"start_angle",
                                          mPolyarc->GetRadarc()->GetBeginningRadialBearingTrue().GetBearing() + " deg",
                                          " "};
   usmtf::InputBlockProperty czstopAngle{"stop_angle",
                                         mPolyarc->GetRadarc()->GetEndingRadialBearingTrue().GetBearing() + " deg",
                                         " "};
   circularZone.AddProperty(cztype);
   circularZone.AddProperty(czposition);
   circularZone.AddProperty(czmaxRadius);
   circularZone.AddProperty(czminAlt);
   circularZone.AddProperty(czmaxAlt);
   circularZone.AddProperty(czstartAngle);
   circularZone.AddProperty(czstopAngle);
   AddAmplificationBlockIfPresent(circularZone);


   zoneSet.AddBlock(circularZone);

   InputBlock                polygonZone{"zone", ""};
   usmtf::InputBlockProperty type{"polygonal", "", ""};
   usmtf::InputBlockProperty pointFormat{"lat_lon", "", ""};
   polygonZone.AddProperty(type);
   polygonZone.AddProperty(pointFormat);
   const std::vector<LatLon>& polygonPoints = mPolyarc->GetPolygon()->GetPoints();
   std::vector<UtLLPos>       converted{};
   std::transform(polygonPoints.cbegin(),
                  polygonPoints.cend(),
                  std::back_inserter(converted),
                  [](const LatLon& pp) -> UtLLPos { return pp.GetUtLLPos(); });


   for (const auto& position : converted)
   {
      usmtf::InputBlockProperty aSinglePoint{"point", LatLon::GetFormattedLatLon(position), " "};
      polygonZone.AddProperty(aSinglePoint);
   }
   usmtf::InputBlockProperty minAlt{"minimum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) + " FT",
                                    " "};
   polygonZone.AddProperty(minAlt);
   polygonZone.AddProperty(maxAlt);
   AddAmplificationBlockIfPresent(polygonZone);

   zoneSet.AddBlock(polygonZone);
   return zoneSet;
}
} // namespace usmtf
