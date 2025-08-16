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

#ifndef POLARPLOTOPTIONS_HPP
#define POLARPLOTOPTIONS_HPP

#include <QObject>
#include <QPoint>

#include "Angle.hpp"

class QSettings;

namespace PatternVisualizer
{
class PolarPlotOptions : public QObject
{
   Q_OBJECT
public:
   PolarPlotOptions();
   ~PolarPlotOptions() override = default;

   void LoadSettings(QSettings& aSettings);
   void SaveSettings(QSettings& aSettings);

   Angle  GetConstantAzimuth() const { return mConstantAzimuth; }
   Angle  GetConstantElevation() const { return mConstantElevation; }
   QPoint GetOriginOffset() const { return mOriginOffset; }
   float  GetZoom2D() const { return mZoom2D; }

public slots:
   void SetConstantAzimuth(Angle aAngle);
   void SetConstantElevation(Angle aAngle);
   void SetOriginOffset(const QPoint& aPoint);
   void SetZoom2D(float aZoom2D);
   void ResetConstantAzimuth();
   void ResetConstantElevation();
   void ResetOriginOffset();
   void ResetZoom2D();

signals:
   void ConstantAzimuthChanged(Angle aAngle);
   void ConstantElevationChanged(Angle aAngle);
   void OriginOffsetChanged(const QPoint& aPoint);
   void Zoom2D_Changed(float aZoom2D);

private:
   Angle  mConstantAzimuth;
   Angle  mConstantElevation;
   QPoint mOriginOffset;
   float  mZoom2D;
};
} // namespace PatternVisualizer

#endif // POLARPLOTOPTIONS_HPP
