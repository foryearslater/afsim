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

#ifndef PATTERNGROUPLAYER_HPP
#define PATTERNGROUPLAYER_HPP

#include "PatternMeshLayer.hpp"
#include "PatternPolarPlotLayer.hpp"

namespace PatternVisualizer
{
class GlobalPlotOptions;
class GraphicsEngine;
class PolarPlotOptions;

class PatternGroupLayer : public CanvasLayer
{
   Q_OBJECT

public:
   PatternGroupLayer(Session*           aSessionPtr,
                     GlobalPlotOptions* aGlobalPlotOptionsPtr,
                     PolarPlotOptions*  aPolarPlotOptionsPtr,
                     Canvas&            aCanvas,
                     Pattern&           aPattern,
                     GraphicsEngine*    aGraphicsEnginePtr,
                     const QMatrix4x4&  aTransform3D);
   ~PatternGroupLayer() override = default;

   bool IsGroupForPattern(const Pattern* aPatternPtr) const { return aPatternPtr == mPatternPtr; }
   void Render(QPainter& aPainter, bool aIs3DMode) override;
   void SetStale() override;
   void Set3DTransform(const QMatrix4x4& aTransform3D) override;
   void Update() override;
   void Update2DPlot();
   void UpdateScale(float aDbMin, float aDbMax, float aDbInc) override;

private:
   std::unique_ptr<PatternPolarPlotLayer> mLayer2DPtr;
   std::unique_ptr<PatternMeshLayer>      mLayer3DPtr;
};
} // namespace PatternVisualizer

#endif
