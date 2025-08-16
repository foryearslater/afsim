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

#ifndef COLORLINEARGRADIENT_HPP
#define COLORLINEARGRADIENT_HPP

#include <vector>

#include <QColor>

namespace PatternVisualizer
{
class ColorLinearGradient
{
public:
   struct Stop
   {
      Stop() = default;
      Stop(float aValue, const QColor& aColor)
         : mValue{aValue}
         , mColor{aColor}
      {
      }

      friend bool operator<(const Stop& a, const Stop& b) { return a.mValue < b.mValue; }

      float  mValue{};
      QColor mColor{};
   };

   void AddStop(float aValue, const QColor& aColor);

   QColor GetColor(float aValue) const;
   size_t GetNumStops() const { return mStops.size(); }
   Stop   GetStop(size_t aIndex) const { return mStops[aIndex]; }

   std::vector<Stop>::const_iterator begin() { return mStops.begin(); }
   std::vector<Stop>::const_iterator end() { return mStops.end(); }

private:
   std::vector<Stop> mStops;
};
} // namespace PatternVisualizer

#endif
