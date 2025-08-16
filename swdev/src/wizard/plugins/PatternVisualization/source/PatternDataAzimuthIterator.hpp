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

#ifndef PATTERNDATAAZIMUTHITERATOR_HPP
#define PATTERNDATAAZIMUTHITERATOR_HPP

#include "PatternData.hpp"
#include "PolarCoordinate.hpp"

namespace PatternVisualizer
{
// This class is an iterator to a PatternData instance that walks over
// azimuth values. In other words, this walks over all azimuth values
// for a given constant elevation.
class PatternDataAzimuthIterator
{
public:
   using value_type        = PolarCoordinate;
   using difference_type   = std::ptrdiff_t;
   using reference         = const PolarCoordinate&;
   using pointer           = const PolarCoordinate*;
   using iterator_category = std::forward_iterator_tag;

   PatternDataAzimuthIterator() = default;
   PatternDataAzimuthIterator(const PatternData* aContainerPtr, PatternData::Iterator aCurrent)
      : mContainerPtr{aContainerPtr}
      , mCurrent{aCurrent}
   {
   }

   bool operator!=(const PatternDataAzimuthIterator& aOther) const { return !(*this == aOther); }
   bool operator==(const PatternDataAzimuthIterator& aOther) const
   {
      return mContainerPtr == aOther.mContainerPtr && mCurrent == aOther.mCurrent;
   }

   PatternDataAzimuthIterator& operator++()
   {
      ++mCurrent;
      return *this;
   }
   PatternDataAzimuthIterator operator++(int)
   {
      auto result = *this;
      ++mCurrent;
      return result;
   }

   PolarCoordinate operator*() const
   {
      assert(mContainerPtr);
      return PolarCoordinate{mContainerPtr->GetAzimuthAngle(mCurrent), *mCurrent};
   }

private:
   const PatternData*    mContainerPtr{};
   PatternData::Iterator mCurrent{};
};
} // namespace PatternVisualizer

#endif
