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

#ifndef PATTERNDATAITERATOR_HPP
#define PATTERNDATAITERATOR_HPP

#include "PatternData.hpp"
#include "PolarCoordinate.hpp"

namespace PatternVisualizer
{
enum PatternType
{
   AZIMUTH,
   ELEVATION
};

class PatternDataIterator
{
public:
   using value_type        = PolarCoordinate;
   using difference_type   = std::ptrdiff_t;
   using reference         = const PolarCoordinate&;
   using pointer           = const PolarCoordinate*;
   using iterator_category = std::forward_iterator_tag;

   PatternDataIterator()          = default;
   virtual ~PatternDataIterator() = default;

   // An instance created with this constructor walks over all azimuth values
   // for a given constant elevation
   PatternDataIterator(const PatternData* aContainerPtr, const PatternData::Iterator& aCurrent)
      : mPatternType(AZIMUTH)
      , mContainerPtr(aContainerPtr)
      , mCurrent(aCurrent)
   {
   }

   // An instance created with this constructor walks over all elevation values
   // for a given constant azimuth
   PatternDataIterator(const PatternData*           aContainerPtr,
                       const PatternData::Iterator& aFirstHalfCurrent,
                       const PatternData::Iterator& aFirstHalfEnd,
                       const PatternData::Iterator& aSecondHalfCurrent,
                       const PatternData::Iterator& aSecondHalfEnd)
      : mPatternType(ELEVATION)
      , mContainerPtr(aContainerPtr)
      , mFirstHalfCurrent(aFirstHalfCurrent)
      , mFirstHalfEnd(aFirstHalfEnd)
      , mSecondHalfCurrent(aSecondHalfCurrent)
      , mSecondHalfEnd(aSecondHalfEnd)
   {
   }

   bool operator!=(const PatternDataIterator& aOther) const { return !(*this == aOther); }

   virtual bool operator==(const PatternDataIterator& aOther) const
   {
      if (mPatternType != aOther.mPatternType || mContainerPtr != aOther.mContainerPtr)
      {
         return false;
      }
      if (mPatternType == AZIMUTH)
      {
         return (mCurrent == aOther.mCurrent);
      }
      else
      {
         return (mFirstHalfCurrent == aOther.mFirstHalfCurrent && mFirstHalfEnd == aOther.mFirstHalfEnd &&
                 mSecondHalfCurrent == aOther.mSecondHalfCurrent && mSecondHalfEnd == aOther.mSecondHalfEnd);
      }
   }

   PatternDataIterator& operator++()
   {
      Advance();
      return *this;
   }
   PatternDataIterator operator++(int)
   {
      Advance();
      return *this;
   }

   PolarCoordinate operator*()
   {
      assert(mContainerPtr);
      if (mPatternType == AZIMUTH)
      {
         return PolarCoordinate(mContainerPtr->GetAzimuthAngle(mCurrent), *mCurrent);
      }
      else
      {
         PatternData::Iterator& current    = GetCurrentIterator();
         bool                   isReversed = (mFirstHalfCurrent == mFirstHalfEnd);
         Angle                  angle      = mContainerPtr->GetElevationAngle(current, isReversed);
         angle.Normalize(-AnglePi, AnglePi);
         return PolarCoordinate(angle, *current);
      }
   }

   PatternType GetPatternType() const { return mPatternType; }

private:
   void Advance()
   {
      if (mPatternType == AZIMUTH)
      {
         ++mCurrent;
      }
      else
      {
         ++GetCurrentIterator();
      }
   }

   PatternData::Iterator& GetCurrentIterator()
   {
      return (mFirstHalfCurrent != mFirstHalfEnd ? mFirstHalfCurrent : mSecondHalfCurrent);
   }

   PatternType        mPatternType;
   const PatternData* mContainerPtr;

   // Used only for azimuth iterator
   PatternData::Iterator mCurrent{};

   // Used only for elevation iterator
   PatternData::Iterator mFirstHalfCurrent{};
   PatternData::Iterator mFirstHalfEnd{};
   PatternData::Iterator mSecondHalfCurrent{};
   PatternData::Iterator mSecondHalfEnd{};
};
} // namespace PatternVisualizer

#endif // PATTERNDATAITERATOR_HPP
