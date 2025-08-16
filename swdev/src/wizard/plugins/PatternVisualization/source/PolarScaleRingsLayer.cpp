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

#include "PolarScaleRingsLayer.hpp"

#include "Angle.hpp"
#include "GlobalPlotOptions.hpp"
#include "MaterialColors.hpp"
#include "PolarPlotOptions.hpp"
#include "Session.hpp"
#include "UtMath.hpp"

namespace PatternVisualizer
{
namespace
{
constexpr Angle gMarksStepAngle    = Angle::FromRadians(UtMath::cTWO_PI / 36.0);
constexpr Angle gMarksStartAngle   = 0_deg;
constexpr float gLabelRadiusFactor = 0.5f;
constexpr int   gLabelBoxSize      = 20;
} // namespace

// static
float PolarScaleRingsLayer::sDistanceBetweenScaleLines = 40;

PolarScaleRingsLayer::PolarScaleRingsLayer(GlobalPlotOptions* aGlobalPlotOptionsPtr,
                                           PolarPlotOptions*  aPolarPlotOptionsPtr,
                                           Canvas&            aCanvas)
   : CanvasLayer(aCanvas)
   , mGlobalPlotOptionsPtr{aGlobalPlotOptionsPtr}
   , mPolarPlotOptionsPtr{aPolarPlotOptionsPtr}
   , mRingPen(QPen(QBrush(), 1.0, Qt::PenStyle::DotLine))
{
}

void PolarScaleRingsLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   // Return if we're in 3D mode, or if there are no scale steps (i.e., minDB >= maxDB)
   if (aIs3DMode)
   {
      return;
   }

   const DataScale& dataScale = mGlobalPlotOptionsPtr->GetDataScale();
   size_t           nsteps    = dataScale.GetNumScaleSteps();
   if (nsteps == 0)
   {
      return;
   }

   // Draw the radial lines
   float deltaDist = PolarScaleRingsLayer::sDistanceBetweenScaleLines * mPolarPlotOptionsPtr->GetZoom2D();
   float radius    = deltaDist * nsteps;
   DrawMarks(aPainter, radius, gMarksStartAngle, gMarksStepAngle);

   // Draw concentric circles for the dB values in the scale
   radius = deltaDist;
   for (const auto& scaleStep : mGlobalPlotOptionsPtr->GetDataScale())
   {
      mRingPen.setColor(scaleStep.mColor);
      DrawRing(aPainter, radius);
      radius += deltaDist;
   }

   // Draw labels for the radials
   DrawLabels(aPainter, radius - gLabelRadiusFactor * deltaDist, gMarksStartAngle, gMarksStepAngle);
}

void PolarScaleRingsLayer::DrawRing(QPainter& aPainter, float aRadius) const
{
   aPainter.setPen(mRingPen);
   aPainter.drawEllipse(QPointF{}, aRadius, aRadius);
}

void PolarScaleRingsLayer::DrawMarks(QPainter& aPainter, float aRingRadius, Angle aStartAngle, Angle aStepAngle) const
{
   aPainter.setPen(QColor(150, 150, 150, 255));

   for (Angle angle = aStartAngle; angle < AnglePi; angle += aStepAngle)
   {
      double x = aRingRadius * cos(angle);
      double y = aRingRadius * sin(angle);
      aPainter.drawLine(-x, -y, x, y);
   }
}

void PolarScaleRingsLayer::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   static int pixelHalfSpan = 320;

   // Return if the min/max dB values are out of sequence
   if (aDbMin >= aDbMax)
   {
      return;
   }

   int nsteps                                       = static_cast<int>(floor((aDbMax - aDbMin) / aDbInc) + 1);
   PolarScaleRingsLayer::sDistanceBetweenScaleLines = pixelHalfSpan / nsteps;
}

void PolarScaleRingsLayer::DrawLabels(QPainter& aPainter, float aRadius, Angle aStartAngle, Angle aStepAngle) const
{
   aPainter.save();

   auto font = aPainter.font();
   font.setPointSize(10);
   font.setBold(true);
   aPainter.setFont(font);
   aPainter.setPen(MaterialColors::Gray500);

   for (auto angle = aStartAngle; angle <= AnglePi; angle += aStepAngle)
   {
      QString text = QString("%1").arg(angle.GetDegrees());
      aPainter.drawText(LabelRect(angle, aRadius), Qt::AlignCenter | Qt::AlignHCenter, text);
   }

   for (auto angle = -aStepAngle; angle > -AnglePi + aStepAngle; angle -= aStepAngle)
   {
      QString text = QString("%1").arg(angle.GetDegrees());
      aPainter.drawText(LabelRect(angle, aRadius), Qt::AlignCenter | Qt::AlignHCenter, text);
   }

   aPainter.restore();
}

QRect PolarScaleRingsLayer::LabelRect(Angle aAngle, float aRadius)
{
   // We use -aAngle here because the painter's coordinate system has a different sense.
   double cosAngle = cos(-aAngle);
   double sinAngle = sin(-aAngle);
   QPoint textSpot(aRadius * cosAngle, aRadius * sinAngle);
   QPoint topLeft(textSpot.x() - gLabelBoxSize, textSpot.y() - gLabelBoxSize);
   QPoint bottomRight(textSpot.x() + gLabelBoxSize, textSpot.y() + gLabelBoxSize);
   return QRect{topLeft, bottomRight};
}
} // namespace PatternVisualizer
