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

#ifndef CANVAS3D_HPP
#define CANVAS3D_HPP

#include <memory>

#include <QMatrix4x4>
#include <QOpenGLDebugMessage>

#include "Session.hpp"
#include "ShaderLocator.hpp"
#include "UtQtOpenGLWidget.hpp"

namespace PatternVisualizer
{
class CanvasLayer;
class Camera;
class GlobalPlotOptions;
class GraphicsEngine;
class LegendLayer;
class LocatorMeshLayer;
class Pattern;
class Session;
class PolarScaleRingsLayer;
class PolarPlotOptions;
class ValueAtCursorLayer;
enum class PlotStyle;

class Canvas : public QOpenGLWidget
{
   Q_OBJECT

public:
   Canvas(Session* aSessionPtr, QWidget* aParentPtr = nullptr);
   ~Canvas() override;

   bool AddOrUpdatePattern(Pattern& aPattern, bool aChanged);
   void RemovePattern(Pattern& aPattern);

public slots:
   void        UpdateScale(float aDbMin, float aDbMax, float aDbInc);
   void        CleanupGL();
   static void OnDebugMessageLogged(const QOpenGLDebugMessage& aDebugMessage);
   void        OnOriginOffsetChanged(QPoint aNewValue);
   void        OnPlotStyleChanged(PlotStyle aPlotStyle);
   void        Update2DPlot();
   void        Update3DProjection();

private:
   void initializeGL() override;
   void leaveEvent(QEvent* aEvent) override;
   void paintGL() override;
   void resizeGL(int aWidth, int aHeight) override;
   void wheelEvent(QWheelEvent* aEvent) override;
   void mousePressEvent(QMouseEvent* aEvent) override;
   void mouseReleaseEvent(QMouseEvent* aEvent) override;
   void mouseMoveEvent(QMouseEvent* aEvent) override;

   QMatrix4x4 Compute3DTransform(int aWidth, int aHeight);
   void       PanPlot2D(QPoint aDeltaPosition);
   void       ValidateContext();

   Session*                                  mSessionPtr;
   std::unique_ptr<GraphicsEngine>           mGraphicsEnginePtr;
   std::vector<std::unique_ptr<CanvasLayer>> mLayers;
   LegendLayer*                              mLegendLayerPtr;
   PolarScaleRingsLayer*                     mPolarScaleRingsLayerPtr;
   ValueAtCursorLayer*                       mValueAtCursorLayerPtr;
   LocatorMeshLayer*                         mLocatorMeshLayerPtr;
   QOpenGLDebugLogger*                       mLoggerPtr;
   bool                                      mIsPanning;
   bool                                      mLastMouseEventShiftPressed;
   Camera*                                   mCameraPtr;
   QPoint                                    mPanningPreviousLocation;
   QColor                                    mBGColor;
   QMatrix4x4                                mTransform3D;
   GlobalPlotOptions*                        mGlobalPlotOptionsPtr;
   PolarPlotOptions*                         mPolarPlotOptionsPtr;
};
} // namespace PatternVisualizer

#endif
