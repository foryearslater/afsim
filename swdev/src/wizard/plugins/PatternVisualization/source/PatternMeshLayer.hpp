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

#ifndef PATTERNMESHLAYER_HPP
#define PATTERNMESHLAYER_HPP

#include <QPainter>

#include "Canvas.hpp"
#include "CanvasLayer.hpp"
#include "GraphicsEngine.hpp"
#include "Pattern.hpp"
#include "PatternMesh.hpp"

namespace PatternVisualizer
{
class PatternMeshLayer : public CanvasLayer
{
   Q_OBJECT

public:
   PatternMeshLayer(Session*          aSessionPtr,
                    Canvas&           aCanvas,
                    const Pattern&    aPattern,
                    GraphicsEngine&   aGraphicsEngine,
                    const QMatrix4x4& aTransform);
   ~PatternMeshLayer() override = default;

   void Render(QPainter& aPainter, bool aIs3DMode) override;
   void Update() override;
   void Set3DTransform(const QMatrix4x4& aTransform) override;
   void UpdateScale(float aDbMin, float aDbMax, float aDbInc) override;

private:
   GraphicsEngine&              mGraphicsEngine;
   std::unique_ptr<PatternMesh> mMeshPtr;
};
} // namespace PatternVisualizer

#endif
