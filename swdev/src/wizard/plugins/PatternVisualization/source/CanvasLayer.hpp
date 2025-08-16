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

#ifndef CANVASLAYER_HPP
#define CANVASLAYER_HPP

#include <QMatrix4x4>
#include <QPainter>

#include "Canvas.hpp"
#include "UtQtOpenGLWidget.hpp"

namespace PatternVisualizer
{
class Pattern;

class CanvasLayer : public QObject
{
   Q_OBJECT
public:
   explicit CanvasLayer(Canvas& aCanvas, const Pattern* aPatternPtr = nullptr)
      : QObject(&aCanvas)
      , mCanvas(aCanvas)
      , mPatternPtr(aPatternPtr)
   {
   }

   ~CanvasLayer() override = default;

   virtual void Render(QPainter& aPainter, bool aIs3DMode) = 0;
   virtual void Resize(int aWidth, int aHeight, const QMatrix4x4& aTransform3D) {}
   virtual void SetStale() {}
   virtual void Set3DTransform(const QMatrix4x4& aTransform3D) {}
   virtual void Update() {}
   virtual void UpdateScale(float aDbMin, float aDbMax, float aDbInc) {}

   bool IsForPattern(const Pattern* aPatternPtr) { return aPatternPtr == mPatternPtr; }
   bool IsPatternLayer() { return (mPatternPtr != nullptr); }

protected:
   Canvas&        mCanvas;
   const Pattern* mPatternPtr;
};
} // namespace PatternVisualizer

#endif
