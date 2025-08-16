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

#include "PatternGroupLayer.hpp"

#include "GlobalPlotOptions.hpp"
#include "GraphicsEngine.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{
PatternGroupLayer::PatternGroupLayer(Session*           aSessionPtr,
                                     GlobalPlotOptions* aGlobalPlotOptionsPtr,
                                     PolarPlotOptions*  aPolarPlotOptionsPtr,
                                     Canvas&            aCanvas,
                                     Pattern&           aPattern,
                                     GraphicsEngine*    aGraphicsEnginePtr,
                                     const QMatrix4x4&  aTransform3D)
   : CanvasLayer(aCanvas, &aPattern)
   , mLayer2DPtr{new PatternPolarPlotLayer{aSessionPtr, aGlobalPlotOptionsPtr, aPolarPlotOptionsPtr, aCanvas, aPattern}}
   , mLayer3DPtr{new PatternMeshLayer{aSessionPtr, aCanvas, aPattern, *aGraphicsEnginePtr, aTransform3D}}
{
}

void PatternGroupLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   if (aIs3DMode)
   {
      mLayer3DPtr->Render(aPainter, aIs3DMode);
   }
   else
   {
      mLayer2DPtr->Render(aPainter, aIs3DMode);
   }
}

void PatternGroupLayer::SetStale()
{
   mLayer2DPtr->SetStale();
}

void PatternGroupLayer::Update()
{
   mLayer2DPtr->SetStale();
   mLayer3DPtr->Update();
}

void PatternGroupLayer::Update2DPlot()
{
   mLayer2DPtr->SetStale();
}

void PatternGroupLayer::Set3DTransform(const QMatrix4x4& aTransform3D)
{
   mLayer3DPtr->Set3DTransform(aTransform3D);
}

void PatternGroupLayer::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   mLayer2DPtr->SetStale();
   mLayer3DPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
}
} // namespace PatternVisualizer
