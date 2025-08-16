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

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QMatrix4x4>
#include <QObject>

#include "Angle.hpp"

class QSettings;

namespace PatternVisualizer
{
class Camera : public QObject
{
   Q_OBJECT
public:
   Camera();
   ~Camera() = default;

   void StartDrag(const QPoint& aPoint);
   void ContinueDrag(const QPoint& aPoint);

   QMatrix4x4 Transformation() const;

   Angle GetAzimuth() const { return mAzimuth; }
   Angle GetElevation() const { return mElevation; }
   float GetZoom() const { return mZoom; }

   void LoadSettings(QSettings& aSettings);
   void SaveSettings(QSettings& aSettings);

   static float GetMinZoom();
   static float GetMaxZoom();

public slots:
   void SetAzimuth(Angle aAzimuth);
   void SetElevation(Angle aElevation);
   void SetAngles(Angle aAzimuth, Angle aElevation);
   void SetZoom(float aZoom);
   void ResetCameraAngles();
   void ResetCameraZoom();

signals:
   void CameraRotationChanged(const Angle& aAzimuth, const Angle& aElevation);
   void CameraZoomChanged(float aZoom);

private:
   Angle  mAzimuth;
   Angle  mElevation;
   float  mZoom;
   QPoint mPrevious;
};
} // namespace PatternVisualizer

#endif // CAMERA_HPP
