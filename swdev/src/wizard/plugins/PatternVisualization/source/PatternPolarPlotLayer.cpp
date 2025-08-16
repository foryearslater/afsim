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

#include "PatternPolarPlotLayer.hpp"

#include <limits>

#include <QColor>

#include "Debug.hpp"
#include "GlobalPlotOptions.hpp"
#include "Pattern.hpp"
#include "PatternDataIterator.hpp"
#include "PolarCoordinate.hpp"
#include "PolarPlotOptions.hpp"
#include "PolarScaleRingsLayer.hpp"
#include "Session.hpp"
#include "UtException.hpp"
#include "UtMath.hpp"

namespace PatternVisualizer
{
namespace
{
class PlotBuilder
{
public:
   explicit PlotBuilder(PatternDataIterator aStart,
                        PatternDataIterator aEnd,
                        float               aMinDb,
                        float               aMaxDb,
                        float               aMinRadial,
                        float               aMaxRadial)
      : mCurrent(aStart)
      , mEnd(aEnd)
      , mMinDb(aMinDb)
      , mMaxDb(aMaxDb)
      , mMinRadial(aMinRadial)
      , mMaxRadial(aMaxRadial)
      , mXmin{std::numeric_limits<float>::max()}
      , mXmax{std::numeric_limits<float>::min()}
      , mYmin{std::numeric_limits<float>::max()}
      , mYmax{std::numeric_limits<float>::min()}
   {
   }

   QPainterPath Build()
   {
      QPointF point = GetProjectedValue(*mCurrent);
      mXmin = mXmax = point.x();
      mYmin = mYmax = point.y();

      QPainterPath path;
      path.moveTo(point);
      ++mCurrent;

      for (; mCurrent != mEnd; ++mCurrent)
      {
         point = GetProjectedValue(*mCurrent);
         mXmin = std::min(mXmin, static_cast<float>(point.x()));
         mXmax = std::max(mXmax, static_cast<float>(point.x()));
         mYmin = std::min(mYmin, static_cast<float>(point.y()));
         mYmax = std::max(mYmax, static_cast<float>(point.y()));
         path.lineTo(point);
      }

      path.closeSubpath();
      point = path.currentPosition();

      return path;
   }

   QRectF GetRect() { return QRectF(mXmin, mYmin, mXmax - mXmin, mYmax - mYmin); }

private:
   QPointF GetProjectedValue(PolarCoordinate aCoordinate) const
   {
      const float radial    = GetRadial(aCoordinate);
      const Angle azimuth   = aCoordinate.GetAngle();
      const Angle elevation = 0_deg;

      // This is a standard spherical to rectangular coordinate projection. Azimuth
      // is already in the proper range, but the elevation is measured in "upwards"
      // from the x-axis rather than "downwards" from the z-axis. Thus we need
      // pi/2 - elevation for the spherical to rectangular conversion.
      //
      // Also note, that Qt has +y down towards the bottom of the screen, but we
      // want +y upwards. We'll need to negate the y-axis too.
      const Angle  phi          = AnglePiOverTwo - elevation;
      const double sinElevation = sin(phi);
      const double x            = radial * sinElevation * cos(azimuth);
      const double y            = radial * sinElevation * sin(azimuth);

      return QPointF{x, -y};
   }

   float GetRadial(PolarCoordinate aCoordinate) const
   {
      // Special case: when min and max dB are the same, the lerp will divide by zero.
      if (mMinDb == mMaxDb)
      {
         return mMinRadial; // Should never reach this point, but just in case ...
      }

      // Scale the given dB range to the current polar scale rings dimensions.
      float lerpVal = UtMath::Lerp(aCoordinate.GetRadial(), mMinDb, mMaxDb, mMinRadial, mMaxRadial);
      return std::max(mMinRadial, std::min(mMaxRadial, lerpVal));
   }

   PatternDataIterator mCurrent;
   PatternDataIterator mEnd;
   float               mMinDb; // dB value of inner ring
   float               mMaxDb; // dB value of outer ring (<= user-defined max dB)
   float               mMinRadial;
   float               mMaxRadial;

   float mXmin;
   float mXmax;
   float mYmin;
   float mYmax;
};
} // namespace

PatternPolarPlotLayer::PatternPolarPlotLayer(Session*           aSessionPtr,
                                             GlobalPlotOptions* aGlobalPlotOptionsPtr,
                                             PolarPlotOptions*  aPolarPlotOptionsPtr,
                                             Canvas&            aCanvas,
                                             const Pattern&     aPattern)
   : CanvasLayer(aCanvas, &aPattern)
   , mSessionPtr{aSessionPtr}
   , mGlobalPlotOptionsPtr{aGlobalPlotOptionsPtr}
   , mPolarPlotOptionsPtr{aPolarPlotOptionsPtr}
   , mPen(Qt::black)
   , mIsPlotPathStale(true)
{
   mPen.setWidth(2);
}

void PatternPolarPlotLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   if (mIsPlotPathStale)
   {
      switch (mGlobalPlotOptionsPtr->GetPlotStyle())
      {
      case PlotStyle::PolarConstantAzimuth:
         BuildConstantAzimuthPlotPath();
         break;
      case PlotStyle::PolarConstantElevation:
         BuildConstantElevationPlotPath();
         break;
      default:
         return;
      }
   }

   aPainter.setPen(mPen);
   aPainter.drawPath(mPlotPath);
}

void PatternPolarPlotLayer::BuildConstantAzimuthPlotPath()
{
   PatternData* patternDataPtr = mPatternPtr->GetPatternData();
   if (patternDataPtr)
   {
      const Angle         azimuth = mPolarPlotOptionsPtr->GetConstantAzimuth();
      PatternDataIterator start   = patternDataPtr->EnumerateElevationsPolarBegin(azimuth);
      PatternDataIterator end     = patternDataPtr->EnumerateElevationsPolarEnd(azimuth);

      BuildPlot(start, end);
   }
}

void PatternPolarPlotLayer::BuildConstantElevationPlotPath()
{
   PatternData* patternDataPtr = mPatternPtr->GetPatternData();
   if (patternDataPtr)
   {
      const Angle         elevation = mPolarPlotOptionsPtr->GetConstantElevation();
      PatternDataIterator start     = patternDataPtr->EnumerateAzimuthsPolarBegin(elevation);
      PatternDataIterator end       = patternDataPtr->EnumerateAzimuthsPolarEnd(elevation);

      BuildPlot(start, end);
   }
}

void PatternPolarPlotLayer::BuildPlot(PatternDataIterator aStart, PatternDataIterator aEnd)
{
   const DataScale& scale  = mSessionPtr->GetScale();
   size_t           nsteps = scale.GetNumScaleSteps();
   if (nsteps == 0)
   {
      return;
   }

   // PlotBuilder is a class that exists in a single stack frame, thus these values from
   // Session cannot change during this class's lifetime.
   float       zoom      = mPolarPlotOptionsPtr->GetZoom2D();
   float       minDB     = scale.GetMinDB();
   float       maxDB     = minDB + (nsteps - 1) * scale.GetIncDB();
   float       minRadial = PolarScaleRingsLayer::GetDistBetweenScaleLines() * zoom;
   float       maxRadial = minRadial * nsteps;
   PlotBuilder builder(aStart, aEnd, minDB, maxDB, minRadial, maxRadial);
   mPlotPath        = builder.Build();
   mIsPlotPathStale = false;
}
} // namespace PatternVisualizer
