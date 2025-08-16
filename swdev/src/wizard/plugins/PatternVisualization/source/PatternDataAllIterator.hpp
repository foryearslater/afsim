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

#ifndef PATTERNDATAALLITERATOR_HPP
#define PATTERNDATAALLITERATOR_HPP

#include "Angle.hpp"
#include "PatternData.hpp"

namespace PatternVisualizer
{
struct PatternDataDatum
{
   Angle Azimuth;
   Angle Elevation;
   float DB{};

   PatternDataDatum() = default;

   PatternDataDatum(Angle aAzimuth, Angle aElevation, float aDb)
      : Azimuth{aAzimuth}
      , Elevation{aElevation}
      , DB{aDb}
   {
   }

   bool operator==(const PatternDataDatum& aOther) const
   {
      return Azimuth == aOther.Azimuth && Elevation == aOther.Elevation && DB == aOther.DB;
   }
};

// This class is an iterator to a PatternData instance that walks over
// all the data in the table. For each element, it'll return the azimuth,
// elevation, and db values for that data point.
class PatternDataAllIterator
{
public:
   using value_type        = PatternDataDatum;
   using difference_type   = std::ptrdiff_t;
   using reference         = const PatternDataDatum&;
   using pointer           = const PatternDataDatum*;
   using iterator_category = std::forward_iterator_tag;

   PatternDataAllIterator() = default;
   PatternDataAllIterator(const PatternData* aContainerPtr, const PatternData::Iterator& aCurrent)
      : mContainerPtr{aContainerPtr}
      , mCurrent{aCurrent}
   {
   }

   bool operator!=(const PatternDataAllIterator& aOther) const { return !(*this == aOther); }
   bool operator==(const PatternDataAllIterator& aOther) const
   {
      return mContainerPtr == aOther.mContainerPtr && mCurrent == aOther.mCurrent;
   }

   PatternDataAllIterator& operator++()
   {
      ++mCurrent;
      return *this;
   }
   PatternDataAllIterator operator++(int)
   {
      ++mCurrent;
      return *this;
   }

   PatternDataDatum operator*() const
   {
      assert(mContainerPtr);

      Angle azimuth   = mContainerPtr->GetAzimuthAngle(mCurrent);
      Angle elevation = mContainerPtr->GetElevationAngle(mCurrent, false);
      float db        = *mCurrent;

      return PatternDataDatum{azimuth, elevation, db};
   }

   const PatternData*    mContainerPtr;
   PatternData::Iterator mCurrent;
};
} // namespace PatternVisualizer

#endif
