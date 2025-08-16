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

#ifndef POLARSCALERINGSLAYER_HPP
#define POLARSCALERINGSLAYER_HPP

#include <QPainter>
#include <QPen>

#include "Angle.hpp"
#include "Canvas.hpp"
#include "CanvasLayer.hpp"

namespace PatternVisualizer
{
class GlobalPlotOptions;
class PolarPlotOptions;

class PolarScaleRingsLayer : public CanvasLayer
{
   Q_OBJECT
public:
   static float GetDistBetweenScaleLines() { return sDistanceBetweenScaleLines; }

   PolarScaleRingsLayer(GlobalPlotOptions* aGlobalPlotOptionsPtr, PolarPlotOptions* aPolarPlotOptionsPtr, Canvas& aCanvas);

   void Render(QPainter& aPainter, bool aIs3DMode) override;
   void UpdateScale(float aDbMin, float aDbMax, float aDbInc) override;

private:
   void         DrawRing(QPainter& aPainter, float aRadius) const;
   void         DrawMarks(QPainter& aPainter, float aRingRadius, Angle aStartAngle, Angle aStepAngle) const;
   void         DrawLabels(QPainter& aPainter, float aRadius, Angle aStartAngle, Angle aStepAngle) const;
   static QRect LabelRect(Angle aAngle, float aRadius);

   static float sDistanceBetweenScaleLines;

   GlobalPlotOptions* mGlobalPlotOptionsPtr;
   PolarPlotOptions*  mPolarPlotOptionsPtr;
   QPen               mRingPen;
};
} // namespace PatternVisualizer

#endif
