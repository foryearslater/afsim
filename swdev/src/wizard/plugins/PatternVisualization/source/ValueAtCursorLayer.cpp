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

#include "ValueAtCursorLayer.hpp"

#include "MaterialColors.hpp"
#include "PolarPlotOptions.hpp"
#include "PolarScaleRingsLayer.hpp"
#include "Session.hpp"
#include "UtMath.hpp"

namespace PatternVisualizer
{
void ValueAtCursorLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   if (aIs3DMode)
   {
      return;
   }

   // Don't draw anything if the mouse cursor is out of bounds of the canvas.
   const QPoint cursorPos = mCanvas.mapFromGlobal(QCursor::pos());
   if (cursorPos.x() < 0 || cursorPos.x() >= mCanvas.width() || cursorPos.y() < 0 || cursorPos.y() >= mCanvas.height())
   {
      return;
   }

   // Return if there are no scale steps (i.e., minDB <= maxDB)
   const DataScale& scale  = mSessionPtr->GetScale();
   size_t           nsteps = scale.GetNumScaleSteps();
   if (nsteps == 0)
   {
      return;
   }

   // Compute the distance from the mouse cursor from the center of the radial canvas
   const QPoint canvasOrigin = QPoint(mCanvas.width() / 2, mCanvas.height() / 2) + mPolarPlotOptionsPtr->GetOriginOffset();
   const QPoint locationOnCanvas = cursorPos - canvasOrigin;
   const double radialDistance =
      sqrt(locationOnCanvas.x() * locationOnCanvas.x() + locationOnCanvas.y() * locationOnCanvas.y());

   // Scale the radial distance to the dB range rings scale
   const float minDB      = scale.GetMinDB();
   const float incDB      = scale.GetIncDB();
   float       zoomedDist = PolarScaleRingsLayer::GetDistBetweenScaleLines() * mPolarPlotOptionsPtr->GetZoom2D();
   float       maxDBStep  = minDB + (nsteps - 1) * incDB;
   double      dbValue    = UtMath::Lerp(radialDistance, zoomedDist, nsteps * zoomedDist, minDB, maxDBStep);

   // Compute the angle
   float angleDeg = atan2(-locationOnCanvas.y(), locationOnCanvas.x()) * UtMath::cDEG_PER_RAD;

   // Create the text to be displayed
   const QString text = QString("dB = %1\nAngle = %2").arg(dbValue).arg(angleDeg);

   aPainter.save();
   aPainter.resetTransform();
   aPainter.translate(10, 10);

   // Increase the font size.
   QFont font = aPainter.font();
   font.setPointSize(10);
   font.setBold(true);
   aPainter.setFont(font);

   aPainter.setPen(MaterialColors::Green500);
   aPainter.drawText(QRect(0, 0, 200, 100), Qt::AlignTop | Qt::AlignLeft, text);

   aPainter.restore();
}
} // namespace PatternVisualizer
