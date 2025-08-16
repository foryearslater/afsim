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

#ifndef PATTERNDATA_HPP
#define PATTERNDATA_HPP

#include <algorithm>

#include "Angle.hpp"
#include "RowColumnVector.hpp"
#include "WsfAcousticSignature.hpp"
#include "WsfAntennaPattern.hpp"
#include "WsfInfraredSignature.hpp"
#include "WsfOpticalSignature.hpp"
#include "WsfRadarSignature.hpp"

namespace PatternVisualizer
{
class PatternDataAllIterator;
class PatternDataIterator;

// PatternData efficiently holds the DB data for a given antenna pattern.
// This class effectively rasterizes the antenna pattern and stores a
// two-dimensional bitmap of the DB data. The layout of this two-dimensional
// data is as follows:
//
//  * Azimuth is defined by the column.
//  * Elevation is defined by the row.
//  * Azimuth starts at -pi to the left, and increases to the right.
//  * The right-most azimuth column is the raster column just before +pi.
//  * Elevation goes from +pi/2 to -pi from top to bottom.
//
// In other words, the center angle (0 degrees azimuth and elevation) is
// at the table, with +azimuth tending right and +elevation tending up.
// The top-left corner of the table will have (az,el) of (-pi, +pi/2).
//
// Because +pi and -pi azimuth would be the same, PatternData doesn't
// repeat +pi on the right-most column. Instead, the right-most column
// would be the azimuth values just before +pi. So, conceptually, if the
// raster was NumColumns wide, then: ColumnData[0] == ColumnData[NumColumns].
class PatternData
{
public:
   using DataType  = RowColumnVector<float>;
   using IndexType = DataType::size_type;
   using Iterator  = DataType::Iterator;

   PatternData(WsfAntennaPattern& aWsfPattern, float aFreq);
   PatternData(WsfRadarSignature& aWsfPattern, WsfStringId aState, WsfEM_Types::Polarization aPolarization, float aFreq);
   PatternData(WsfOpticalSignature& aWsfPattern, WsfStringId aState);
   PatternData(WsfInfraredSignature& aWsfPattern, WsfStringId aState, WsfEM_Types::InfraredBand aIRBand);
   PatternData(WsfAcousticSignature& aWsfPattern, WsfStringId aState, float aFreq);

   // Testing mock.
   template<typename Iter>
   PatternData(uint32_t aNumRows, uint32_t aNumColumns, Iter aFirst, Iter aLast)
      : mData(aNumRows, aNumColumns, aFirst, aLast)
   {
      auto minmax = std::minmax_element(mData.begin(), mData.end());
      mMinDB      = *minmax.first;
      mMaxDB      = *minmax.second;
   }

   float GetMinDB() const { return mMinDB; }
   float GetMaxDB() const { return mMaxDB; }
   float GetMinDBAllFreqs() const { return mMinDBAllFreqs; }
   float GetMaxDBAllFreqs() const { return mMaxDBAllFreqs; }

   size_t GetNumRows() const { return mData.GetNumRows(); }
   size_t GetNumColumns() const { return mData.GetNumColumns(); }
   size_t GetNumElements() const { return mData.GetNumElements(); }

   bool UpdateGainTable(WsfAntennaPattern& aWsfPattern, float aFreq);
   bool UpdateGainTable(WsfRadarSignature& aWsfPattern, float aFreq);
   bool UpdateGainTable(WsfOpticalSignature& aWsfPattern, float aFreq = 0);
   bool UpdateGainTable(WsfInfraredSignature& aWsfPattern, WsfEM_Types::InfraredBand aIRBand);
   bool UpdateGainTable(WsfAcousticSignature& aWsfPattern, float aFreq);

   WsfEM_Types::InfraredBand GetIRBand() const { return mIRBand; }

   // Enumerates over all the datum values in this dataset (in row major order).
   PatternDataAllIterator EnumerateAllBegin() const;
   PatternDataAllIterator EnumerateAllEnd() const;

   // Enumerates over all the azimuth values (full circle) for the given constant elevation
   // returning a PolarCoordinate for each entry.
   PatternDataIterator EnumerateAzimuthsPolarBegin(Angle aConstantElevation) const;
   PatternDataIterator EnumerateAzimuthsPolarEnd(Angle aConstantElevation) const;

   // Enumerates over all the elevation values (full circle) for the given constant azimuth
   // returning a PolarCoordinate for each entry.
   PatternDataIterator EnumerateElevationsPolarBegin(Angle aConstantAzimuth) const;
   PatternDataIterator EnumerateElevationsPolarEnd(Angle aConstantAzimuth) const;

   float GetValueAtAngles(Angle aAzimuth, Angle aElevation) const;

private:
   friend class PatternDataIterator;
   friend class PatternDataAllIterator;
   friend class PatternDataAzimuthIterator;
   friend class PatternDataElevationIterator;

   Angle GetAzimuthAngle(Iterator aIterator) const;
   Angle GetElevationAngle(Iterator aIterator, bool aIsReversed) const;

   Angle GetAzimuthForColumn(IndexType aColumn) const;
   Angle GetElevationForRow(IndexType aRow) const;

   IndexType GetColumnForAzimuth(Angle aAzimuth) const;
   IndexType GetRowForElevation(Angle aElevation) const;

   static IndexType MapAngleToIndex(Angle aAngle, Angle aMinAngle, Angle aMaxAngle, const IndexType& aMaxIndex);

   DataType                  mData;
   float                     mCurrentFrequency = -1.0f;
   WsfStringId               mState;
   WsfEM_Types::Polarization mPolarization;
   WsfEM_Types::InfraredBand mIRBand = WsfEM_Types::InfraredBand::cIRBAND_COUNT;
   float                     mMinDB{std::numeric_limits<float>::max()};
   float                     mMaxDB{-mMinDB};
   float                     mMinDBAllFreqs{std::numeric_limits<float>::max()};
   float                     mMaxDBAllFreqs{-mMinDBAllFreqs};
};
} // namespace PatternVisualizer

#endif
