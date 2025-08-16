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

#include "PolygonalZone.hpp"

#include <forward_list>

#include "PositionUtils.hpp"
#include "ZoneFactory.hpp"

namespace usmtf
{
PolygonalZone::PolygonalZone(const std::string& aPolygonalZoneType, const Segment& aSegment) noexcept
   : Zone(aSegment)
   , mPolygonalZoneType(aPolygonalZoneType)
{
   // Setting for cpp check, use for a period is still being decided by customer use case.
   mAperiod  = nullptr;
   mAcmid    = ExtractSet<Acmid>("ACMID", "Polygonal Zone");
   mEfflevel = ExtractSet<Efflevel>("EFFLEVEL", "Polygonal zone");

   if (aPolygonalZoneType == ZoneType::cPOLYGON)
   {
      mPolygon = ExtractSet<Polygon>("POLYGON", "Polygonal zone");
   }
   else if (aPolygonalZoneType == ZoneType::cCORRIDOR)
   {
      mCorridor = ExtractSet<Corridor>("CORRIDOR", "Polygonal zone");
   }
   else if (aPolygonalZoneType == ZoneType::cLINE)
   {
      mGeoline = ExtractSet<Geoline>("GEOLINE", "Polygonal zone");
   }
}

InputBlock PolygonalZone::GetInputBlock() const
{
   if (!IsValid())
   {
      return Zone::GetInputBlock();
   }

   if (mPolygonalZoneType == ZoneType::cPOLYGON)
   {
      return GetPolygonInputBlock();
   }
   else if (mPolygonalZoneType == ZoneType::cCORRIDOR)
   {
      return GetCorridorInputBlock();
   }
   else if (mPolygonalZoneType == ZoneType::cLINE)
   {
      return GetLineInputBlock();
   }
   else
   {
      return Zone::GetInputBlock();
   }
}

InputBlock PolygonalZone::GetPolygonInputBlock() const
{
   const std::vector<LatLon>& position = mPolygon->GetPoints();
   InputBlock                 block{"zone", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   usmtf::InputBlockProperty  type{"polygonal", "", ""};
   usmtf::InputBlockProperty  pointFormat{"lat_lon", "", ""};
   block.AddProperty(type);
   block.AddProperty(pointFormat);
   for (size_t i = 0; i < position.size(); i++)
   {
      usmtf::InputBlockProperty aSinglePoint{"point", position[i].GetFormattedLatLon(), " "};
      block.AddProperty(aSinglePoint);
   }
   int minAltitude = mEfflevel->GetVerticalDimension().GetNumericalMinAltitude();
   int maxAltitude = mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude();
   if (minAltitude == maxAltitude)
   {
      maxAltitude = minAltitude + 1;
   }
   usmtf::InputBlockProperty minAlt{"minimum_altitude", std::to_string(minAltitude) + " FT", " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude", std::to_string(maxAltitude) + " FT", " "};
   block.AddProperty(minAlt);
   block.AddProperty(maxAlt);
   AddAmplificationBlockIfPresent(block);
   return block;
}

InputBlock PolygonalZone::GetCorridorInputBlock() const
{
   return BuildWall(mCorridor->GetPoints(), mCorridor->GetWidth().GetRadiusInMeters());
}

InputBlock PolygonalZone::GetLineInputBlock() const
{
   return BuildWall(mGeoline->GetPoints(), 1);
}

InputBlock PolygonalZone::BuildWall(const std::vector<LatLon>& positions, double widthInMeters) const
{
   InputBlock                block{"zone", ReplaceSpaces(mAcmid->GetAirspaceControlMeansIdentifier(), '_')};
   usmtf::InputBlockProperty type{"polygonal", "", ""};
   usmtf::InputBlockProperty pointFormat{"lat_lon", "", ""};
   block.AddProperty(type);
   block.AddProperty(pointFormat);
   const std::vector<LatLon>& allPositions = positions;
   std::vector<UtLLPos>       leftSide{};
   std::forward_list<UtLLPos> rightSide{}; // for efficient front push

   std::vector<UtLLPos> finalPolygon{};
   for (size_t i = 0; i < allPositions.size(); i++)
   {
      const auto begin = allPositions[i];
      size_t     next  = i + 1;
      if (next <= allPositions.size())
      {
         if (i == allPositions.size() - 1) // last leg end square
         {
            const auto newEnd   = allPositions[i];
            const auto newBegin = allPositions[i - 1];
            const auto newPoints =
               FindSquareOfRefrencePositions(newBegin.GetUtLLPos(), newEnd.GetUtLLPos(), widthInMeters);

            leftSide.push_back(newPoints[1]);
            rightSide.push_front(newPoints[2]);
            continue;
         }
         const auto end    = allPositions[next];
         const auto points = FindSquareOfRefrencePositions(begin.GetUtLLPos(), end.GetUtLLPos(), widthInMeters);

         if (i == 0) // first leg needs all points
         {
            leftSide.push_back(points[0]);
            rightSide.push_front(points[3]);
         }
         else // remainder loop needs only calculated points.
         {
            const auto heading = FindVectorTrueNorthHeading(begin.GetUtLLPos(), end.GetUtLLPos());

            // need prev to know if were turning left or right.
            const auto prevHeading = FindVectorTrueNorthHeading(allPositions[i - 1].GetUtLLPos(), begin.GetUtLLPos());

            const auto prevPoints =
               FindSquareOfRefrencePositions(allPositions[i - 1].GetUtLLPos(), begin.GetUtLLPos(), widthInMeters);

            const auto hDiff = heading - prevHeading;

            if (hDiff > 0) // right
            {
               LineSegment prev{{prevPoints[2].GetLat(), prevPoints[2].GetLon()},
                                {prevPoints[3].GetLat(), prevPoints[3].GetLon()}};

               LineSegment curr{{points[2].GetLat(), points[2].GetLon()}, {points[3].GetLat(), points[3].GetLon()}};

               UtLLPos intersection = prev.Intersect(curr);

               leftSide.push_back(prevPoints[1]);
               leftSide.push_back(points[0]);
               rightSide.push_front(intersection);
            }
            else if (hDiff < 0) // left
            {
               LineSegment prev{{prevPoints[0].GetLat(), prevPoints[0].GetLon()},
                                {prevPoints[1].GetLat(), prevPoints[1].GetLon()}};

               LineSegment curr{{points[0].GetLat(), points[0].GetLon()}, {points[1].GetLat(), points[1].GetLon()}};

               UtLLPos intersection = prev.Intersect(curr);

               leftSide.push_back(intersection);
               rightSide.push_front(prevPoints[2]);
               rightSide.push_front(points[3]);
            }
            else // straight!
            {
               leftSide.push_back(prevPoints[1]);
               rightSide.push_front(prevPoints[2]);
            }
         }
      }
   }

   finalPolygon.insert(finalPolygon.end(), leftSide.begin(), leftSide.end());
   finalPolygon.insert(finalPolygon.end(), rightSide.begin(), rightSide.end());
   for (const UtLLPos& ll : finalPolygon)
   {
      usmtf::InputBlockProperty point{"point", LatLon::GetFormattedLatLon(ll), " "};
      block.AddProperty(point);
   }
   int minAltitude = mEfflevel->GetVerticalDimension().GetNumericalMinAltitude();
   int maxAltitude = mEfflevel->GetVerticalDimension().GetNumericalMaxAltitude();
   if (minAltitude == maxAltitude)
   {
      maxAltitude = minAltitude + 1;
   }
   usmtf::InputBlockProperty minAlt{"minimum_altitude", std::to_string(minAltitude) + " FT", " "};
   usmtf::InputBlockProperty maxAlt{"maximum_altitude", std::to_string(maxAltitude) + " FT", " "};
   block.AddProperty(minAlt);
   block.AddProperty(maxAlt);
   AddAmplificationBlockIfPresent(block);
   return block;
}
} // namespace usmtf
