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

#ifndef DATASCALE_HPP
#define DATASCALE_HPP

#include <vector>

#include <QColor>

namespace PatternVisualizer
{
struct DataScaleStep
{
   DataScaleStep() = default;

   DataScaleStep(float aDb, const QColor& aColor)
      : mDB(aDb)
      , mColor(aColor)
   {
   }

   float  mDB;
   QColor mColor;
};

class DataScale
{
public:
   DataScale();

   bool IsManualScaling() const { return mManualScaling; }
   bool IsResettingScale() const { return mResettingScale; }
   void SetResettingScale(bool aValue) { mResettingScale = aValue; }
   void SetManualScaling(bool aValue) { mManualScaling = aValue; }
   bool SetMinDBManual(float aValue);
   bool SetMaxDBManual(float aValue);
   bool SetIncDBManual(float aValue);
   void SetMinMaxDB(float& aMinDB, float& aMaxDB);

   float  GetMinDB() const { return mMinDB; }
   float  GetMaxDB() const { return mMaxDB; }
   float  GetIncDB() const { return mIncDB; }
   size_t GetNumScaleSteps() const { return mSteps.size(); }

   std::vector<DataScaleStep>::const_iterator begin() const { return mSteps.begin(); }
   std::vector<DataScaleStep>::const_iterator end() const { return mSteps.end(); }

   std::vector<DataScaleStep>::const_reverse_iterator rbegin() const { return mSteps.rbegin(); }
   std::vector<DataScaleStep>::const_reverse_iterator rend() const { return mSteps.rend(); }

private:
   void ComputeSteps();
   void SetColorSteps();
   void UpdateSteps();

   std::vector<DataScaleStep> mSteps;
   float                      mMinDB;
   float                      mMaxDB;
   float                      mIncDB;
   bool                       mManualScaling  = false;
   bool                       mResettingScale = false;

   static std::vector<std::pair<int, int>> sDBIncAndRangeCutOff;
};
} // namespace PatternVisualizer

#endif
