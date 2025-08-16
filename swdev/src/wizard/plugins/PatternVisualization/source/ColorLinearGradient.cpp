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

#include "ColorLinearGradient.hpp"

#include <cassert>
#include <stdexcept>

#include "UtMath.hpp"

namespace PatternVisualizer
{
void ColorLinearGradient::AddStop(float aValue, const QColor& aColor)
{
   const Stop stop(aValue, aColor);

   if (binary_search(mStops.begin(), mStops.end(), stop))
   {
      throw std::invalid_argument("value already exists in the linear gradient");
   }

   const auto i = upper_bound(mStops.begin(), mStops.end(), stop);
   mStops.insert(i, stop);
}

QColor ColorLinearGradient::GetColor(float aValue) const
{
   const auto i = upper_bound(mStops.begin(), mStops.end(), Stop(aValue, QColor()));

   Stop leftStop;
   Stop rightStop;

   if (i == mStops.end())
   {
      leftStop  = *(i - 1);
      rightStop = leftStop;
   }
   else if (i == mStops.begin())
   {
      leftStop  = *i;
      rightStop = *i;
   }
   else
   {
      leftStop  = *(i - 1);
      rightStop = *i;
   }

   assert(leftStop.mValue <= rightStop.mValue);

   const double t = UtMath::Lerp(aValue, leftStop.mValue, rightStop.mValue, 0.0, 1.0);

   if (t <= 0.0)
   {
      return leftStop.mColor;
   }
   if (t >= 1.0)
   {
      return rightStop.mColor;
   }

   const QColor fromColor = leftStop.mColor;
   const QColor toColor   = rightStop.mColor;

   const double r = UtMath::Lerp(t, 0.0, 1.0, fromColor.redF(), toColor.redF());
   const double g = UtMath::Lerp(t, 0.0, 1.0, fromColor.greenF(), toColor.greenF());
   const double b = UtMath::Lerp(t, 0.0, 1.0, fromColor.blueF(), toColor.blueF());
   const double a = UtMath::Lerp(t, 0.0, 1.0, fromColor.alphaF(), toColor.alphaF());

   return QColor::fromRgbF(r, g, b, a);
}
} // namespace PatternVisualizer
