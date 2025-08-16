// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Camera.hpp"

#include <QSettings>

namespace PatternVisualizer
{
namespace
{
// Used in the camera translation
constexpr float cZOOM_MIN        = 0.25f;
constexpr float cZOOM_MAX        = 5.0f;
constexpr float cCAMERA_DIST_MIN = -5.0f;
constexpr float cCAMERA_DIST_MAX = -0.4f;

// scaling from distance on sceen to angle values
constexpr double cCAMERA_AZIMUTH_DRAG_GAIN   = 180.0 / 500.0;
constexpr double cCAMERA_ELEVATION_DRAG_GAIN = 90.0 / 250.0;

// Default camera properties
constexpr Angle cDEFAULT_AZIMUTH   = 45.0_deg;
constexpr Angle cDEFAULT_ELEVATION = 20.0_deg;
constexpr float cDEFAULT_ZOOM      = 1.0;

// Settings property names
const QString cCAMERA_ZOOM_KEY{"Zoom3D"};
const QString cCAMERA_AZIMUTH_KEY{"CameraAzimuth"};
const QString cCAMERA_ELEVATION_KEY{"CameraElevation"};
} // namespace

Camera::Camera()
   : mAzimuth{cDEFAULT_AZIMUTH}
   , mElevation{cDEFAULT_ELEVATION}
   , mZoom{cDEFAULT_ZOOM}
   , mPrevious{}
{
}

void Camera::StartDrag(const QPoint& aPoint)
{
   mPrevious = aPoint;
}

void Camera::ContinueDrag(const QPoint& aPoint)
{
   QPoint diff = aPoint - mPrevious;
   mPrevious   = aPoint;

   double deltaAzimuth   = cCAMERA_AZIMUTH_DRAG_GAIN * diff.x();
   double deltaElevation = cCAMERA_ELEVATION_DRAG_GAIN * diff.y();

   double nextAz = UtMath::NormalizeAngleMinus180_180(mAzimuth.GetDegrees() + deltaAzimuth);
   double nextEl = std::max(-90.0, std::min(90.0, mElevation.GetDegrees() + deltaElevation));

   SetAngles(Angle::FromDegrees(nextAz), Angle::FromDegrees(nextEl));
}

QMatrix4x4 Camera::Transformation() const
{
   QMatrix4x4 retval{};
   retval.setToIdentity();

   float     distance = UtMath::Lerp(mZoom, cZOOM_MIN, cZOOM_MAX, cCAMERA_DIST_MIN, cCAMERA_DIST_MAX);
   double    cosEl    = cos(mElevation);
   double    sinEl    = sin(mElevation);
   double    cosAz    = cos(mAzimuth);
   double    sinAz    = sin(mAzimuth);
   QVector3D eyePos(-distance * cosEl * cosAz, -distance * cosEl * sinAz, distance * sinEl);
   QVector3D lookPos{0.0f, 0.0f, 0.0f};
   QVector3D upVec(-sinEl * cosAz, -sinEl * sinAz, -cosEl);
   retval.lookAt(eyePos, lookPos, upVec);

   return retval;
}

void Camera::LoadSettings(QSettings& aSettings)
{
   if (aSettings.contains(cCAMERA_ZOOM_KEY))
   {
      SetZoom(aSettings.value(cCAMERA_ZOOM_KEY).toFloat());
   }
   else
   {
      SetZoom(cDEFAULT_ZOOM);
   }

   if (aSettings.contains(cCAMERA_AZIMUTH_KEY))
   {
      SetAzimuth(Angle::FromRadians(aSettings.value(cCAMERA_AZIMUTH_KEY).toFloat()));
   }
   else
   {
      SetAzimuth(cDEFAULT_AZIMUTH);
   }

   if (aSettings.contains(cCAMERA_ELEVATION_KEY))
   {
      SetElevation(Angle::FromRadians(aSettings.value(cCAMERA_ELEVATION_KEY).toFloat()));
   }
   else
   {
      SetElevation(cDEFAULT_ELEVATION);
   }
}

void Camera::SaveSettings(QSettings& aSettings)
{
   aSettings.setValue(cCAMERA_ZOOM_KEY, GetZoom());
   aSettings.setValue(cCAMERA_AZIMUTH_KEY, GetAzimuth().GetRadians());
   aSettings.setValue(cCAMERA_ELEVATION_KEY, GetElevation().GetRadians());
}

float Camera::GetMinZoom()
{
   return cZOOM_MIN;
}

float Camera::GetMaxZoom()
{
   return cZOOM_MAX;
}

void Camera::SetAzimuth(Angle aAzimuth)
{
   mAzimuth = aAzimuth;
   emit CameraRotationChanged(mAzimuth, mElevation);
}

void Camera::SetElevation(Angle aElevation)
{
   mElevation = aElevation;
   emit CameraRotationChanged(mAzimuth, mElevation);
}

void Camera::SetAngles(Angle aAzimuth, Angle aElevation)
{
   mAzimuth   = aAzimuth;
   mElevation = aElevation;
   emit CameraRotationChanged(mAzimuth, mElevation);
}

void Camera::SetZoom(float aZoom)
{
   mZoom = aZoom;
   emit CameraZoomChanged(mZoom);
}

void Camera::ResetCameraAngles()
{
   mAzimuth   = cDEFAULT_AZIMUTH;
   mElevation = cDEFAULT_ELEVATION;
   emit CameraRotationChanged(mAzimuth, mElevation);
}

void Camera::ResetCameraZoom()
{
   mZoom = cDEFAULT_ZOOM;
   emit CameraZoomChanged(mZoom);
}
} // namespace PatternVisualizer
