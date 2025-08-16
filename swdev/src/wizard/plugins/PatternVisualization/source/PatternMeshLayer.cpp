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

#include "PatternMeshLayer.hpp"

#include "Pattern.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{
PatternMeshLayer::PatternMeshLayer(Session*          aSessionPtr,
                                   Canvas&           aCanvas,
                                   const Pattern&    aPattern,
                                   GraphicsEngine&   aGraphicsEngine,
                                   const QMatrix4x4& aTransform)
   : CanvasLayer(aCanvas, &aPattern)
   , mGraphicsEngine(aGraphicsEngine)
{
   const DataScale& dataScale = aSessionPtr->GetScale();
   float            dbMin     = dataScale.GetMinDB();
   float            dbMax     = dataScale.GetMaxDB();

   mMeshPtr = ut::make_unique<PatternMesh>(aSessionPtr, aPattern, aGraphicsEngine, dbMin, dbMax);
   mMeshPtr->SetTransform(aTransform);
}

void PatternMeshLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   if (aIs3DMode)
   {
      mMeshPtr->Render();
   }
}

void PatternMeshLayer::Set3DTransform(const QMatrix4x4& aTransform)
{
   mMeshPtr->SetTransform(aTransform);
}

void PatternMeshLayer::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   mMeshPtr->UpdateScale(aDbMin, aDbMax);
}

void PatternMeshLayer::Update()
{
   mMeshPtr->FillBuffers();
}
} // namespace PatternVisualizer
