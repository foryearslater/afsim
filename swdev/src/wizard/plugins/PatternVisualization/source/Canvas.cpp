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

#include "Canvas.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPainter>
#include <QWheelEvent>

#include "Clamp.hpp"
#include "GraphicsEngine.hpp"
#include "LegendLayer.hpp"
#include "LocatorMeshLayer.hpp"
#include "MaterialColors.hpp"
#include "PatternGroupLayer.hpp"
#include "PatternMeshLayer.hpp"
#include "PatternPolarPlotLayer.hpp"
#include "PolarPlotOptions.hpp"
#include "PolarScaleRingsLayer.hpp"
#include "Session.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "ValueAtCursorLayer.hpp"

namespace PatternVisualizer
{
namespace
{
// Used in the perspective transformation
constexpr float cNEAR_PLANE    = 0.1f;
constexpr float cFAR_PLANE     = 100.0f;
constexpr float cFIELD_OF_VIEW = 45.0f;
} // namespace

Canvas::Canvas(Session* aSessionPtr, QWidget* aParentPtr)
   : QOpenGLWidget(aParentPtr)
   , mSessionPtr(aSessionPtr)
   , mGraphicsEnginePtr{nullptr}
   , mLayers{}
   , mLegendLayerPtr(nullptr)
   , mPolarScaleRingsLayerPtr(nullptr)
   , mValueAtCursorLayerPtr(nullptr)
   , mLocatorMeshLayerPtr(nullptr)
   , mLoggerPtr(nullptr)
   , mIsPanning(false)
   , mLastMouseEventShiftPressed{false}
   , mCameraPtr{aSessionPtr->GetCamera()}
   , mPanningPreviousLocation{}
   , mBGColor(QColor(212, 208, 200))
   , mTransform3D{}
   , mGlobalPlotOptionsPtr{mSessionPtr->GetGlobalPlotOptions()}
   , mPolarPlotOptionsPtr{mSessionPtr->GetPolarPlotOptions()}
{
   setAutoFillBackground(true);
   setMouseTracking(true);

   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::PlotStyleChanged, this, &Canvas::OnPlotStyleChanged);
   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::ScaleChanged, this, &Canvas::UpdateScale);

   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::Zoom2D_Changed, this, &Canvas::Update2DPlot);
   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::OriginOffsetChanged, this, &Canvas::OnOriginOffsetChanged);
   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::ConstantAzimuthChanged, this, &Canvas::Update2DPlot);
   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::ConstantElevationChanged, this, &Canvas::Update2DPlot);

   connect(mCameraPtr, &Camera::CameraRotationChanged, this, &Canvas::Update3DProjection);
   connect(mCameraPtr, &Camera::CameraZoomChanged, this, &Canvas::Update3DProjection);
}

Canvas::~Canvas()
{
   CleanupGL();
   mSessionPtr->SetCanvas(nullptr);
}

bool Canvas::AddOrUpdatePattern(Pattern& aPattern, bool aChanged)
{
   makeCurrent();

   // Update pattern layer if it already exists
   bool isNewPattern = true;
   for (std::vector<std::unique_ptr<CanvasLayer>>::iterator i = mLayers.begin(); i != mLayers.end(); ++i)
   {
      if (i->get()->IsPatternLayer())
      {
         PatternGroupLayer* layerGroup = static_cast<PatternGroupLayer*>(i->get());
         if (layerGroup->IsGroupForPattern(&aPattern))
         {
            layerGroup->Update();
            isNewPattern = false;
            break;
         }
      }
   }

   // Create the pattern layers
   if (isNewPattern)
   {
      mLayers.emplace_back(ut::make_unique<PatternGroupLayer>(mSessionPtr,
                                                              mSessionPtr->GetGlobalPlotOptions(),
                                                              mSessionPtr->GetPolarPlotOptions(),
                                                              *this,
                                                              aPattern,
                                                              mGraphicsEnginePtr.get(),
                                                              mTransform3D));
   }

   doneCurrent();
   update();
   return isNewPattern;
}

void Canvas::CleanupGL()
{
   makeCurrent();

   delete mLoggerPtr;
   mLoggerPtr = nullptr;

   doneCurrent();
}

QMatrix4x4 Canvas::Compute3DTransform(int aWidth, int aHeight)
{
   float aspectRatio = static_cast<float>(aWidth) / aHeight;

   QMatrix4x4 proj;
   proj.setToIdentity();
   proj.perspective(cFIELD_OF_VIEW, aspectRatio, cNEAR_PLANE, cFAR_PLANE);

   QMatrix4x4 transform = proj * mCameraPtr->Transformation();
   return transform;
}

void Canvas::initializeGL()
{
   Q_ASSERT(!mGraphicsEnginePtr);

   ValidateContext();

   QOpenGLContext* glContext = context();
   connect(glContext, &QOpenGLContext::aboutToBeDestroyed, this, &Canvas::CleanupGL);

   mLoggerPtr = new QOpenGLDebugLogger(this);
   mLoggerPtr->initialize();
   connect(mLoggerPtr, &QOpenGLDebugLogger::messageLogged, this, &Canvas::OnDebugMessageLogged);
   mLoggerPtr->startLogging();

   auto functionsPtr  = glContext->versionFunctions<QOpenGLFunctions_3_3_Core>();
   mGraphicsEnginePtr = ut::make_unique<GraphicsEngine>(*functionsPtr, mBGColor);

   // Add legend layer
   mLayers.emplace_back(ut::make_unique<LegendLayer>(mSessionPtr, *this));
   mLegendLayerPtr = static_cast<LegendLayer*>(mLayers[mLayers.size() - 1].get());

   // Add PolarScaleRingsLayer and ValueAtCursorLayer instances for 2D plots
   mLayers.emplace_back(ut::make_unique<PolarScaleRingsLayer>(mSessionPtr->GetGlobalPlotOptions(),
                                                              mSessionPtr->GetPolarPlotOptions(),
                                                              *this));
   mPolarScaleRingsLayerPtr = static_cast<PolarScaleRingsLayer*>(mLayers[mLayers.size() - 1].get());
   mLayers.emplace_back(ut::make_unique<ValueAtCursorLayer>(mSessionPtr, mSessionPtr->GetPolarPlotOptions(), *this));
   mValueAtCursorLayerPtr = static_cast<ValueAtCursorLayer*>(mLayers[mLayers.size() - 1].get());

   // Add locator plane layer for 3D plots
   const DataScale& dataScale = mSessionPtr->GetScale();
   float            dbInc     = dataScale.GetIncDB();
   float            dbMin     = dataScale.GetMinDB();
   float            dbMax     = dataScale.GetMaxDB();
   makeCurrent();
   mLayers.emplace_back(ut::make_unique<LocatorMeshLayer>(mSessionPtr,
                                                          *this,
                                                          *mGraphicsEnginePtr,
                                                          mTransform3D,
                                                          dbMin,
                                                          dbMax,
                                                          dbInc,
                                                          mSessionPtr->GetLocatorOptions()));
   doneCurrent();
   mLocatorMeshLayerPtr = static_cast<LocatorMeshLayer*>(mLayers[mLayers.size() - 1].get());
}

void Canvas::leaveEvent(QEvent* aEvent)
{
   QWidget::leaveEvent(aEvent);
   if (mGlobalPlotOptionsPtr->IsPlot2D())
   {
      update();
   }
}

void Canvas::mouseMoveEvent(QMouseEvent* aEvent)
{
   QOpenGLWidget::mouseMoveEvent(aEvent);

   QPoint mousePosition = aEvent->pos();
   bool   shiftPressed  = aEvent->modifiers() == Qt::ShiftModifier;
   bool   needToUpdate  = false;

   if (mGlobalPlotOptionsPtr->IsPlot2D())
   {
      needToUpdate = true;
      if (mIsPanning)
      {
         QPoint deltaPosition     = mousePosition - mPanningPreviousLocation;
         mPanningPreviousLocation = mousePosition;
         PanPlot2D(deltaPosition);
      }
   }
   else if (mIsPanning)
   {
      needToUpdate = true;
      if (mLocatorMeshLayerPtr->IsShown() && shiftPressed)
      {
         if (mLastMouseEventShiftPressed)
         {
            mLocatorMeshLayerPtr->UpdateDrag(mousePosition);
         }
         else
         {
            mLocatorMeshLayerPtr->StartDrag(mousePosition);
         }
      }
      else
      {
         if (mLocatorMeshLayerPtr->IsShown() && mLastMouseEventShiftPressed)
         {
            mCameraPtr->StartDrag(mousePosition);
         }
         else
         {
            mCameraPtr->ContinueDrag(mousePosition);
         }
      }
   }

   mLastMouseEventShiftPressed = shiftPressed;
   if (needToUpdate)
   {
      update();
   }
}

void Canvas::mousePressEvent(QMouseEvent* aEvent)
{
   QOpenGLWidget::mousePressEvent(aEvent);
   mIsPanning = true;

   if (mGlobalPlotOptionsPtr->IsPlot2D())
   {
      mLastMouseEventShiftPressed = false;
      mPanningPreviousLocation    = aEvent->pos();
   }
   else
   {
      mLastMouseEventShiftPressed = aEvent->modifiers() == Qt::ShiftModifier;
      if (mLocatorMeshLayerPtr->IsShown() && mLastMouseEventShiftPressed)
      {
         mLocatorMeshLayerPtr->StartDrag(aEvent->pos());
      }
      else
      {
         mCameraPtr->StartDrag(aEvent->pos());
      }
   }
}

void Canvas::mouseReleaseEvent(QMouseEvent* aEvent)
{
   QOpenGLWidget::mouseReleaseEvent(aEvent);
   mIsPanning                  = false;
   mLastMouseEventShiftPressed = false;
}

void Canvas::OnDebugMessageLogged(const QOpenGLDebugMessage& aDebugMessage)
{
   qDebug() << aDebugMessage;
}

void Canvas::OnOriginOffsetChanged(QPoint aNewValue)
{
   update();
}

void Canvas::OnPlotStyleChanged(PlotStyle aPlotStyle)
{
   for (std::unique_ptr<CanvasLayer>& layer : mLayers)
   {
      if (layer->IsPatternLayer())
      {
         layer->SetStale();
      }
   }
   update();
}

void Canvas::paintGL()
{
   QPainter painter(this);
   painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

   // Offset the canvas by the origin set in the session, but use the center of the widget as
   // the anchor point rather than the top-left corner.
   QPoint     canvasOrigin = mPolarPlotOptionsPtr->GetOriginOffset() + QPoint(width() / 2, height() / 2);
   QTransform transform    = QTransform::fromTranslate(canvasOrigin.x(), canvasOrigin.y());
   painter.setWorldTransform(transform);

   // Clear the canvas
   Q_ASSERT(mGraphicsEnginePtr != nullptr);
   mGraphicsEnginePtr->ClearCanvas();

   bool is3D = mGlobalPlotOptionsPtr->IsPlot3D();
   if (is3D)
   {
      painter.beginNativePainting();
      mGraphicsEnginePtr->StartGraphicsFrame();

      // Draw the 3D patterns
      for (std::unique_ptr<CanvasLayer>& layer : mLayers)
      {
         if (layer->IsPatternLayer())
         {
            layer->Render(painter, is3D);
         }
      }

      // Draw the locator plane (must be drawn after patterns since locator sectors are translucent)
      mLocatorMeshLayerPtr->Render(painter, is3D);

      mGraphicsEnginePtr->EndGraphicsFrame();
      painter.endNativePainting();

      // Draw the legend
      mLegendLayerPtr->Render(painter, is3D);
   }
   else if (mSessionPtr->HasPatterns())
   {
      // Draw the 2D patterns
      for (std::unique_ptr<CanvasLayer>& layer : mLayers)
      {
         layer->Render(painter, is3D);
      }
   }
}

void Canvas::PanPlot2D(QPoint aDeltaPosition)
{
   // Nudge the canvas origin based on how far the mouse has moved since the last update.
   QPoint canvasOrigin = mPolarPlotOptionsPtr->GetOriginOffset() + aDeltaPosition;

   // Clamp the panning so the center point never completely goes out of view.
   QPoint canvasBounds = QPoint(width(), height()) / 2;
   canvasOrigin.setX(Clamp(canvasOrigin.x(), -canvasBounds.x(), canvasBounds.x()));
   canvasOrigin.setY(Clamp(canvasOrigin.y(), -canvasBounds.y(), canvasBounds.y()));

   // Actually set the final canvas origin.
   mPolarPlotOptionsPtr->SetOriginOffset(canvasOrigin);
}

void Canvas::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   makeCurrent();

   for (std::unique_ptr<CanvasLayer>& layer : mLayers)
   {
      layer->UpdateScale(aDbMin, aDbMax, aDbInc);
   }

   doneCurrent();
   update();
}

void Canvas::RemovePattern(Pattern& aPattern)
{
   if (mLayers.empty())
   {
      return;
   }

   // Find the layer to remove and determine if any other patterns are displayed
   std::vector<std::unique_ptr<CanvasLayer>>::iterator layerToRemove = mLayers.end();
   for (std::vector<std::unique_ptr<CanvasLayer>>::iterator i = mLayers.begin(); i != mLayers.end(); ++i)
   {
      if (i->get()->IsForPattern(&aPattern))
      {
         layerToRemove = i;
         break;
      }
   }

   if (layerToRemove != mLayers.end())
   {
      // Remove the pattern layer
      if (mGlobalPlotOptionsPtr->IsPlot3D())
      {
         makeCurrent();
         mLayers.erase(layerToRemove);
         doneCurrent();
      }
      else
      {
         mLayers.erase(layerToRemove);
      }

      update();
   }
}

void Canvas::resizeGL(int aWidth, int aHeight)
{
   mTransform3D = Compute3DTransform(aWidth, aHeight);

   // Note, resize all layers, even if they are not visible
   for (std::unique_ptr<CanvasLayer>& layer : mLayers)
   {
      layer->SetStale();                   // Forces 2D patterns to recompute
      layer->Set3DTransform(mTransform3D); // Forces 3D patterns to recompute
   }
   update();
}

void Canvas::Update2DPlot()
{
   for (std::vector<std::unique_ptr<CanvasLayer>>::iterator i = mLayers.begin(); i != mLayers.end(); ++i)
   {
      if (i->get()->IsPatternLayer())
      {
         static_cast<PatternGroupLayer*>(i->get())->Update2DPlot();
      }
   }
   update();
}

void Canvas::Update3DProjection()
{
   mTransform3D = Compute3DTransform(width(), height());

   makeCurrent();
   for (std::vector<std::unique_ptr<CanvasLayer>>::iterator i = mLayers.begin(); i != mLayers.end(); ++i)
   {
      if (i->get()->IsPatternLayer())
      {
         i->get()->Set3DTransform(mTransform3D);
      }
   }

   if (mLocatorMeshLayerPtr != nullptr)
   {
      mLocatorMeshLayerPtr->Set3DTransform(mTransform3D);
   }
   doneCurrent();
   update();
}

void Canvas::wheelEvent(QWheelEvent* aEvent)
{
   QOpenGLWidget::wheelEvent(aEvent);

   if (isVisible())
   {
      // Mouse wheel events are reporting in eights of a degree. Positive indicates a zoom in,
      // and negative indicates a zoom out.
      const float scrolledDegrees = aEvent->angleDelta().y() / 8.0f;

      const float minZoom = Camera::GetMinZoom();
      const float maxZoom = Camera::GetMaxZoom();

      // For every 15 degrees the mouse wheel is moved, change the zoom by 12.5%
      float zoomStepScalar = 0.125f / 15.0f;

      // Zooming is slowed down by a factor of 8 while the shift key is held down
      if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
      {
         zoomStepScalar /= 8.0f;
      }

      // Update the session and canvas with the new scale step
      if (mGlobalPlotOptionsPtr->IsPlot2D())
      {
         float zoom = mPolarPlotOptionsPtr->GetZoom2D();
         zoom += scrolledDegrees * zoomStepScalar;
         zoom = Clamp(zoom, minZoom, maxZoom);
         mPolarPlotOptionsPtr->SetZoom2D(zoom);
      }
      else
      {
         float zoom = mCameraPtr->GetZoom();
         zoom += scrolledDegrees * zoomStepScalar;
         zoom = Clamp(zoom, minZoom, maxZoom);
         mCameraPtr->SetZoom(zoom);
      }
   }

   aEvent->accept();
}

void Canvas::ValidateContext()
{
   QOpenGLContext* contextPtr = context();
   QSurfaceFormat  format     = contextPtr->format();
   if (format.majorVersion() < 3 || format.minorVersion() < 3)
   {
      QMessageBox::critical(nullptr,
                            QString("OpenGL Error"),
                            QString("Unable to create an OpenGL 3.3 context. Pattern Visualizer requires compatibility "
                                    "with OpenGL version 3.3."));
      exit(1);
   }
}
} // namespace PatternVisualizer
