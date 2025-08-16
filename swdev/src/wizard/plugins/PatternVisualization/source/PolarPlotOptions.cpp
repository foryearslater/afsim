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

#include "PolarPlotOptions.hpp"

#include <QSettings>

namespace PatternVisualizer
{
namespace
{
const QString cSETTINGS_CONSTANT_AZIMUTH("ConstantAzimuth2D");
const QString cSETTINGS_CONSTANT_ELEVATION("ConstantElevation2D");
const QString cSETTINGS_ZOOM_2D("Zoom2D");

constexpr static Angle cDEFAULT_CONSTANT_AZIMUTH   = 0.0_deg;
constexpr static Angle cDEFAULT_CONSTANT_ELEVATION = 0.0_deg;
constexpr static float cDEFAULT_ZOOM_2D            = 1.0f;
} // namespace

PolarPlotOptions::PolarPlotOptions()
   : mConstantAzimuth{cDEFAULT_CONSTANT_AZIMUTH}
   , mConstantElevation{cDEFAULT_CONSTANT_ELEVATION}
   , mOriginOffset{}
   , mZoom2D{cDEFAULT_ZOOM_2D}
{
}

void PolarPlotOptions::LoadSettings(QSettings& aSettings)
{
   Angle azimuth{cDEFAULT_CONSTANT_AZIMUTH};
   if (aSettings.contains(cSETTINGS_CONSTANT_AZIMUTH))
   {
      azimuth = Angle::FromRadians(aSettings.value(cSETTINGS_CONSTANT_AZIMUTH).toDouble());
   }
   SetConstantAzimuth(azimuth);

   Angle elevation{cDEFAULT_CONSTANT_ELEVATION};
   if (aSettings.contains(cSETTINGS_CONSTANT_ELEVATION))
   {
      elevation = Angle::FromRadians(aSettings.value(cSETTINGS_CONSTANT_ELEVATION).toDouble());
   }
   SetConstantElevation(elevation);

   float zoom{cDEFAULT_ZOOM_2D};
   if (aSettings.contains(cSETTINGS_ZOOM_2D))
   {
      zoom = aSettings.value(cSETTINGS_ZOOM_2D).toFloat();
   }
   SetZoom2D(zoom);
}

void PolarPlotOptions::SaveSettings(QSettings& aSettings)
{
   aSettings.setValue(cSETTINGS_CONSTANT_AZIMUTH, mConstantAzimuth.GetRadians());
   aSettings.setValue(cSETTINGS_CONSTANT_ELEVATION, mConstantElevation.GetRadians());
   aSettings.setValue(cSETTINGS_ZOOM_2D, mZoom2D);
}

void PolarPlotOptions::SetConstantAzimuth(Angle aAngle)
{
   if (mConstantAzimuth != aAngle)
   {
      mConstantAzimuth = aAngle;
      emit ConstantAzimuthChanged(mConstantAzimuth);
   }
}

void PolarPlotOptions::SetConstantElevation(Angle aAngle)
{
   if (mConstantElevation != aAngle)
   {
      mConstantElevation = aAngle;
      emit ConstantElevationChanged(mConstantElevation);
   }
}

void PolarPlotOptions::SetOriginOffset(const QPoint& aPoint)
{
   if (mOriginOffset != aPoint)
   {
      mOriginOffset = aPoint;
      emit OriginOffsetChanged(mOriginOffset);
   }
}

void PolarPlotOptions::SetZoom2D(float aZoom2D)
{
   if (mZoom2D != aZoom2D)
   {
      mZoom2D = aZoom2D;
      emit Zoom2D_Changed(mZoom2D);
   }
}

void PolarPlotOptions::ResetConstantAzimuth()
{
   SetConstantAzimuth(cDEFAULT_CONSTANT_AZIMUTH);
}

void PolarPlotOptions::ResetConstantElevation()
{
   SetConstantElevation(cDEFAULT_CONSTANT_ELEVATION);
}

void PolarPlotOptions::ResetOriginOffset()
{
   SetOriginOffset(QPoint{});
}

void PolarPlotOptions::ResetZoom2D()
{
   SetZoom2D(cDEFAULT_ZOOM_2D);
}

} // namespace PatternVisualizer
