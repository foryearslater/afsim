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

#ifndef LOCATORMESHLAYER_HPP
#define LOCATORMESHLAYER_HPP

#include "Canvas.hpp"
#include "CanvasLayer.hpp"
#include "GraphicsEngine.hpp"
#include "LocatorMesh.hpp"

namespace PatternVisualizer
{
class LocatorOptions;
class Session;

class LocatorMeshLayer : public CanvasLayer
{
   Q_OBJECT

public:
   LocatorMeshLayer(Session*          aSessionPtr,
                    Canvas&           aCanvas,
                    GraphicsEngine&   aGraphicsEngine,
                    const QMatrix4x4& aTransform,
                    float             aDbMin,
                    float             aDbMax,
                    float             aDbInc,
                    LocatorOptions*   aLocatorOptionsPtr);
   ~LocatorMeshLayer() override = default;

   void Render(QPainter& aPainter, bool aIs3DMode) override;
   void Set3DTransform(const QMatrix4x4& aTransform) override;

   void StartDrag(const QPoint& aPosition);
   void UpdateDrag(const QPoint& aPosition);

   bool IsShown() const;

public slots:
   void OnLocatorSelected(bool aShow);
   void OnAzElChanged(Angle aAzimuth, Angle aElevation);

private:
   void       UpdateScale(float aDbMin, float aDbMax, float aDbInc) override;
   QMatrix4x4 AzimuthPlaneTransform(const QMatrix4x4& aInitialTransform);
   QMatrix4x4 ElevationPlaneTransform(const QMatrix4x4& aInitialTransform);

   Session*                     mSessionPtr;
   GraphicsEngine&              mGraphicsEngine;
   std::unique_ptr<LocatorMesh> mAzArcsPtr;
   std::unique_ptr<LocatorMesh> mElArcsPtr;
   std::unique_ptr<LocatorMesh> mAzRadialsPtr;
   std::unique_ptr<LocatorMesh> mElRadialsPtr;
   std::unique_ptr<LocatorMesh> mAzSectorsPtr;
   std::unique_ptr<LocatorMesh> mElSectorsPtr;
   LocatorOptions*              mLocatorOptionsPtr;
   QPoint                       mDragOrigin;
};
} // namespace PatternVisualizer

#endif // LOCATORMESHLAYER_HPP
