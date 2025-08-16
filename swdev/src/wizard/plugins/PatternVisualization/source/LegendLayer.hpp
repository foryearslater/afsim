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

#ifndef LEGENDLAYER_HPP
#define LEGENDLAYER_HPP

#include <QPainter>
#include <QPen>

#include "Canvas.hpp"
#include "CanvasLayer.hpp"

namespace PatternVisualizer
{
class LegendLayer : public CanvasLayer
{
   Q_OBJECT
public:
   explicit LegendLayer(Session* aSessionPtr, Canvas& aCanvas)
      : CanvasLayer(aCanvas)
      , mSessionPtr{aSessionPtr}
   {
   }

   void Render(QPainter& aPainter, bool aIs3DMode) override;

private:
   Session* mSessionPtr;
   QPen     mLegendPen;
};
} // namespace PatternVisualizer

#endif
