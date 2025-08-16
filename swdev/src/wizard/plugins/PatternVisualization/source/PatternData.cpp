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

#include "PatternData.hpp"

#include "Angle.hpp"
#include "PatternDataAllIterator.hpp"
#include "PatternDataIterator.hpp"
#include "UtMath.hpp"
#include "WsfMilRadarSignature.hpp"
#include "WsfStandardAcousticSignature.hpp"
#include "WsfStandardInfraredSignature.hpp"

namespace PatternVisualizer
{
namespace
{
const uint32_t NumAzimuthReadings   = 360 * 2;
const uint32_t NumElevationReadings = 180 * 2;

const Angle StartAzimuth   = -AnglePi;
const Angle EndAzimuth     = AnglePi;
const Angle StartElevation = AnglePiOverTwo;
const Angle EndElevation   = -AnglePiOverTwo;
} // namespace

PatternData::PatternData(WsfAntennaPattern& aWsfPattern, float aFreq)
   : mData(NumElevationReadings, NumAzimuthReadings)
   , mState("")                                             // not used
   , mPolarization(WsfEM_Types::Polarization::cPOL_DEFAULT) // not used
{
   // Extract the gains for the requested frequency
   UpdateGainTable(aWsfPattern, aFreq);

   // Determine the min/max DB gains over all frequencies
   double             peakGain   = 0;
   TblIndVarU<double> tableFreqs = aWsfPattern.GetGainAdjustmentTable().mFrequency;
   size_t             nfreqs     = tableFreqs.GetSize();
   if (nfreqs > 0)
   {
      for (size_t i = 0; i < nfreqs; ++i)
      {
         double tableFreq = std::pow(10.0, tableFreqs.Get(i));
         peakGain         = std::max(peakGain, aWsfPattern.GetPeakGain(tableFreq));
      }
   }
   else
   {
      peakGain = aWsfPattern.GetPeakGain(1);
   }

   mMinDBAllFreqs = static_cast<float>(UtMath::SafeLinearToDB(aWsfPattern.GetMinimumGain()));
   mMaxDBAllFreqs = static_cast<float>(UtMath::SafeLinearToDB(peakGain));
}

PatternData::PatternData(WsfRadarSignature& aWsfPattern, WsfStringId aState, WsfEM_Types::Polarization aPolarization, float aFreq)
   : mData(NumElevationReadings, NumAzimuthReadings)
   , mState(aState)
   , mPolarization(aPolarization)
{
   // Extract the gains for the requested state, polarization, and frequency
   UpdateGainTable(aWsfPattern, aFreq);

   auto minmax_gain = aWsfPattern.GetSignatureLimits(mState, mPolarization);
   mMinDBAllFreqs   = static_cast<float>(UtMath::SafeLinearToDB(minmax_gain.first));
   mMaxDBAllFreqs   = static_cast<float>(UtMath::SafeLinearToDB(minmax_gain.second));
}

PatternData::PatternData(WsfOpticalSignature& aWsfPattern, WsfStringId aState)
   : mData(NumElevationReadings, NumAzimuthReadings)
   , mState(aState)
   , mPolarization(WsfEM_Types::Polarization::cPOL_DEFAULT) // not used
{
   // Extract the gains for the requested state
   UpdateGainTable(aWsfPattern);

   // Optical patterns are not frequency dependent, so the overall min/max DB values
   // are already computed
   mMinDBAllFreqs = mMinDB;
   mMaxDBAllFreqs = mMaxDB;
}

PatternData::PatternData(WsfInfraredSignature& aWsfPattern, WsfStringId aState, WsfEM_Types::InfraredBand aIRBand)
   : mData(NumElevationReadings, NumAzimuthReadings)
   , mState(aState)
   , mPolarization(WsfEM_Types::Polarization::cPOL_DEFAULT) // not used
{
   // Extract the gains for the requested state and IR band
   UpdateGainTable(aWsfPattern, aIRBand);
   mIRBand = aIRBand;

   // Determine the min/max DB gains over all IR bands for the requested state
   double minSig  = std::numeric_limits<double>::max();
   double maxSig  = std::numeric_limits<double>::min();
   double simTime = 0;
   for (int iband = 0; iband < WsfEM_Types::InfraredBand::cIRBAND_COUNT; iband++)
   {
      for (int az = 0; az < 360; az += 5)
      {
         double azRad = az * UtMath::cRAD_PER_DEG;
         for (int el = -90; el <= 90; el += 5)
         {
            double       elRad = el * UtMath::cRAD_PER_DEG;
            const double gain  = static_cast<double>(
               aWsfPattern.GetSignature(simTime, mState, (WsfEM_Types::InfraredBand)iband, azRad, elRad));
            minSig = std::min(minSig, gain);
            maxSig = std::max(maxSig, gain);
         }
      }
   }

   mMinDBAllFreqs = static_cast<float>(UtMath::SafeLinearToDB(minSig));
   mMaxDBAllFreqs = static_cast<float>(UtMath::SafeLinearToDB(maxSig));
}

PatternData::PatternData(WsfAcousticSignature& aWsfPattern, WsfStringId aState, float aFreq)
   : mData(NumElevationReadings, NumAzimuthReadings)
   , mState(aState)
   , mPolarization(WsfEM_Types::Polarization::cPOL_DEFAULT) // not used
{
   // Extract the gains for the requested state and frequency
   UpdateGainTable(aWsfPattern, aFreq);

   // Determine the min/max DB gains over all frequencies for the requested state
   auto minmax_absolute = aWsfPattern.GetNoisePressureLimits(mState);
   mMinDBAllFreqs       = static_cast<float>(UtMath::SafeLinearToDB(minmax_absolute.first));
   mMaxDBAllFreqs       = static_cast<float>(UtMath::SafeLinearToDB(minmax_absolute.second));
}

bool PatternData::UpdateGainTable(WsfAntennaPattern& aWsfPattern, float aFreq)
{
   if (mCurrentFrequency == aFreq)
   {
      // Requested frequency is already used in the the current gain table
      return false;
   }
   mCurrentFrequency = aFreq;

   // Extract and store the pattern gains for the requested frequency, and compute the
   // min/max gains of this pattern
   mMinDB = std::numeric_limits<float>::max();
   mMaxDB = -mMinDB;
   for (uint32_t row = 0u; row < mData.GetNumRows(); ++row)
   {
      const Angle elevation = GetElevationForRow(row);

      for (uint32_t column = 0u; column < mData.GetNumColumns(); ++column)
      {
         const Angle  azimuth = GetAzimuthForColumn(column);
         const double gain    = aWsfPattern.GetGain(aFreq, azimuth.GetRadians(), elevation.GetRadians(), 0, 0);
         const float  db      = static_cast<float>(UtMath::SafeLinearToDB(gain));

         mMinDB             = std::min(mMinDB, db);
         mMaxDB             = std::max(mMaxDB, db);
         mData(row, column) = db;
      }
   }

   return true;
}

bool PatternData::UpdateGainTable(WsfRadarSignature& aWsfPattern, float aFreq)
{
   if (mCurrentFrequency == aFreq)
   {
      // Requested frequency is already used in the the current gain table
      return false;
   }
   mCurrentFrequency = aFreq;

   // Extract and store the pattern gains for the requested frequency, and compute the
   // min/max gains of this pattern
   mMinDB = std::numeric_limits<float>::max();
   mMaxDB = -mMinDB;
   for (uint32_t row = 0u; row < mData.GetNumRows(); ++row)
   {
      const double elevation = GetElevationForRow(row).GetRadians();

      for (uint32_t column = 0u; column < mData.GetNumColumns(); ++column)
      {
         const double azimuth = GetAzimuthForColumn(column).GetRadians();
         const float  gain =
            aWsfPattern.GetSignature(mState, mPolarization, aFreq, azimuth, elevation, azimuth, elevation, nullptr, nullptr);
         const float db = static_cast<float>(UtMath::SafeLinearToDB(gain));

         mMinDB             = std::min(mMinDB, db);
         mMaxDB             = std::max(mMaxDB, db);
         mData(row, column) = db;
      }
   }

   return true;
}

bool PatternData::UpdateGainTable(WsfOpticalSignature& aWsfPattern, float aFreq)
{
   if (mMinDBAllFreqs < mMaxDBAllFreqs)
   {
      // Gain table is already extracted
      return false;
   }

   // Extract and store the pattern gains for the requested state, and compute the
   // min/max gains of this pattern
   mMinDB = std::numeric_limits<float>::max();
   mMaxDB = -mMinDB;
   for (uint32_t row = 0u; row < mData.GetNumRows(); ++row)
   {
      const double elevation = GetElevationForRow(row).GetRadians();

      for (uint32_t column = 0u; column < mData.GetNumColumns(); ++column)
      {
         double       simTime = 0;
         const double azimuth = GetAzimuthForColumn(column).GetRadians();
         const float  gain    = aWsfPattern.GetSignature(simTime, mState, azimuth, elevation);
         const float  db      = static_cast<float>(UtMath::SafeLinearToDB(gain));

         mMinDB             = std::min(mMinDB, db);
         mMaxDB             = std::max(mMaxDB, db);
         mData(row, column) = db;
      }
   }

   return true;
}

bool PatternData::UpdateGainTable(WsfInfraredSignature& aWsfPattern, WsfEM_Types::InfraredBand aIRBand)
{
   if (aIRBand == mIRBand)
   {
      // Current gain table already reflects the requested IR band
      return false;
   }

   // Extract and store the pattern gains for the requested IR band, and compute the
   // min/max gains of this pattern
   mMinDB = std::numeric_limits<float>::max();
   mMaxDB = -mMinDB;
   for (uint32_t row = 0u; row < mData.GetNumRows(); ++row)
   {
      const double elevation = GetElevationForRow(row).GetRadians();

      for (uint32_t column = 0u; column < mData.GetNumColumns(); ++column)
      {
         double       simTime = 0;
         const double azimuth = GetAzimuthForColumn(column).GetRadians();
         const float  gain    = aWsfPattern.GetSignature(simTime, mState, aIRBand, azimuth, elevation);
         const float  db      = static_cast<float>(UtMath::SafeLinearToDB(gain));

         mMinDB             = std::min(mMinDB, db);
         mMaxDB             = std::max(mMaxDB, db);
         mData(row, column) = db;
      }
   }

   return true;
}

bool PatternData::UpdateGainTable(WsfAcousticSignature& aWsfPattern, float aFreq)
{
   if (mCurrentFrequency == aFreq)
   {
      // Requested frequency is already used in the the current gain table
      return false;
   }
   mCurrentFrequency = aFreq;

   // Extract and store the pattern gains for the requested state, and compute the
   // min/max gains of this pattern
   mMinDB = std::numeric_limits<float>::max();
   mMaxDB = -mMinDB;
   for (uint32_t row = 0u; row < mData.GetNumRows(); ++row)
   {
      const double elevation = GetElevationForRow(row).GetRadians();

      for (uint32_t column = 0u; column < mData.GetNumColumns(); ++column)
      {
         const double azimuth = GetAzimuthForColumn(column).GetRadians();
         const double gain    = aWsfPattern.GetNoisePressure(mState, aFreq, azimuth, elevation);
         const float  db      = static_cast<float>(UtMath::SafeLinearToDB(gain));

         mMinDB             = std::min(mMinDB, db);
         mMaxDB             = std::max(mMaxDB, db);
         mData(row, column) = db;
      }
   }

   return true;
}

PatternDataAllIterator PatternData::EnumerateAllBegin() const
{
   return PatternDataAllIterator{this, mData.begin()};
}

PatternDataAllIterator PatternData::EnumerateAllEnd() const
{
   return PatternDataAllIterator{this, mData.end()};
}

PatternDataIterator PatternData::EnumerateAzimuthsPolarBegin(Angle aConstantElevation) const
{
   uint32_t row = GetRowForElevation(aConstantElevation);
   return PatternDataIterator(this, mData.RowBegin(row));
}

PatternDataIterator PatternData::EnumerateAzimuthsPolarEnd(Angle aConstantElevation) const
{
   uint32_t row = GetRowForElevation(aConstantElevation);
   return PatternDataIterator(this, mData.RowEnd(row));
}

PatternDataIterator PatternData::EnumerateElevationsPolarBegin(Angle aConstantAzimuth) const
{
   uint32_t firstColumn  = GetColumnForAzimuth(aConstantAzimuth);
   uint32_t secondColumn = GetColumnForAzimuth(aConstantAzimuth - AnglePi);

   return PatternDataIterator{this,
                              mData.ColumnBegin(firstColumn),
                              mData.ColumnEnd(firstColumn),
                              mData.ColumnReverseBegin(secondColumn),
                              mData.ColumnReverseEnd(secondColumn)};
}

PatternDataIterator PatternData::EnumerateElevationsPolarEnd(Angle aConstantAzimuth) const
{
   uint32_t firstColumn  = GetColumnForAzimuth(aConstantAzimuth);
   uint32_t secondColumn = GetColumnForAzimuth(aConstantAzimuth - AnglePi);

   DataType::Iterator firstColumnEnd  = mData.ColumnEnd(firstColumn);
   DataType::Iterator secondColumnEnd = mData.ColumnEnd(secondColumn);

   return PatternDataIterator(this, firstColumnEnd, firstColumnEnd, secondColumnEnd, secondColumnEnd);
}

float PatternData::GetValueAtAngles(Angle aAzimuth, Angle aElevation) const
{
   IndexType colLow  = GetColumnForAzimuth(aAzimuth);
   IndexType colHigh = colLow + 1;
   if (colHigh >= mData.GetNumColumns())
   {
      // Azimuth wraps around
      colHigh = 0;
   }

   IndexType rowLow  = GetRowForElevation(aElevation);
   IndexType rowHigh = rowLow + 1;
   if (rowHigh >= mData.GetNumRows())
   {
      // This would be getting exactly the upper limit
      rowHigh = rowLow;
   }

   // if we have two rows
   float midRowLow;
   float midRowHigh;
   Angle azLow  = GetAzimuthForColumn(colLow);
   Angle azHigh = GetAzimuthForColumn(colHigh);
   if (rowHigh != rowLow)
   {
      // two interps needed
      Angle elLow  = GetElevationForRow(rowLow);
      Angle elHigh = GetElevationForRow(rowHigh);

      float lowLow   = mData(rowLow, colLow);
      float lowHigh  = mData(rowLow, colHigh);
      float highLow  = mData(rowHigh, colLow);
      float highHigh = mData(rowHigh, colHigh);

      midRowLow = static_cast<float>(
         UtMath::Lerp(aElevation.GetRadians(), elLow.GetRadians(), elHigh.GetRadians(), lowLow, highLow));
      midRowHigh = static_cast<float>(
         UtMath::Lerp(aElevation.GetRadians(), elLow.GetRadians(), elHigh.GetRadians(), lowHigh, highHigh));
   }
   else
   {
      // no interp needed in elevation
      midRowLow  = mData(rowLow, colLow);
      midRowHigh = mData(rowLow, colHigh);
   }

   // now the final interp across azimuth
   return static_cast<float>(
      UtMath::Lerp(aAzimuth.GetRadians(), azLow.GetRadians(), azHigh.GetRadians(), midRowLow, midRowHigh));
}

Angle PatternData::GetAzimuthAngle(Iterator aIterator) const
{
   DataType::Location location = mData.GetLocation(aIterator);
   return GetAzimuthForColumn(location.mColumn);
}

Angle PatternData::GetElevationAngle(Iterator aIterator, bool aIsReversed) const
{
   DataType::Location location = mData.GetLocation(aIterator);
   Angle              angle    = GetElevationForRow(location.mRow);
   if (aIsReversed)
   {
      angle = LerpAngle(angle, StartElevation, EndElevation, EndElevation, StartElevation) + AnglePi;
   }
   return angle;
}

Angle PatternData::GetAzimuthForColumn(IndexType aColumn) const
{
   // Remember that the max column index is equal to EndAzimuth because how this
   // class handles wrapping.
   return LerpAngle(aColumn, 0u, mData.GetNumColumns(), StartAzimuth, EndAzimuth);
}

Angle PatternData::GetElevationForRow(IndexType aRow) const
{
   return LerpAngle(aRow, 0u, mData.GetNumRows() - 1, StartElevation, EndElevation);
}

uint32_t PatternData::GetColumnForAzimuth(Angle aAzimuth) const
{
   // Remember to add one to the max column to account for the fact that the rightmost
   // column in the dataset is just before the EndAzimuth column.
   uint32_t numColumns = mData.GetNumColumns();
   uint32_t index      = MapAngleToIndex(aAzimuth, StartAzimuth, EndAzimuth, numColumns + 1);
   assert(index < numColumns);
   return index;
}

uint32_t PatternData::GetRowForElevation(Angle aElevation) const
{
   return MapAngleToIndex(aElevation, StartElevation, EndElevation, mData.GetNumRows());
}

uint32_t PatternData::MapAngleToIndex(Angle aAngle, Angle aMinAngle, Angle aMaxAngle, const IndexType& aMaxIndex)
{
   aAngle.Normalize(aMinAngle, aMaxAngle);

   double t = UtMath::Lerp(aAngle.GetRadians(), aMinAngle.GetRadians(), aMaxAngle.GetRadians(), 0.0, aMaxIndex - 1);

   IndexType index = static_cast<IndexType>(t);

   assert(index < aMaxIndex);
   return index;
}
} // namespace PatternVisualizer
