// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PATTERNDATAELEVATIONITERATOR_HPP
#define PATTERNDATAELEVATIONITERATOR_HPP

#include "PatternData.hpp"
#include "PolarCoordinate.hpp"

namespace PatternVisualizer
{
// This class is an iterator to a PatternData instance that walks over
// elevation values. In other words, this walks over all elevation values
// for a given constant azimuth.
class PatternDataElevationIterator
{
public:
   using value_type        = PolarCoordinate;
   using difference_type   = std::ptrdiff_t;
   using reference         = const PolarCoordinate&;
   using pointer           = const PolarCoordinate*;
   using iterator_category = std::forward_iterator_tag;

   PatternDataElevationIterator() = default;
   PatternDataElevationIterator(const PatternData*    aContainerPtr,
                                PatternData::Iterator aFirstHalfCurrent,
                                PatternData::Iterator aFirstHalfEnd,
                                PatternData::Iterator aSecondHalfCurrent,
                                PatternData::Iterator aSecondHalfEnd)
      : mContainerPtr{aContainerPtr}
      , mFirstHalfCurrent{aFirstHalfCurrent}
      , mFirstHalfEnd{aFirstHalfEnd}
      , mSecondHalfCurrent{aSecondHalfCurrent}
      , mSecondHalfEnd{aSecondHalfEnd}
   {
   }

   bool operator!=(const PatternDataElevationIterator& aOther) const { return !(*this == aOther); }
   bool operator==(const PatternDataElevationIterator& aOther) const
   {
      return mContainerPtr == aOther.mContainerPtr && mFirstHalfCurrent == aOther.mFirstHalfCurrent &&
             mFirstHalfEnd == aOther.mFirstHalfEnd && mSecondHalfCurrent == aOther.mSecondHalfCurrent &&
             mSecondHalfEnd == aOther.mSecondHalfEnd;
   }

   PatternDataElevationIterator& operator++()
   {
      Advance();
      return *this;
   }
   PatternDataElevationIterator operator++(int)
   {
      auto result = *this;
      Advance();
      return result;
   }

   PolarCoordinate operator*()
   {
      assert(mContainerPtr);

      auto& current = GetCurrentIterator();

      auto isReversed = !IsInFirstHalf();
      auto angle      = mContainerPtr->GetElevationAngle(current, isReversed);
      angle.Normalize(-AnglePi, AnglePi);

      return PolarCoordinate{angle, *current};
   }

private:
   void Advance()
   {
      auto& iterator = GetCurrentIterator();
      ++iterator;
   }

   PatternData::Iterator& GetCurrentIterator() { return IsInFirstHalf() ? mFirstHalfCurrent : mSecondHalfCurrent; }

   bool IsInFirstHalf() const { return mFirstHalfCurrent != mFirstHalfEnd; }

   const PatternData*    mContainerPtr{};
   PatternData::Iterator mFirstHalfCurrent{};
   PatternData::Iterator mFirstHalfEnd{};
   PatternData::Iterator mSecondHalfCurrent{};
   PatternData::Iterator mSecondHalfEnd{};
};
} // namespace PatternVisualizer

#endif
