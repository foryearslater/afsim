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

#include "LegendLayer.hpp"

#include "Session.hpp"

namespace PatternVisualizer
{
void LegendLayer::Render(QPainter& aPainter, bool aIs3DMode)
{
   // Return if no patterns are being displayed or if there are no scale steps (i.e., minDB >= maxDB)
   const DataScale& scale  = mSessionPtr->GetScale();
   int              nsteps = static_cast<int>(scale.GetNumScaleSteps());
   if (!mSessionPtr->HasPatterns() || nsteps == 0)
   {
      return;
   }

   aPainter.save();
   aPainter.resetTransform();

   // Increase the font size
   QFont font = aPainter.font();
   font.setPointSize(11);
   font.setBold(true);
   font.setFamily("Arial");
   aPainter.setFont(font);

   // Determine the line spacing and position of the right edge of the text box
   const int lineSpacing = aPainter.fontMetrics().lineSpacing();
   QRect     textBox     = aPainter.fontMetrics().boundingRect(QString("-300.000 dB"));
   int       xRight      = textBox.x() + textBox.width();
   int       xHeight     = aPainter.fontMetrics().xHeight();

   // Start in the bottom left corner of the canvas, but at the top of the legend.
   // We'll draw the legend items down, item-by-item. Note we walk through the ring
   // colors backwards since we want the smallest dB value on the bottom.
   int y0 = mCanvas.height() - nsteps * lineSpacing;

   int j = 0;
   for (auto i = scale.rbegin(); i != scale.rend(); ++i)
   {
      // Set the text color
      const DataScaleStep& scaleStep = *i;
      mLegendPen.setColor(scaleStep.mColor);
      aPainter.setPen(mLegendPen);

      // Draw the legend item text
      QString text = QString("%1 dB").arg(scaleStep.mDB, 0, 'g', 3);
      textBox      = aPainter.fontMetrics().boundingRect(text);
      int w        = textBox.width() + 5;
      int h        = textBox.height();
      textBox      = QRect(10 + xRight - w, y0 + j * lineSpacing - xHeight, w, h);
      aPainter.drawText(textBox, Qt::AlignBottom | Qt::AlignRight, text);
      j++;
   }

   aPainter.restore();
}
} // namespace PatternVisualizer
