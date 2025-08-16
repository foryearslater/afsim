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

#include "CircularZone.hpp"

#include <string>

#include "ZoneFactory.hpp"

namespace usmtf
{
CircularZone::CircularZone(const std::string& aCircularZoneType, const Segment& aSegment) noexcept
   : Zone(aSegment)
   , mCirculareZoneType(aCircularZoneType)
{
   // Setting for cpp check, use for a period is still being decided by customer use case.
   mAperiod  = nullptr;
   mAcmid    = ExtractSet<Acmid>("ACMID", "Circular Zone");
   mEfflevel = ExtractSet<Efflevel>("EFFLEVEL", "Circular zone");

   if (aCircularZoneType == ZoneType::cRADARC)
   {
      mRadarc = ExtractSet<Radarc>("RADARC", "Circular zone");
   }
   else if (aCircularZoneType == ZoneType::cPOINT)
   {
      mPoint = ExtractSet<APoint>("APOINT", "Circular zone");
   }
   else if (aCircularZoneType == ZoneType::cCIRCLE)
   {
      mCircle = ExtractSet<Circle>("CIRCLE", "Circular zone");
   }
}

InputBlock CircularZone::GetInputBlock() const
{
   if (!IsValid())
   {
      return Zone::GetInputBlock();
   }

   if (mCirculareZoneType == ZoneType::cRADARC)
   {
      return GetRadarcZoneInputBlock();
   }
   else if (mCirculareZoneType == ZoneType::cPOINT)
   {
      return GetPointZoneInputBlock();
   }
   else if (mCirculareZoneType == ZoneType::cCIRCLE)
   {
      return GetCircleZoneInputBlock();
   }
   else
   {
      return Zone::GetInputBlock();
   }
}

InputBlock CircularZone::GetRadarcZoneInputBlock() const
{
   InputBlock                block{"zone", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   usmtf::InputBlockProperty type{"circular", "", ""};
   usmtf::InputBlockProperty position{"position", mRadarc->GetOriginOfBearing().GetFormattedLatLon(), " "};
   usmtf::InputBlockProperty minRadius{"minimum_radius",
                                       mRadarc->GetInnerRadius().GetRadiusValue() + " " +
                                          mRadarc->GetInnerRadius().GetUnitOfMeasure(),
                                       " "};
   usmtf::InputBlockProperty maxRadius{"maximum_radius",
                                       mRadarc->GetOuterRadius().GetRadiusValue() + " " +
                                          mRadarc->GetOuterRadius().GetUnitOfMeasure(),
                                       " "};
   usmtf::InputBlockProperty minAlt{"minimum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty startAngle{"start_angle", mRadarc->GetBeginningRadialBearingTrue().GetBearing() + " deg", " "};
   usmtf::InputBlockProperty stopAngle{"stop_angle", mRadarc->GetEndingRadialBearingTrue().GetBearing() + " deg", " "};
   block.AddProperty(type);
   block.AddProperty(position);
   block.AddProperty(minRadius);
   block.AddProperty(maxRadius);
   block.AddProperty(minAlt);
   block.AddProperty(maxAlt);
   block.AddProperty(startAngle);
   block.AddProperty(stopAngle);
   AddAmplificationBlockIfPresent(block);
   return block;
}

InputBlock CircularZone::GetPointZoneInputBlock() const
{
   InputBlock                block{"zone", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   usmtf::InputBlockProperty type{"circular", "", ""};
   usmtf::InputBlockProperty position{"position", mPoint->GetAirSpacePoint().GetFormattedLatLon(), " "};
   usmtf::InputBlockProperty maxRadius{"maximum_radius", "1 KM", " "}; // there is no idea of a point in AFSIM. I was advised
                                                                       // to just create a circle with a defaults.
   usmtf::InputBlockProperty minAlt{"minimum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty startAngle{"start_angle", "0 deg", " "};
   usmtf::InputBlockProperty stopAngle{"stop_angle", "360 deg", " "};
   block.AddProperty(type);
   block.AddProperty(position);
   block.AddProperty(maxRadius);
   block.AddProperty(minAlt);
   block.AddProperty(maxAlt);
   block.AddProperty(startAngle);
   block.AddProperty(stopAngle);
   AddAmplificationBlockIfPresent(block);
   return block;
}

InputBlock CircularZone::GetCircleZoneInputBlock() const
{
   InputBlock                block{"zone", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   usmtf::InputBlockProperty type{"circular", "", ""};
   usmtf::InputBlockProperty position{"position", mCircle->GetCenterOfCircle().GetFormattedLatLon(), " "};
   usmtf::InputBlockProperty maxRadius{"maximum_radius",
                                       mCircle->GetRadius().GetRadiusValue() + " " +
                                          mCircle->GetRadius().GetUnitOfMeasure(),
                                       " "};
   usmtf::InputBlockProperty startAngle{"start_angle", "0 deg", " "};
   usmtf::InputBlockProperty stopAngle{"stop_angle", "360 deg", " "};
   usmtf::InputBlockProperty minAlt{"minimum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMinAltitude()) + " FT",
                                    " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude",
                                    std::to_string(mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude()) + " FT",
                                    " "};
   block.AddProperty(type);
   block.AddProperty(position);
   block.AddProperty(maxRadius);
   block.AddProperty(startAngle);
   block.AddProperty(stopAngle);
   block.AddProperty(minAlt);
   block.AddProperty(maxAlt);
   AddAmplificationBlockIfPresent(block);
   return block;
}
} // namespace usmtf
