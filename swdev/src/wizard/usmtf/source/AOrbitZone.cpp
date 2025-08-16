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

#include "AOrbitZone.hpp"

namespace usmtf
{
AOrbitZone::AOrbitZone(const std::string& aAOrbitZoneType, const Segment& aSegment) noexcept
   : Zone(aSegment)
{
   // Setting for cpp check, use for a period is still being decided by customer use case.
   mAperiod  = nullptr;
   mAcmid    = ExtractSet<Acmid>("ACMID", "Orbit Zone");
   mEfflevel = ExtractSet<Efflevel>("EFFLEVEL", "Orbit zone");
   mAOrbit   = ExtractSet<AOrbit>("AORBIT", "Orbit zone");
}

InputBlock AOrbitZone::GetInputBlock() const
{
   if (!IsValid())
   {
      return Zone::GetInputBlock();
   }

   InputBlock zoneSet{"zone_set", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};

   std::vector<UtLLPos> allPoints = mAOrbit->GetAlignmentOffsetPoints();
   // Zone 1, semi circle

   InputBlock                circularZone1{"zone", ""};
   usmtf::InputBlockProperty cz1type{"circular", "", ""};
   usmtf::InputBlockProperty cz1position{"position", LatLon::GetFormattedLatLon(allPoints[4]), " "};
   std::string               radius = UtStringUtil::ToString(mAOrbit->GetWidth().GetRadius() / 2);
   std::string               uom    = mAOrbit->GetWidth().GetUnitOfMeasure();
   usmtf::InputBlockProperty cz1maxRadius{"maximum_radius", radius + " " + uom, " "};
   usmtf::InputBlockProperty cz1minAlt{"minimum_altitude",
                                       std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) +
                                          " FT",
                                       " "};
   usmtf::InputBlockProperty cz1maxAlt{"maximum_altitude",
                                       std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) +
                                          " FT",
                                       " "};
   circularZone1.AddProperty(cz1type);
   circularZone1.AddProperty(cz1position);
   circularZone1.AddProperty(cz1maxRadius);
   circularZone1.AddProperty(cz1minAlt);
   circularZone1.AddProperty(cz1maxAlt);
   AddAmplificationBlockIfPresent(circularZone1);
   zoneSet.AddBlock(circularZone1);

   // Zone 2, semi circle

   InputBlock                circularZone2{"zone", ""};
   usmtf::InputBlockProperty cz2type{"circular", "", ""};
   usmtf::InputBlockProperty cz2position{"position", LatLon::GetFormattedLatLon(allPoints[5]), " "};
   usmtf::InputBlockProperty cz2maxRadius{"maximum_radius",
                                          UtStringUtil::ToString(mAOrbit->GetWidth().GetRadius() / 2) + " " +
                                             mAOrbit->GetWidth().GetUnitOfMeasure(),
                                          " "};
   usmtf::InputBlockProperty cz2minAlt{"minimum_altitude",
                                       std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) +
                                          " FT",
                                       " "};
   usmtf::InputBlockProperty cz2maxAlt{"maximum_altitude",
                                       std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) +
                                          " FT",
                                       " "};
   circularZone2.AddProperty(cz2type);
   circularZone2.AddProperty(cz2position);
   circularZone2.AddProperty(cz2maxRadius);
   circularZone2.AddProperty(cz2minAlt);
   circularZone2.AddProperty(cz2maxAlt);
   AddAmplificationBlockIfPresent(circularZone2);
   zoneSet.AddBlock(circularZone2);

   InputBlock                polygonZone{"zone", ""};
   usmtf::InputBlockProperty type{"polygonal", "", ""};
   usmtf::InputBlockProperty pointFormat{"lat_lon", "", ""};
   polygonZone.AddProperty(type);
   polygonZone.AddProperty(pointFormat);


   for (size_t i = 0; i < 4; i++) // Corners only
   {
      auto                      position = allPoints[i];
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
