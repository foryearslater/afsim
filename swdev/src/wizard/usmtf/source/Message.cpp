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

#include "Message.hpp"

#include <memory>
#include <string>

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"

namespace usmtf
{
Message::Message(const std::string& aMTF, std::vector<std::unique_ptr<Set>> aSets) noexcept
   : mMessageTextFormat(aMTF)
   , mSets(std::move(aSets))
{
}

const std::string& Message::GetType() const noexcept
{
   return mMessageTextFormat;
}

const Set& Message::GetSet(Segment::SetPosition aSetIndex) const
{
   if (aSetIndex >= mSets.size())
   {
      throw MessageError(
         "Cannot retrieve Set at " +
         std::to_string(aSetIndex) + ". Doing so would result in buffer overflow. Make sure index is within the Message SetCount and Message is not empty.");
   }
   return *(mSets[aSetIndex]);
}

Segment::SetLocations Message::GetSets(const std::string& aSTF) const noexcept
{
   Segment::SetLocations results;
   Segment::SetPosition  position = 0;
   for (auto& set : mSets)
   {
      if (set->GetType() == aSTF)
      {
         Segment::SetLocation res{*set, position};
         results.push_back(res);
      }
      position++;
   }
   return results;
}

Segment::SetLocations Message::GetSets(const std::vector<std::string>& aSTF) const noexcept
{
   Segment::SetLocations results;
   Segment::SetPosition  position = 0;
   for (auto& set : mSets)
   {
      if (FFIRN_Validator::IsInSelection(set->GetType(), aSTF))
      {
         Segment::SetLocation res{*set, position};
         results.push_back(res);
      }
      position++;
   }
   return results;
}

Segment Message::GetSegment(const Segment::SetPosition& startPosition, const Segment::SetPosition& stopPosition) const
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
   return Segment{results};
}

std::vector<Segment> Message::GetSegments(const std::string& aSTF) const
{
   std::vector<Segment>  segments;
   Segment::SetLocations setPositions = GetSets(aSTF);
   Segment::SetPosition  vectorIndex  = 0;
   for (const auto& position : setPositions)
   {
      Segment::SetPosition currentSetPosition = position.second;
      Segment::SetPosition nextSetPosition =
         vectorIndex + 1 >= setPositions.size() ? GetNumberOfSets() - 1 : setPositions[vectorIndex + 1].second - 1;
      segments.push_back(GetSegment(currentSetPosition, nextSetPosition));
      vectorIndex++;
   }
   return segments;
}

size_t Message::GetNumberOfSets() const noexcept
{
   return mSets.size();
}

bool Message::IsEmpty() const noexcept
{
   return mSets.empty();
}

bool Message::IsSetPositionOutOfBounds(const Segment::SetPosition& aPos) const noexcept
{
   if (aPos >= mSets.size())
   {
      return true;
   }
   return false;
}
} // namespace usmtf
