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

#include "Segment.hpp"

#include <algorithm>

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
Segment::Segment(const SetLocations& aSets) noexcept
   : mSets(aSets){};

const Segment::SetLocation Segment::GetSet(const std::string& aSTF) const
{
   auto location = std::find_if(mSets.cbegin(),
                                mSets.cend(),
                                [aSTF](const SetLocation& loc) -> bool { return loc.first.GetType() == aSTF; });

   if (location != mSets.cend())
   {
      return *location;
   }

   throw SetError("No Set of type " + aSTF + " in segment");
}

const Set& Segment::GetSet(Segment::SetPosition aSetIndex) const
{
   if (aSetIndex >= mSets.size())
   {
      throw MessageError(
         "Cannot retrieve Set at " +
         std::to_string(aSetIndex) + ". Doing so would result in buffer overflow. Make sure index is within the Message SetCount and Message is not empty.");
   }
   return mSets[aSetIndex].first;
}

std::vector<Segment::SetLocations> Segment::GetSubSegments(const std::string& aSTF) const
{
   std::vector<SetLocations> mSetLocations;
   Segment::SetLocations     setPositions = GetSets(aSTF);
   Segment::SetPosition      vectorIndex  = 0;
   for (const auto& position : setPositions)
   {
      Segment::SetPosition currentSetPosition = position.second;
      Segment::SetPosition nextSetPosition =
         vectorIndex + 1 >= setPositions.size() ? GetNumberOfSets() - 1 : setPositions[vectorIndex + 1].second - 1;
      mSetLocations.push_back(GetSubSegment(currentSetPosition, nextSetPosition));
      vectorIndex++;
   }
   return mSetLocations;
}

size_t Segment::GetNumberOfSets() const noexcept
{
   return mSets.size();
}

Segment::SetLocations Segment::GetSubSegment(const Segment::SetPosition& startPosition,
                                             const Segment::SetPosition& stopPosition) const
{
   if (IsSetPositionOutOfBounds(startPosition) || IsSetPositionOutOfBounds(stopPosition))
   {
      throw MessageError(
         "Cannot retrieve Sets from " + std::to_string(startPosition) + " to " +
         std::to_string(stopPosition) + ". Doing so would result in buffer overflow. Make sure indexes are within the Message SetCount and Message is not empty.");
   }

   Segment::SetLocations results;
   for (Segment::SetPosition position = startPosition; position <= stopPosition; position++)
   {
      Segment::SetLocation res{GetSet(position), position};
      results.push_back(res);
   }
   return results;
}

Segment::SetLocations Segment::GetSets(const std::string& aSTF) const noexcept
{
   Segment::SetLocations results;
   Segment::SetPosition  position = 0;
   for (auto& set : mSets)
   {
      if (set.first.GetType() == aSTF)
      {
         Segment::SetLocation res{set.first, position};
         results.push_back(res);
      }
      position++;
   }
   return results;
}

bool Segment::HasSet(const std::string& aSTF) const
{
   auto location = std::find_if(mSets.cbegin(),
                                mSets.cend(),
                                [aSTF](const SetLocation& loc) -> bool { return loc.first.GetType() == aSTF; });

   if (location != mSets.cend())
   {
      return true;
   }
   return false;
}

Segment::SetLocations::const_iterator Segment::begin() const
{
   return mSets.cbegin();
}

Segment::SetLocations::const_iterator Segment::end() const
{
   return mSets.cend();
}

bool Segment::IsSetPositionOutOfBounds(const Segment::SetPosition& aPos) const noexcept
{
   if (aPos >= mSets.size())
   {
      return true;
   }
   return false;
}
} // namespace usmtf
