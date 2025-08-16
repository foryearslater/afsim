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

#ifndef PATTERNPOLARPLOTLAYER_HPP
#define PATTERNPOLARPLOTLAYER_HPP

#include <QPen>

#include "CanvasLayer.hpp"
#include "Pattern.hpp"

namespace PatternVisualizer
{
class GlobalPlotOptions;
class PolarPlotOptions;
class PolarCoordinate;
class Session;

class PatternPolarPlotLayer : public CanvasLayer
{
   Q_OBJECT

public:
   PatternPolarPlotLayer(Session*           aSessionPtr,
                         GlobalPlotOptions* aGlobalPlotOptionsPtr,
                         PolarPlotOptions*  aPolarPlotOptionsPtr,
                         Canvas&            aCanvas,
                         const Pattern&     aPattern);
   ~PatternPolarPlotLayer() override = default;

   void Render(QPainter& aPainter, bool aIs3DMode) override;
   void SetStale() override { mIsPlotPathStale = true; }

private:
   void BuildConstantAzimuthPlotPath();
   void BuildConstantElevationPlotPath();
   void BuildPlot(PatternDataIterator aStart, PatternDataIterator aEnd);

   Session*           mSessionPtr;
   GlobalPlotOptions* mGlobalPlotOptionsPtr;
   PolarPlotOptions*  mPolarPlotOptionsPtr;
   QPainterPath       mPlotPath;
   QPen               mPen;
   bool               mIsPlotPathStale;
};
} // namespace PatternVisualizer

#endif
