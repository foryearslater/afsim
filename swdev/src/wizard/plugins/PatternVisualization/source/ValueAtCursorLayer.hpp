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

#ifndef VALUEATCURSORLAYER_HPP
#define VALUEATCURSORLAYER_HPP

#include "Canvas.hpp"
#include "CanvasLayer.hpp"

namespace PatternVisualizer
{
class PolarPlotOptions;
class Session;

class ValueAtCursorLayer : public CanvasLayer
{
   Q_OBJECT

public:
   explicit ValueAtCursorLayer(Session* aSessionPtr, PolarPlotOptions* aPolarPlotOptionsPtr, Canvas& aCanvas)
      : CanvasLayer(aCanvas)
      , mSessionPtr{aSessionPtr}
      , mPolarPlotOptionsPtr{aPolarPlotOptionsPtr}
   {
   }

   void Render(QPainter& aPainter, bool aIs3DMode) override;

private:
   Session*          mSessionPtr;
   PolarPlotOptions* mPolarPlotOptionsPtr;
};
} // namespace PatternVisualizer

#endif
