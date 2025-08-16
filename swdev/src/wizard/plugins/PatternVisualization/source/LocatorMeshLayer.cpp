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

#include "LocatorMeshLayer.hpp"

#include "LocatorOptions.hpp"
#include "PlaneType.hpp"
#include "Session.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{
namespace
{
// 180 degress in 500 pixels - these are the same for now.
constexpr double cLOCATOR_AZIMUTH_DRAG_GAIN   = 180.0f / 500.0;
constexpr double cLOCATOR_ELEVATION_DRAG_GAIN = 90.0f / 250.0;
} // namespace

LocatorMeshLayer::LocatorMeshLayer(Session*          aSessionPtr,
                                   Canvas&           aCanvas,
                                   GraphicsEngine&   aGraphicsEngine,
                                   const QMatrix4x4& aTransform,
                                   float             aDbMin,
                                   float             aDbMax,
                                   float             aDbInc,
                                   LocatorOptions*   aLocatorOptionsPtr)
   : CanvasLayer(aCanvas)
   , mSessionPtr{aSessionPtr}
   , mGraphicsEngine(aGraphicsEngine)
   , mAzArcsPtr{ut::make_unique<LocatorMesh>(aSessionPtr, aGraphicsEngine, PlaneType::AZ, LocatorComponent::ARC, aDbMin, aDbMax, aDbInc)}
   , mElArcsPtr{ut::make_unique<LocatorMesh>(aSessionPtr, aGraphicsEngine, PlaneType::EL, LocatorComponent::ARC, aDbMin, aDbMax, aDbInc)}
   , mAzRadialsPtr{ut::make_unique<LocatorMesh>(aSessionPtr,
                                                aGraphicsEngine,
                                                PlaneType::AZ,
                                                LocatorComponent::RADIALS,
                                                aDbMin,
                                                aDbMax,
                                                aDbInc)}
   , mElRadialsPtr{ut::make_unique<LocatorMesh>(aSessionPtr,
                                                aGraphicsEngine,
                                                PlaneType::EL,
                                                LocatorComponent::RADIALS,
                                                aDbMin,
                                                aDbMax,
                                                aDbInc)}
   , mAzSectorsPtr{ut::make_unique<LocatorMesh>(aSessionPtr,
                                                aGraphicsEngine,
                                                PlaneType::AZ,
                                                LocatorComponent::SECTORS,
                                                aDbMin,
                                                aDbMax,
                                                aDbInc)}
   , mElSectorsPtr{ut::make_unique<LocatorMesh>(aSessionPtr,
                                                aGraphicsEngine,
                                                PlaneType::EL,
                                                LocatorComponent::SECTORS,
                                                aDbMin,
                                                aDbMax,
                                                aDbInc)}
   , mLocatorOptionsPtr{aLocatorOptionsPtr}
   , mDragOrigin{}
{
   Set3DTransform(aTransform);

   // Connect this layer to the locator options, either to update the dock from user input in the
   // 3d display, or update the 3d display from user input in the dock.
   connect(aLocatorOptionsPtr, &LocatorOptions::AzElChanged, this, &LocatorMeshLayer::OnAzElChanged);
   connect(aLocatorOptionsPtr, &LocatorOptions::ShowLocatorChanged, this, &LocatorMeshLayer::OnLocatorSelected);
}

void LocatorMeshLayer::StartDrag(const QPoint& aPosition)
{
   mDragOrigin = aPosition;
}

void LocatorMeshLayer::UpdateDrag(const QPoint& aPosition)
{
   QPoint diff = aPosition - mDragOrigin;
   mDragOrigin = aPosition;

   double deltaAzimuth   = -cLOCATOR_AZIMUTH_DRAG_GAIN * diff.x();
   double deltaElevation = -cLOCATOR_ELEVATION_DRAG_GAIN * diff.y();

   double nextAz = UtMath::NormalizeAngleMinus180_180(mLocatorOptionsPtr->GetAzimuth().GetDegrees() + deltaAzimuth);
   double nextEl = std::max(-90.0, std::min(90.0, mLocatorOptionsPtr->GetElevation().GetDegrees() + deltaElevation));

   mLocatorOptionsPtr->SetAzEl(Angle::FromDegrees(nextAz), Angle::FromDegrees(nextEl));
}

bool LocatorMeshLayer::IsShown() const
{
   return mLocatorOptionsPtr->GetShowLocator();
}

void LocatorMeshLayer::OnLocatorSelected(bool aShow)
{
   mCanvas.repaint();
}

void LocatorMeshLayer::OnAzElChanged(Angle aAzimuth, Angle aElevation)
{
   mCanvas.Update3DProjection();
}

void LocatorMeshLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   if (!mLocatorOptionsPtr->GetShowLocator() || !aIs3DMode || !mSessionPtr->HasPatterns())
   {
      return;
   }

   // Draw the opaque components (arcs, radials)
   mAzArcsPtr->Render();
   mAzRadialsPtr->Render();
   mElArcsPtr->Render();
   mElRadialsPtr->Render();

   // Draw the translucent components (sectors)
   mGraphicsEngine.SetTranslucent();
   mAzSectorsPtr->Render();
   mElSectorsPtr->Render();
}

void LocatorMeshLayer::Set3DTransform(const QMatrix4x4& aTransform)
{
   QMatrix4x4 aziTrans = AzimuthPlaneTransform(aTransform);
   mAzArcsPtr->SetTransform(aziTrans);
   mAzRadialsPtr->SetTransform(aziTrans);
   mAzSectorsPtr->SetTransform(aziTrans);

   QMatrix4x4 eleTrans = ElevationPlaneTransform(aTransform);
   mElArcsPtr->SetTransform(eleTrans);
   mElRadialsPtr->SetTransform(eleTrans);
   mElSectorsPtr->SetTransform(eleTrans);
}

void LocatorMeshLayer::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   mAzArcsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
   mAzRadialsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
   mAzSectorsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
   mElArcsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
   mElRadialsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
   mElSectorsPtr->UpdateScale(aDbMin, aDbMax, aDbInc);
}

QMatrix4x4 LocatorMeshLayer::AzimuthPlaneTransform(const QMatrix4x4& aInitialTransform)
{
   QMatrix4x4 retval{aInitialTransform};
   retval.rotate(mLocatorOptionsPtr->GetAzimuth().GetDegrees(), 0.0f, 0.0f, 1.0f);
   retval.rotate(mLocatorOptionsPtr->GetElevation().GetDegrees(), 0.0f, 1.0f, 0.0f);
   return retval;
}

QMatrix4x4 LocatorMeshLayer::ElevationPlaneTransform(const QMatrix4x4& aInitialTransform)
{
   QMatrix4x4 retval{aInitialTransform};
   retval.rotate(mLocatorOptionsPtr->GetAzimuth().GetDegrees(), 0.0f, 0.0f, 1.0f);
   return retval;
}
} // namespace PatternVisualizer
